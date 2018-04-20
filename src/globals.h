/*
	Global variables and constants.
*/

#pragma once

#include "opencv2/core/core.hpp"

#define ESC 27
#define RETURN 13

#define HUE_MAX 180.
#define LUM_MAX 255.
#define SAT_MAX 255.

#define DEBUG_BACKGROUND_SUBTRACTION 1
#define DEBUG_FIELD_BACKGROUND 1
#define DEBUG_FIELD_TRACKING 1
#define DEBUG_PLAYER_DETECTION 1
#define DEBUG_PLAYER_TRACKING 1
#define DEBUG_PREPROCESSING 1
#define DEBUG_SPLIT_AND_MERGE_COLORS 1

const cv::Scalar white(255, 255, 255);

const std::string tmpVideoFile = "video.avi";
const std::string tmpTrackFile = "tracks.pt"; // player tracks
const std::string tmpHomographyFile = "homography.hg"; // homography

const std::string mainWindowName = "Football Players Tracking System";

const cv::Mat testHomography = (cv::Mat_<double>(3, 3) <<
	1.012672920543956, -0.6520642394904465, -3.419692393460303,
	-0.03969960801557436, 0.0138475476856274, 35.67548256067071,
	0.0001263327422817515, -0.003768113267358985, 1);

const int fieldModelScale = 3;

// ###### variables ######

extern bool showFieldModels;
extern bool showClusters;
extern bool showMatches;
extern bool showTracks;
extern bool finishTracking;

void processGlobalKeyInput(int key);
