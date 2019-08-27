#ifndef TEMPERATURECORRECTION_H
#define TEMPERATURECORRECTION_H
#include <iostream>
#include "computetaylor.h"
#include <vector>
#include <numeric>
#include <QMap>
#include "common/math/polynomials/singleVariablePolynomial.h"


// This is intended as a specialized class
// That applies temperature correction to Echelles by evaluating a taylor polynomial
// created by using the computeTaylor class




class TemperatureCorrection
{
public:
    TemperatureCorrection();

    void storePixelandWavelengthOrderTable();
    void retrieveBetas(int id);
    void calculateShifts();
    QList<double> wavelengthInterface(double OL, double x);



    ComputeTaylor taylorPolynomialX;
    ComputeTaylor taylorPolynomialY;



    std::vector<double> betasX;
    std::vector<double> betasY;





    //Storage container in the form
    //QMap<order,vector<pair<Yvalue,Wavelength>>
    QMap<int, std::vector<pair<double,double>>> unCorrected;

    QMap<int,std::vector<double>> wavelength;

    QMap<int,std::vector<double>> pixelY;


    QMap<int,std::vector<double>> correctedX;
    QMap<int,std::vector<double>> correctedY;


    // Variables

    double X_mean=2048/2.0;
    double Y_mean=512/2.0;
    double OL_mean=21;
    double temperature_mean=25;


};

#endif // TEMPERATURECORRECTION_H
