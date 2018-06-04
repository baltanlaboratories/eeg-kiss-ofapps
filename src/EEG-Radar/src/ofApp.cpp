
#include "ofApp.h"
#include <math.h>
#include <assert.h>
#include "ImageExporter.h"

#define _DEBUG

const int kOscReceivePort = 7110;

//--------------------------------------------------------------
void ofApp::setup() {
	bSmooth = true;
	bInfoText = true;
	iSampleCounter = 0;
	iFrameRate = appSettings.framerate;
	width = ofGetWidth();
	height = ofGetHeight();
    
    fSignalScalingFactor = 1.0;

	ofBackground(0, 0, 0);
	ofSetCircleResolution(appSettings.circleResolution);
	ofSetWindowTitle(appSettings.windowTitle);
	ofSetVerticalSync(false);

	ofSetFrameRate(iFrameRate); // if vertical sync is off, we can go a bit fast... this caps the framerate

	ofEnableAntiAliasing();

	windowResized(width, height);

	// Adjust fMagnification to scale the entire radar image
	fMagnification = appSettings.magnification;

	// Initialize an multi-dimensional array to hold the EEG samples
	iSampleCounters = new int*[eegSettings.nrOfHeadsets];
	fSamples = new float**[eegSettings.nrOfHeadsets];

	for (int i = 0; i < eegSettings.nrOfHeadsets; i++) {
		int fraction = i < eegSettings.nrOfKissers ? 0 : i - eegSettings.nrOfKissers + 1;
		samplesToFade.push_back(eegSettings.nrOfSamples / (eegSettings.nrOfHeadsets - eegSettings.nrOfKissers + 1));
		hsHasData.push_back(false);
		iSampleCounters[i] = new int[eegSettings.nrOfChannels]();
		fSamples[i] = new float*[eegSettings.nrOfChannels];
		for (int j = 0; j < eegSettings.nrOfChannels; j++) {
			fSamples[i][j] = new float[eegSettings.nrOfSamples]();

			iSampleCounters[i][j] = eegSettings.nrOfSamples / (eegSettings.nrOfHeadsets - eegSettings.nrOfKissers + 1) * fraction;
		}
	}


	// Listen on the given port
	cout << "Listening for osc messages on port " << kOscReceivePort << endl;
	receiver.setup(kOscReceivePort);

	bDemo = 0;
}

//--------------------------------------------------------------
void ofApp::exit(ofEventArgs &args) {
	int i, j;
	for (i = 0; i < eegSettings.nrOfHeadsets; i++) {
		for (j = 0; j < eegSettings.nrOfChannels; j++) {
			delete[] fSamples[i][j];
			fSamples[i][j] = NULL;
		}
		delete[] fSamples[i];
		fSamples[i] = NULL;
		delete[] iSampleCounters[i];
	}
	delete[] fSamples;
	delete[] iSampleCounters;
}

//--------------------------------------------------------------
void ofApp::update()
{
	//    iSampleCounter = (iSampleCounter + 1) % eegSettings.nrOfSamples;

	// Check for plotting full circle (sample-buffer size) from the moment the marker was plotted
	//    if (bStartKiss)
	//    {
	//        if (++iStartKissCount >= eegSettings.nrOfSamples)
	//        {
	//            bStartKiss = false;
	//        }
	//    }
	//    if (bStopKiss)
	//    {
	//        if (++iStopKissCount >= eegSettings.nrOfSamples)
	//        {
	//            bStopKiss = false;
	//        }
	//    }

	// check for waiting messages
	while (receiver.hasWaitingMessages()) {
		// get the next message
		ofxOscMessage m;
		receiver.getNextMessage(&m);

		parseOscMessage(m); 
	}

	//    if (bKissing)
	//        ofBackground(ofColor::red);
	//    else
	//        ofBackground(ofColor::black);
	
	if (bDemo) {
		for (int i = 0; i < 5; i++) {
			iCounter++;
			addSample(FIRST, 0, 0.3 * std::sin(iCounter / 10.));
			addSample(FIRST, 1, 0.3 * std::sin(iCounter / 12.));
			addSample(FIRST, 2, 0.3 * std::sin(iCounter / 13.));
			addSample(FIRST, 3, 0.3 * std::sin(iCounter / 14.));

			addSample(SECOND, 0, 0.3 * std::sin(iCounter / 15.));
			addSample(SECOND, 1, 0.3 * std::sin(iCounter / 13.));
			addSample(SECOND, 2, 0.3 * std::sin(iCounter / 11.));
			addSample(SECOND, 3, 0.3 * std::sin(iCounter /  9.));
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw()
{
	std::string timestampString;
	for (int hs = 0; hs < eegSettings.nrOfHeadsets; hs++)
	{
		if (!hsHasData[hs]) {
			continue;
		}
		int samplesToFade = this->samplesToFade[hs];
		for (int channel = 0; channel < eegSettings.nrOfChannels; channel++)
		{
			float fInnerRadius = fMinRadius + channel * fRadius * 1.25;

			for (int i = samplesToFade; i < eegSettings.nrOfSamples; i++)
			{
				int alpha = (i - samplesToFade) * 255 / (eegSettings.nrOfSamples - samplesToFade);

				ofColor color = eegSettings.colors[hs];
				ofSetColor(color, alpha);

				int sampleIndex = (i + iSampleCounters[hs][channel] + eegSettings.nrOfSamples) % eegSettings.nrOfSamples;
				float s1 = fSamples[hs][channel][sampleIndex];
				float s2 = fSamples[hs][channel][(sampleIndex + 1) % eegSettings.nrOfSamples];
                
                s1 *= fSignalScalingFactor;
                s2 *= fSignalScalingFactor;

				float amplitude1 = fInnerRadius + fRadius * s1;
				float amplitude2 = fInnerRadius + fRadius * s2;
				float x1, y1, x2, y2;

				float r1 = (((float)(sampleIndex) / eegSettings.nrOfSamples)) * 2. * PI;
				float r2 = (((float)((sampleIndex + 1) % eegSettings.nrOfSamples)) / eegSettings.nrOfSamples) * 2. * PI;

				x1 = width / 2 + amplitude1 * sin(r1) * fMagnification;
				y1 = height / 2 + amplitude1 * cos(r1) * fMagnification;
				x2 = width / 2 + amplitude2 * sin(r2) * fMagnification;
				y2 = height / 2 + amplitude2 * cos(r2) * fMagnification;

				ofLine(x1, y1, x2, y2);

				if (!hs && (channel == 3))
				{
					if (bStartKiss && (sampleIndex == iStartKissIndex))
					{
						x1 = width / 2;
						y1 = height / 2;
						x2 = x1 + amplitude1 * sin(r1) * fMagnification * 1.5;
						y2 = y1 + amplitude1 * cos(r1) * fMagnification * 1.5;

						ofSetColor(ofColor::yellow, alpha);
						ofSetLineWidth(2.0);
						ofLine(x1, y1, x2, y2);
						ofSetLineWidth(1.0);
					}
					if (bStopKiss && (sampleIndex == iStopKissIndex))
					{
						x1 = width / 2;
						y1 = height / 2;
						x2 = x1 + amplitude1 * sin(r1) * fMagnification * 1.5;
						y2 = y1 + amplitude1 * cos(r1) * fMagnification * 1.5;

						ofSetColor(ofColor::cyan, alpha);
						ofSetLineWidth(2.0);
						ofLine(x1, y1, x2, y2);
						ofSetLineWidth(1.0);
					}
				}
			}
		}
	}

	if (bStartKiss && (iCaptureStartKissCount > 0))
	{
		if (++iCaptureStartKissCount > 16)
		{
			screenImg.grabScreen(0, 0, width, height);
			saveScreenshot(screenImg, "startkiss.png");
			iCaptureStartKissCount = 0;
		}
	}
	if (bStopKiss && (iCaptureStopKissCount > 0))
	{
		if (++iCaptureStopKissCount > 16)
		{
			screenImg.grabScreen(0, 0, width, height);
			saveScreenshot(screenImg, "stopkiss.png");
			iCaptureStopKissCount = 0;
		}
	}

	// Draw UI
	if (bInfoText) {
		stringstream ss;
		ss << "Framerate: " << ofToString(ofGetFrameRate(), 0)
			<< " Fade kissers: " << samplesToFade[0]
			<< " Fade others: " << (eegSettings.nrOfHeadsets >=2 ? samplesToFade[2] : 0)
			<< " Magnification: " << ofToString(fMagnification, 2)
            << " Signal scaling: " << ofToString(fSignalScalingFactor, 2) << endl;
		ss << "(i) toggle this menu" << endl;
		ss << "(f) toggle full screen" << endl;
		ss << "(q) increase fade for kissers" << endl;
		ss << "(a) decrease fade fpr kissers" << endl;
		ss << "(t) increase fade for others" << endl;
		ss << "(g) decrease fade fpr others" << endl;
		ss << "(w) increase max framerate" << endl;
		ss << "(s) decrease max framerate" << endl;
		ss << "(e) increase magnification" << endl;
		ss << "(d) decrease magnification" << endl;
		ss << "(r) reset magnification" << endl;
		ss << "(c) capture screenshot" << endl;
		ss << "(v) export separate images" << endl;
        ss << "([) decrease signal scaling factor" << endl;
        ss << "(]) increase signal scaling factor" << endl;
#ifdef _DEBUG
		ss << "(p) play/stop demo" << endl;
#endif

		ofSetColor(ofColor::white);
		ofDrawBitmapString(ss.str().c_str(), 20, 20);
	}
}

/***************************************************************
* Read an argument from a received ofxOscMessage as float.
**************************************************************/
float ofApp::getOscArg(const ofxOscMessage& m, int argIndex)
{
	float value = 0.0;
	switch (m.getArgType(argIndex))
	{
	case OFXOSC_TYPE_INT32:
		//cout << "int32" << endl;
		value = m.getArgAsInt32(argIndex);
		break;
	case OFXOSC_TYPE_INT64:
		//cout << "int64" << endl;
		value = m.getArgAsInt64(argIndex);
		break;
	case OFXOSC_TYPE_FLOAT:
		//cout << "float" << endl;
		value = m.getArgAsFloat(argIndex);
		break;
	default:
		//cout << "unsupported type: " << m.getArgType(argIndex) << endl;
		break;
	}

	return value;
}

/***************************************************************
* Checks if str start with startStr
**************************************************************/
bool startsWith(const string& str, const string& startStr)
{
	return (str.compare(0, startStr.length(), startStr) == 0);
}

void addToPolyline(float value, ofPolyline& pl, int yOffset)
{
	if (pl.size() >= ofGetWindowWidth())
	{
		pl.clear();
	}
	float yScale = ofGetWindowHeight() / 4; // set scale to 1/4th of window height
	pl.lineTo(pl.size(), yOffset + value * yScale);
}

/***************************************************************
* Parses the incoming osc message and adds values to appropriate buffers
**************************************************************/
void ofApp::parseOscMessage(ofxOscMessage m)
{

	// get the current pattern
	string pattern = m.getAddress();
	float value = getOscArg(m, 0);
	if (startsWith(pattern, "/EEG_")) {
		// get the value of the pattern
		float value = getOscArg(m, 0);

		EEGDevice headsetId = NONE;
		if (startsWith(pattern, "/EEG_0/"))
		{
			headsetId = FIRST;
		}
		else if (startsWith(pattern, "/EEG_1/"))
		{
			headsetId = SECOND;
		}
		else if (startsWith(pattern, "/EEG_2/"))
		{
			headsetId = THIRD;
		}
		else if (startsWith(pattern, "/EEG_3/"))
		{
			headsetId = FOURTH;
		}
		else if (startsWith(pattern, "/EEG_4/"))
		{
			headsetId = FIFTH;
		} 
		else if (startsWith(pattern, "/EEG_5/"))
		{
			headsetId = SIXTH;
		}
	
		// check if a valid headset was found
		if (headsetId != NONE && headsetId <= eegSettings.nrOfHeadsets)
		{
			// now determine the channel
			string channel = pattern.substr(strlen("/EEG_0/"));
			int chanId = -1;

			if (startsWith(channel, eegSettings.channelPatternPrefix))
			{
				// read channel id using string stream
				istringstream(channel.substr(eegSettings.channelPatternPrefix.length(), 1)) >> chanId;
				chanId--; // decrement to get index
			}

			if (chanId >= 0 && chanId <= eegSettings.nrOfChannels)
			{
				// Add the sample to the buffer
				addSample(headsetId, chanId, value);

				if (!headsetId && (chanId == 3))
				{
					if (bStartKiss)
					{
						if (++iStartKissCount >= eegSettings.nrOfSamples)
						{
							bStartKiss = false;
						}
					}
					if (bStopKiss)
					{
						if (++iStopKissCount >= eegSettings.nrOfSamples)
						{
							bStopKiss = false;
						}
					}
				}
				// TODO: return true -> then break from while-loop
				//                    if (!(iSampleCounters[0][0] % eegSettings.nrOfChannels))
				//                    {
				//                        break;
				//                    }
			}
			else
			{
				cout << "got bad channel for :" << pattern << " | " << channel.substr(eegSettings.channelPatternPrefix.length(), 1) << endl;
			}
		}
	}
	else if (startsWith(pattern, "/muse")) {

		// seems like values coming from MUSE are 10-bit (lick finger, stick finger in air, guess scaling factor)
		float scale = 1 << 10; // 1024

		// get the value of the pattern
		float v0 = getOscArg(m, 0) / scale;
		float v1 = getOscArg(m, 1) / scale;
		float v2 = getOscArg(m, 2) / scale;
		float v3 = getOscArg(m, 3) / scale;

		EEGDevice device = NONE;
		if (pattern.find("/muse/1/eeg") != string::npos) {
			device = FIRST;
		}
		else if (pattern.find("/muse/2/eeg") != string::npos) {
			device = SECOND;
		}
		else if (pattern.find("/muse/3/eeg") != string::npos) {
			device = THIRD;
		}
		else if (pattern.find("/muse/4/eeg") != string::npos) {
			device = FOURTH;
		}
		else if (pattern.find("/muse/5/eeg") != string::npos) {
			device = FIFTH;
		}
		else if (pattern.find("/muse/6/eeg") != string::npos) {
			device = SIXTH;
		}
		else {
			// unknonwn message, we print the address for debugging purposes
			string msg_string;
			msg_string = pattern;
			msg_string += ": ";
			for (int i = 0; i < m.getNumArgs(); i++) {
				// get the argument type
				msg_string += m.getArgTypeName(i);
				msg_string += ":";
				// display the argument - make sure we get the right type
				if (m.getArgType(i) == OFXOSC_TYPE_INT32) {
					msg_string += ofToString(m.getArgAsInt32(i));
				}
				else if (m.getArgType(i) == OFXOSC_TYPE_FLOAT) {
					msg_string += ofToString(m.getArgAsFloat(i));
				}
				else if (m.getArgType(i) == OFXOSC_TYPE_STRING) {
					msg_string += m.getArgAsString(i);
				}
				else {
					msg_string += "unknown";
				}
			}

			ofLogNotice() << "unknown OSC >> " << msg_string;
		}

		if (device != NONE) {
			addSample(device, 0, v0);
			addSample(device, 1, v1);
			addSample(device, 2, v2);
			addSample(device, 3, v3);
		}
	}
	else if (startsWith(pattern, eegSettings.markersPattern))
	{
		//                bKissing = bool(value); was used for changing background-color

		// Stop-kiss value = 0/4096 = 0.0
		// Start-kiss value = 1/4096 = 0.000244
		// Point-of-interest value = 2/4096 = 0.000488
		if ((value > 0.0) && (value < 0.0003))
		{
			if (!bStartKiss)
			{
				bStartKiss = true;
				iStartKissIndex = iSampleCounters[0][3];
				iStartKissCount = 0;
				iCaptureStartKissCount++;
			}
		}
		else if (value == 0.0)
		{
			if (!bStopKiss)
			{
				bStopKiss = true;
				iStopKissIndex = iSampleCounters[0][3];
				iStopKissCount = 0;
				iCaptureStopKissCount++;
			}
		}
	}
}

void ofApp::addSample(EEGDevice device, int ch, float val) {
	if (device < 0 || device >= eegSettings.nrOfHeadsets) return;
	if (ch < 0 || ch >= eegSettings.nrOfChannels) return;
	hsHasData[device] = true;


	const float MAX_AMPLITUDE = eegSettings.maxAmplitude;
	val = MIN(MAX(val, -MAX_AMPLITUDE), MAX_AMPLITUDE);

	iSampleCounters[device][ch] = (iSampleCounters[device][ch] + 1) % eegSettings.nrOfSamples;
	fSamples[device][ch][iSampleCounters[device][ch]] = val;
}

void ofApp::saveScreenshot(ofImage image, string filename)
{
	ofFile file;
	ofDirectory dir;
	string timestamp;
	string path = "";

	if (file.doesFileExist("..\\Source\\subfolder.txt", false))
	{
		file.open("..\\..\\Source\\subfolder.txt");
		ofBuffer buff = file.readToBuffer();
		file.close();
		//cout << buff << endl;
		path = "..\\..\\Source\\records\\";
		if (!dir.doesDirectoryExist(path, true))
		{
			dir.createDirectory(path, true);
			//cout << path << endl;
		}
		path += buff;
		if (!dir.doesDirectoryExist(path, true))
		{
			dir.createDirectory(path, true);
			//cout << path << endl;
		}
		path += "\\";
	}
	path += ofGetTimestampString("%y%m%d_%H%M%S_");
	path += filename;
	//cout << path << endl;

	image.saveImage(path);
}

void ofApp::printVectorImage()
{
	std::thread thread([this] {

		std::cout << eegSettings.nrOfHeadsets << " " << eegSettings.nrOfChannels << " " << eegSettings.nrOfSamples << std::endl;
		std::vector<std::vector<std::vector<float>>> data;
		for (int headset = 0; headset < eegSettings.nrOfHeadsets; headset++) {
			std::vector<std::vector<float>> head;
			for (int channel = 0; channel < eegSettings.nrOfChannels; channel++) {
				head.push_back(std::vector<float>(fSamples[headset][channel], fSamples[headset][channel] + eegSettings.nrOfSamples));
			}
			data.push_back(head);
		}

		std::vector<std::vector<int>> counters;
		for (int i = 0; i < eegSettings.nrOfHeadsets; i++) {
			counters.push_back(std::vector<int>(iSampleCounters[i], iSampleCounters[i] + eegSettings.nrOfChannels));
		}

		m_threadCounter++;
		ImageExporter::exportVectorImages(data, fRadius, fMinRadius, fMagnification, samplesToFade, counters);
		m_threadCounter--;
	});
	thread.detach();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key)
{
	switch (key)
	{
	case 'f':
		ofToggleFullscreen();
		break;
	case 'i':
		// toggle info on screen
		bInfoText = !bInfoText;
		break;
	case 'q': {
		// increase fade
		for (int hs = 0; hs < 2 && hs < eegSettings.nrOfHeadsets; hs++) {
			if (samplesToFade[hs] <= eegSettings.nrOfSamples - 10) samplesToFade[hs] += 10;
		}
		break;
	}
	case 'a': {
		// decrease fade
		for (int hs = 0; hs < 2 && hs < eegSettings.nrOfHeadsets; hs++) {
			if (samplesToFade[hs] >= 10) samplesToFade[hs] -= 10;
		}
		break;
	}
	case 't': {
		// increase fade
		for (int hs = 2; hs < eegSettings.nrOfHeadsets; hs++) {
			if (samplesToFade[hs] <= eegSettings.nrOfSamples - 10) samplesToFade[hs] += 10;
		}
		break;
	}
	case 'g': {
		// decrease fade
		for (int hs = 2; hs < eegSettings.nrOfHeadsets; hs++) {
			if (samplesToFade[hs] >= 10) samplesToFade[hs] -= 10;
		}
		break;
	}
	case 'w':
		// increase framerate
		iFrameRate++;
		ofSetFrameRate(iFrameRate);
		break;
	case 's':
		// decrease framerate
		iFrameRate--;
		ofSetFrameRate(iFrameRate);
		break;
	case 'e':
		// increase Magnification
		fMagnification += 0.01;
		break;
	case 'd':
		// decrease Magnification
		if (fMagnification>0.01) fMagnification -= 0.01;
		break;
	case 'r':
		// reset Magnification
		fMagnification = appSettings.magnification;
		break;
	case 'c':
		// capture screenshot
		screenImg.grabScreen(0, 0, width, height);
		saveScreenshot(screenImg, "screenshot.png");
		break;
    case '[':
        fSignalScalingFactor -= 1.0/100.0;
        break;
    case ']':
        fSignalScalingFactor += 1.0/100.0;
        break;
	case 'v':
		printVectorImage();
		break;
#ifdef _DEBUG
	case 'p':
		//TODO: reset buffers
		bDemo = !bDemo;
		break;
#endif
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {
	width = w;
	height = h;

	float minVal = min(width, height);

	fMinRadius = minVal * .15;
	fRadius = ((minVal * .25) / (float)(eegSettings.nrOfChannels - 1)) / 1.25;
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}
