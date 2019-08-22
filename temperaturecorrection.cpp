#include "temperaturecorrection.h"
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QString>

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
        taylorPolynomialX.initTaylorPoly();
        taylorPolynomialY.initTaylorPoly();


        //Retrieve model specific betas
        retrieveBetas(1);

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
            QSqlQuery query("SELECT X from taylorcoefficients WHERE id='1'");
            while(query.next()){

            }
        }

}
