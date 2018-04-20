#pragma once

#include "inputs.h"
#include "preprocessing.h"

#include <opencv2/video.hpp>

class BackgroundEliminator {
	cv::Ptr<cv::BackgroundSubtractorMOG2> pMog2;
public:
	BackgroundEliminator() {
		pMog2 = cv::createBackgroundSubtractorMOG2();
		pMog2->setShadowValue(mog2ShadowValue);
		pMog2->setShadowThreshold(mog2ShadowThreshold);
	}
	void detectForeground(FrameData &fd);
};
