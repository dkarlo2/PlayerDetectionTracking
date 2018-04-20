#include "util.h"

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
#include <windows.h>

cv::Vec3b bgr2hls(uchar b, uchar g, uchar r) {
    cv::Mat m(1, 1, CV_8UC3, cv::Scalar(b, g, r));
    cvtColor(m, m, CV_BGR2HLS);
    return m.at<cv::Vec3b>(0, 0);
}

cv::Vec3b hls2bgr(uchar h, uchar l, uchar s) {
    cv::Mat m(1, 1, CV_8UC3, cv::Scalar(h, l, s));
    cvtColor(m, m, CV_HLS2BGR);
    return m.at<cv::Vec3b>(0, 0);
}

std::vector<uchar> vec2vector(cv::Vec3b v) {
    return std::vector<uchar>{{v[0], v[1], v[2]}};
}

cv::Vec3b vector2vec(std::vector<uchar> v) {
    return cv::Vec3b(v[0], v[1], v[2]);
}

double mod (double a, double b) {
    int p = (int)a / (int)b;
    return a - p * b;
}

std::string concat(const char* pref, int id) {
	std::string s = pref;
	s += std::to_string(id);
    return s;
}

std::chrono::milliseconds millisSinceEpoch() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

std::pair<double, double> getLineIntersection(double rho1, double theta1, double rho2, double theta2) {
    double x1 = rho1 * cos(theta1);
    double y1 = rho1 * sin(theta1);
    double x2 = rho2 * cos(theta2);
    double y2 = rho2 * sin(theta2);

    static const std::pair<double, double> infPair = std::make_pair(0/1, 0/1);

    if (y1 == 0) {
        if (y2 == 0) {
            return infPair;
        }

        double a = -1 / (y2 / x2);
        double b = y2 - a * x2;

        double x = x1;
        double y = a*x + b;

        return std::make_pair(x, y);
    } else {
        if (y2 == 0) {
            double a = -1 / (y1 / x1);
            double b = y1 - a * x1;

            double x = x2;
            double y = a*x + b;

            return std::make_pair(x, y);
        }
        double a1 = -1 * x1 / y1;
        double a2 = -1 / (y2 / x2);

        double b1 = y1 - a1 * x1;
        double b2 = y2 - a2 * x2;

        if (a1 == a2) {
            return infPair;
        } else {
            double x = (b2 - b1) / (a1 - a2);
            double y = x * a1 + b1;

            return std::make_pair(x, y);
        }
    }
}

std::pair<double, double> getLineIntersection(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) {
    if (x2 == x1 && x4 == x3) {
        return std::make_pair(-1, -1);
    }

    double a1, b1, a2, b2, x, y;

    if (x3 != x4) {
        a2 = (y4 - y3) / (double)(x4 - x3);
        b2 = y3 - x3 * a2;
    }

    if (x1 != x2) {
        a1 = (y2 - y1) / (double)(x2 - x1);
        b1 = y1 - x1 * a1;
    }

    if (x1 == x2) {
        x = x1;
        y = a2 * x + b2;
    } else if (x3 == x4) {
        x = x3;
        y = a1 * x + b1;
    } else {
        x = (b2 - b1) / (a1 - a2);
        y = a1 * x + b1;
    }

    return std::make_pair(x, y);
}

void dilateImage(cv::Mat image, int dilate_size) {
    cv::Mat elementDilate = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*dilate_size+1, 2*dilate_size+1),
        cv::Point(dilate_size, dilate_size));
    dilate(image, image, elementDilate);
}

void erodeImage(cv::Mat image, int erode_size) {
    cv::Mat elementErode = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*erode_size+1, 2*erode_size+1),
        cv::Point(erode_size, erode_size));
    erode(image, image, elementErode);
}

bool pointsDirection(cv::Point p1, cv::Point p2, cv::Point p3) {
    return p1.x*(p2.y-p3.y) + p2.x*(p3.y-p1.y) + p3.x*(p1.y-p2.y) > 0;
}

bool isPointInsidePolygon(cv::Point point, std::vector<cv::Point> points) {
    bool dir = pointsDirection(point, points[points.size()-1], points[0]);
    for (size_t i = 0; i < points.size()-1; i++) {
        cv::Point p2 = points[i];
        cv::Point p3 = points[i+1];
        if (pointsDirection(point, p2, p3) != dir) {
            return false;
        }
    }
    return true;
}

double dist(cv::Point p1, cv::Point p2) {
    int dx = p1.x - p2.x;
    int dy = p1.y - p2.y;
    return sqrt(dx*dx + dy*dy);
}

void myFillPoly(cv::Mat image, std::vector<cv::Point> points, cv::Scalar color) {
    int *polySize = new int[1];
    polySize[0] = points.size();
    const cv::Point *polys[1] = {&points[0]};
    fillPoly(image, polys, polySize, 1, color);
}

double resizeImageArea(cv::Mat &image, double maxImgArea) {
    double area = image.rows * image.cols;
    double factor = sqrt(maxImgArea / area);
    if (factor < 1) {
        resize(image, image, cv::Size(image.cols * factor, image.rows * factor));
        return factor;
    }
    return 1;
}

static inline char separator()
{
#ifdef _WIN32
	return '\\';
#else
	return '/';
#endif
}

std::string makePath(std::string first, std::string second) {
	return first + separator() + second;
}

cv::Point2d getRectCenter(cv::Rect r) {
	return cv::Point2d(r.x + r.width / 2.0, r.y + r.height / 2.0);
}
