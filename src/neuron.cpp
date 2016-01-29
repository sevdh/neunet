#include "neuron.h"

Neuron::Neuron(float a, float b, float c, float d, int ID, bool type, ofVec2f loc, ofxXmlSettings* settings) {
    this->a = a;
    this->b = b;
    this->c = c;
    this->d = d;
    this->v = vrest;
    this->u = b*v;
    this->I = 0.0;
    this->ID = ID;
    this->loc = loc;
    this->type = type;
    this->settings = settings;
    scaleFactor = 1.0 / (70+vpeak);
    r = 20;
    
    pulseColorRG = 0;
    pulseColorB = 0;
}

void Neuron::setID(int ID) {
    this->ID = ID;
}

int Neuron::getID() {
    return ID;
}

void Neuron::resetV(){
    v = vrest;
}

// Receive input from synapse
void Neuron::receiveInput(float weight){
    this->I += weight * samplespms;
}

float Neuron::returnV() {
    //return clip(v,-70.0,vpeak);
    return (70+v)*scaleFactor;
}

void Neuron::display() {
    ofSetLineWidth(2);

    if (type) {
        if (spiked) {
            pulseColorRG = 140;
            pulseColorB = 40;
        }
        ofSetColor(240-pulseColorRG,240-pulseColorRG,240-pulseColorB);
        pulseColorRG *= 0.8;
        pulseColorB *= 0.8;
        ofFill();
        ofDrawCircle(loc, r);
        ofNoFill();
        ofSetColor(100,100,200);
        ofDrawCircle(loc, r);
    }
    if (!type) {
        if (spiked){
            pulseColorRG = 140;
            pulseColorB = 40;
        }
        ofSetColor(240-pulseColorB,240-pulseColorRG,240-pulseColorRG);
        pulseColorRG *= 0.8;
        pulseColorB *= 0.8;
        ofFill();
        ofDrawCircle(loc, r);
        ofNoFill();
        ofSetColor(200,100,100);
        ofDrawCircle(loc, r);
    }
    ofSetColor(0);
    ofDrawBitmapString(ofToString(ID+1), loc.x-4, loc.y+4);
    spiked = false;
}

int Neuron::checkClick(ofVec2f mouse) {
    float dist = mouse.distance(loc);
    if (dist < r) return ID;
    else return -1;
}

ofVec2f Neuron::getLoc() {
    return loc;
}

bool Neuron::getType() {
    return type;
}

void Neuron::saveSettings() {
    if (!settings->tagExists("Neurons")) settings->addTag("Neurons");

    settings->pushTag("Neurons");
        settings->addTag("Neuron");
        settings->pushTag("Neuron",ID);
            settings->addValue("a", a);
            settings->addValue("b", b);
            settings->addValue("c", c);
            settings->addValue("d", d);
            settings->addValue("ID", ID);
            settings->addValue("type", type);
            settings->addValue("x", loc.x);
            settings->addValue("y", loc.y);
            settings->popTag();
    settings->popTag();
}

