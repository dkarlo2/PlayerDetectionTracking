#ifndef FIELD_MODEL_H
#define FIELD_MODEL_H

#include <opencv2/core/core.hpp>
#include <string>

class Line {
public:
	cv::Point2d start;
	cv::Point2d end;
	Line(cv::Point2d S, cv::Point2d E) : start(S), end(E) {}
	double norm() {
		double dx = start.x - end.x;
		double dy = start.y - end.y;
		return sqrt(dx*dx + dy*dy);
	}
};

class Arc {
public:
	cv::Point2d center;
	double radius;
	double startAngle;
	double endAngle;
	Arc(cv::Point2d C, double R, double SA, double EA) : center(C), radius(R), startAngle(SA), endAngle(EA) {}
};

class FieldModel {
	cv::Size imageSize;
	int thickness;
	std::vector<Line> lines;
	std::vector<Arc> arcs;
public:
	FieldModel(double scale = 1);
	cv::Mat getImage(int t = -1, cv::Mat homography = cv::Mat::eye(3, 3, CV_64FC1), cv::Size size = cv::Size());
	double calculateScore(cv::Mat imageDT, cv::Mat homography, double lineResolution, double arcResolution);
};

#endif
