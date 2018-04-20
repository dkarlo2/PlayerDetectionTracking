/*
	Defines class for reading video files.
*/

#ifndef VIDEO_MANAGER_H
#define VIDEO_MANAGER_H

#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <chrono>

#define FRAME_END 1

class VideoManager {
    cv::VideoCapture vid;
    int fw, fh, fps;
    int frameCount = 0;
    cv::Mat frame;
    std::chrono::milliseconds timeStartPlay;
public:
    VideoManager(const std::string p);
    void resetTime();
    int nextFrame();
    cv::Mat& getFrame();
    int waitMillis();
	cv::VideoCapture getVideoCapture() {
		return vid;
	}
};

#endif
