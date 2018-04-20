#pragma once

#include <map>
#include <opencv2/core.hpp>
#include "track.h"

void drawTrack(cv::Mat image, track_ptr track, double factor, std::map<track_ptr, bool> &used);

void drawFactorRect(cv::Mat &image, cv::Rect rect, double factor, cv::Scalar color, int thickness = 1);

void drawFactorLineBetweenRects(cv::Mat &image, cv::Rect r1, cv::Rect r2, double factor, cv::Scalar color, int thickness = 1);
