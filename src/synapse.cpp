#include "synapse.h"

Synapse::Synapse(int preSynID, int postSynID, bool ExcInh, int ID, float weight, int delayTime, vector<Neuron> *net, ofxXmlSettings* settings){
    this->ExcInh = ExcInh;
    this->preSynID = preSynID;
    this->postSynID = postSynID;
    this->ID = ID;
    this->net = net;
    this->settings = settings;
    this->delayTime = delayTime;
    this->weight = weight;
    
    if (ExcInh){
        weight = 7.0;
        delayTime = round((1 + (randomMax(D-1))) * samplespms);
    }
    else{
        delayTime = round((1 + (randomMax(D-1))) * samplespms);
        weight = -5.0;
    }
    
    reset();    // Init synapse
    resetDisplay();
}

Synapse::~Synapse(){
    delay.clear();
}

void Synapse::reset(){
    preFirstSpike = false;
    preCounter = 0;
    postFirstSpike = false;
    postCounter = std::numeric_limits<unsigned long>::max();
    preDelta = 0;
    postDelta = 0;
    deltaT = 0;
    WCR = 0.0;
    spiked = false;
    weightDerivative = 0.0;
    delVecSize = 0;
    delay.clear();
}

void Synapse::resetDelays(){
    preFirstSpike = false;
    preCounter = 0;
    postFirstSpike = false;
    postCounter = std::numeric_limits<unsigned long>::max();
    preDelta = 0;
    postDelta = 0;
    deltaT = 0;
    WCR = 0.0;
    spiked = false;
    weightDerivative = 0.0;
    delVecSize = 0;
    delay.clear();
    
    if (ExcInh){
        //weight = 7.0;
        delayTime = round((1 + (randomMax(D-1))) * samplespms);
    }
    else{
        delayTime = samplespms;
        //weight = -5.0;
    }
}

void Synapse::setWCR(float wcr){
    WCR = wcr;
}

void Synapse::preSpike(){
    spiked = true;
}


int Synapse::getTimePassed() {
    // Compare pre- & postsynaptic timing, return smallest
    if (preDelta <= postDelta) deltaT = preDelta;
    else deltaT = -postDelta;
    
    return deltaT;
}


void Synapse::addToDelay(int delay){
    delayTime = clip(delayTime+delay*10.0, samplespms, D*samplespms);
}

void Synapse::addToWeight(int weight){
    this->weight = clip(this->weight+weight, 0, sm);
}


void Synapse::display(){
    ofSetColor(0);
    ofSetLineWidth((weight+0.5) / (sm*0.5));
    ofNoFill();
    ofDrawBezier(preX, preY,
                 brezierPoint.x, brezierPoint.y,
                 brezierPoint.x, brezierPoint.y,
                 postX, postY);
    
    ofSetLineWidth(1);
    ofFill();
    ofDrawCircle(pointPosition,5);
    ofDrawBitmapString("d "+ofToString(round(100.0*(delayTime/samplespms))/100.0 ), box1.x, box1.y+9);
    ofDrawBitmapString("w "+ofToString(weight), box2.x, box2.y+9);
}

void Synapse::resetDisplay() {
    float curvature = 0.1;
    preX = (*net)[preSynID].getLoc().x;
    preY = (*net)[preSynID].getLoc().y;
    postX = (*net)[postSynID].getLoc().x;
    postY = (*net)[postSynID].getLoc().y;
    x_dist = (*net)[postSynID].getLoc().x - (*net)[preSynID].getLoc().x;
    y_dist = (*net)[postSynID].getLoc().y - (*net)[preSynID].getLoc().y;
    
    brezierPoint.set(preX*0.5 +postX*0.5 + y_dist * curvature,preY*0.5 + postY*0.5 - x_dist * curvature);
    
    pointPosition.set(preX+ x_dist*0.8, preY + y_dist*0.8);
    pointPosition+=ofVec2f(y_dist*curvature*0.4,-x_dist*curvature*0.4); // rotation
    //boxes for collision detection
    box1.set(pointPosition.x+8, pointPosition.y-5, 55, 10);
    box2.set(pointPosition.x+8, pointPosition.y-16, 40, 10);
}

synapticID Synapse::checkClick(ofVec2f mouse){
    synapticID ID;
    ID.preSynID = preSynID;
    ID.postSynID = postSynID;
    
    if ((mouse.x > box1.x) && (mouse.x < box1.x + box1.width) && (mouse.y > box1.y) && (mouse.y < box1.y + box1.height))
        ID.box = 1;
    else if ((mouse.x > box2.x) && (mouse.x < box2.x + box2.width) && (mouse.y > box2.y) && (mouse.y < box2.y + box2.height))
        ID.box = 2;
    
    else {
        ID.preSynID = -1;
        ID.postSynID = -1;
        ID.box = 0;
    }
    return ID;
}

void Synapse::saveSettings() {
    if (!settings->tagExists("Synapses")) settings->addTag("Synapses");
    
    settings->pushTag("Synapses");
        settings->addTag("Synapse");
        settings->pushTag("Synapse",ID);
        settings->addValue("preSynID", preSynID);
        settings->addValue("postSynID", postSynID);
        settings->addValue("ExcInh", ExcInh);
        settings->addValue("ID", ID);
        settings->addValue("weight", weight);
        settings->addValue("delayTime", int(delayTime));
        settings->popTag();
    settings->popTag();
}

bool Synapse::getExcInh(){
    return ExcInh;
}

int Synapse::getPreSynID(){
    return preSynID;
}

int Synapse::getPostSynID(){
    return postSynID;
}

void Synapse::setPreSynID(int ID){
    preSynID = ID;
    resetDisplay();
}

void Synapse::setPostSynID(int ID){
    postSynID = ID;
    resetDisplay();
}
