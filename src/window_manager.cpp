#include "window_manager.h"

#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>
/*#include <QtCore\QString>
#include <QtWidgets\QInputDialog>*/

static double scalex = 1;
static double scaley = 1;

static void internalMouseCallback(int event, int x, int y, int flags, void* data) {
    MyMouseCallback mouseCallback = *(MyMouseCallback *)(data);
    if (mouseCallback.set) {
        mouseCallback.onMouse(event, x / scalex, y / scaley, flags, mouseCallback.data);
    }
}

WindowManager::WindowManager(std::string wn) : winName(wn) {
    cv::namedWindow(winName);
    cv::moveWindow(winName, 20, 20);

    mainHwnd = FindWindow((LPCSTR)"Main HighGUI class", (LPCSTR)winName.c_str());
    LONG newStyle = GetWindowLong(mainHwnd, GWL_STYLE) & ~WS_THICKFRAME;
    SetWindowLong(mainHwnd, GWL_STYLE, newStyle);

    hwnd = (HWND)cvGetWindowHandle(winName.c_str());

    cv::setMouseCallback(winName, internalMouseCallback, &mouseCallback);

    setArrowCursor();
}

void WindowManager::showImage(cv::Mat image, cv::Size size, int keepRatio) {
    scalex = 1;
    scaley = 1;
    if (size.area() != 0) {
        double scale;
        switch (keepRatio) {
            case DONT_KEEP_RATIO:
                cv::resizeWindow(winName, size.width, size.height);
                cv::resize(image, image, size);
                scalex = size.width / (double) image.cols;
                scaley = size.height / (double) image.rows;
                break;
            case KEEP_RATIO_WIDTH:
                scale = size.width / (double) image.cols;
                cv::resizeWindow(winName, image.cols * scale, image.rows * scale);
                cv::resize(image, image, cv::Size(image.cols * scale, image.rows * scale));
                scalex = scale;
                scaley = scale;
                break;
            case KEEP_RATIO_HEIGHT:
                scale = size.height / (double) image.rows;
                cv::resizeWindow(winName, image.cols * scale, image.rows * scale);
                cv::resize(image, image, cv::Size(image.cols * scale, image.rows * scale));
                scalex = scale;
                scaley = scale;
                break;
        }
    }
    imshow(winName, image);
}

bool WindowManager::isVisible() {
    return IsWindowVisible(hwnd);
}

void WindowManager::setMouseCallback(cv::MouseCallback onMouse, void* data) {
    mouseCallback = MyMouseCallback(onMouse, data);
}

void WindowManager::removeMouseCallback() {
    mouseCallback = MyMouseCallback();
}

void WindowManager::close() {
    cv::destroyWindow(winName);
}

void WindowManager::setCursor(LPCSTR style) {
    HCURSOR hCursor = LoadCursor(NULL, (LPCSTR)style);
    SetCursor(hCursor);
    SetClassLongPtr(hwnd, GCLP_HCURSOR, (LONG_PTR) hCursor);
}

void WindowManager::setArrowCursor() {
    setCursor((LPCSTR)IDC_ARROW);
}

void WindowManager::setNESWCursor() {
    setCursor((LPCSTR)IDC_SIZENESW);
}

void WindowManager::setNWSECursor() {
    setCursor((LPCSTR)IDC_SIZENWSE);
}

void WindowManager::setCrossCursor() {
    setCursor((LPCSTR)IDC_CROSS);
}

void WindowManager::disableWindow() {
    EnableWindow(mainHwnd, false);
    LONG newStyle = GetWindowLong(mainHwnd, GWL_STYLE) & ~WS_BORDER;
    SetWindowLong(mainHwnd, GWL_STYLE, newStyle);
}

// void WindowManager::createTextbox() {
	/*HWND hwndTextbox = CreateWindowEx(WS_EX_CLIENTEDGE, (LPCWSTR) "Main HighGUI class", (LPCWSTR) "Line one",
		WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT,
		CW_USEDEFAULT, CW_USEDEFAULT, 200, 24,	// x, y, w, h
		mainHwnd, (HMENU)(123),
		(HINSTANCE)GetWindowLongPtr(mainHwnd, GWLP_HINSTANCE), NULL);

	ShowWindow(hwndTextbox, SW_SHOWDEFAULT);
	UpdateWindow(hwndTextbox);*/
	// MessageBox(hwnd, (LPCWSTR) "bok", NULL, MB_OK);
	/*DialogBox((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),                   // application instance
		MAKEINTRESOURCE(IDD_PASSWORD), // dialog box resource
		hWnd,                          // owner window
		PasswordProc                    // dialog box window procedure
	);*/
	/*bool ok;
	QString text = QInputDialog::getText(NULL, "ime", "text", QLineEdit::Normal, QString::null, &ok);
	if (ok && !text.isEmpty()) {

	} else {

	}*/
// }
