#include "DC_Filter.h"


DCFilter::DCFilter() {
    a = 0.999;
    y = 0.0;
    prevX = 0.0;
    prevY = 0.0;
}

float DCFilter::filter(float sample){
    //y[n] = x[n] - x[n-1] + a * y[n-1]
    y = sample - prevX + a * prevY;
    prevX = sample;
    prevY = y;
    
    return y;
}