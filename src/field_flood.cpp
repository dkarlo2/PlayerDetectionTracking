#include "field_flood.h"

#include "inputs.h"
#include "util.h"

#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>

static std::vector<cv::Point> getFloodPoints(cv::Size imageSize) {
    std::vector<cv::Point> points;
    cv::Point center(imageSize.width/2, imageSize.height/2);
    for (double i = -(floodPointsHor - 1) / 2.; i <= (floodPointsHor - 1) / 2.; i++) {
        for (double j = -(floodPointsVer - 1) / 2.; j <= (floodPointsVer - 1) / 2.; j++) {
            points.push_back(cv::Point(center.x + i * floodPointsMargin, center.y + j * floodPointsMargin));
        }
    }
    return points;
}

static bool sortAreas(std::pair<int, cv::Mat> area1, std::pair<int, cv::Mat> area2) {
    return area1.first > area2.first;
}

static bool uniqueAreas(std::pair<int, cv::Mat> area1, std::pair<int, cv::Mat> area2) {
    return area1.first == area2.first;
}

cv::Mat drawFloodPoints(cv::Mat image) {
    std::vector<cv::Point> floodPoints = getFloodPoints(image.size());
    for (size_t i = 0; i < floodPoints.size(); i++) {
        circle(image, floodPoints[i], 2, cv::Scalar(0, 255, 0));
    }
    return image;
}

cv::Mat getFieldMaskFromFlood(cv::Mat edges) {
    std::vector<cv::Point> floodPoints = getFloodPoints(edges.size());
    std::vector<std::pair<int, cv::Mat>> areas;
    for (size_t i = 0; i < floodPoints.size(); i++) {
        cv::Point p = floodPoints[i];
		if (edges.at<uchar>(p) == 255) {
			continue;
		}
        cv::Mat e = edges.clone();
        floodFill(e, p, cv::Scalar(100));
        cv::Mat flooded;
        inRange(e, cv::Scalar(100), cv::Scalar(100), flooded); // TODO ovo se moze sa 'mask' parametrom u 'floodFill'
        int area = countNonZero(flooded);
        areas.push_back(std::make_pair(area, flooded.clone()));
    }

    std::sort(areas.begin(), areas.end(), sortAreas);
    areas.erase(std::unique(areas.begin(), areas.end(), uniqueAreas), areas.end());

	cv::Mat fieldMask = areas[0].second.clone();
    if (areas.size() > 1) {
		fieldMask = fieldMask | areas[1].second;
    }

	dilateImage(fieldMask, fieldFloodDilate);

    return fieldMask;
}
