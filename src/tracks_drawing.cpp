#include "tracks_drawing.h"

#include <opencv2/imgproc.hpp>

void drawTrack(cv::Mat image, track_ptr track, double factor, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return;
	}
	used[track] = true;
	cv::Scalar color(rand() % 256, rand() % 256, rand() % 256);
	std::vector<Measurement> ms = (*track)->getMeasurements();
	//if (ms.size() > 1) {
	for (int i = 0; i < ms.size() - 1; i++) {
		cv::Point p1(ms[i].rect.x + ms[i].rect.width / 2, ms[i].rect.y + ms[i].rect.height / 2);
		cv::Point p2(ms[i + 1].rect.x + ms[i + 1].rect.width / 2, ms[i + 1].rect.y + ms[i + 1].rect.height / 2);
		p1.x /= factor;
		p1.y /= factor;
		p2.x /= factor;
		p2.y /= factor;
		cv::line(image, p1, p2, color, 2);
	}
	//}
	Measurement r = ms[ms.size() - 1];
	cv::Point lastPoint(r.rect.x + r.rect.width / 2, r.rect.y + r.rect.height / 2);
	lastPoint.x /= factor;
	lastPoint.y /= factor;
	std::vector<track_ptr> cs = (*track)->getChildren();
	for (int i = 0; i < cs.size(); i++) {
		std::vector<Measurement> cms = (*cs[i])->getMeasurements();
		if (cms.size() > 0) {
			cv::Point p(cms[0].rect.x + cms[0].rect.width / 2, cms[0].rect.y + cms[0].rect.height / 2);
			p.x /= factor;
			p.y /= factor;
			cv::line(image, lastPoint, p, color, 2);
			drawTrack(image, cs[i], factor, used);
		}
	}
}

void drawFactorRect(cv::Mat &image, cv::Rect rect, double factor, cv::Scalar color, int thickness) {
	cv::Rect r = rect;
	r.x /= factor;
	r.y /= factor;
	r.width /= factor;
	r.height /= factor;
	cv::rectangle(image, r, color, thickness);
}

void drawFactorLineBetweenRects(cv::Mat &image, cv::Rect r1, cv::Rect r2, double factor, cv::Scalar color, int thickness) {
	cv::Point p1(r1.x + r1.width / 2, r1.y + r1.height / 2);
	cv::Point p2(r2.x + r2.width / 2, r2.y + r2.height / 2);
	p1.x /= factor;
	p1.y /= factor;
	p2.x /= factor;
	p2.y /= factor;
	cv::line(image, p1, p2, color, thickness);
}
