#include "computetaylor.h"
#include "multipoly.h"
#include <math.h>
#include <functional>
#include <algorithm>
#include <bitset>
#include <QSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>



ComputeTaylor::ComputeTaylor()
{



    // Number of indeterminates
    // This is much more likely to change than the max power of the exponent according to Greger.

    // From DB

    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE");

    //db.setDatabaseName(":memory:");

    db.setDatabaseName("computeTaylor.db");
    bool openOk =db.open();

    if(openOk){

        QSqlQuery query("SELECT value from taylorexpansion");
        while (query.next())
        {
            numberOfIndeterminates=query.value(0).toInt();
            //std::cout<<"Number of Indeterminants from DB is: "<<numberOfIndeterminates<<"\n";
        }
    }
    // It can't be larger than 4 for now because poly<n,T> currently has n=4
    // This is easily changed to accomodate a max of 5 but the
    // monomialvector template also needs to be changed to handle 5 indeterminants
    // See multipoly.h line 1161
    if(numberOfIndeterminates>4){
        std::cout<<"Number of Indeterminants is too high. Setting to default of 4";
        numberOfIndeterminates=4;
    }


    // Maxiumum exponent power
    // According to Greger, this should really be hardcoded to 2 because it is unlikely ever to change

    maxTermExponent=2;




}

void ComputeTaylor::initTaylorPoly(){


    // Create the decimal vector for representing the exponents in polyexpression
    // It is filled in with powers of 2 in descending order. The descensing order is
    // due to how Greger and I defined the order before the implementation


    std::vector<int> decimalVector;

    for (int i=numberOfIndeterminates; i>=1;i--){
        decimalVector.push_back(pow(2,(i-1)));
        //debugging
        //std::cout<<"Just pushed back: "<<pow(2,(i-1))<<"\n";
    }




    createPolynomialExpression(decimalVector);

}

void ComputeTaylor::createPolynomialExpression(std::vector<int> &decimalVector,bool includeNonBasisTerms){
    // The pattern for the exponents for 4 indeterminates is
    // 4 choose 1 ====> 0001,0010,0100,1000   these are the basis terms
    // 4 choose 1 multipled by 2 ====> 0002,0020,0200,2000  these are the quadratic basis terms

    // 4 choose2 2 ===> 1100,1010,1001,0110,0101,0011

    // Calculate the binomial coefficients n!/(k!(n-k)!)
    // n is the maximum number of indeterminates; k is capped to maximum power of exponent

    std::vector<char> polynomialExpression;
    integerPolynomialExpression.clear();

    // Non-basis terms, i.e. terms created by 4 choose 2 combination have additional requiremenent of being optional
    int startIndexforPolyExpression;
    if(includeNonBasisTerms){
        startIndexforPolyExpression=1;
    }else{
        startIndexforPolyExpression=2;
    }

    for (int k=startIndexforPolyExpression;k<=maxTermExponent;k++){
        std::vector<int>combination=for_each_combination(decimalVector.begin(),
                                                         decimalVector.begin()+k,
                                                         decimalVector.end(),
                                                         combinationFunc(decimalVector.size()));

        std::vector<char> exponentsVector;
        std::vector<int> exponentsVectorInt;

        exponentsVector=zeroPaddedBinaryConversion(combination);


        // Append to polynomialExpression
        polynomialExpression.insert(polynomialExpression.end(),exponentsVector.begin(),exponentsVector.end());


        // The quadratic basis terms are just the same as the basis vector

        if(k==1){


            // 4 choose 1 except scalar multipled by maxTermExponent, which is hardcoded to 2


            // Same as multiplication by maxTermExponet but faster because no additional
            // conversions are needed
            for (int i=0;i<exponentsVector.size();i++){
                if(exponentsVector.at(i)=='1'){
                    exponentsVector.at(i)='0'+maxTermExponent;

                }
            }
            // Append this quadratic basis vector to the main polynomial expression vector
            polynomialExpression.insert(polynomialExpression.end(),exponentsVector.begin(),exponentsVector.end());

        }
    }



    // Convert to int vector
    for (int i=0;i<polynomialExpression.size();i++){
        integerPolynomialExpression.push_back(polynomialExpression.at(i)-'0');
    }


    //Print vector for debugging

    //    for (int i=0; i<integerPolynomialExpression.size();i++){
    //        std::cout<<integerPolynomialExpression.at(i)<<"\n";
    //    }

    // Create the model based on the expression
    //createTaylorPolynomial(integerPolynomialExpression);

}

std::vector<char> ComputeTaylor::zeroPaddedBinaryConversion(std::vector<int> &decimalVector){
    // Convert to binary and add leading zeros so length matches number of indeterminates
    // This is to aid creation of monomial terms when using multipoly.h
    std::vector<char> binaryValue;

    for (int i=0; i<decimalVector.size();i++){
        std::string individualDigitConvertedtoBinary;

        // Binary Conversion
        individualDigitConvertedtoBinary=(std::bitset<sizeof(decimalVector.at(i))>(decimalVector.at(i))).to_string();


        // Push individual digits
        for (auto &&c:individualDigitConvertedtoBinary){
            binaryValue.push_back(c);

        }

    }

    return binaryValue;

}




void ComputeTaylor::createTaylorPolynomial(){
    // Creates a polynomial from a polynomial expression
    // Represented as: beta0+beta1*monomial1+beta2*monomial2...
    // Monomials have the indeterminate variables as input with their exponents
    // being in polyExpression

//    if(integerPolynomialExpression.size()!=betas.size()+1){
//        std::cout<<"Polynomial expression and/or betas have bad dimensions \n";
//        return;
//    }


    taylorPolynomial= betas.at(0);

    // Determine the number of terms based on the lenght of the expression
    int totalNumberofTerms=integerPolynomialExpression.size()/numberOfIndeterminates;
    std::vector<unsigned int> monomialExponentsVector;
    for (int i=0;i<totalNumberofTerms;i++){
        //std::cout<<"Term number "<<i<<"\n";
        for (int j=0;j<numberOfIndeterminates;j++){
            monomialExponentsVector.push_back(integerPolynomialExpression.at(j+i*numberOfIndeterminates));

        }

        // Create monomial term;
        // TODO put in check if number of betas is not as expected
        taylorPolynomial+=betas.at(i+1)*monomialVector<double>(monomialExponentsVector);
        monomialExponentsVector.clear();

    }

    std::cout<<"Total number of terms is: "<<totalNumberofTerms<<"\n";

    displayTaylorPolynomial();

}

std::vector<double> ComputeTaylor::applyModelToOrder(std::vector<std::vector<double>> &orderVector){
    // For now the storage container is a 2D vector like:
    // {x0,x1,x2....xn},
    // {y1,y2,y3....yn},
    // {o1,o2,o3....on}
    double evaluatedCorrection;
    std::vector<double> correctedOrderLine;
    for (int i=0;i<orderVector.size();i++){

        // Calculate the correction and stuff vector
        evaluatedCorrection=taylorPolynomial(orderVector[i].at(0))(orderVector[i].at(1))(orderVector[i].at(2))(orderVector[i].at(3));
        correctedOrderLine.push_back(evaluatedCorrection);
    }
    return correctedOrderLine;
}

bool ComputeTaylor::displayTaylorPolynomial(){
    // Debugging function

    std::cout<<"The Taylor polynomial is: "<<taylorPolynomial;
}

bool ComputeTaylor::updateBeta(std::vector<double> beta){

    // TODO There are better ways to create the final taylorPolynomial than
    // doing it this way

    betas.clear();
    //Copy it
    betas.assign(beta.begin(),beta.end());

}


