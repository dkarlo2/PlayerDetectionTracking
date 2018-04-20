#pragma once

#include "field_background.h"
#include "team_modeling.h"

class FrameData {
public:
	cv::Mat frame; // original frame

	cv::Mat image; // scaled frame
	double factor; // frame scale factor (for reducing computation time)

	cv::Mat fieldMask; // field mask (field area)
	std::vector<cv::Point> fieldHull; // convex hull of the field mask
	int minY, maxY; // field area y-coordinate range

	FieldBackground fieldBackground; // field background model

	cv::Mat bgMask; // background mask (combined information about field colors and field area)

	cv::Mat foreground; // foreground mask after motion detection

	cv::Mat homography; // frame's homography

	std::vector<std::pair<cv::Rect, std::vector<float>>> detectedPlayers; // pairs of player bounding rectangle and player color features

	TeamModel teamModel;

	int numOfSubgraphs = 0;

	void addDetectedPlayer(cv::Rect rect, std::vector<float> features) {
		detectedPlayers.push_back(std::make_pair(rect, features));
	}
};