#ifndef UTIL_H
#define UTIL_H

#include <chrono>
#include "opencv2/core/core.hpp"
#include <string>
#include <vector>

#define pr(x) (static_cast<unsigned>(x))

cv::Vec3b bgr2hls(uchar b, uchar g, uchar r);

cv::Vec3b hls2bgr(uchar h, uchar l, uchar s);

std::vector<uchar> vec2vector(cv::Vec3b v);

cv::Vec3b vector2vec(std::vector<uchar> v);

double mod(double a, double b);

std::string concat(const char* pref, int id);

std::chrono::milliseconds millisSinceEpoch();

std::pair<double, double> getLineIntersection(double rho1, double theta1, double rho2, double theta2);

std::pair<double, double> getLineIntersection(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4);

void dilateImage(cv::Mat image, int dilate_size);

void erodeImage(cv::Mat image, int erode_size);

bool pointsDirection(cv::Point p1, cv::Point p2, cv::Point p3);

bool isPointInsidePolygon(cv::Point point, std::vector<cv::Point> points);

double dist(cv::Point p1, cv::Point p2);

void myFillPoly(cv::Mat image, std::vector<cv::Point> points, cv::Scalar color);

double resizeImageArea(cv::Mat &image, double maxImgArea);

std::string makePath(std::string first, std::string second);

cv::Point2d getRectCenter(cv::Rect r);

#endif
