#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "neuron.h"
#include "synapse.h"
#include "audioIO.h"
#include "DC_Filter.h"


class ofApp : public ofBaseApp{
    
public:
    void setup();
    void update();
    void draw();
    void exit();
    void clear();
    void create();
    void mousePressed(int x, int y, int button);
    void mouseDragged(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void keyPressed(int key);
    void keyReleased(int key);

    void audioOut(float * input, int bufferSize, int nChannels);
    void audioIn(float * input, int bufferSize, int nChannels);
    
    void clearButtonPressed();
    void ResetSynapseButtonPressed();
    void ResetDelaysButtonPressed();
    void setPresetButtonPressed();
    void getPresetButtonPressed();
    void deleteNeuron(int ID);
    
private:
    int N;                                  // All neurons
    int Ne;                                 // Excitatory neurons
    int Ni;                                 // Inhibitory neurons
    int S;                                  // All synapses
    float Ne_a,Ne_b,Ne_c,Ne_d;              // Neural dynamics
    float Ni_a,Ni_b,Ni_c,Ni_d;
    
    vector<Neuron> net;                     // Neurons
    vector<Synapse> synapses;               // Synapses
    vector<bool> spikes;                    // List of spikes of 1 frame
    
    // GUI
    ofxPanel gui;
    ofxButton Clear;
    ofxButton ResetSynapse;
    ofxButton resetDelays;
    ofxButton setPreset;
    ofxButton getPreset;
    ofParameter<float> WCR;
    ofParameter<bool> outputAll;
    ofParameter<bool> onlySpike;
    ofParameter<float> noise;
    ofParameter<float> noiseLvl;
    ofxXmlSettings settings;
    string preset;
    
    ofSoundStream stream;
    mutex audioMutex;
    
    bool leftMouseHold;
    bool clickedOnSynapse;
    bool connection;
    bool alt;
    bool cmd;
    bool clickedOnInput;
    bool clickedOnOutput;
    int synapseVecID;
    synapticID checkIDSynapse;
    
    // Audio
    ofRtAudioSoundStream soundStream;
    DCFilter DCf[OUTPUTCHANNELS];
    float signal[OUTPUTCHANNELS];
    float volume;
    
    vector<audioIO> inputs;         // Input connections
    vector<audioIO> outputs;        // Output connections
    vector<float> inputBuffer;      // Input buffer
    AudioFrom audioFrom;
    float noiseConstant;
    
    vector<Neuron> emptyNeuron;
    vector<Synapse> emptySynapse;
    vector<bool> emptyBool;
    vector<audioIO> emptyAudioIO;
};
