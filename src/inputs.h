/*
System inputs, which are read from a config file. All the listed variables have to be set in the config file.
*/

#pragma once

#include "config_parsing.h"

extern std::string videoPath; // path to the input video file that is processed

extern std::string tmpDirectory; // path to the program's temporary directory

extern int imageCalculationPixelArea;	// pixel area to which the input frame is scaled when performing calculations
								// (because of the calculation speed)

extern double fieldWidth; // field width in meters
extern double fieldLength; // field length in meters

extern double fieldAreaCannyMin; // first threshold for the Canny edge detection that is used for finding field area
extern double fieldAreaCannyMax; // second threshold for the Canny edge detection that is used for finding field area
extern int fieldAreaCannyAper; // aperture size for the Sobel operator in the Canny edge detection that is used for finding field area
extern int fieldAreaCannyDilate; // amount of dilatation applied to the field edge mask

extern int floodPointsHor; // horizontal number of flood points
extern int floodPointsVer; // vertical number of flood points
extern int floodPointsMargin; // margin between two neighboring flood points
extern int fieldFloodDilate; // amount of dilatation applied to the field mask, which is computed by flooding an edge mask

extern double splitAndMergeSplitThresh; // Split threshold for Split and Merge algorithm used for background/foreground color separation.
extern double splitAndMergeMergeThresh; // Merge threshold for Split and Merge algorithm used for background/foreground color separation.

extern int bgCalcFramePeriod; // every bgCalcFramePeriod frames background (field) color is being recalculated

extern int mog2ShadowValue; // shadow value for BackgroundSubtractorMOG2
extern double mog2ShadowThreshold; // shadow threshold for BackgroundSubtractorMOG2

extern double fieldTrackScoreLineRes; // line resolution when scoring particle homography in field tracking
extern double fieldTrackScoreArcRes; // arc resolution when scoring particle homography in field tracking

extern int maxContourRectArea; // maximal area of a contour's bounding rectangle for which is considered to represent players (and not eg. the whole field)

extern double avgHeightMin; // average player height in the field point farthest from the camera in pixels
extern double avgHeightMax; // average player height in the field point nearest to the camera in pixels
extern double avgWidthMin; // average player width in the field point farthest from the camera in pixels
extern double avgWidthMax; // average player width in the field point nearest to the camera in pixels
extern double heightOffset; // maximal relative offset from the average height at each field point [0, 1]
extern double widthOffset; // maximal relative offset from the average width at each field point [0, 1]
extern double areaRatio; // minimal ratio between a player's contour area and the associated bounding rectangle area
extern double perspectivePower;	// the influence of the camera perspective on how equal lengths appear in different relative distances from the camera:
							//		at minimum relative distance d = 0	--> length l = minL
							//		at maximum relatve distance d = 1	--> length l = maxL
							//		at relative distance 0 < d < 1		--> length l = pow(d, perspectivePower) * (maxL - minL) + minL

extern int maxClusters; // maximal number of clusters that are big contours tried to be divided into

extern double matchDistancesMin; // maximal distance between a measurement and an active track in the field point farthest from the camera such that the corresponding edge is added to the matchings graph (in pixels)
extern double matchDistancesMax; // maximal distance between a measurement and an active track in the field point nearest to the camera such that the corresponding edge is added to the matchings graph (in pixels)

extern int initTrackFrames; // number of frames that need to pass before tracking starts (eg. because motion detector needs some time to initialize)
extern int framesToRetire; // number of frames that need to pass without some track being matched with any new measurement for it to be retired

extern int shortSubgraph; // maximal number of measurements for which some subgraph is considered to be short (and should be filtered out)
extern double staticSubgraph; // maximal standard deviation of measurements for which some subgraph is considered to be static (and should be filtered out)

extern double teamModelProb; // probability that a player's image is used for training the team model
extern int maxTeamModelSize; // maximal size of players' images used for training the model

extern std::string outputFile; // path to the file containing tracked players' paths

void loadInputs(char* configFilename);
