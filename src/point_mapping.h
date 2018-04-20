#pragma once

#include <algorithm>
#include "opencv2\core\core.hpp"
#include <vector>

typedef std::pair< std::vector<cv::Point2f>, std::vector<cv::Point2f> > Mappings;

Mappings getPointMappings(cv::Mat image, cv::Mat modelImage);
