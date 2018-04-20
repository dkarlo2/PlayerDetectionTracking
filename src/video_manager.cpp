#include "video_manager.h"

#include "util.h"

#include <algorithm>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>

VideoManager::VideoManager(std::string path) : vid(cv::VideoCapture(path)) {
    if (!vid.isOpened()) {
        std::cout << "Cannot open video file." << std::endl;
		std::cout << "Path: " << path << std::endl;
        return;
    }

    fw = vid.get(CV_CAP_PROP_FRAME_WIDTH);
    fh = vid.get(CV_CAP_PROP_FRAME_HEIGHT);
    fps = vid.get(CV_CAP_PROP_FPS);
}

void VideoManager::resetTime() {
    frameCount = 0;
}

int VideoManager::nextFrame() {
    if (frameCount == 0) {
        timeStartPlay = millisSinceEpoch();
    }
    if (!vid.read(frame)) return FRAME_END;
    frameCount++;
    return 0;
}

cv::Mat& VideoManager::getFrame() {
    return frame;
}

int VideoManager::waitMillis() {
    std::chrono::milliseconds now = millisSinceEpoch();
    std::chrono::milliseconds timePast = now - timeStartPlay;
    int millis = 1000 / fps - (timePast.count() - frameCount * 1000 / fps);
    /*if (millis < 1) {
        std::cerr << "Late " << millis << " milliseconds." << std::endl;
    }*/
    return millis;
}
