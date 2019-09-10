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

        //Retrieve model specific betas
        retrieveBetas(1);

        // Create Taylor polynomial for X
        taylorPolynomialX.updateBeta(betasX);
        taylorPolynomialX.createTaylorPolynomial();

        // Create Taylor polynomial for Y
        taylorPolynomialY.updateBeta(betasY);
        taylorPolynomialY.createTaylorPolynomial();

        // Test Evaluation
        // All points must be doubles or floats for unwanted rounding to not occur


        //float test=taylorPolynomialY.taylorPolynomial(-19.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
        //OL X Y Temp
        //float testy=taylorPolynomialY.taylorPolynomial(-20.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
        float testy=taylorPolynomialY.taylorPolynomial(-19.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
        float testx=taylorPolynomialX.taylorPolynomial(-19.0)(-1024.0)(455.6606730301-508.0/2.0)(0.0);
        std::cout<<"Test eval Y: "<<testy<<"\n";
        std::cout<<"Test eval X: "<<testx<<"\n";

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
                betasX.push_back(query.value(0).toFloat());
            }

            // betasY

            query.exec("SELECT Y from taylorcoefficients WHERE id='1'");
            while(query.next()){
                betasY.push_back(query.value(0).toFloat());
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
                    //std::cout<<line.toStdString()<<"\n";

                    //There should be 45 columns

                    //std::cout<<"The size of the row QStringList is: "<<split_line.size()<<"\n";

                    for(int i=0;i<split_line.size();i++){
                        wavelength[i].push_back(split_line.at(i).toFloat());
                    }

                    //Plugging in each line from file
                    //Temperature is 0 for now.
//                    float X_data=split_line.at(0).toFloat();
//                    float Y_data=split_line.at(1).toFloat();
//                    float order_Line=split_line.at(2).toFloat();
//                    datapoint_evaluated=temperature_model(X_data)(Y_data)(order_Line)(0.0);
//                    model_corrected_data.push_back(datapoint_evaluated);
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
                    //std::cout<<line.toStdString()<<"\n";

                    //There should be 45 columns

                    //std::cout<<"The size of the row QStringList is: "<<split_line.size()<<"\n";

                    for(int i=0;i<split_line.size();i++){
                        pixelY[i].push_back(split_line.at(i).toFloat());
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

//        QMapIterator<int,std::vector<float>> i(pixelY);
//        std::cout<<"printing the key\n";
//        while(i.hasNext()){
//            i.next();
//            tempvec=i.value();
//            std::cout<<"Printing out the contents of vector number: "<<i.key()<<"\n";
//            for(int j=0;j<tempvec.size();j++){
//                std::cout<<tempvec.at(j)<<"\n"; //value
//                std::cout<<j<<"\n";
//            }

//        }


        //calculateShifts();


}

void TemperatureCorrection::calculateShifts(){
    // This function uses evaluates the correction to each of these axes by using the
    // axis specific taylor polynomial created in previous steps.

    // To begin with, the following corrections need to be done

    // Correction 1. X shift
    // taylorPolynomialX(X index value for order N)(Y value for order N)(OrderLine)(Temperature)
    // The result of the above operation is in form QMap<order number, vector of 2048 floats>

    // With greger's new data this order is actually
    // taylorPolynomialX(OrderLine)(X index value for order N)(Y value for order N)(Temperature)


    // Correction 2. Y shift
    // taylorPolynomialY(X index value for order N)(Y value for order N)(OrderLine)(Temperature)
    // The result of the above operation is in form QMap<order number, vector of 2048 float>

    Rigaku::Common::Math::Polynomials::SingleVariablePolynomial lambdaWRTx;

    Rigaku::Common::Math::Polynomials::SingleVariablePolynomial YWRToldLambda;


    //vector<float> xPixelInts(2048);
    //std::iota(xPixelInts.begin(),xPixelInts.end(),0);


    std::vector<float> tempVec;
    xPlusDeltaX.clear();
    yPlusDeltaY.clear();
    newWavelength.clear();
    correctedY.clear();

    QMapIterator<int,std::vector<float>> i(pixelY);
    //std::cout<<"printing the key\n";
    while(i.hasNext()){
        i.next();
        tempVec=i.value(); //TODO is this copy needed?
        // Evaluate both Taylor polynomials
        float deltaX;
        float deltaY;
        for(int j=0;j<tempVec.size();j++){
            //std::cout<<tempVec.at(j)<<"\n"; //value
            //std::cout<<j<<"\n";

            // Subtracting the mean
            float OL=(i.key()*1.0)-OL_mean;
            float X=j*1.0-X_mean;
            float Y=tempVec.at(j)*1.0-Y_mean;
            //Temp isn't used for now
            float T=0.0;


            //Correct X
            deltaX=taylorPolynomialX.taylorPolynomial(OL)(X)(Y)(T);

            //qDebug()<<"Printing out evalPointX: "<<evalPointX<<"\n";

            xPlusDeltaX[i.key()].push_back(deltaX+j);


            // Calculate delta Y and add it to original Y
            deltaY=taylorPolynomialY.taylorPolynomial(OL)(X)(Y)(T);
            //qDebug()<<"Printing out evalPointY: "<<evalPointY<<"\n";
            //taylorPolynomialX(j)(tempVec.at(j))(i.key())(0.0);

            yPlusDeltaY[i.key()].push_back(deltaY+tempVec.at(j));
        }

        // Now we have to find the polynomial that describes the relationship between the old wavelength
        // and the new X's

        // ie. rc=(X1, lamba0,5)

        lambdaWRTx.DataFit(xPlusDeltaX[i.key()],wavelength[i.key()],5);

        // Evaluate the polynomial created above
        // at integer values of X from 0-2047 to get the new wavelengthValues
        for(int xIndex=0;xIndex<2048;xIndex++){
                newWavelength[i.key()].push_back(lambdaWRTx.Evaluate(xIndex));
        }

        // Now we have to find the polynomial that describes Y+deltaY (dependent)
        // with respect to the old wavelength

        YWRToldLambda.DataFit(wavelength[i.key()],yPlusDeltaY[i.key()],5);

        // Evaluate the polynomial created above to find the corrected Y value
        // by plugging in the new wavelength values created on line 267


        for(int yIndex=0;yIndex<2048;yIndex++){
                correctedY[i.key()].push_back(YWRToldLambda.Evaluate(newWavelength[i.key()].at(yIndex)));
        }

    }


    //debugging
    //qDebug()<<"Printing out the corrected X vector\n";
    //qDebug()<<correctedX[0];

    qDebug()<<"Printing out the corrected Y vector\n";
    qDebug()<<correctedY;

}


void TemperatureCorrection::printCSV(){

}


std::vector<float> TemperatureCorrection::findWavelengthAndYValue(int OL, float x){

    // This is the interface function that determines
    // The input is OL and X and the output should be corrected Y and delta


    Rigaku::Common::Math::Polynomials::SingleVariablePolynomial wavelengthPoly;


    std::vector<float> XVectorToInterpolate;
    //std::vector<float> YVectorToInterpolate;

    std::vector<float> YandLambda;

    // Need to extract the correct vector from the container before interpolating

    //XVectorToInterpolate=deltaX[OL];
    //YVectorToInterpolate=correctedY[OL];



    // Now interpolate the X values

    std::vector<float> boundaryPointsX;

    // Testing to find bounds. Find two closest values to x
    boundaryPointsX=findKClosestValue(XVectorToInterpolate,x,2);

    // Debug

    qDebug()<<boundaryPointsX;



    // TODO this return the actual values but we need to find the indexes of the values as well because
    // we use the indexes to extract Y and wavelength values

    // Finding index

    //std::vector<float>::iterator vectorIterator=std::find(XVectorToInterpolate.begin(),XVectorToInterpolate.end(),number);
//    int index;
//    if(vectorIterator!=XVectorToInterpolate.end()){
//        qDebug()<<"XVectorToInterpolate contains element"<<endl;
//        index=std::distance(XVectorToInterpolate.begin(),vectorIterator);

          // Use index to return corrected Y value and Wavelength
            //YandLambda.push_back(correctedY[OL].at(index));
            //YandLambda.push_back(wavelength[OL].at(index));


//        qDebug()<<"Index is: "<<index;
//    }else{
//        qDebug()<<"XVectorToInterpolate does not contain this element"<<endl;
//    }





    return YandLambda;

}


std::vector<float> TemperatureCorrection::findKClosestValue(std::vector<float> unsortedVector, float numberOfInterest, int k){
    // Given an unsorted vector, find the k closest values to numberOfInterest. "Closest" meaning absolute value distance
    // TODO: Is this slower or faster than sorting the unsortedVector and then doing a binary search?
    // This method is O(n Log k) time so it's fast

    std::vector<float> matches;

    // Make a heap of difference with first k elements.
    priority_queue<pair<float, int> > differenceHeap;
    for (int i = 0; i < k; i++)
        differenceHeap.push({ abs(unsortedVector.at(i) - numberOfInterest), i });

    // Now process remaining elements.
    for (int i = k; i < unsortedVector.size(); i++) {

        float difference = abs(unsortedVector.at(i) - numberOfInterest);



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

float TemperatureCorrection::interpolatePoint(float x1, float x2, float x){
    // Linear interpolation between two points x1 and x2 to find x.
    /*   x1                    x                x2
       |---------d1------------|-------d2-------|
       |--------------------d-------------------|

       The fraction is calculated as fraction=d1/d=(x-x1)/(x2-x1)
   */

    float fraction=(x-x1)/(x2-x1);

    return (x1*(1.0-fraction))+(x2*fraction);
}



//void TemperatureCorrection::applyCorrections
