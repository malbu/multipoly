#include "temperaturecorrection.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "common/math/polynomials/singleVariablePolynomial.h"
#include <queue>
#include "spline.h"

#include "alglib/interpolation.h"

TemperatureCorrection::TemperatureCorrection()
{
    // Create a Taylor polynomial


    // The polynomial creation sequence happens in two major steps

    // Step 1. Given that the number of indeterminates N and maximum term exponent power
    // are known, a polynomial expression is generated that represents the exponents of each interminant variable
    // in each of the multivariate terms. They are simply represented in a 1D vector.
    // This step needs to only be done once.

    // Step 2. The polynomial "expression" created in the previous step is used to create a symbolic represenation of the
    // taylor polynomial by multiplying the monomial multivariate terms with a paired model coefficient "beta" and the result is summed
    // This represenation can then be used to evaluate a specific correction. Each coordinate axis has different model coefficients
    // so in it's current form it needs to happen twice.




    // Commence Step 1.
    // TODO this only needs to be done once ideally
    // TODO all these functions should tell you if the previous steps
    //      weren't completed before it was called...
    taylorPolynomialX.initTaylorPoly();
    taylorPolynomialY.initTaylorPoly();


    // Step 2.

    //Retrieve model specific betas from database (toy db for now)
    retrieveBetas(1);

    // Create Taylor polynomial for X
    taylorPolynomialX.updateBeta(betasX);
    taylorPolynomialX.createTaylorPolynomial();

    // Create Taylor polynomial for Y
    taylorPolynomialY.updateBeta(betasY);
    taylorPolynomialY.createTaylorPolynomial();

    // Test Evaluation
    // All points must be doubles or doubles for unwanted rounding to not occur

    // For debugging
    //float test=taylorPolynomialY.taylorPolynomial(-19.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
    //OL X Y Temp
    //float testy=taylorPolynomialY.taylorPolynomial(-20.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
    //float testy=taylorPolynomialY.taylorPolynomial(-19.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
    //float testx=taylorPolynomialX.taylorPolynomial(-19.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
    //std::cout<<"Test eval Y: "<<testy<<"\n";
    //std::cout<<"Test eval X: "<<testx<<"\n";

    storePixelandWavelengthOrderTable();

}


void TemperatureCorrection::retrieveBetas(int id){
    //Clear vectors
    betasX.clear();
    betasY.clear();

    //Open DB
    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("computeTaylor.db");
    bool openOk=db.open();

    //Retrieve the X betas
    if(openOk){
        //            QSqlQuery query.exec(QString("SELECT X from taylorcoefficients WHERE id=%1;").arg(id));
        QSqlQuery query;
        // betasX
        query.exec("SELECT X from taylorcoefficients WHERE id='1'");
        while(query.next()){
            //std::cout<<query.value(0).toFloat()<</*query.value(1).toFloat()<<query.value(2).toFloat()<<*/"\n";
            betasX.push_back(query.value(0).toDouble());
        }

        // betasY

        query.exec("SELECT Y from taylorcoefficients WHERE id='1'");
        while(query.next()){
            betasY.push_back(query.value(0).toDouble());
            //std::cout<<query.value(0).toFloat()<<"\n";
        }

    }

    // Close the db
    db.close();

}

void TemperatureCorrection::storePixelandWavelengthOrderTable(){
    //Open wavelenght csv



    QFile wavelengths_file(QString("./sampleWavelengths.csv"));

    if(wavelengths_file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&wavelengths_file);
        QString line=stream.readLine();
        while(!line.isNull()){
            QStringList split_line=line.split(',');


            //debugging

            //There should be 45 columns

            //std::cout<<"The size of the row QStringList is: "<<split_line.size()<<"\n";

            for(int i=0;i<split_line.size();i++){
                wavelength[i].push_back(split_line.at(i).toDouble());
            }

            line=stream.readLine();
        }
        wavelengths_file.close();
    }


    // Open the pixel y csv

    QFile pixely_file(QString("./YPixelBeforeCorrection.csv"));

    if(pixely_file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&pixely_file);
        QString line=stream.readLine();
        while(!line.isNull()){
            QStringList split_line=line.split(',');


            //debugging


            //There should be 45 columns

            //std::cout<<"The size of the row QStringList is: "<<split_line.size()<<"\n";

            for(int i=0;i<split_line.size();i++){
                pixelY[i].push_back(split_line.at(i).toDouble());
            }


            line=stream.readLine();
        }
        pixely_file.close();
    }

    //debugging

    //qDebug()<<pixelY;
    //qDebug()<<pixelY.size(); //45
    //qDebug()<<pixelY[0].size();

    //std::vector<float> tempvec;

    //QMapIterator<int,std::vector<float>> i(pixelY);
    //std::cout<<"printing the key\n";
    //while(i.hasNext()){
    //  i.next();
    //  tempvec=i.value();
    //  std::cout<<"Printing out the contents of vector number: "<<i.key()<<"\n";
    //  for(int j=0;j<tempvec.size();j++){
    //      std::cout<<tempvec.at(j)<<"\n"; //value
    //      std::cout<<j<<"\n";
    //   }

    //}

    //truncateData(200);
    calculateShifts();


}

void TemperatureCorrection::truncateData(int numberofPointstoCutfromSides){

    // No longer needed. Truncation can be done when splines are fitted. See below  on line 352ish

    // First truncate Y
    QMap<int,std::vector<double>> truncatedYData;
    //Truncate pixel Y data
    std::vector<double> truncatedYVector;
    // Process entire map of vectors
    QMapIterator<int,std::vector<double>> yIterator(pixelY);
    while(yIterator.hasNext()){
        yIterator.next();

        truncatedYVector=yIterator.value();

        // Method 1:
//        Erase points and use swap to compact memory
        std::vector<double>(truncatedYVector.begin()+numberofPointstoCutfromSides,truncatedYVector.end()-numberofPointstoCutfromSides).swap(truncatedYVector);
        truncatedYData[yIterator.key()].swap(truncatedYVector);

        // Method 2:

        // Use a loop

        //TODO: which is faster? Method 1 appears to be faster
//        for(int j=numberofPointstoCutfromSides;j<truncatedYVector.size()-numberofPointstoCutfromSides;j++){
//            // Loop over each
//            truncatedYData[yIterator.key()].push_back(truncatedYVector.at(j));
//        }
    }


    // Then truncate wavelength

    QMap<int,std::vector<double>> truncatedWavelengthData;

    std::vector<double> truncatedWavelengthVector;

    QMapIterator<int,std::vector<double>> wavelengthIterator(wavelength);
    while(wavelengthIterator.hasNext()){
        wavelengthIterator.next();
        truncatedWavelengthVector=wavelengthIterator.value();
        std::vector<double>(truncatedWavelengthVector.begin()+numberofPointstoCutfromSides,truncatedWavelengthVector.end()-numberofPointstoCutfromSides).swap(truncatedWavelengthVector);
        truncatedWavelengthData[wavelengthIterator.key()].swap(truncatedWavelengthVector);

    }


    pixelY.clear();
    pixelY=truncatedYData;

    wavelength.clear();
    wavelength=truncatedWavelengthData;

    qDebug()<<"Truncated pixelY size "<<pixelY[0].size()<<endl;
    qDebug()<<"Truncated wavelength size "<<wavelength[0].size()<<endl;




}

void TemperatureCorrection::calculateShifts(){
    // This function uses evaluates the correction to each of these axes by using the
    // axis specific taylor polynomial created in previous steps.

    // To begin with, the following corrections need to be done

    // Correction 1. X shift. Will be referred to as deltaX
    // taylorPolynomialX(OrderLine)(X index value for order N)(Y value for order N)(Temperature)
    // The result of the above operation is in form QMap<order number, vector of 2048 floats>
    // Mean values are subtracted from each element before evaluation. These values were determined by Greger
    // and are hardcoded in the header file



    // Correction 2. Y shift. Will be referred to as deltaY
    // taylorPolynomialY(OrderLine)(X index value for order N)(Y value for order N)(Temperature)
    // The result of the above operation is in form QMap<order number, vector of 2048 float>

    Rigaku::Common::Math::Polynomials::SingleVariablePolynomial lambdaWRTx;

    // This is no longer needed before this has been replaced by a fit with cubic splines.
    //Rigaku::Common::Math::Polynomials::SingleVariablePolynomial YWRToldLambda;


    std::vector<double> tempVec;
    xPlusDeltaX.clear();
    yPlusDeltaY.clear();
    newWavelength.clear();
    correctedY.clear();

    QMapIterator<int,std::vector<double>> i(pixelY);
    //std::cout<<"printing the key\n";
    while(i.hasNext()){
        i.next();
        tempVec=i.value(); //TODO is this copy needed?
        // Evaluate both Taylor polynomials
        double deltaX;
        double deltaY;
        for(int j=0;j<tempVec.size();j++){
            //std::cout<<tempVec.at(j)<<"\n"; //value
            //std::cout<<j<<"\n";

            // Subtract the mean
            double OL=(i.key()*1.0)-OL_mean;
            double X=j*1.0-X_mean;
            double Y=tempVec.at(j)*1.0-Y_mean;
            //Temp isn't used for now
            double T=30-temperature_mean;


            // Calculate the X shift (deltaX) and add it to the original X
            deltaX=taylorPolynomialX.taylorPolynomial(double(0.0))(double(X))(double(Y))(double(T));

            //qDebug()<<"Printing out evalPointX: "<<evalPointX<<"\n";

            xPlusDeltaX[i.key()].push_back(deltaX+j);


            // Calculate the Y shift (deltaY) and add it to original Y
            deltaY=taylorPolynomialY.taylorPolynomial(double(0.0))(double(X))(double(Y))(double(T));
            //qDebug()<<"Printing out evalPointY: "<<evalPointY<<"\n";
            //taylorPolynomialX(j)(tempVec.at(j))(i.key())(0.0);

            yPlusDeltaY[i.key()].push_back(deltaY+tempVec.at(j));
        }

        // Now we have to find the polynomial that describes the relationship between the old wavelength
        // and the new X's

        // ie. rc=(X1, lamba0,5)

        std::vector<double> wavelengthSpline(wavelength[i.key()].begin(),wavelength[i.key()].end());
        std::vector<double> xPlusDeltaXSpline(xPlusDeltaX[i.key()].begin(),xPlusDeltaX[i.key()].end());


        //Alglib uses custom arrays
        alglib::real_1d_array wavelengthSplineAlglib;
        alglib::real_1d_array xPlusDeltaXSplineAlglib;

        wavelengthSplineAlglib.setcontent(wavelengthSpline.size(),&(wavelengthSpline[0]));
        xPlusDeltaXSplineAlglib.setcontent(xPlusDeltaXSpline.size(),&(xPlusDeltaXSpline[0]));


        //using TK spline library
        tk::spline lambdaWRTxSpline;

        lambdaWRTxSpline.set_points(xPlusDeltaXSpline,wavelengthSpline);


        //using Alglib spline library

        alglib::spline1dinterpolant spline;

        int naturalBoundType=2;

        alglib::spline1dbuildcubic(xPlusDeltaXSplineAlglib,wavelengthSplineAlglib,wavelengthSpline.size(),naturalBoundType,0.0,naturalBoundType,0.0,spline);



        //lambdaWRTx.DataFit(xPlusDeltaX[i.key()],wavelength[i.key()],5);

        // Evaluate the polynomial created above
        // at integer values of X from 0-2047 to get the new wavelengthValues
        for(int xIndex=0;xIndex<2048;xIndex++){
            //5th order poly
            //newWavelength[i.key()].push_back(lambdaWRTx.Evaluate(xIndex));
            //tk spline
            //newWavelength[i.key()].push_back(lambdaWRTxSpline(xIndex));
            //alglib spline
            newWavelength[i.key()].push_back(alglib::spline1dcalc(spline,xIndex));
        }

        // Now we have to find the equation that describes Y+deltaY (dependent)
        // with respect to the old wavelength. In a previous commit, this was done with a high order
        // polynomial fit. Splines are used instead


        // Convert to double for spline fit
        // TODO: remove this if you convert everything to double precision
        //std::vector<double> wavelengthSpline(wavelength[i.key()].begin(),wavelength[i.key()].end());
        std::vector<double> yPlusDeltaYSpline(yPlusDeltaY[i.key()].begin(),yPlusDeltaY[i.key()].end());

        // Doing fit with splines because polynomial fit needed a high order fit to generate worse
        // results and took much longer to evaluate

        tk::spline YWRToldLambdaSpline;

        YWRToldLambdaSpline.set_points(wavelengthSpline,yPlusDeltaYSpline);


        // Alglib splines
        // Alglib uses custom arrays
        alglib::real_1d_array yPlusDeltaYSplineAlglib;

        yPlusDeltaYSplineAlglib.setcontent(yPlusDeltaYSpline.size(),&(yPlusDeltaYSpline[0]));


        alglib::spline1dinterpolant spline2;

        // Alglib cubic spline

       //alglib::spline1dbuildcubic(wavelengthSplineAlglib,yPlusDeltaYSplineAlglib,wavelengthSpline.size(),naturalBoundType,0.0,naturalBoundType,0.0,spline2);

       alglib::spline1dbuildmonotone(wavelengthSplineAlglib,yPlusDeltaYSplineAlglib,wavelengthSpline.size(),spline2);



        // Evaluate the polynomial created above to find the corrected Y value
        // by plugging in the new wavelength values created on line 267


        for(int yIndex=0;yIndex<2048;yIndex++){

            // Evaluating with splines
            // Tk
            //double evalUsingSpline=YWRToldLambdaSpline(newWavelength[i.key()].at(yIndex));

            double evalUsingSpline=alglib::spline1dcalc(spline2,newWavelength[i.key()].at(yIndex));


            double splineCheck=YWRToldLambdaSpline(wavelengthSpline.at(yIndex));
            correctedY[i.key()].push_back(evalUsingSpline);
            splineCheckMap[i.key()].push_back(splineCheck);

        }

    }


    //debugging

    //qDebug()<<"Printing out the corrected Y vector\n";
    qDebug()<<correctedY[0];

    printCSV();

}


void TemperatureCorrection::printCSV(){

    // This function is just for debugging
    QFile out_file(QString("./out_file.csv"));
    if (out_file.open(QFile::ReadWrite)) {
        QTextStream stream(&out_file);
        for (int j=0;j<correctedY[0].size();j++) {
            for(int i=0;i<correctedY.size();i++){
                stream<<correctedY[i].at(j)<<",";
            }
            stream<<endl;
        }
    }
    out_file.close();

    // This function is just for debugging
    QFile out_filewavelength(QString("./out_filewavelength.csv"));
    if (out_filewavelength.open(QFile::ReadWrite)) {
        QTextStream stream(&out_filewavelength);
        for (int j=0;j<newWavelength[0].size();j++) {
            for(int i=0;i<newWavelength.size();i++){
                stream<<newWavelength[i].at(j)<<",";
            }
            stream<<endl;
        }
    }
    out_filewavelength.close();


    QFile splinecheck(QString("./splinecheck.csv"));
    if (splinecheck.open(QFile::ReadWrite)) {
        QTextStream stream(&splinecheck);
        for (int j=0;j<splineCheckMap[0].size();j++) {
            for(int i=0;i<splineCheckMap.size();i++){
                stream<<splineCheckMap[i].at(j)<<",";
            }
            stream<<endl;
        }
    }
    splinecheck.close();

    QFile uncorrected_wavelength(QString("./uncorrected_wavelength.csv"));
    if (uncorrected_wavelength.open(QFile::ReadWrite)) {
        QTextStream stream(&uncorrected_wavelength);
        for (int j=0;j<wavelength[0].size();j++) {
            for(int i=0;i<wavelength.size();i++){
                stream<<wavelength[i].at(j)<<",";
            }
            stream<<endl;
        }
    }
    uncorrected_wavelength.close();



    QFile yPlusDeltaYfile(QString("./yPlusDeltaY.csv"));
    if (yPlusDeltaYfile.open(QFile::ReadWrite)) {
        QTextStream stream(&yPlusDeltaYfile);
        for (int j=0;j<yPlusDeltaY[0].size();j++) {
            for(int i=0;i<yPlusDeltaY.size();i++){
                stream<<yPlusDeltaY[i].at(j)<<",";
            }
            stream<<endl;
        }
    }
    yPlusDeltaYfile.close();
}

std::vector<double> TemperatureCorrection::calculateMeanAndStDev(std::vector<double> inputVector){

    // Naive implementation that calculates mean and standard deviation
    // This might not be needed anymore

    double sum=std::accumulate(inputVector.begin(),inputVector.end(),0);
    double mean=sum/inputVector.size();

    double squareSum = std::inner_product(inputVector.begin(), inputVector.end(), inputVector.begin(), 0.0);

    double stDev=sqrt(squareSum / inputVector.size() - mean * mean);

    std::vector<double> conditionedVector;

    for(int i=0; i<inputVector.size();i++){
        // Condition each value in vector
        // (x-mean)/stdev

        conditionedVector.push_back((inputVector.at(i)-mean)/stDev);
    }
    return conditionedVector;
}


std::vector<double> TemperatureCorrection::findCorrectedWavelengthAndYValue(int OL, double x){

    // This is the interface function that determines
    // The input is OL and X and the output should be corrected Y and delta

    // Returned vector is in format
    // [Corrected Y, Corrected wavelength]
    std::vector<double> YandLambda;
    // order width is hardcoded for now to 2;
    int width=2;

    double sum=0.0f;

    for(int i=-width;i<=width;i++){
        // Check if value exists
        try {
            sum+=correctedY[OL].at(x+i);
        } catch (const std::out_of_range& oor) {
            qDebug()<<"Out of range error in findCorrectedWavelengthAndYValue"<<endl;
        }
    }

//    // Retrieve Y value at index X and push it
    YandLambda.push_back(correctedY[OL].at(x));

    // Push the sum
    //YandLambda.push_back(sum);

    // Retrieve Wavelength value at index x and push it
    YandLambda.push_back(newWavelength[OL].at(x));



    return YandLambda;

}


std::vector<double> TemperatureCorrection::findKClosestValue(std::vector<double> unsortedVector, double numberOfInterest, int k){
    // Given an unsorted vector, find the k closest values to numberOfInterest. "Closest" meaning absolute value distance
    // TODO: Is this slower or faster than sorting the unsortedVector and then doing a binary search?
    // This method is O(n Log k) time so it's fast

    // This function might also not be needed anymore

    std::vector<double> matches;

    // Make a heap of difference with first k elements.
    priority_queue<pair<double, int> > differenceHeap;
    for (int i = 0; i < k; i++)
        differenceHeap.push({ abs(unsortedVector.at(i) - numberOfInterest), i });

    // Now process remaining elements.
    for (int i = k; i < unsortedVector.size(); i++) {

        double difference = abs(unsortedVector.at(i) - numberOfInterest);



        // If difference with current element is more than root, then ignore it.
        if (difference > differenceHeap.top().first)
            continue;

        // Else remove root and insert
        differenceHeap.pop();
        differenceHeap.push({ difference, i });

    }

    // Store contents of heap
    while (differenceHeap.empty() == false) {

        qDebug() << unsortedVector.at(differenceHeap.top().second) << " ";
        matches.push_back(unsortedVector.at(differenceHeap.top().second));

        differenceHeap.pop();
    }

    return matches;
}

double TemperatureCorrection::interpolatePoint(double x1, double x2, double x){
    // This function is no longer needed but could be included in shared code
    // Linear interpolation between two points x1 and x2 to find x.
    /*   x1                    x                x2
       |---------d1------------|-------d2-------|
       |--------------------d-------------------|

       The fraction is calculated as fraction=d1/d=(x-x1)/(x2-x1)
   */

    double fraction=(x-x1)/(x2-x1);

    return (x1*(1.0-fraction))+(x2*fraction);
}



//void TemperatureCorrection::applyCorrections
