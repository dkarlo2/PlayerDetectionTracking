#ifndef FIELD_TRACKER_H
#define FIELD_TRACKER_H

#include "config_parsing.h"
#include "field_background.h"
#include "field_model.h"
#include "preprocessing.h"

#include <opencv2/core/core.hpp>
#include <vector>

class FTParticle {
public:
	cv::Mat homography;
	double weight;
	double accumulator;
	FTParticle(cv::Mat H, double W) : homography(H), weight(W) {}
	FTParticle clone() {
		return FTParticle(homography.clone(), weight);
	}
};

class FieldTracker {
	std::vector<FTParticle> particles;
	void initParticles(cv::Mat homography);
	cv::Mat getHomography();
	void predict();
	void correct(FieldModel &fm, cv::Mat distanceTransform);
public:
	void init(FrameData &fd, FieldModel &fm, cv::Mat testHomography = cv::Mat());
    void calculateHomography(FrameData &fd, FieldModel &fm);
};

#endif
