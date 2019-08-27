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


        //double test=taylorPolynomialX.taylorPolynomial(946.1754)(-162.6883)(16.0)(0.0);
        //std::cout<<"Test eval: "<<test<<"\n";

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
                std::cout<<query.value(0).toDouble()<<"\n";
            }

        }


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

                    std::cout<<"The size of the row QStringList is: "<<split_line.size()<<"\n";

                    for(int i=0;i<split_line.size();i++){
                        wavelength[i].push_back(split_line.at(i).toDouble());
                    }

                    //Plugging in each line from file
                    //Temperature is 0 for now.
//                    double X_data=split_line.at(0).toDouble();
//                    double Y_data=split_line.at(1).toDouble();
//                    double order_Line=split_line.at(2).toDouble();
//                    datapoint_evaluated=temperature_model(X_data)(Y_data)(order_Line)(0.0);
//                    model_corrected_data.push_back(datapoint_evaluated);
                    line=stream.readLine();
                }
                wavelengths_file.close();
            }


        // Open the pixel y csv

        QFile pixely_file(QString("./samplePixelPositions.csv"));

        if(pixely_file.open(QIODevice::ReadOnly))
            {
                QTextStream stream(&pixely_file);
                QString line=stream.readLine();
                while(!line.isNull()){
                    QStringList split_line=line.split(',');
                    //debugging
                    //std::cout<<line.toStdString()<<"\n";

                    //There should be 45 columns

                    std::cout<<"The size of the row QStringList is: "<<split_line.size()<<"\n";

                    for(int i=0;i<split_line.size();i++){
                        pixelY[i].push_back(split_line.at(i).toDouble());
                    }


                    line=stream.readLine();
                }
                pixely_file.close();
            }

        //debugging

        qDebug()<<pixelY;
        qDebug()<<pixelY.size(); //45
        qDebug()<<pixelY[0].size();

        std::vector<double> tempvec;

//        QMapIterator<int,std::vector<double>> i(pixelY);
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

        calculateShifts();


}

void TemperatureCorrection::calculateShifts(){
    // This function uses evaluates the correction to each of these axes by using the
    // axis specific taylor polynomial created in previous steps.

    // To begin with, the following corrections need to be done

    // Correction 1. X shift
    // taylorPolynomialX(X index value for order N)(Y value for order N)(OrderLine)(Temperature)
    // The result of the above operation is in form QMap<order number, vector of 2048 doubles>


    // Correction 2. Y shift
    // taylorPolynomialY(X index value for order N)(Y value for order N)(OrderLine)(Temperature)
    // The result of the above operation is in form QMap<order number, vector of 2048 doubles>

    std::vector<double> tempVec;
    correctedX.clear();
    correctedY.clear();

    QMapIterator<int,std::vector<double>> i(pixelY);
    std::cout<<"printing the key\n";
    while(i.hasNext()){
        i.next();
        tempVec=i.value(); //TODO is this copy needed?
        // Evaluate both model
        double evalPointX;
        double evalPointY;
        for(int j=0;j<tempVec.size();j++){
            std::cout<<tempVec.at(j)<<"\n"; //value
            std::cout<<j<<"\n";

            //Correct X
            evalPointX=taylorPolynomialX.taylorPolynomial(j*1.0)(tempVec.at(j)*1.0)(i.key()*1.0)(0.0*1.0);

            qDebug()<<"Printing out evalPointX: "<<evalPointX<<"\n";

            correctedX[i.key()].push_back(evalPointX);


            //Correct Y
            evalPointY=taylorPolynomialY.taylorPolynomial(j*1.0)(tempVec.at(j)*1.0)(i.key()*1.0)(0.0*1.0);
            qDebug()<<"Printing out evalPointY: "<<evalPointY<<"\n";
            //taylorPolynomialX(j)(tempVec.at(j))(i.key())(0.0);

            correctedY[i.key()].push_back(evalPointY);
        }

    }


    //debugging
    qDebug()<<"Printing out the corrected X vector\n";
    qDebug()<<correctedX;

    qDebug()<<"Printing out the corrected Y vector\n";
    qDebug()<<correctedY;

}

QList<double> TemperatureCorrection::wavelengthInterface(double OL, double x){

    // This is the interface function that determines
    // The input is OL and X and the output should be corrected Y and delta

    //Trying a simple version while I work on getting singlevariablePolynomial imported to this project




}



//void TemperatureCorrection::applyCorrections
