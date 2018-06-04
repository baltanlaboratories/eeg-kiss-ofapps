#pragma once
#include <vector>
class ImageExporter
{
public:
	static void exportVectorImages(
		std::vector<std::vector<std::vector<float>>> data
		,float fRadius
		,float fMinRadius
		,float fMagnification
		,std::vector<int> samplesToFade, std::vector<std::vector<int>> counters);

protected:
	static int m_activeThreads;
};

