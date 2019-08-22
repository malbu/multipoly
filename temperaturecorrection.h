#ifndef TEMPERATURECORRECTION_H
#define TEMPERATURECORRECTION_H
#include <iostream>
#include "computetaylor.h"
#include <vector>
#include <numeric>


// This is intended as a specialized class
// That applies temperature correction to Echelles by evaluating a taylor polynomial
// created by using the computeTaylor class




class TemperatureCorrection
{
public:
    TemperatureCorrection();


    ComputeTaylor taylorPolynomialX;
    ComputeTaylor taylorPolynomialY;


    void retrieveBetas(int id);

    std::vector<double> betasX;
    std::vector<double> betasY;

};

#endif // TEMPERATURECORRECTION_H
