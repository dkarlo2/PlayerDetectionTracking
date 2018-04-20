#pragma once

#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <vector>

#define track_ptr std::shared_ptr<std::unique_ptr<Track>>

extern long long trackIdGenerator;

class Measurement {
public:
	cv::Rect rect;
	std::vector<float> features;
	int frame;
	Measurement(cv::Rect r, std::vector<float> fs, int f) : rect(r), features(fs), frame(f) {}
};

class Track {
	long long trackId;

	std::vector<Measurement> measurements;

	std::vector<track_ptr> children;
	std::vector<track_ptr> parents;

	bool active;
	int framesToRetire;

	int startFrame;
	int endFrame;

	bool processed = false;
public:
	Track(Measurement init) : trackId(trackIdGenerator++), active(true), framesToRetire(-1) {
		measurements.push_back(init);
		startFrame = init.frame;
		endFrame = startFrame;
		if (children.size() != 0) {
			std::cerr << "tu smo" << std::endl;
			system("pause");
		}
	}
	Track(std::vector<Measurement> ms, track_ptr child) : trackId(trackIdGenerator++), active(false), framesToRetire(0) {
		for (int i = 0; i < ms.size(); i++) {
			measurements.push_back(ms[i]);
		}
		children.push_back(child);
		if (measurements.size() == 0) {
			std::cerr << "ma kako" << std::endl;
			system("pause");
		} else {
			startFrame = measurements[0].frame;
			endFrame = measurements[measurements.size() - 1].frame;
		}
	}

	Track(long long id, std::vector<Measurement> ms, bool a, int ftr, int sf, int ef) : trackId(id), active(a), framesToRetire(ftr), startFrame(sf), endFrame(ef) {
		for (int i = 0; i < ms.size(); i++) {
			measurements.push_back(ms[i]);
		}
		if (measurements.size() == 0) {
			std::cerr << "ma kakoo" << std::endl;
			system("pause");
		}
	}

	int getId() {
		return trackId;
	}

	int getStartFrame() {
		return startFrame;
	}
	int getEndFrame() {
		return endFrame;
	}
	bool isActive() {
		return active;
	}
	void setActive(bool a) {
		active = a;
	}
	Measurement lastMeasurement() {
		return measurements[measurements.size() - 1];
	}
	void addChild(track_ptr c) {
		children.push_back(c);
		if ((*c)->getMeasurements().size() == 0) {
			std::cerr << "oho" << std::endl;
			throw "";
		}
	}
	void addParent(track_ptr p) {
		parents.push_back(p);
	}
	void retire(int frames = 0) {
		framesToRetire = frames;
	}
	void stopRetire() {
		framesToRetire = -1;
	}
	bool isRetiring() {
		return framesToRetire >= 0;
	}
	bool updateRetired() {
		if (!active) {
			return false;
		}

		if (framesToRetire == 0) {			
			active = false;
			return true;
		}

		if (framesToRetire >= 0) {
			framesToRetire--;
		}

		return false;
	}
	void addMeasurement(Measurement m) {
		/*if (!active) {
		std::cerr << "Cannot assign measurement to an inactive track" << std::endl;
		throw "";
		return;
		}*/
		measurements.push_back(m);
		endFrame = m.frame;
	}
	std::vector<Measurement> getMeasurements() {
		return measurements;
	}
	bool isRoot() {
		return parents.empty();
	}
	bool isLeaf() {
		return children.empty();
	}
	std::vector<track_ptr> getChildren() {
		return children;
	}
	std::vector<track_ptr> getParents() {
		return parents;
	}
	void clearChildren() {
		children.clear();
	}
	void clearParent(track_ptr track) {
		auto position = std::find(parents.begin(), parents.end(), track);
		if (position == parents.end()) {
			std::cerr << "something's wrong" << std::endl;
			throw "";
		}
		parents.erase(position);
	}
	int getFramesToRetire() {
		return framesToRetire;
	}
	bool isProcessed() {
		return processed;
	}
	void setProcessed() {
		processed = true;
	}
};
