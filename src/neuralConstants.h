#ifndef NeuralNet_Header_h
#define NeuralNet_Header_h

#define INPUTCHANNELS 8
#define OUTPUTCHANNELS 2
#define SAMPLERATE 48000
#define BUFFERSIZE 1024

#include "math.h"
#include "ofMain.h"

// Global constants
const float speed = 1.0/16.0;
const float samplespms = SAMPLERATE/1000.0/speed;   // Number of samples per ms
const float tau = (1.0/samplespms);                 // Timescale
const float neuronResponseTime = 1.85 * samplespms;  // Charge time of neuron

const int D  = 20;                                  // Maximal axonal conduction delay in ms
const float	sm = 20.0;                              // Maximal synaptic strength

const float vrest = -65.0;                          // Resting potential
const float vpeak = 35.0;                           // Max membrane potential
const float STDPwindow = 20 * samplespms;           // Window size in ms

float clip(float n, float lower, float upper);      // Clip function
float randomMax(float x);

struct synapticID {                                 // Synapse struct with connection ID's
    int preSynID;
    int postSynID;
    int box;
};

struct AudioFrom {
    bool key;
    int ID;
};


#endif