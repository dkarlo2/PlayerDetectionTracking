#include "background_subtraction.h"
#include "globals.h"
#include "util.h"

#include <opencv2/highgui.hpp>

void BackgroundEliminator::detectForeground(FrameData &fd) {
	/*cv::Mat image = 255 - fd.getBgMask();
	cv::Mat foreground;
	pMog2->apply(image, foreground);*/
	pMog2->apply(fd.image, fd.foreground);

#if DEBUG_BACKGROUND_SUBTRACTION
	imshow("image", fd.image);
	imshow("foreground", fd.foreground);
#endif
}
