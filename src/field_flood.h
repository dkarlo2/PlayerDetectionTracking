#pragma once

#include <opencv2/core/core.hpp>

cv::Mat getFieldMaskFromFlood(cv::Mat edges);

cv::Mat drawFloodPoints(cv::Mat image);
