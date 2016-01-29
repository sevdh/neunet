#include "ofApp.h"

float RS[] = {0.02,0.2,-65,8};  //Regular spiking
float FS[] = {0.02,0.25,-65,2}; //Fast spiking
float IB[] = {0.02,0.2,-55,4};  //Intrinsically bursting
float CH[] = {0.02,0.2,-50,12}; //Chattering
float RZ[] = {0.1,0.26,-65,2};  //Resonator

//--------------------------------------------------------------
void ofApp::setup(){
    srand(static_cast <unsigned> (time(0)));
    ofBackground(240);
    ofSetFrameRate(26);
    ofEnableSmoothing();
    
    clear();
    
    // Setup GUI
    gui.setup();
    Clear.addListener(this, &ofApp::clearButtonPressed);
    ResetSynapse.addListener(this, &ofApp::ResetSynapseButtonPressed);
    setPreset.addListener(this, &ofApp::setPresetButtonPressed);
    getPreset.addListener(this, &ofApp::getPresetButtonPressed);

    gui.add(outputAll.set("All -> audio out", false));
    gui.add(onlySpike.set("Only spike", false));
    gui.add(noise.set("Noise", false));
    gui.add(WCR.set("Weight Change Rate", 0,0,1.0));
    gui.add(Clear.setup("Clear"));
    gui.add(ResetSynapse.setup("Reset synapse"));
    gui.add(setPreset.setup("Save settings"));
    gui.add(getPreset.setup("Get settings"));
    gui.setPosition(ofGetWidth()-gui.getWidth()-5, 5);
    gui.minimizeAll();
    preset = "settings.xml";
    
    leftMouseHold = false;
    clickedOnSynapse = false;
    audioFrom.key = false;
    clickedOnInput = false;
    clickedOnOutput = false;
    
    // Setup audio
    //soundStream.listDevices();
    soundStream.setInDeviceID(4);
    soundStream.setOutDeviceID(2);
    volume = 0.9f;
    inputBuffer.assign(BUFFERSIZE * INPUTCHANNELS, 0.0);
    soundStream.setup(this, OUTPUTCHANNELS+0, INPUTCHANNELS, SAMPLERATE, BUFFERSIZE, 2);
    noiseConstant = 0.03*powf(speed,0.5);
    
    //create();
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw() {
//    ofHideCursor();
//    ofShowCursor();
    
    for (int i = 0; i < INPUTCHANNELS; ++i) inputs[i].display();
    for (int i = 0; i < OUTPUTCHANNELS; ++i) outputs[i].display();
    for (int i=0; i<S; ++i) synapses[i].display();
    for (int i=0; i<N; ++i) net[i].display();
    
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::exit() {
    soundStream.close();
    synapses.swap(emptySynapse);
    net.swap(emptyNeuron);
    spikes.swap(emptyBool);
    inputs.swap(emptyAudioIO);
    outputs.swap(emptyAudioIO);
    inputBuffer.clear();
    
    gui.clear();
    Clear.removeListener(this,&ofApp::ResetSynapseButtonPressed);
    ResetSynapse.removeListener(this,&ofApp::ResetSynapseButtonPressed);
    setPreset.removeListener(this,&ofApp::setPresetButtonPressed);
    getPreset.removeListener(this,&ofApp::getPresetButtonPressed);
}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels){
    unique_lock<mutex> lock(audioMutex);
    // ----------------- Update neurons, synapses and audio buffers  -----------------
    for (int i = 0; i < BUFFERSIZE; ++i){
        // Audio input
        for (int j=0; j<INPUTCHANNELS; ++j) {
            for (int k=0; k<inputs[j].getVecSize(); ++k){
                net[inputs[j].targets[k]].receiveInput(fabs(inputBuffer[i*INPUTCHANNELS+j])*20.0);
            }
        }
        
        // Noise input to network
        if(noise.get() && N != 0) {
            int randNeuron = randomMax(N);
            if(net[randNeuron].getType()) net[randNeuron].receiveInput(randomMax(N * noiseConstant));
            else net[randNeuron].receiveInput(randomMax(N * 0.3f * noiseConstant));
        }
        
        // Neurons
        for (int j=0; j<N; ++j){
            // Keep a list of spiked neurons of this frame
            net[j].calculate();
            spikes[j] = net[j].fired();
        }
        
        // Synapses
        for (int s = 0; s < S; ++s) {
            // set the Weight Change Rate
            synapses[s].setWCR(WCR.get());
            
            // get weights from presynaptic- to postsynaptic neurons
            float weight = synapses[s].getWeight();
            
            //if current synapse returns a weighting, return it to its postNeuron
            if(weight != 0.0) net[synapses[s].getPostSynID()].receiveInput(weight);
            
            // If pre-synaptic neuron spikes
            if (spikes[synapses[s].getPreSynID()]) synapses[s].preSpike();
            
            // If post-synaptic neuron spikes
            if (spikes[synapses[s].getPostSynID()]) synapses[s].postSpike();

            // Update synapse
            synapses[s].update();
        }

        // Calculate action potential of neurons and send to audio output
        float sig = 0;
        for(int j = 0; j<OUTPUTCHANNELS; ++j) signal[j] = 0.0;
        
        if (leftMouseHold && audioFrom.key){
            sig = net[audioFrom.ID].returnV();
            for (int c=0; c<OUTPUTCHANNELS; ++c) {
                signal[c] = sig;
            }
        }
        else if (outputAll) {
            for (int j=0; j<N; ++j) {
                sig = net[j].returnV() / N;
                for (int c=0; c<OUTPUTCHANNELS; ++c) {
                    signal[c] += sig;
                }
            }
        }
        else {
            // Audio output
            for(int j=0; j < N; ++j){
                for (int k=0; k<OUTPUTCHANNELS; ++k) {
                    for (int l=0; l<outputs[k].getVecSize(); ++l) {
                        if(outputs[k].targets[l] == j){
                            if (onlySpike.get()) {
                                if (spikes[j])
                                       signal[k] += 1.0 / (float)outputs[k].getVecSize();
                            }
                            else signal[k] += net[j].returnV() / (float)outputs[k].getVecSize();
                        }
                    }
                }
            }
        }
        for (int k=0; k<OUTPUTCHANNELS; ++k) {
            output[i * (OUTPUTCHANNELS+0)+k+0] = DCf[k].filter(signal[k]) * volume;
        }
    }// for i < buffersize
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels){
    for (int j=0; j<INPUTCHANNELS; ++j) {
        for (int i = 0; i < bufferSize; ++i) {
            inputBuffer[i*INPUTCHANNELS+j] = input[i*INPUTCHANNELS+j];
        }
    }
}

//--------------------------------------------------------------
void ofApp::clear() {
    unique_lock<mutex> lock(audioMutex);
    N = Ne = Ni = S = 0;
    usleep(100);
    if (!net.empty()) net.clear();
    if (!synapses.empty()) synapses.clear();
    if (!spikes.empty()) spikes.clear();
    if (!inputs.empty()) inputs.clear();
    if (!outputs.empty()) outputs.clear();
    for (int i = 0; i < INPUTCHANNELS; ++i) inputs.push_back(audioIO(i, false, &net, &settings));
    for (int i = 0; i < OUTPUTCHANNELS; ++i) outputs.push_back(audioIO(i, true, &net, &settings));
}

//--------------------------------------------------------------
void ofApp::deleteNeuron(int ID) {
        for(int i = S-1; i >= 0; --i) {
            if(synapses[i].getPreSynID() == ID || synapses[i].getPostSynID() == ID) {
                synapses.erase(synapses.begin()+i);
                --S;
            }
        }
    
    for (int i = 0; i < INPUTCHANNELS; ++i) inputs[i].disconnect(ID);
    for (int i = 0; i < OUTPUTCHANNELS; ++i) outputs[i].disconnect(ID);
    
    if (net[ID].getType()) --Ne;
    else --Ni;
    
    --N;
    spikes.erase(spikes.begin()+ID);
    net.erase(net.begin()+ID);

    
    for (int i = ID; i <= N; ++i) {
        if (ID != N) net[i].setID(net[i].getID()-1);
        for (int j = 0; j < S; ++j) {
            if(synapses[j].getPreSynID() == i) synapses[j].setPreSynID(synapses[j].getPreSynID()-1);
            if(synapses[j].getPostSynID() == i) synapses[j].setPostSynID(synapses[j].getPostSynID()-1);
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    ofVec2f loc(x,y);
    bool clickedOnNeuron = false;
    static int neuronID = 0;
    static int preNeuron, postNeuron = 0;
    
    static int inputID;
    
    static int outputID;
    
    // Check if clicked on synapse
    if (alt && !leftMouseHold && !cmd) {
        leftMouseHold = true;
        for (int i=0; i<S; ++i) {
            synapticID c = synapses[i].checkClick(loc);
            if (c.preSynID >= 0) {
                synapseVecID = i;
                checkIDSynapse = c;
                clickedOnSynapse = true;
                break;
            }
        else clickedOnSynapse = false;
        }
    };

  if (button == 0) {
      leftMouseHold = true;
        // Check if clicked on neuron
        if (!clickedOnSynapse) {
            for (int i=0; i<N; ++i) {
                int c = net[i].checkClick(loc);
                if (c >= 0) {
                    if (cmd) {
                        deleteNeuron(c);
                        clickedOnNeuron = false;
                    } else {
                    neuronID = c;
                    clickedOnNeuron = true;
                    break;
                    }
                }
                else clickedOnNeuron = false;
            }
        };

        // Check if already clicked on a neuron
        if (clickedOnNeuron) {
            audioFrom.ID = neuronID;
            
            // connect inputs (shift for multiple)
            if (audioFrom.key && clickedOnInput) {
                inputs[inputID].connect(neuronID);
                connection = false;
            }else if (!audioFrom.key) {
                if (clickedOnInput){
                    inputs[inputID].connect(neuronID);
                    connection = false;
                }
                clickedOnInput = false;
            }
            
            // connect outputs (shift for multiple)
            if (audioFrom.key && clickedOnOutput) {
                outputs[outputID].connect(neuronID);
                connection = false;
            }else if (!audioFrom.key) {
                if (clickedOnOutput) {
                    outputs[outputID].connect(neuronID);
                    connection = false;
                }
                 clickedOnOutput = false;
            }

            // If not clicked on pre neuron, set pre neuron
            if (!connection) {
                preNeuron = neuronID;
                connection = true;
            }
            // If already clicked on pre neuron, set post neuron and add synapse
            else {
                postNeuron = neuronID;
                if (preNeuron != postNeuron && net[preNeuron].getType() | net[postNeuron].getType()) {
                    bool exists = false;
                    for (int s = 0; s < S; ++s){
                        // Check if synapse already exists
                        if (synapses[s].getPreSynID() == preNeuron && synapses[s].getPostSynID()== postNeuron) {
                            exists = true;
                        }
                    }
                    // If bool is still false synapse doesnt exist yet
                    if(!exists){
                        float weight; int delayTime;
                        if (net[preNeuron].getType()) {
                            weight = 7.0;
                            delayTime = round((1 + (randomMax(D-1))) * samplespms);
                            synapses.push_back(Synapse(preNeuron, postNeuron, true, S, weight, delayTime, &net, &settings));
                        }
                        else {
                            weight = -5;
                            delayTime = samplespms;
                            synapses.push_back(Synapse(preNeuron, postNeuron, false, S, weight, delayTime, &net, &settings));
                        }
                        if (!audioFrom.key) connection = false;
                        S++;
                    }
                }
                else connection = false;
            }
        };

        // Check if clicked on input
        if (!clickedOnSynapse && !clickedOnNeuron) {
            for (int i=0; i<INPUTCHANNELS; ++i) {
                int c = inputs[i].checkClick(loc);
                if (c >= 0) {
                    inputID = c;
                    clickedOnInput = true;
                    clickedOnOutput = false;
                    connection = false;
                    break;
                }
                else clickedOnInput = false;
            }
        };
    
        // Check if clicked on output
        if (!clickedOnSynapse && !clickedOnNeuron && !clickedOnInput) {
            for (int i=0; i<OUTPUTCHANNELS; ++i) {
                int c = outputs[i].checkClick(loc);
                if (c >= 0) {
                    outputID = c;
                    clickedOnInput = false;
                    clickedOnOutput = true;
                    connection = false;
                    break;
                }
                else clickedOnOutput = false;
            }
        };
    }// If button = 0

    // If clicked on nothing, make new neuron
    if (!clickedOnNeuron && !clickedOnSynapse && !clickedOnInput && !clickedOnOutput && !cmd) {
        if (button == 0) {  // Exatory
            Ne_a = RS[0];
            Ne_b = RS[1];
            Ne_c = RS[2] + 15 * pow(ofRandom(1),2);
            Ne_d = RS[3] - 6 * pow(ofRandom(1),2);
            
            net.push_back(Neuron (Ne_a, Ne_b, Ne_c, Ne_d, N, true, loc, &settings));
            ++Ne;
        }
        else if (button == 2) { // Inhibitory
            Ni_a = FS[0] + 0.08 * ofRandom(1);
            Ni_b = FS[1] - 0.05 * ofRandom(1);
            Ni_c = FS[2];
            Ni_d = FS[3];
            
            net.push_back(Neuron (Ni_a, Ni_b, Ni_c, Ni_d, N, false, loc, &settings));
            ++Ni;
        }
        
        spikes.push_back(false);
        N = Ne + Ni;
        connection = false;
    };
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    if (leftMouseHold && clickedOnSynapse) {
        if(checkIDSynapse.box == 1)
            synapses[synapseVecID].addToDelay(ofGetPreviousMouseY()-y); // inverted because of negative y values
        
        else synapses[synapseVecID].addToWeight((ofGetPreviousMouseY()-y));
    }
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    leftMouseHold = false;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == OF_KEY_SHIFT) audioFrom.key = true;
    if(key == OF_KEY_ALT) alt = true;
    if(key == OF_KEY_LEFT_COMMAND || key == OF_KEY_RIGHT_COMMAND) cmd = true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key == OF_KEY_SHIFT) {audioFrom.key = false; connection = false; clickedOnInput = false; clickedOnOutput = false;}
    if(key == OF_KEY_ALT) {alt = false; clickedOnSynapse = false;}
    if(key == OF_KEY_LEFT_COMMAND || key == OF_KEY_RIGHT_COMMAND) {cmd = false;}
}

//--------------------------------------------------------------
void ofApp::getPresetButtonPressed() {
    clear();
    settings.loadFile(preset);

    if (settings.tagExists("Neurons")) {
        settings.pushTag("Neurons");
        for (int i=0; i<settings.getNumTags("Neuron"); ++i) {
            settings.pushTag("Neuron",i);
            
            ofVec2f loc (settings.getValue("x",0.0),settings.getValue("y",0.0));
            net.push_back(Neuron (settings.getValue("a",0.0),settings.getValue("b",0.0),
                                  settings.getValue("c",0.0), settings.getValue("d",0.0),
                                  settings.getValue("ID",0),settings.getValue("type",0), loc, &settings));
            
            spikes.push_back(false);
            if (settings.getValue("type",0)) Ne++;
            else Ni++;
            settings.popTag();
        }
        N = Ne + Ni;
        settings.popTag();
    }
    
    if (settings.tagExists("Synapses")) {
        settings.pushTag("Synapses");
        for (int i=0; i<settings.getNumTags("Synapse"); ++i) {
            settings.pushTag("Synapse",i);
            synapses.push_back(Synapse (settings.getValue("preSynID",0),settings.getValue("postSynID",0),
                                        settings.getValue("ExcInh",0), settings.getValue("ID",0),
                                        settings.getValue("weight", 0.0), settings.getValue("delayTime", 0),
                                        &net, &settings));
            settings.popTag();
            ++S;
        }
        settings.popTag();
    }
     
    if (settings.tagExists("Inputs")) {
        settings.pushTag("Inputs");
        for (int i=0; i<settings.getNumTags("audioIO"); ++i) {
            settings.pushTag("audioIO",i);
            for (int j=0; j<settings.getNumTags("target"); ++j)
                inputs[i].connect(settings.getValue("target",0, j));
    
            settings.popTag();
        }
        settings.popTag();
    }
    if (settings.tagExists("Outputs")) {
        settings.pushTag("Outputs");
        for (int i=0; i<settings.getNumTags("audioIO"); ++i) {
            settings.pushTag("audioIO",i);
            for (int j=0; j<settings.getNumTags("target"); ++j)
                outputs[i].connect(settings.getValue("target",0, j));
            
            settings.popTag();
        }
        settings.popTag();
    }
}

//--------------------------------------------------------------
void ofApp::setPresetButtonPressed() {
    settings.clear();
    for (int i=0; i<N; ++i)
        net[i].saveSettings();
    for (int i=0; i<S; ++i)
        synapses[i].saveSettings();
    for (int i=0; i<INPUTCHANNELS; ++i)
        inputs[i].saveSettings();
    for (int i=0; i<OUTPUTCHANNELS; ++i)
        outputs[i].saveSettings();
    
    settings.saveFile(preset);
}

//--------------------------------------------------------------
void ofApp::clearButtonPressed() {
    clear();
}

//--------------------------------------------------------------
void ofApp::ResetSynapseButtonPressed() {
    for (int i = 0; i < S; ++i)
        synapses[i].reset();
}

//--------------------------------------------------------------
void ofApp::create() {
    int cols = 10, rows = 5;
    int timeDiff = 5;
    
    for(int r=0; r<rows; ++r) {
        for (int c=0; c<cols; ++c) {
            Ne_a = RS[0];
            Ne_b = RS[1];
            Ne_c = RS[2];// + 15 * pow(ofRandom(1),2);
            Ne_d = RS[3];// - 6 * pow(ofRandom(1),2);
            
            ofVec2f loc( ofGetWidth()*0.75 * (c+1)/cols, ofGetHeight()*0.75 * (r+1)/rows );
            net.push_back(Neuron (Ne_a, Ne_b, Ne_c, Ne_d, Ne, true, loc, &settings));
            spikes.push_back(false);
            ++Ne; ++N;
        }
    }
    
    for(int r = 0; r<rows; ++r) {
        for(int i = r*cols; i < r*cols+cols; ++i){
            for(int j = r*cols; j<r*cols+cols; ++j) {
                if(i != j){
                    if ( i % cols == round(cols/2)-1) inputs[0].connect(i);
                     if ( i % cols == round(cols/2)) inputs[1].connect(i);
                        
                    float weight = 10.0;
                    if(i == j - 1 || i == j + cols-1) { // to next neuron
                        float delayTime = (5+r*timeDiff) * samplespms - neuronResponseTime;
                        synapses.push_back(Synapse(i, j, true, S, weight, delayTime, &net, &settings));
                        ++S;
                    }
                    if(i == j - 2 || i == j + cols-2) { // to neuron after next
                        float delayTime = (10+r*2*timeDiff) * samplespms - neuronResponseTime;
                        synapses.push_back(Synapse(i, j, true, S, weight, delayTime, &net, &settings));
                        ++S;
                    }
                    
                    if(i - 1 == j || i + cols - 1 == j) { // to previous neuron
                        float delayTime = (5+r*timeDiff) * samplespms - neuronResponseTime;
                        synapses.push_back(Synapse(i, j, true, S, weight, delayTime, &net, &settings));
                        ++S;
                    }
                    if(i - 2 == j || i + cols - 2 == j) { // to neuron after previous neuron
                        float delayTime = (10+r*2*timeDiff) * samplespms - neuronResponseTime;
                        synapses.push_back(Synapse(i, j, true, S, weight, delayTime, &net, &settings));
                        ++S;
                    }
                    
                    
                }
            }
        }
    }
}

