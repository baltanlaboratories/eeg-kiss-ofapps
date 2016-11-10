#include "ImageExporter.h"
#include <thread>
#include "ofCairoRenderer.h"

int ImageExporter::m_activeThreads = 0;

void ImageExporter::exportVectorImages(
	std::vector<std::vector<std::vector<float>>> data
	,float fRadius
	,float fMinRadius
	,float fMagnification, int samplesToFade, std::vector<std::vector<int>> counters
)
{
	std::cout << "Start: " << data.size() << std::endl;
	ofCairoRenderer renderer;

	std::string timestampString = ofGetTimestampString();

	for (int hs = 0; hs < data.size(); hs++)
	{
		std::cout << "headset: " << hs << std::endl;
		auto head = data.at(hs);

		// char iStr[4];
		// itoa(hs, iStr, 4);

		std::stringstream ss;
		ss << "screenshot-" << timestampString << "_" << hs << ".svg";
		std::string filename = ss.str();
		double SIZE = 4096;
		double l_ratio = 2.2;

		renderer.setup(filename, ofCairoRenderer::Type::SVG, false, false, ofRectangle(0, 0, SIZE, SIZE));

		renderer.background(0);

		double l_radius = fRadius * l_ratio;
		double l_minRadius = fMinRadius * l_ratio;
		double l_magnification = fMagnification * l_ratio;
		double l_centerX = SIZE / 2;
		double l_centerY = SIZE / 2;

		for (int channel = 0; channel < head.size(); channel++)
		{
			auto chan = head.at(channel);

			std::cout << "  channel: " << channel << ": " << chan.size() << std::endl;

			float fInnerRadius = l_minRadius + channel * l_radius * 1.25;

			for (int i = samplesToFade; i < chan.size() - 1; i++)
			{
				int alpha = (i - samplesToFade) * 255 / (chan.size() - samplesToFade);

				if (hs)
					renderer.setColor(ofColor::white, alpha);
				else
					renderer.setColor(ofColor::green, alpha);

				int sampleIndex = (i + counters[hs][channel] + chan.size()) % chan.size();
				float s1 = chan[sampleIndex];
				float s2 = chan[(sampleIndex + 1) % chan.size()];

				float amplitude1 = fInnerRadius + l_radius * s1;
				float amplitude2 = fInnerRadius + l_radius * s2;
				float x1, y1, x2, y2;

				float r1 = (((float)(sampleIndex) / chan.size())) * 2. * PI;
				float r2 = (((float)((sampleIndex + 1) % chan.size())) / chan.size()) * 2. * PI;

				x1 = l_centerX + amplitude1 * sin(r1) * l_magnification;
				y1 = l_centerY + amplitude1 * cos(r1) * l_magnification;
				x2 = l_centerX + amplitude2 * sin(r2) * l_magnification;
				y2 = l_centerY + amplitude2 * cos(r2) * l_magnification;

				renderer.drawLine(x1, y1, 0, x2, y2, 0);
			}
		}
		renderer.close();
	}
}
