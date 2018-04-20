/*
	Defines field model based on foreground and background (field) color separation.
*/

#ifndef FIELD_BACKGROUND_H
#define FIELD_BACKGROUND_H

#include "inputs.h"
#include "opencv2/core/core.hpp"
#include <vector>

class FieldBackground {
    int calculated = 0;
    std::vector<bool> useGroup;
public:
    bool isCalculated() {
		if (calculated == 0) {
			calculated = bgCalcFramePeriod;
			return false;
		}
		calculated--;
        return true;
    }
    void calculateBgColors(cv::Mat image);
    cv::Mat extractBgMask(cv::Mat image);
};

#endif
