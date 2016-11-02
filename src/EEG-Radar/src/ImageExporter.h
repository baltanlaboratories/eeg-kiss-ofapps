#pragma once
#include <vector>
class ImageExporter
{
public:
	static void exportImages(
		std::vector<std::vector<std::vector<float>>> data
		,float fRadius
		,float fMinRadius
		,float fMagnification
		,int samplesToFade, std::vector<std::vector<int>> counters);

protected:
	static int m_activeThreads;
};

