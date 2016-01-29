/*
 
 Synapse class
 Connection between pre- & postsynaptic neuron.
 Keeps track of pre- and postsynaptic timing, how to scale weightings and when to send weightings (spike).
 
*/


#ifndef __synapse__NeuralNet__
#define __synapse__NeuralNet__

#include <iostream>
#include <queue>
#include <vector>

#include "neuron.h"
#include "neuralConstants.h"


class Synapse{
    
public:
    Synapse(int preSynID, int postSynID, bool ExcInh, int ID, float weight, int delayTime, vector<Neuron> *net, ofxXmlSettings* settings);
    ~Synapse();
    
    void reset();
    void resetDelays();
    void preSpike();
    int getTimePassed();
    
    void update(){
        // If presynaptic neuron spiked, add delayTime to conduction delays vector
        if (spiked) {
            delay.insert(delay.begin(), delayTime);
            ++delVecSize;
            spiked = false;
        }
        
        for (int i = 0; i < delVecSize; ++i) delay[i]--;
        
        if(ExcInh){
            if(preFirstSpike) ++preCounter;   // Update presynaptic timer
            if(postFirstSpike) ++postCounter; // Update postsynaptic timer
        }
    }
    
    void postSpike(){
        if(ExcInh){
            if (!preFirstSpike) preCounter = std::numeric_limits<unsigned int>::max();
            // Reset postsynaptic timer
            postCounter = 0;
            postFirstSpike = true;
            preDelta = preCounter;
            // Calculate weight change
            STDP(getTimePassed());
        }
    }
    
    float getWeight() {
        // After conduction delay, calculate and return weighting
        if (!delay.empty()) {
            if (delay.back() <= 0) {
                delay.pop_back();
                --delVecSize;
                // Excitatory
                if (ExcInh){
                    // Reset presynaptic timer
                    preCounter = 0;
                    preFirstSpike = true; // to see first spike
                    postDelta = postCounter;
                    return weight;
                }
                // Inhibitory fixed weighting
                else return weight;
            }
            else return 0.0;
        }
        else return 0.0;
    }
    
    void STDP(int deltaT) {
        // Check if deltaT is out of time window
        if (abs(deltaT) < STDPwindow && abs(deltaT) > samplespms) {
            // LTP
            if (deltaT > 0) weightDerivative =  0.1*exp(-deltaT/STDPwindow);  // A+e(-x/t+)
            // LTD
            else            weightDerivative = -0.12*exp(deltaT/STDPwindow);  // A_e(x/t-)
        }else weightDerivative = 0.0;
        
        // Update weighting
        if (weightDerivative > 0) weight += WCR * weightDerivative * (sm - weight);
        else if (weightDerivative < 0) weight += WCR * weightDerivative * weight;
    }
    
    bool getExcInh();
    int getPreSynID();
    int getPostSynID();
    void setPreSynID(int ID);
    void setPostSynID(int ID);
    void setWCR(float wcr);
    void addToDelay(int delay);
    void addToWeight(int weight);
    
    void display();
    synapticID checkClick(ofVec2f loc);
    ofVec2f getLoc();
    void saveSettings();
    void resetDisplay();
    
protected:
    int preSynID;                           // Pre-synaptic ID
    int postSynID;                          // Post-synaptic ID
    bool ExcInh;                            // Exatory or inhibitory connection
    int ID;
    vector<Neuron> *net;                    // Neurons in network
    
    bool preFirstSpike, postFirstSpike;     // If pre- or postsynaptic neuron fired
    unsigned int preCounter,postCounter;    // Pre- & postsynaptic timing
    unsigned int preDelta, postDelta;       // Pre- & postsynaptic time differences
    int deltaT;                             // Time difference between pre & post

    std::vector<int> delay;                 // Conduction delay buffer
    unsigned int delayTime;                 // Conduction delay time
    unsigned int delVecSize;
    
    bool spiked;
    float weight;                           // Weight received from presynaptic neuron
    float weightDerivative;                 // Change of weight
    float WCR;                              // Weight change rate
    
    // Interface
    ofVec2f pointPosition;
    ofVec2f brezierPoint;
    ofRectangle box1, box2;
    ofxXmlSettings* settings;
    
    int preX;
    int preY;
    int postX;
    int postY;
    float x_dist;
    float y_dist;

};

#endif
