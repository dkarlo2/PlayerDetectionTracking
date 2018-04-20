#include "globals.h"
#include "min_edge_cover.h"
#include "player_tracking.h"
#include "time_measurement.h"
#include "tmp_files_write.h"
#include "tracks_drawing.h"
#include "window_manager.h"

#include <iomanip>
#include <iostream>
#include <map>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2\video\tracking.hpp>
#include <queue>
#include <random>
#include <set>

#undef min
#undef max

PlayerTracker::PlayerTracker(cv::VideoCapture &vc) {
	tfw.initFiles(vc);
}

void PlayerTracker::track(FrameData &fd, bool debug) {
	if (trackCountdown > 0) {
		trackCountdown--;
		return;
	}

	trackFrame++;

	tfw.saveVideoFrame(fd.frame);
	tfw.saveHomography(fd.homography, trackFrame, fd.factor);

	MyTime mt;

	std::vector<std::pair<cv::Rect, std::vector<float>>> measurements = fd.detectedPlayers;

	if (tracks.empty()) {
		for (int i = 0; i < measurements.size(); i++) {
			tracks.addTrack(measurements[i].first, measurements[i].second, trackFrame);
		}
		return;
	}

	int size = measurements.size();
	for (int j = 0; j < tracks.size(); j++) {
		if (tracks[j].isActive()) {
			size++;
		}
	}

	Graph g(size);

	std::vector<Node> mNodes;
	for (int i = 0; i < measurements.size(); i++) {
		Node n(true, i);
		g.addNode(n);
		mNodes.push_back(n);
	}

	std::map<int, Node> tNodes;
	for (int j = 0; j < tracks.size(); j++) {
		if (tracks[j].isActive()) {
			Node n(false, j);
			g.addNode(n);
			tNodes[j] = n;
		}
	}

	g.init();

	double minY = fd.minY;
	double maxY = fd.maxY;

	for (int i = 0; i < measurements.size(); i++) {
		cv::Point2d mxy = getRectCenter(measurements[i].first);
		for (int j = 0; j < tracks.size(); j++) {
			if (!tracks[j].isActive()) {
				continue;
			}
			cv::Point2d txy = getRectCenter(tracks[j].lastMeasurement().rect);
			cv::Point2d dxy = mxy - txy;
			double dist = sqrt(dxy.x * dxy.x + dxy.y * dxy.y);
			double alpha = ((mxy.y + txy.y) / 2 - minY) / (maxY - minY);
			double maxMatchDist = pow(alpha, perspectivePower) * (matchDistancesMax - matchDistancesMin) + matchDistancesMin;
			if (dist > maxMatchDist) {
				continue;
			}
			g.addEdge(mNodes[i], tNodes[j], dist + 1); // +1 to avoid problems with zero-distances
		}
	}

	g.updateNeisCount();

#if DEBUG_PLAYER_TRACKING
	cv::Mat debugDrawing = fd.frame.clone();
	double factor = fd.factor;
	cv::Mat debugLines = fd.frame.clone();

	for (int i = 0; i < measurements.size(); i++) {
		drawFactorRect(debugDrawing, measurements[i].first, factor, cv::Scalar(255, 255, 255));
		drawFactorRect(debugLines, measurements[i].first, factor, cv::Scalar(255, 255, 255));
	}

	int cnt = 0;
	for (int i = 0; i < tracks.size(); i++) {
		if (tracks[i].isActive()) {
			cnt++;
			drawFactorRect(debugDrawing, tracks[i].lastMeasurement().rect, factor, cv::Scalar(0, 0, 0));
			drawFactorRect(debugLines, tracks[i].lastMeasurement().rect, factor, cv::Scalar(0, 0, 0));
		}
	}
	std::cerr << "Active tracks: " << cnt << std::endl;
#endif

	for (int i = 0; i < mNodes.size(); i++) {
		if (g.isEmpty(mNodes[i])) {
			int measurementIndex = mNodes[i].getTypeIndex();
			tracks.addTrack(measurements[measurementIndex].first, measurements[measurementIndex].second, trackFrame);
#if DEBUG_PLAYER_TRACKING
			drawFactorRect(debugDrawing, measurements[measurementIndex].first, factor, cv::Scalar(255, 0, 0), 2);
#endif
		}
	}
	for (std::map<int, Node>::iterator it = tNodes.begin(); it != tNodes.end(); it++) {
		int trackIndex = it->second.getTypeIndex();
		if (g.isEmpty(it->second)) {
			if (!tracks[trackIndex].isRetiring()) {
				tracks[trackIndex].retire(framesToRetire);
			}
#if DEBUG_PLAYER_TRACKING
			drawFactorRect(debugDrawing, tracks[trackIndex].lastMeasurement().rect, factor, cv::Scalar(0, 0, 255), 2);
#endif
		} else if (tracks[trackIndex].isRetiring()) {
			tracks[trackIndex].stopRetire();
#if DEBUG_PLAYER_TRACKING
			drawFactorRect(debugDrawing, tracks[trackIndex].lastMeasurement().rect, factor, cv::Scalar(0, 255, 0), 2);
#endif
		}
	}

	mt.start();
	std::map<Node, std::set<Node>> cover = findMinimalEdgeCover(g);
#if DEBUG_PLAYER_TRACKING
	std::cerr << "Finding cover: " << mt.time() << std::endl;
#endif

	/*std::cerr << "Cover:" << std::endl;
	for (std::map<Node, std::set<Node>>::iterator it = cover.begin(); it != cover.end(); it++) {
		std::cerr << it->first.getId() << " --> (";
		for (std::set<Node>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
			std::cerr << jt->getId() << ", ";
		}
		std::cerr << std::endl;
	}*/

	for (auto it = cover.begin(); it != cover.end(); it++) {
		if (it->first.isType()) {
			int measurementIndex = it->first.getTypeIndex();

			size_t childIndex = -1;
			if (it->second.size() > 1) {
				childIndex = tracks.addTrack(measurements[measurementIndex].first, measurements[measurementIndex].second, trackFrame);
#if DEBUG_PLAYER_TRACKING
				drawFactorRect(debugDrawing, measurements[measurementIndex].first, factor, cv::Scalar(0, 255, 255), 2);
				std::cerr << "child " << measurements[measurementIndex].first << std::endl;
#endif
			}
			for (auto jt = it->second.begin(); jt != it->second.end(); jt++) {
				if (jt->isType()) {
					std::cerr << "should not happen" << std::endl;
					throw "";
				}
				// nasao matching trackove
				int trackIndex = jt->getTypeIndex();
				
				if (childIndex != -1) {
					if (cover[*jt].size() == 1) {
						tracks[trackIndex].addChild(tracks.get(childIndex));
						tracks[trackIndex].retire();
						tracks[childIndex].addParent(tracks.get(trackIndex));
#if DEBUG_PLAYER_TRACKING
						drawFactorRect(debugDrawing, tracks[trackIndex].lastMeasurement().rect, factor, cv::Scalar(255, 255, 0), 2);
						drawFactorLineBetweenRects(debugDrawing, tracks[childIndex].lastMeasurement().rect,
							tracks[trackIndex].lastMeasurement().rect, factor, cv::Scalar(255, 255, 255), 2);
						cv::Scalar c(rand() % 256, rand() % 256, rand() % 256);
						drawFactorLineBetweenRects(debugLines, tracks[childIndex].lastMeasurement().rect,
							tracks[trackIndex].lastMeasurement().rect, factor, c, 2);
						std::cerr << "track " << tracks[trackIndex].lastMeasurement().rect << std::endl;
#endif
					} else {
						// TODO ne bi se smjelo dogadat
						std::cerr << "nooo" << std::endl;
						throw "";
					}
				} else if (cover[*jt].size() > 1) {
					size_t childIndex = tracks.addTrack(measurements[measurementIndex].first,
						measurements[measurementIndex].second, trackFrame);
					tracks[trackIndex].addChild(tracks.get(childIndex));
					tracks[trackIndex].retire(); // will be retired more times, but it's not important
					tracks[childIndex].addParent(tracks.get(trackIndex));
#if DEBUG_PLAYER_TRACKING
					drawFactorRect(debugDrawing, measurements[measurementIndex].first, factor, cv::Scalar(255, 0, 255), 2);
					drawFactorRect(debugDrawing, tracks[trackIndex].lastMeasurement().rect, factor, cv::Scalar(39, 127, 255), 2);
					drawFactorLineBetweenRects(debugDrawing, tracks[childIndex].lastMeasurement().rect,
						tracks[trackIndex].lastMeasurement().rect, factor, cv::Scalar(255, 255, 255), 2);
					cv::Scalar c(rand() % 256, rand() % 256, rand() % 256);
					drawFactorLineBetweenRects(debugLines, tracks[childIndex].lastMeasurement().rect,
						tracks[trackIndex].lastMeasurement().rect, factor, c, 2);
#endif
				} else {
					tracks[trackIndex].addMeasurement(
						Measurement(measurements[measurementIndex].first, measurements[measurementIndex].second, trackFrame));
				}

				if (showMatches) {
					cv::Mat drawing = fd.image.clone();
					cv::rectangle(drawing, measurements[measurementIndex].first, cv::Scalar(0, 255, 255), 2);
					cv::rectangle(drawing, tracks[trackIndex].lastMeasurement().rect, cv::Scalar(0, 0, 0));
					while (true) {
						imshow("matching", drawing);
						int wk = cv::waitKey(0);
						if (wk == RETURN) {
							break;
						}
						if (wk == ESC) {
							showMatches = false;
							break;
						}
					}
				}
			}
		} else {
			for (std::set<Node>::iterator jt = it->second.begin(); jt != it->second.end(); jt++) {
				if (!jt->isType()) {
					std::cerr << "also should not happen" << std::endl;
					throw "";
				}
			}
		}
	}

	int tfSize = tracks.size();
	for (size_t i = 0; i < tfSize; i++) {
		if (tracks[i].updateRetired() && !tracks.isSubgraphActiveOrProcessed(i)) {
			tracks.markSubgraphProcessed(i);
			if (!tracks.isSubgraphLengthSmall(i) && !tracks.isStaticSubgraph(i)) {
				int rootIndex = tracks.filterShortExpands(i);
				tfw.saveSubgraph(tracks.get(rootIndex), factor);
				fd.numOfSubgraphs++;
				tracks.addEraseSubgraph(rootIndex);
			} else {
				tracks.addEraseSubgraph(i);
			}
		}
	}
	tracks.commitErases();

	tracks.checkGraph(true);

	if (showTracks) {
		cv::Mat drawing = fd.frame.clone();
		double factor = fd.factor;

		std::cerr << "showing tracks..." << std::endl;
		int myCnt = 0;
		for (int i = 0; i < tracks.size(); i++) {
			if (tracks[i].isRoot()) {
				std::map<track_ptr, bool> used;
				drawTrack(drawing, tracks.get(i), factor, used);
			}
			std::cerr << ++myCnt << " of " << tracks.size() << std::endl;
		}

		static WindowManager wmt("Tracks");
		wmt.showImage(drawing, cv::Size(800, 600));

		while (true) {
			int wk = cv::waitKey(0);
			if (wk == RETURN) {
				break;
			}
			if (wk == ESC) {
				showTracks = false;
				break;
			}
		}

		wmt.close();
	}

#if DEBUG_PLAYER_TRACKING
	cv::Mat drawing = fd.frame.clone();

	for (int i = 0; i < tracks.size(); i++) {
		if (tracks[i].isActive()) {
			cv::Rect rm = tracks[i].lastMeasurement().rect;
			rm.x /= factor;
			rm.y /= factor;
			rm.width /= factor;
			rm.height /= factor;
			cv::rectangle(drawing, rm, cv::Scalar(0, 255, 0), 2);
		}
	}

	static WindowManager wmt("Tracking");
	wmt.showImage(drawing, cv::Size(800, 600));

	static WindowManager dd("Debug tracking");
	dd.showImage(debugDrawing, cv::Size(800, 600));
	/*static WindowManager dl("Debug lines");
	dl.showImage(debugLines, cv::Size(800, 600));*/
#endif

	if (finishTracking) {
		int tfSize = tracks.size();
		for (size_t i = 0; i < tfSize; i++) {
			if (!tracks[i].isProcessed()) {
				tracks.markSubgraphProcessed(i);
				if (!tracks.isSubgraphLengthSmall(i) && !tracks.isStaticSubgraph(i)) {
					int rootIndex = tracks.filterShortExpands(i);
					tfw.saveSubgraph(tracks.get(rootIndex), factor);
					fd.numOfSubgraphs++;
					tracks.addEraseSubgraph(rootIndex);
				} else {
					tracks.addEraseSubgraph(i);
				}
			}
		}
		tracks.commitErases();

		if (tracks.size() != 0) {
			std::cout << "Size (should be 0):" << tracks.size() << std::endl;
			throw "";
		}

		tracks.checkGraph(true);

		tfw.closeFiles();
	}
}
