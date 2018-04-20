#include "globals.h"
#include "player_detection.h"

#include "util.h"
#include "time_measurement.h"
#include "window_manager.h"

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <random>

#undef min
#undef max

static std::default_random_engine generator;
static std::uniform_real_distribution<double> uniform;

static std::vector<std::pair<cv::Rect, cv::Mat>> splitByClusters(cv::Mat image, cv::Mat mask, int clusterCount) {
	cv::Mat origImage = image.clone();

	cv::GaussianBlur(image, image, cv::Size(3, 3), 1, 1);

	int sampleCount = cv::countNonZero(mask);

	if (sampleCount == 0) {
		return std::vector<std::pair<cv::Rect, cv::Mat>>();
	}

	cv::Mat points(sampleCount, 5, CV_32F);
	std::vector<cv::Point> pointsPos;
	int s = 0;
	for (int i = 0; i < image.rows; i++) {
		for (int j = 0; j < image.cols; j++) {
			if (!mask.at<uchar>(i, j)) {
				continue;
			}
			cv::Vec3b color = image.at<cv::Vec3b>(i, j);
			// normalized
			points.at<float>(s, 0) = color[0] / 255.f;
			points.at<float>(s, 1) = color[1] / 255.f;
			points.at<float>(s, 2) = color[2] / 255.f;
			points.at<float>(s, 3) = i / (float)image.rows;
			points.at<float>(s, 4) = j / (float)image.cols;
			pointsPos.push_back(cv::Point(j, i));
			s++;
		}
	}

	cv::Mat labels, centers;
	double compactness = cv::kmeans(points, std::min(sampleCount, clusterCount), labels,
		cv::TermCriteria(cv::TermCriteria::COUNT, 100, 0.1), 3, cv::KMEANS_PP_CENTERS, centers);
	// std::cerr << "Compactness: " << compactness << std::endl;

	std::vector<std::pair<cv::Point, cv::Point>> rectPoints;
	std::vector<cv::Mat> masks;
	for (int i = 0; i < clusterCount; i++) {
		rectPoints.push_back(std::make_pair(
			cv::Point(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
			cv::Point(std::numeric_limits<int>::min(), std::numeric_limits<int>::min())));
		masks.push_back(cv::Mat::zeros(image.size(), 0));
	}

	for (int s = 0; s < sampleCount; s++) {
		int c = labels.at<int>(s);
		cv::Point p = pointsPos[s];
		rectPoints[c].first.x = std::min(rectPoints[c].first.x, p.x);
		rectPoints[c].first.y = std::min(rectPoints[c].first.y, p.y);
		rectPoints[c].second.x = std::max(rectPoints[c].second.x, p.x);
		rectPoints[c].second.y = std::max(rectPoints[c].second.y, p.y);
		masks[c].at<uchar>(p.y, p.x) = 255;
	}

	std::vector<std::pair<cv::Rect, cv::Mat>> players;

	for (int i = 0; i < clusterCount; i++) {
		players.push_back(std::make_pair(cv::Rect(rectPoints[i].first, rectPoints[i].second), masks[i]));
	}

	if (showClusters) {
		cv::Vec3b clusterColors[] = {
			cv::Vec3b(255, 0, 0), cv::Vec3b(0, 255, 0), cv::Vec3b(0, 0, 255), cv::Vec3b(0, 255, 255),
			cv::Vec3b(255, 255, 0), cv::Vec3b(255, 0, 255)};
		cv::Mat playerClustered = cv::Mat::zeros(image.size(), image.type());
		for (int s = 0; s < sampleCount; s++) {
			int c = labels.at<int>(s);
			playerClustered.at<cv::Vec3b>(pointsPos[s].y, pointsPos[s].x) = clusterColors[c];
		}

		while (true) {
			static WindowManager wm0("player mask");
			wm0.showImage(mask, cv::Size(100, 100));
			static WindowManager wm1("player contour");
			wm1.showImage(origImage, cv::Size(100, 100));
			static WindowManager wm2("player clustered");
			wm2.showImage(playerClustered, cv::Size(100, 100));
			int key = cv::waitKey(0);
			if (key == RETURN) break;
			if (key == ESC) {
				showClusters = false;
				break;
			}
		}
	}

	return players;
}

static std::vector<float> getPlayerFeatures(cv::Mat mask, cv::Mat image, int erodeSize = 1) {
	if (erodeSize > 0) {
		erodeImage(mask, erodeSize);
	}

	std::vector<float> features(3);

	if (cv::countNonZero(mask) == 0) {
		return features;
	}

	features = cv::Mat(1, 3, CV_32F);

	cv::Scalar color = cv::mean(image, mask);
	features[0] = color[0] / 255.0;
	features[1] = color[1] / 255.0;
	features[2] = color[2] / 255.0;

	return features;
}

static std::vector<float> getPlayerFeatures(std::vector<cv::Point> contour, cv::Mat image, int erodeSize = 1) {
	cv::Mat mask = cv::Mat::zeros(image.size(), 0);
	std::vector<std::vector<cv::Point>> cs;
	cs.push_back(contour);
	drawContours(mask, cs, 0, cv::Scalar(255), -1);

	return getPlayerFeatures(mask, image, erodeSize);
}

struct SizeBounds {
	double heightLower;
	double heightUpper;
	double widthLower;
	double widthUpper;
};

static SizeBounds getBoundsForRect(cv::Rect rect, double minY, double maxY) {
	SizeBounds sb;

	double y = rect.y + rect.height / 2.0;
	double alpha = (y - minY) / (maxY - minY);
	double avgHeight = pow(alpha, perspectivePower) * (avgHeightMax - avgHeightMin) + avgHeightMin;
	double avgWidth = pow(alpha, perspectivePower) * (avgWidthMax - avgWidthMin) + avgWidthMin;

	sb.heightLower = (1 - heightOffset) * avgHeight;
	sb.heightUpper = (1 + heightOffset) * avgHeight;
	sb.widthLower = (1 - widthOffset) * avgWidth;
	sb.widthUpper = (1 + widthOffset) * avgWidth;

	return sb;
}

void detectPlayers(FrameData &fd) {
	cv::Mat image = fd.image;

    MyTime mt;
    mt.start();

	cv::Mat playersMask = fd.foreground;

	dilateImage(playersMask, 1);

    cv::Mat playersMaskedImage;
    image.copyTo(playersMaskedImage, playersMask);

#if DEBUG_PLAYER_DETECTION
	imshow("foreground", fd.foreground);
	imshow("Players mask", playersMask);
    imshow("Foreground players", playersMaskedImage);
    std::cerr << "Players mask: " << mt.time() << std::endl;
#endif

    mt.start();

    std::vector< std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours(playersMask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

#if DEBUG_PLAYER_DETECTION
    std::cerr << "find contours " << mt.time() << std::endl;
#endif

    mt.start();

#if DEBUG_PLAYER_DETECTION
    cv::Mat drawing = cv::Mat::zeros(image.size(), CV_8UC3);
    for(size_t i = 0; i < contours.size(); i++) {
        cv::Scalar color = cv::Scalar(rand()%256, rand()%256, rand()%256);
        drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
        }
    imshow("Players contours", drawing);
#endif

	double minY = fd.minY;
    double maxY = fd.maxY;

	cv::Mat invBgMask = 255 - fd.bgMask;

	std::vector<cv::Rect> origDetect;
	std::vector<cv::Rect> origBig;

    for (size_t i = 0; i < contours.size(); i++) {
		std::vector<cv::Point> contour = contours[i];

		cv::Mat contourMask = cv::Mat::zeros(image.size(), 0);
		drawContours(contourMask, contours, i, cv::Scalar(255), -1);
		int area = cv::countNonZero(contourMask);
        // double area = contourArea(contours[i]);

        cv::Rect rect = boundingRect(contours[i]);
		if (rect.width * rect.height > maxContourRectArea) continue;
        int rectArea = rect.area();

		SizeBounds sb = getBoundsForRect(rect, minY, maxY);

		if (rect.height >= sb.heightLower && area / (double) rectArea > areaRatio) {
			origDetect.push_back(rect);
			if (rect.height > sb.heightUpper || rect.width > sb.widthUpper) {
				origBig.push_back(rect);
				cv::Mat player = image(rect);
				cv::Mat playerMask(invBgMask, rect);
				int clusterCount = 2;
				while (true) {
					std::vector<std::pair<cv::Rect, cv::Mat>> players = splitByClusters(player.clone(), playerMask, clusterCount);
					bool split = true;
					if (clusterCount < maxClusters) {
						for (int i = 0; i < players.size(); i++) {
							cv::Rect r = players[i].first;
							r.x += rect.x;
							r.y += rect.y;
							SizeBounds sbi = getBoundsForRect(r, minY, maxY);
							if (r.height > sbi.heightUpper) {
								split = false;
								break;
							}
						}
					}
					if (split) {
						for (int i = 0; i < players.size(); i++) {
							cv::Rect r = players[i].first;
							r.x += rect.x;
							r.y += rect.y;
							SizeBounds sbi = getBoundsForRect(r, minY, maxY);
							if (r.height >= sbi.heightLower && r.width <= sbi.widthUpper) {
								cv::Mat mask = cv::Mat::zeros(image.size(), 0);
								players[i].second.copyTo(mask(rect));
								std::vector<float> fs = getPlayerFeatures(mask, image, 0);
								fd.addDetectedPlayer(r, fs);
								// TODO add to team model features?
							}
						}
						break;
					} else {
						clusterCount++;
					}
				}
			} else {
				std::vector<float> fs = getPlayerFeatures(contour, image, 0);
				fd.addDetectedPlayer(rect, fs);
				if (uniform(generator) < teamModelProb) {
					fd.teamModel.addFeatures(fs);
				}
			}
		}
    }

#if DEBUG_PLAYER_DETECTION
    drawing = image.clone();
	/*for (size_t i = 0; i < origDetect.size(); i++) {
		rectangle(drawing, origDetect[i], cv::Scalar(255, 0, 0), 1);
	}
	for (size_t i = 0; i < origBig.size(); i++) {
		rectangle(drawing, origBig[i], cv::Scalar(0, 255, 255), 1);
	}*/

    for (size_t i = 0; i < fd.detectedPlayers.size(); i++) {
        rectangle(drawing, fd.detectedPlayers[i].first, cv::Scalar(0, 0, 255), 1);
    }

    static WindowManager wpd("Player Detection");
    wpd.showImage(drawing, cv::Size(800, 600));

	/*while (true) {
		if (cv::waitKey(1) == 13) break;
	}*/

    /*static WindowManager wpd("Player Detection");
    Contours ct(drawing, contours, avgHeights, minY, maxY);
    wpd.setMouseCallback(contoursMouse, &ct);
    while (true) {
        wpd.showImage(ct.showDrawing, cv::Size(800, 600));
        if (cv::waitKey(1) == 32) {
            break;
        }
    }*/
    std::cerr << "debug " << mt.time() << std::endl;
#endif

}
