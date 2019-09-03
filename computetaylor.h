#ifndef COMPUTETAYLOR_H
#define COMPUTETAYLOR_H
#include <vector>
#include "multipoly.h"
#include "hinnant.h"
#include <iostream>
#include <numeric>
#include <cstdint>
#include <cassert>

// Create combination terms for polynomial expression
// Print out range separated by commas

template <class It> unsigned createVariableCombinations(It begin, It end){
    unsigned count = 0;
    unsigned chosenPowerOfTwo= 0;
    unsigned int xored=0;

    if (begin != end)
    {

        xored=*begin;

        ++count;
        for (++begin; begin != end; ++begin)
        {
            chosenPowerOfTwo=*begin;

            // Bitwise xor to aid in translating to binary
            // This section might also be highly dependent on
            // assumption that maxTermExponent will always be 2
            xored=xored^chosenPowerOfTwo;
            ++count;
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

    void createTaylorPolynomial();

    void createPolynomialExpression(std::vector<int> &decimalVector,bool includeNonBasisTerms=true);

    std::vector<float> applyModelToOrder(std::vector<std::vector<float>> &orderVector);

    bool displayTaylorPolynomial();

    std::vector<char> zeroPaddedBinaryConversion(std::vector<int> &decimalVector);

    void initTaylorPoly();

    bool updateBeta(std::vector<float> beta);


    //Variables

    //Betas from Greger
    //TODO: create DB interface
//    std::vector<float> betas{21.9057,      //beta0
//                              12.7557/1000, //X, b1
//                              0.2332,       //Y,b2
//                              2.1753,       //o,b3
//                              0,            //T,b4
//                              -0.000014988,  //X^2, b5
//                              -0.00059408,   //Y^2, b6
//                              -0.026,        //o^2, b7
//                              0,            //T^2, b8
//                              0,
//                              0,
//                              0,
//                              0,
//                              0,
//                              0};

    std::vector<float> betas;


    std::vector<float> correctedOrderline;


    poly<4,float> taylorPolynomial;

    int numberOfIndeterminates;

    int maxTermExponent;


    std::vector<int> integerPolynomialExpression;




    //Functor called for each combination

    class combinationFunc
    {
        unsigned len;
        std::uint64_t count;
        std::vector<int> combinations;
    public:
        explicit combinationFunc(unsigned l) : len(l), count(0),combinations(0) {}

        template <class It>
        bool operator()(It first, It last)  // called for each combination
        {
            // Count the number of times this is called
            ++count;

            unsigned assembledCombinations= createVariableCombinations(first,last);
            combinations.push_back(assembledCombinations);
            return false;  // Don't break out of the loop
        }


        operator std::vector<int>() const {return combinations;}
    };


};

#endif // COMPUTETAYLOR_H
