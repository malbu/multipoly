#ifndef COMPUTETAYLOR_H
#define COMPUTETAYLOR_H
#include <vector>
#include "multipoly.h"
#include "hinnant.h"
#include <iostream>
#include <vector>
#include <numeric>
#include <cstdint>
#include <cassert>

//Create combination terms for polynomial expression
//Print out range separated by commas

template <class It> unsigned createPolynomialExpressionTerm(It begin, It end){
    unsigned r = 0;
    unsigned s= 0;
    unsigned int xored=0;

    if (begin != end)
    {
        //std::cout <<"test"<< *begin;
        //unsigned test2;
        xored=*begin;




        //std::cout<<"Before xored "<<xored<<"\n";


        ++r;
        for (++begin; begin != end; ++begin)
        {
            s=*begin;

            //bitwise xor to aid in translating to binary
            xored=xored^s;
           // std::cout << " s " << s<<"\n";
           // std::cout<< "Xor'd "<<xored<<"\n";
            ++r;
        }
    }
    return xored;
}
using namespace std;
class ComputeTaylor
{
public:

    ComputeTaylor();

    //Functions

    void createModel(std::vector<int> polyExpression);
    std::vector<double> applyModelToOrder(std::vector<std::vector<double>> &orderVector);
    bool displayTaylorPolynomial();
    void createPolynomialExpression(std::vector<int> &decimalVector,bool includeNonBasisTerms=true);
    std::vector<char> zeroPaddedBinaryConversion(std::vector<int> &decimalVector);

    //Variables

    //Betas from Greger
    //TODO: create DB interface
    std::vector<double> betas{21.9057,      //beta0
                              12.7557/1000, //X, b1
                              0.2332,       //Y,b2
                              2.1753,       //o,b3
                              0,            //T,b4
                             -0.000014988,  //X^2, b5
                             -0.00059408,   //Y^2, b6
                             -0.026,        //o^2, b7
                              0,            //T^2, b8
                              0,
                              0,
                              0,
                              0,
                              0,
                              0};


    std::vector<double> correctedOrderline;


    poly<4,double> temperatureModel;

    int numberOfIndeterminates;

    int maxTermExponent;




    //Functor called for each combination

    class combinationFunctor
    {
        unsigned len;
        std::uint64_t count;
        std::vector<int> combinations;
    public:
        explicit combinationFunctor(unsigned l) : len(l), count(0),combinations(0) {}

        template <class It>
            bool operator()(It first, It last)  // called for each combination
            {
                // count the number of times this is called
                ++count;

                unsigned r= createPolynomialExpressionTerm(first,last);
                //std::cout<<"r!!!"<<r<<"\n";
                combinations.push_back(r);
                //std::cout<<"\n";
                return false;  // Don't break out of the loop
            }


        operator std::vector<int>() const {return combinations;}
    };


};

#endif // COMPUTETAYLOR_H
