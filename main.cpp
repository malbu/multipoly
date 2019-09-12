#include <QCoreApplication>
#include "multipoly.h"
#include "computetaylor.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <cstdint>
#include <cassert>
#include <qfile.h>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QElapsedTimer>
#include "temperaturecorrection.h"




int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //Time how long it takes
    QElapsedTimer timer;
    timer.start();
    TemperatureCorrection correction;
    //ComputeTaylor taylor;

    std::cout<<"\nCompleted in: "<<timer.nsecsElapsed()<<" ns\n";

    //Testing the interface function
    // Ask user for OL and X

    int OL;
    int x;

    std::cout<<"Enter an OL"<<std::endl;
    std::cin>>OL;

    std::cout<<"Enter a corrected X value"<<std::endl;

    std::cin>>x;

    std::vector<float> interfaceOutput;

    interfaceOutput=correction.findWavelengthAndYValue(OL,x);

    qDebug()<<"Printing corrected Y and corrected Wavelength"<<endl;
    qDebug()<<interfaceOutput<<endl;



    return a.exec();
}
