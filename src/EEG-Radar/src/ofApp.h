#pragma once

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
        SECOND = 1
    };
    
    enum DataTypes {
        IMEC_EEG_DATA,
        MUSE_EEG_DATA
    };
    
	struct EEGSettings {
		int nrOfHeadsets = 2;
		int nrOfChannels = 4;
		int nrOfSamples = 1024;

		string headsetPatternPrefix = "/EEG_";
		string channelPatternPrefix = "channel_";
		string markersPattern = "/markers";
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
	int         samplesToFade;  // number of samples to fadeout (these are skipped during draw())
	int         iFrameRate;     // max frame rate
	float       fMagnification; // magnification factor of the radar
	float       initialMagnification;
    
    float       fSignalScalingFactor;

	float getOscArg(const ofxOscMessage& m, int argIndex);
	void  addValueToChannelBuffer(DataTypes dataType, const string& pat, float value);
    void  addMultichannelSample(DataTypes dataType, EEGDevice device, float vch1, float vch2, float vch3, float vch4);
	void  saveScreenshot(ofImage image, string filename);
	void  printVectorImage();
    
    /// add sample to correct device-channel buffer
    void addSample(EEGDevice device, int ch, float val) {
        
        if(device > eegSettings.nrOfHeadsets) return;
        if(ch > eegSettings.nrOfChannels) return;
        
        iSampleCounters[device][ch] = (iSampleCounters[device][ch] + 1) % eegSettings.nrOfSamples;
        fSamples[device][ch][iSampleCounters[device][ch]] = val;
    }

	ofxOscReceiver	receiver;
	ofImage			screenImg;

	int iSampleCounters[2][4];

	int		iCounter;
	bool	bDemo;
	int		m_threadCounter;
};
