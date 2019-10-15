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
    std::vector<double> findCorrectedWavelengthAndYValue(int OL, double x);

    std::vector<double> findKClosestValue(std::vector<double> unsortedVector, double numberOfInterest, int k);

    void printCSV();

    double interpolatePoint(double x1, double x2, double x);

    std::vector<double> calculateMeanAndStDev(std::vector<double> inputVector);

    void truncateData(int numberofPointstoCutfromSides);

    ComputeTaylor taylorPolynomialX;
    ComputeTaylor taylorPolynomialY;



    std::vector<double> betasX;
    std::vector<double> betasY;





    //Storage container in the form
    //QMap<order,vector<pair<Yvalue,Wavelength>>
    QMap<int, std::vector<pair<double,double>>> unCorrected;

    QMap<int,std::vector<double>> wavelength;
    QMap<int,std::vector<double>> newWavelength;

    QMap<int,std::vector<double>> pixelY;


    QMap<int,std::vector<double>> xPlusDeltaX;
    QMap<int,std::vector<double>> yPlusDeltaY;

    QMap<int,std::vector<double>> correctedY;
    QMap<int,std::vector<double>> splineCheckMap;


    // Variables

    double X_mean=(2048.0/2.0)-1.0;
    double Y_mean=(508.0/2.0)-1.0;
    double OL_mean=19.0;
    double temperature_mean=31.1843;


};

#endif // TEMPERATURECORRECTION_H
