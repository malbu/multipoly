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

    return a.exec();
}
