/*
	Defines class for managing OpenCV windows.
*/

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <string>
#include <windows.h>

#define DONT_KEEP_RATIO 0
#define KEEP_RATIO_WIDTH 1
#define KEEP_RATIO_HEIGHT 2

class MyMouseCallback {
public:
    cv::MouseCallback onMouse;
    void *data;
    bool set;
    MyMouseCallback(cv::MouseCallback om, void *d) : onMouse(om), data(d), set(true) {};
    MyMouseCallback() : set(false) {};
};

class WindowManager {
    const std::string winName;
    HWND mainHwnd;
    HWND hwnd;
    MyMouseCallback mouseCallback;

    void setCursor(LPCSTR style);
public:
    WindowManager(const std::string wn);
    void showImage(cv::Mat image, cv::Size size = cv::Size(), int keepRatio = KEEP_RATIO_WIDTH);
    bool isVisible();
    void setMouseCallback(cv::MouseCallback onMouse, void* data);
    void removeMouseCallback();
    void close();
    void setArrowCursor();
    void setNESWCursor();
    void setNWSECursor();
    void setCrossCursor();
    const std::string getName() {
        return winName;
    }
    void disableWindow();
	// void createTextbox();
};

#endif
