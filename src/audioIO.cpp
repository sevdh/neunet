#include "audioIO.h"


audioIO::audioIO(int ID, bool type, vector<Neuron> *net, ofxXmlSettings* settings){
    this->ID = ID;
    this->type = type;
    this->net = net;
    this->settings = settings;
    
    size = 30;
    vecSize = 0;
    
    if (type) {
        loc.x = ofGetWidth()-size;
        loc.y = (ofGetHeight()/OUTPUTCHANNELS) * (ID + 1) - (ofGetHeight()/(OUTPUTCHANNELS*2)) ;
    }
    if (!type) {
        loc.x = 0;
        loc.y = (ofGetHeight()/INPUTCHANNELS) * (ID + 1) - (ofGetHeight()/(INPUTCHANNELS*2)) ;
    }
}

audioIO::~audioIO(){
    targets.clear();
}

void audioIO::display(){
    ofSetColor(125);
    ofFill();
    ofSetLineWidth(1.5);
    ofDrawRectangle(loc, size, size);
    for (int i = 0; i < vecSize; ++i){
        ofDrawLine(loc+0.5*size, (*net)[targets[i]].getLoc());
    }
    ofSetColor(255);
    ofDrawBitmapString(ofToString(ID+1), loc.x+10, loc.y+20);
}

void audioIO::connect(int target) {
    bool exists = false;
    for (int i=0; i<vecSize; ++i) {
        if (targets[i] == target) {
            exists = true;
            break;
        }
    }
    if (!exists) {
        targets.push_back(target);
        ++vecSize;
    }
}

void audioIO::disconnect(int target) {
    for (int i=vecSize-1; i >= 0; --i) {
        if (targets[i] == target) {
            --vecSize;
            targets.erase(targets.begin()+i);
            break;
        }
    }
    for (int i = vecSize-1; i >=0; --i) if(targets[i] > target) targets[i]--;
}

int audioIO::checkClick(ofVec2f mouse){
    if ((mouse.x > loc.x) && (mouse.x < loc.x + size) && (mouse.y > loc.y) && (mouse.y < loc.y + size)) return ID;
    else return -1;
}

int audioIO::getVecSize(){
    return vecSize;
}

void audioIO::saveSettings() {
    if (!type) {
        if (!settings->tagExists("Inputs"))
            settings->addTag("Inputs");
        settings->pushTag("Inputs");
    }
    if (type) {
        if (!settings->tagExists("Outputs"))
            settings->addTag("Outputs");
        settings->pushTag("Outputs");
    }

    if (!settings->tagExists("audioIO",ID));
        settings->addTag("audioIO");
    
    settings->pushTag("audioIO",ID);
    for (int i=0; i<vecSize; ++i) {
        settings->addValue("target", targets[i]);
    }
    settings->popTag();
    settings->popTag();
}
