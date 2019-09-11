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
    std::vector<float> findWavelengthAndYValue(int OL, float x);

    std::vector<float> findKClosestValue(std::vector<float> unsortedVector, float numberOfInterest, int k);

    void printCSV();

    float interpolatePoint(float x1, float x2, float x);

    std::vector<float> calculateMeanAndStDev(std::vector<float> inputVector);

    ComputeTaylor taylorPolynomialX;
    ComputeTaylor taylorPolynomialY;



    std::vector<float> betasX;
    std::vector<float> betasY;





    //Storage container in the form
    //QMap<order,vector<pair<Yvalue,Wavelength>>
    QMap<int, std::vector<pair<float,float>>> unCorrected;

    QMap<int,std::vector<float>> wavelength;
    QMap<int,std::vector<float>> newWavelength;

    QMap<int,std::vector<float>> pixelY;


    QMap<int,std::vector<float>> xPlusDeltaX;
    QMap<int,std::vector<float>> yPlusDeltaY;

    QMap<int,std::vector<float>> correctedY;


    // Variables

    float X_mean=2048.0/2.0;
    float Y_mean=508.0/2.0;
    float OL_mean=19.0;
    float temperature_mean=25;


};

#endif // TEMPERATURECORRECTION_H
