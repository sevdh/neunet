#ifndef __NeuralNet__audioIO__
#define __NeuralNet__audioIO__

#include <vector>
#include "ofMain.h"
#include "neuron.h"
#include "neuralConstants.h"

class audioIO {
public:
    audioIO(int ID, bool type, vector<Neuron> *net, ofxXmlSettings* settings);
    ~audioIO();
    void display();
    void connect(int target);
    void disconnect(int target);
    int checkClick(ofVec2f mouse);
    int getVecSize();
    void saveSettings();
    
    vector<int> targets;
    
private:
    int ID;
    bool type;
    vector<Neuron> *net;
    
    ofVec2f loc;
    int size;
    int vecSize;
    
    ofxXmlSettings* settings;
};

#endif