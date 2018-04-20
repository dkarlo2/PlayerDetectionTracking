#include "globals.h"
#include "point_mapping.h"
#include "util.h"
#include "window_manager.h"

#include <iostream>
#include <map>
#include "opencv2/imgproc/imgproc.hpp"

#undef min
#undef max

#define IMAGE_GAP 10
#define MODEL_POINT_RADIUS 5

class PointsMouseData {
public:
	std::vector<cv::Point> *fieldPoints;
	std::vector<cv::Point> *modelPoints;
	cv::Mat image;
	cv::Mat modelImage;
	int fieldPoint = -1;
	int modelPoint = -1;
	cv::Point mousePoint;
	std::map<int, int> pointBindings;
	cv::Mat drawImage;
	cv::Mat drawModelImage;
	int circleRadius;
	PointsMouseData(std::vector<cv::Point> *FP, std::vector<cv::Point> *MP, cv::Mat I, cv::Mat MI) :
		fieldPoints(FP), modelPoints(MP), image(I), modelImage(MI) {
		drawImage = image.clone();
		drawModelImage = modelImage.clone();
		circleRadius = std::min(image.cols, image.rows) / 50;
	};
};

static int findNearestPoint(int x, int y, std::vector<cv::Point> *points, int radius) {
	double minDist;
	int index = -1;
	for (size_t i = 0; i < points->size(); i++) {
		cv::Point point = points->operator[](i);
		double d = dist(point, cv::Point(x, y));
		if (d < radius && (index == -1 || d < minDist)) {
			minDist = d;
			index = i;
		}
	}
	return index;
}

static void pointsMouse(int event, int x, int y, int flags, void* data) {
	PointsMouseData *cmd = (PointsMouseData*)data;

	cmd->mousePoint = cv::Point(x, y);

	if (event == cv::EVENT_LBUTTONDBLCLK) {
		if (x < cmd->image.cols && y < cmd->image.rows) {
			cmd->fieldPoints->push_back(cv::Point(x, y));
			int index = cmd->fieldPoints->size() - 1;
			circle(cmd->drawImage, cv::Point(x, y), cmd->circleRadius, cv::Scalar(0, 0, 255), -1);
			if (cmd->modelPoint >= 0) {
				cmd->pointBindings[index] = cmd->modelPoint;
				cmd->fieldPoint = -1;
				cmd->modelPoint = -1;
			}
			else {
				cmd->fieldPoint = index;
			}
			return;
		}
		else if (x > cmd->image.cols + IMAGE_GAP && y < cmd->modelImage.rows) {
			x -= cmd->image.cols + IMAGE_GAP;
			cmd->modelPoints->push_back(cv::Point(x, y));
			int index = cmd->modelPoints->size() - 1;
			circle(cmd->drawModelImage, cv::Point(x, y), MODEL_POINT_RADIUS, cv::Scalar(0, 0, 255), -1);
			if (cmd->fieldPoint >= 0) {
				cmd->pointBindings[cmd->fieldPoint] = index;
				cmd->fieldPoint = -1;
				cmd->modelPoint = -1;
			}
			else {
				cmd->modelPoint = index;
			}
		}
	}

	bool pointPointed = false;
	if (x < cmd->image.cols) {
		int index = findNearestPoint(x, y, cmd->fieldPoints, cmd->circleRadius);
		if (index != -1) {
			pointPointed = true;
			circle(cmd->drawImage, cmd->fieldPoints->operator[](index), cmd->circleRadius, cv::Scalar(0, 0, 255), -1);
			if (event == cv::EVENT_LBUTTONDOWN) {
				if (cmd->modelPoint >= 0) {
					cmd->pointBindings[index] = cmd->modelPoint;
					cmd->fieldPoint = -1;
					cmd->modelPoint = -1;
				}
				else {
					cmd->fieldPoint = index;
				}
			}
		}
	}
	else if (x > cmd->image.cols + IMAGE_GAP) {
		x -= cmd->image.cols + IMAGE_GAP;
		int index = findNearestPoint(x, y, cmd->modelPoints, MODEL_POINT_RADIUS);
		if (index != -1) {
			pointPointed = true;
			circle(cmd->drawModelImage, cmd->modelPoints->operator[](index), MODEL_POINT_RADIUS, cv::Scalar(0, 0, 255), -1);
			if (event == cv::EVENT_LBUTTONDOWN) {
				if (cmd->fieldPoint >= 0) {
					cmd->pointBindings[cmd->fieldPoint] = index;
					cmd->fieldPoint = -1;
					cmd->modelPoint = -1;
				}
				else {
					cmd->modelPoint = index;
				}
			}
		}
	}

	if (event == cv::EVENT_LBUTTONDOWN && !pointPointed) {
		cmd->fieldPoint = -1;
	}
}

static cv::Mat concatenateImages(cv::Mat image1, cv::Mat image2) {
	cv::Mat concatenated = cv::Mat::zeros(std::max(image1.rows, image2.rows), image1.cols + image2.cols + IMAGE_GAP, CV_8UC3);

	image1.copyTo(concatenated(cv::Rect(0, 0, image1.cols, image1.rows)));
	image2.copyTo(concatenated(cv::Rect(image1.cols + IMAGE_GAP, 0, image2.cols, image2.rows)));

	return concatenated;
}

Mappings getPointMappings(cv::Mat image, cv::Mat modelImage) {
	WindowManager wm("Field mapping");

	std::vector<cv::Point> fieldPoints;
	std::vector<cv::Point> modelPoints;

	PointsMouseData cmd(&fieldPoints, &modelPoints, image, modelImage);
	wm.setMouseCallback(pointsMouse, &cmd);

	while (wm.isVisible()) {
		cv::Mat concat = concatenateImages(cmd.drawImage, cmd.drawModelImage);
		std::map<int, int>::iterator it;
		for (it = cmd.pointBindings.begin(); it != cmd.pointBindings.end(); it++) {
			cv::Point fp = fieldPoints[it->first];
			cv::Point mp = modelPoints[it->second];
			mp.x += image.cols + IMAGE_GAP;
			line(concat, fp, mp, cv::Scalar(255, 0, 0), 2);
		}
		if (cmd.fieldPoint >= 0) {
			cv::Point fp = fieldPoints[cmd.fieldPoint];
			circle(concat, fp, cmd.circleRadius + 3, cv::Scalar(255, 0, 0), -1);
			line(concat, fp, cmd.mousePoint, cv::Scalar(0, 255, 255), 2);
		}
		if (cmd.modelPoint >= 0) {
			cv::Point mp = modelPoints[cmd.modelPoint];
			mp.x += image.cols + IMAGE_GAP;
			circle(concat, mp, MODEL_POINT_RADIUS + 3, cv::Scalar(255, 0, 0), -1);
			line(concat, mp, cmd.mousePoint, cv::Scalar(0, 255, 255), 2);
		}
		wm.showImage(concat, cv::Size(1000, 600));
		switch (cv::waitKey(1)) {
		case RETURN:
			if (cmd.pointBindings.size() < 4) {
				std::cout << "Too few point bindings set. There have to be at least 4." << std::endl;
			}
			else {
				wm.close();
				std::vector<cv::Point2f> srcPoints, dstPoints;
				std::map<int, int>::iterator it;
				for (it = cmd.pointBindings.begin(); it != cmd.pointBindings.end(); it++) {
					srcPoints.push_back(fieldPoints[it->first]);
					dstPoints.push_back(modelPoints[it->second]);
				}
				return std::make_pair(srcPoints, dstPoints);
			}
		}
	}
	wm.close();
}

