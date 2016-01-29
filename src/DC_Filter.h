#ifndef __Neural_audio__DC_Filter__
#define __Neural_audio__DC_Filter__

class DCFilter {
    
public:
    DCFilter();
    float filter(float sample);
    
protected:
    float a;
    
    float y;
    float prevX;
    float prevY;
};

#endif 
