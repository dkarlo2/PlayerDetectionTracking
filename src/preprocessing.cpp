#include "field_flood.h"
#include "globals.h"
#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "preprocessing.h"
#include "time_measurement.h"
#include "util.h"

static std::vector<cv::Point> mask2Points(cv::Mat mask) {
	std::vector<cv::Point> points;
	for (int i = 0; i < mask.rows; i++) {
		for (int j = 0; j < mask.cols; j++) {
			if (mask.at<uchar>(i, j) == 255) {
				points.push_back(cv::Point(j, i));
			}
		}
	}
	return points;
}

static void calculateFieldAreaAndBackground(FrameData &fd) {
	cv::Mat imageGray;
	cvtColor(fd.image, imageGray, CV_BGR2GRAY);

	// GaussianBlur(imageGray, imageGray, cv::Size(3, 3), 1);

	cv::Mat cannyOutput;
	Canny(imageGray, cannyOutput, fieldAreaCannyMin, fieldAreaCannyMax, fieldAreaCannyAper);

	dilateImage(cannyOutput, fieldAreaCannyDilate);

	cv::Mat floodMask = getFieldMaskFromFlood(cannyOutput);

#if DEBUG_PREPROCESSING
	imshow("testCannyOutput", cannyOutput);
	imshow("testFloodPoints", drawFloodPoints(fd.image.clone()));
	imshow("testFieldMask", floodMask);
#endif

	std::vector<cv::Point> fieldPoints = mask2Points(floodMask);

	convexHull(fieldPoints, fd.fieldHull);

	int nFieldHull = fd.fieldHull.size();
	fd.minY = fd.fieldHull[0].y;
	fd.maxY = fd.fieldHull[0].y;
	for (size_t i = 1; i < nFieldHull; i++) {
		fd.minY = std::min(fd.minY, fd.fieldHull[i].y);
		fd.maxY = std::max(fd.maxY, fd.fieldHull[i].y);
	}

	fd.fieldMask = cv::Mat::zeros(fd.image.size(), 0);

	myFillPoly(fd.fieldMask, fd.fieldHull, cv::Scalar(255));

#if DEBUG_PREPROCESSING
		cv::Mat im = fd.image.clone();
		cv::polylines(im, fd.fieldHull, true, cv::Scalar(0, 0, 255), 2);
		imshow("field hull", im);
		imshow("fieldMask", fd.fieldMask);
#endif

	cv::Mat maskedImage;
	fd.image.copyTo(maskedImage, fd.fieldMask);

	fd.fieldBackground.calculateBgColors(maskedImage);
}

void preprocess(FrameData &fd) {
	fd.image = fd.frame.clone();
	fd.factor = resizeImageArea(fd.image, imageCalculationPixelArea);

	if (!fd.fieldBackground.isCalculated()) {
		calculateFieldAreaAndBackground(fd);
	}

	MyTime mt;

	mt.start();

	fd.bgMask = fd.fieldBackground.extractBgMask(fd.image.clone());
	cv::Mat invFieldMask = 255 - fd.fieldMask;
	add(fd.bgMask, invFieldMask, fd.bgMask, invFieldMask);

#if DEBUG_PREPROCESSING
		imshow("bgMask", fd.bgMask);
		std::cerr << "Field background extracting: " << mt.time() << std::endl;
#endif

	fd.homography = cv::Mat();
	fd.detectedPlayers.clear();
}
