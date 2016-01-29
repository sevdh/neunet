/*
 
 Neuron class
 Calculates action potential of neuron.
 Gets thalamic input and receives input from connected neurons via synapse.
 
 */

#ifndef __neuron__NeuralNet__
#define __neuron__NeuralNet__

#include "neuralConstants.h"
#include "math.h"
#include "ofMain.h"
#include "ofxXmlSettings.h"


class Neuron {
public:
    Neuron(float a, float b, float c, float d, int ID, bool type, ofVec2f loc, ofxXmlSettings* settings);
    
    bool fired() {  // Check if neuron fired
        if (v >= vpeak) {
            v = c;  // Membrane voltage reset if fired
            u += d; // Update recovery variable
            spiked = true;
            return true;
        }
        else return false;
    }
    
    void calculate() {
        // Calcalute action potential
        v += tau * ((0.04 * v + 5) * v + 140 - u + I);
        // Calculate recovery value
        u += tau * a * (b * v - u);
        // Reset input
        I = 0;
    }

    void receiveInput(float weight);
    float returnV();
    bool getType();
    void setID(int ID);
    int getID();
    
    void display();
    int checkClick(ofVec2f loc);
    ofVec2f getLoc();
    void saveSettings();

protected:
    bool spiked;
    float v;            // Membrane potential
    float u;            // Recovery value
    float a;            // Time scale of recovery value
    float b;            // Sensitivy of recovery value
    float c;            // After spike reset variable caused by v
    float d;            // After spike reset variable caused by u
    float I;            // Random thalamic input
    int ID;             // ID of neuron
    bool type;          // Excitatory or Inhibitory
    float scaleFactor;  // Scale v for audio ouput
    
    
    ofVec2f loc;        // Interface
    int pulseColorRG;
    int pulseColorB;
    float r;
    ofxXmlSettings* settings;
};

#endif
