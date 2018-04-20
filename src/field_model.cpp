#include "field_model.h"

#include "globals.h"
#include "inputs.h"
#include "time_measurement.h"
#include "util.h"

#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define MARGIN 10

static const cv::Scalar W(255);

static const double goalAreaWidth = 18.32;
static const double goalAreaLength = 5.5;

static const double penaltyAreaWidth = 40.32;
static const double penaltyAreaLength = 16.5;

static const double penaltyMarkLength = 11;

static const double radius = 9.15;

static const double penaltyAngle = atan2(penaltyAreaLength - penaltyMarkLength, radius) + 0.1; // TODO otkud ovo +0.1

static cv::Point2d applyHomography(cv::Point2d point, cv::Mat homography) {
	double p1 = homography.at<double>(0, 0) * point.x + homography.at<double>(0, 1) * point.y + homography.at<double>(0, 2);
	double p2 = homography.at<double>(1, 0) * point.x + homography.at<double>(1, 1) * point.y + homography.at<double>(1, 2);
	double p3 = homography.at<double>(2, 0) * point.x + homography.at<double>(2, 1) * point.y + homography.at<double>(2, 2);
	double px = p1 / p3;
	double py = p2 / p3;
	return cv::Point2d(px, py);
}

cv::Mat FieldModel::getImage(int t, cv::Mat homography, cv::Size size) {
	if (t == -1) {
		t = thickness;
	}

	cv::Mat image;
	if (size.area() == 0) {
		image = cv::Mat::zeros(imageSize.height, imageSize.width, CV_8UC3);
	} else {
		image = cv::Mat::zeros(size, CV_8UC3);
	}

	for (int i = 0; i < lines.size(); i++) {
		cv::Point2d s = applyHomography(lines[i].start, homography);
		cv::Point2d e = applyHomography(lines[i].end, homography);
		cv::line(image, s, e, white, t);
	}

	for (int i = 0; i < arcs.size(); i++) {
		cv::Point c = applyHomography(arcs[i].center, homography);
		for (double a = arcs[i].startAngle; a <= arcs[i].endAngle; a += 0.02) {
			double dx = cos(a) * arcs[i].radius;
			double dy = sin(a) * arcs[i].radius;
			cv::Point p = applyHomography(cv::Point(arcs[i].center.x + dx, arcs[i].center.y + dy), homography);
			int x = p.x;
			int y = p.y;
			cv::circle(image, p, round(t / 2), white, -1);
		}
	}

	return image;
}

double FieldModel::calculateScore(cv::Mat imageDT, cv::Mat homography, double lineResolution, double arcResolution) {
	cv::Mat drawing;
	if (showFieldModels) cv::cvtColor(imageDT, drawing, CV_GRAY2BGR);

	double mean = cv::mean(imageDT)[0];

	double score = 0;
	// int cnt = 0;
	for (int i = 0; i < lines.size(); i++) {
		cv::Point2d s = applyHomography(lines[i].start, homography);
		cv::Point2d e = applyHomography(lines[i].end, homography);
		if (showFieldModels) cv::line(drawing, s, e, cv::Scalar(0, 0, 255), thickness);
		double n = lines[i].norm();
		for (double d = 0; d <= n; d += lineResolution) {
			int x = s.x + d / n * (e.x - s.x);
			int y = s.y + d / n * (e.y - s.y);
			if (y < 0 || y >= imageDT.rows || x < 0 || x >= imageDT.cols) {
				score += mean;
				// continue;
			} else {
				score += imageDT.at<float>(y, x);
			}
			// cnt++;
		}
	}

	for (int i = 0; i < arcs.size(); i++) {
		cv::Point c = applyHomography(arcs[i].center, homography);
		for (double a = arcs[i].startAngle; a <= arcs[i].endAngle; a+= arcResolution) {
			double dx = cos(a) * arcs[i].radius;
			double dy = sin(a) * arcs[i].radius;
			cv::Point p = applyHomography(cv::Point(arcs[i].center.x + dx, arcs[i].center.y + dy), homography);
			int x = p.x;
			int y = p.y;
			if (showFieldModels) cv::circle(drawing, p, round(thickness/2), cv::Scalar(0, 0, 255), -1);
			if (y < 0 || y >= imageDT.rows || x < 0 || x >= imageDT.cols) {
				score += mean;
			} else {
				score += imageDT.at<float>(y, x);
			}
		}
	}

	/*if (cnt == 0) {
		score = std::numeric_limits<double>::max();
	} else {
		score /= cnt;
	}*/

	if (showFieldModels) std::cerr << score << std::endl;

	while (true) {
		imshow("Field Model Drawing", drawing);
		int wk = cv::waitKey(0);
		if (wk == RETURN) break;
		if (wk == ESC) {
			showFieldModels = false;
			break;
		}
	}

	return score;
}

FieldModel::FieldModel(double scale) {
	imageSize = cv::Size((fieldLength + 2 * MARGIN) * scale, (fieldWidth + 2 * MARGIN) * scale);

	// outter lines
	lines.push_back(Line(cv::Point2d(MARGIN, MARGIN), cv::Point2d(MARGIN, MARGIN + fieldWidth)));
	lines.push_back(Line(cv::Point2d(MARGIN, MARGIN + fieldWidth), cv::Point2d(MARGIN + fieldLength, MARGIN + fieldWidth)));
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength, MARGIN + fieldWidth), cv::Point2d(MARGIN + fieldLength, MARGIN)));
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength, MARGIN), cv::Point2d(MARGIN, MARGIN)));

	// goal areas
	lines.push_back(Line(cv::Point2d(MARGIN, MARGIN + fieldWidth/2 - goalAreaWidth/2),
		cv::Point2d(MARGIN + goalAreaLength, MARGIN + fieldWidth / 2 - goalAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN + goalAreaLength, MARGIN + fieldWidth / 2 - goalAreaWidth / 2),
		cv::Point2d(MARGIN + goalAreaLength, MARGIN + fieldWidth / 2 + goalAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN, MARGIN + fieldWidth / 2 + goalAreaWidth / 2),
		cv::Point2d(MARGIN + goalAreaLength, MARGIN + fieldWidth / 2 + goalAreaWidth / 2)));

	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength, MARGIN + fieldWidth / 2 - goalAreaWidth / 2),
		cv::Point2d(MARGIN + fieldLength - goalAreaLength, MARGIN + fieldWidth / 2 - goalAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength - goalAreaLength, MARGIN + fieldWidth / 2 - goalAreaWidth / 2),
		cv::Point2d(MARGIN + fieldLength - goalAreaLength, MARGIN + fieldWidth / 2 + goalAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength, MARGIN + fieldWidth / 2 + goalAreaWidth / 2),
		cv::Point2d(MARGIN + fieldLength - goalAreaLength, MARGIN + fieldWidth / 2 + goalAreaWidth / 2)));

	// penalty areas
	lines.push_back(Line(cv::Point2d(MARGIN, MARGIN + fieldWidth / 2 - penaltyAreaWidth / 2),
		cv::Point2d(MARGIN + penaltyAreaLength, MARGIN + fieldWidth / 2 - penaltyAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN + penaltyAreaLength, MARGIN + fieldWidth / 2 - penaltyAreaWidth / 2),
		cv::Point2d(MARGIN + penaltyAreaLength, MARGIN + fieldWidth / 2 + penaltyAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN, MARGIN + fieldWidth / 2 + penaltyAreaWidth / 2),
		cv::Point2d(MARGIN + penaltyAreaLength, MARGIN + fieldWidth / 2 + penaltyAreaWidth / 2)));
	arcs.push_back(Arc(cv::Point2d(MARGIN + penaltyMarkLength, MARGIN + fieldWidth / 2), radius,
		penaltyAngle - M_PI/2, M_PI/2 - penaltyAngle));

	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength, MARGIN + fieldWidth / 2 - penaltyAreaWidth / 2),
		cv::Point2d(MARGIN + fieldLength - penaltyAreaLength, MARGIN + fieldWidth / 2 - penaltyAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength - penaltyAreaLength, MARGIN + fieldWidth / 2 - penaltyAreaWidth / 2),
		cv::Point2d(MARGIN + fieldLength - penaltyAreaLength, MARGIN + fieldWidth / 2 + penaltyAreaWidth / 2)));
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength, MARGIN + fieldWidth / 2 + penaltyAreaWidth / 2),
		cv::Point2d(MARGIN + fieldLength - penaltyAreaLength, MARGIN + fieldWidth / 2 + penaltyAreaWidth / 2)));
	arcs.push_back(Arc(cv::Point2d(MARGIN + fieldLength - penaltyMarkLength, MARGIN + fieldWidth/2), radius,
		M_PI/2 + penaltyAngle, 3*M_PI/2 - penaltyAngle));

	// center
	lines.push_back(Line(cv::Point2d(MARGIN + fieldLength/2, MARGIN), cv::Point2d(MARGIN + fieldLength / 2, MARGIN + fieldWidth)));
	arcs.push_back(Arc(cv::Point2d(MARGIN + fieldLength/2, MARGIN + fieldWidth/2), radius, 0, 2*M_PI));

	thickness = round(sqrt(scale));

	for (int i = 0; i < lines.size(); i++) {
		lines[i].start *= scale;
		lines[i].end *= scale;
	}

	for (int i = 0; i < arcs.size(); i++) {
		arcs[i].center *= scale;
		arcs[i].radius *= scale;
	}
}
