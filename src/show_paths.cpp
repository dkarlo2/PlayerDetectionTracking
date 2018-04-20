#include "show_paths.h"

#include <fstream>
#include "globals.h"
#include "inputs.h"
#include <iostream>
#include <memory>
#include <opencv2/imgproc/imgproc.hpp>
#include "util.h"
#include "video_manager.h"
#include "window_manager.h"

#undef min
#undef max

#define NO_TYPE -1
#define NEW 0
#define POS 1

class Info {
public:
	int frame;

	virtual int getType() {
		return NO_TYPE;
	}

	Info(int f) : frame(f) {}
};

class InfoNew : public Info {
public:
	long long id;

	int getType() {
		return NEW;
	}

	InfoNew(int f, long long i) : Info(f), id(i) {}
};

class InfoPos : public Info {
public:
	long long id;
	float x;
	float y;
	double team;

	int getType() {
		return POS;
	}

	InfoPos(int f, long long i, float X, float Y, double T) : Info(f), id(i), x(X), y(Y), team(T) {}
};

static void drawPlayerCircles(cv::Mat frame, float x, float y, double team/*, cv::Mat &field, cv::Mat invHom*/) {
	team = std::max(-1., std::min(1., team));
	cv::Scalar color = team < 0 ? cv::Scalar(255 * (-team), 0, 0) : cv::Scalar(0, 0, 255 * team);
	cv::circle(frame, cv::Point(x, y), 10, color, -1);

	/*double xf = invHom.at<double>(0, 0) * x + invHom.at<double>(0, 1) * y + invHom.at<double>(0, 2);
	double yf = invHom.at<double>(1, 0) * x + invHom.at<double>(1, 1) * y + invHom.at<double>(1, 2);
	double pf = invHom.at<double>(2, 0) * x + invHom.at<double>(2, 1) * y + invHom.at<double>(2, 2);

	if (team < 0) {
		color = cv::Scalar(255, 0, 0);
	}
	else {
		color = cv::Scalar(0, 0, 255);
	}

	cv::circle(field, cv::Point(xf / pf, yf / pf), 3, color, -1);*/
}

void showPaths() {
	std::map<int, std::vector<std::shared_ptr<Info>>> infos;

	std::ifstream file;
	file.open(outputFile, std::ios::in);

	while (true) {
		int tf;
		std::string type;
		file >> tf >> type;
		if (tf == -1) {
			break;
		}
		if (type == "new") {
			long long id;
			file >> id;
			infos[tf].push_back(std::make_shared<InfoNew>(tf, id));
		} else if (type == "pos") {
			long long id;
			float x;
			float y;
			double team;
			file >> id >> x >> y >> team;
			infos[tf].push_back(std::make_shared<InfoPos>(tf, id, x, y, team));
		} else {
			std::cerr << "should not happen" << std::endl;
			throw "";
		}
	}

	file.close();

	std::string videoFilePath = makePath(tmpDirectory, tmpVideoFile);
	VideoManager vm(videoFilePath);
	WindowManager wm(mainWindowName);

	int trackFrame = 0;
	cv::Mat frame;

	while (wm.isVisible()) {
		if (vm.nextFrame()) break;

		trackFrame++;

		frame = vm.getFrame();

		std::vector<std::shared_ptr<Info>> frameInfos = infos[trackFrame];
		for (int i = 0; i < frameInfos.size(); i++) {
			if (InfoNew* infoNew = dynamic_cast<InfoNew*>(frameInfos[i].get())) {

			} else if (InfoPos* infoPos = dynamic_cast<InfoPos*>(frameInfos[i].get())) {
				drawPlayerCircles(frame, infoPos->x, infoPos->y, infoPos->team);
			}
		}

		wm.showImage(frame, cv::Size(800, 600));

		int key = cv::waitKey(1);
		if (key == ESC) break;
	}

	wm.close();	
}
