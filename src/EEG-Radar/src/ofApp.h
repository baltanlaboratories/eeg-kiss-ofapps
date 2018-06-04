#pragma once
#include "ofMain.h"
#include "ofxOsc.h"


class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();
	void exit(ofEventArgs &args);

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

private:
	struct ApplicationSettings {
		string windowTitle = "EEG Radar v0.1";
		int magnification = 1.0;
		int framerate = 60;
		int circleResolution = 50;
	};

    // abstraction for EEG devices within the installation
	enum EEGDevice {
		NONE = -1,
		FIRST = 0,
		SECOND = 1,
		THIRD = 2,
		FOURTH = 3,
		FIFTH = 4,
		SIXTH = 5
    };
    
	struct EEGSettings {
		int nrOfHeadsets = 6;
		int nrOfKissers = 2; //2 out of the 5 are kissers.
		int nrOfChannels = 4;
		int nrOfSamples = 2560;

		string headsetPatternPrefix = "/EEG_";
		string channelPatternPrefix = "channel_";
		string markersPattern = "/markers";
		std::vector<ofColor> colors = {ofColor::green, ofColor::white, ofColor(198,43,229), ofColor::orangeRed, ofColor::red, ofColor::purple};
		float maxAmplitude = 0.25; // maximum amplitude
	};

	ApplicationSettings appSettings;
	EEGSettings         eegSettings;

	int         width, height;
	bool	    bSmooth;
	int         iSampleCounter;
	float***    fSamples;
	float       fRadius;
	float       fMinRadius;
	//		bool        bKissing;       // True between the kissing markers
	bool        bStartKiss;
	int         iStartKissIndex;
	int         iStartKissCount;
	int         iCaptureStartKissCount;
	bool        bStopKiss;
	int         iStopKissIndex;
	int         iStopKissCount;
	int         iCaptureStopKissCount;
	bool        bInfoText;      // display text UI or not
	std::vector<int>         samplesToFade;  // number of samples to fadeout (these are skipped during draw())
	int         iFrameRate;     // max frame rate
	float       fMagnification; // magnification factor of the radar
	float       initialMagnification;
    
    float       fSignalScalingFactor;

	float getOscArg(const ofxOscMessage& m, int argIndex);
	void  parseOscMessage(ofxOscMessage m);
	void  saveScreenshot(ofImage image, string filename);
	void  printVectorImage(bool fade);
	void  addSample(EEGDevice device, int ch, float val);

	ofxOscReceiver	receiver;
	ofImage			screenImg;

	int** iSampleCounters;
	std::vector<bool> hsHasData;

	int		iCounter;
	bool	bDemo;
	int		m_threadCounter;
};
