#include <QCoreApplication>
#include "multipoly.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <cstdint>
#include <cassert>
#include <qfile.h>
#include <QDebug>
#include <QString>
#include <QStringList>


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);


    //Betas from somewhere
    //generated for now
    //Beta0=1
    //test vector
    //std::vector<double> betas{1,2,3,4,5,6,7,8,9,10,11,12,13,14};

    //betas from Greger
    std::vector<double> betas{21.9057,   //beta0
                              12.7557/1000,   //X, b1
                              0.2332,   //Y,b2
                              2.1753,    //o,b3
                              0,        //T,b4
                              -0.000014988,        //X^2, b5
                              -0.00059408,   //Y^2, b6
                              -0.026,  //o^2, b7
                              0         //T^2, b8
                             };


    //Polynomial Expression
    //generated with four indeterminates
    //Beta1 paired with X vector and so on
    std::vector<std::vector<double>> poly_expression{
        {1,0,0,0}, //X
        {0,1,0,0}, //Y
        {0,0,1,0}, //o for Orderline
        {0,0,0,1}, //T
        {2,0,0,0}, //X^2
        {0,2,0,0}, //Y^2
        {0,0,2,0}, //o^2
        {0,0,0,2} //T^2
        //{1,1,0,0}, //X*Y
        //{1,0,1,0}, //X*o
        //{1,0,0,1}, //X*T
        //{0,1,1,0}, //Y*o
        //{0,1,0,1}, //Y*T
        //{0,0,1,1} //o*T
    };






    //Assemble the polynomial
    //Starting with constant beta term


    //Up to four indeterminates
    poly<4, double> temperature_model =  betas.at(0);


    //Add rest of terms

    for(int i=1; i<poly_expression.size();i++){


        int e=poly_expression[i-1].at(0);
        int f=poly_expression[i-1].at(1);
        int g=poly_expression[i-1].at(2);
        int h=poly_expression[i-1].at(3);

        //For Validation/Debugging
        std::cout<<"Term Number: "<<i<<" Term Exponents: "<<e<<" "<<f<<" "<<g<<" "<<h<<" "<<"\n";
        std::cout<<"Beta number: "<<betas[i]<<" Beta value: "<<betas.at(i)<<"\n";
        std::cout<<"Combined term: "<<betas.at(i)*Monomial<double>(e,f,g,h)<<"\n\n";


        temperature_model+=betas.at(i)*Monomial<double>(e,f,g,h);


        std::cout<<"Taylor Series after adding "<<i<< " term: "<<temperature_model<<"\n\n\n";
    }



    //Print out
    std::cout<<"\n\n\n Taylor series final form: "<<temperature_model<<"\n";


    //Evaluate the polynomial at a test point f(X,Y,o,T)
    double evaluated = temperature_model(946.1754)(-162.6883)(16.0)(0.0);
    std::cout<<"Evaluated:"<<evaluated<<"\n";

    std::cout<<"Trying to open data file"<<"\n";
    //Open file with data

    double datapoint_evaluated;
    QVector<double> model_corrected_data;

    QFile data_file(QString("./first_test_data.txt"));

    if(data_file.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&data_file);
        QString line=stream.readLine();
        while(!line.isNull()){
            QStringList split_line=line.split('\t');
            //debugging
            //std::cout<<line.toStdString()<<"\n";


            //Plugging in each line from file
            //Temperature is 0 for now.
            double X_data=split_line.at(0).toDouble();
            double Y_data=split_line.at(1).toDouble();
            double order_Line=split_line.at(2).toDouble();
            datapoint_evaluated=temperature_model(X_data)(Y_data)(order_Line)(0.0);
            model_corrected_data.push_back(datapoint_evaluated);
            line=stream.readLine();
        }
        data_file.close();
    }
    std::cout<<"Generated corrected vector"<<"\n";

    std::cout<<"Writing to File"<<"\n";

    QFile output_file(QString("./corrected_data.txt"));
    if(output_file.open(QFile::ReadWrite|QFile::Text))
    {
        QTextStream out_stream(&output_file);
        for (QVector<double>::iterator iter = model_corrected_data.begin(); iter != model_corrected_data.end(); iter++){
            out_stream << *iter;
            out_stream <<"\n";
        }
        output_file.close();
    }
    std::cout<<"Done"<<"\n";

    return a.exec();
}
