#include "neuralConstants.h"


float clip( float n, float lower, float upper ) {
    n = ( n > lower ) * n + !( n > lower ) * lower;
    
    return ( n < upper ) * n + !( n < upper ) * upper;
}

float randomMax(float x){
    return static_cast<float> (rand()) / static_cast <float> (RAND_MAX/x);
}
