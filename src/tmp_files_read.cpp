#include <fstream>
#include <map>
#include "tmp_files_read.h"

#define rc_cp reinterpret_cast<char*>

std::pair<track_ptr, std::pair<std::vector<long long>, std::vector<long long>>> TmpFilesReader::loadTrack(std::ifstream &file) {
	long long id;
	file.read(rc_cp(&id), sizeof(long long));

	if (id == -1) {
		return std::make_pair(nullptr, std::make_pair(std::vector<long long>(), std::vector<long long>()));
	}

	std::vector<Measurement> ms;
	size_t msSize;
	file.read(rc_cp(&msSize), sizeof(size_t));
	for (int j = 0; j < msSize; j++) {
		cv::Rect r;
		int frame;
		file.read(rc_cp(&frame), sizeof(int));
		file.read(rc_cp(&r.x), sizeof(int));
		file.read(rc_cp(&r.y), sizeof(int));
		file.read(rc_cp(&r.width), sizeof(int));
		file.read(rc_cp(&r.height), sizeof(int));

		std::vector<float> features;
		size_t fsSize;
		file.read(rc_cp(&fsSize), sizeof(size_t));
		for (int k = 0; k < fsSize; k++) {
			float f;
			file.read(rc_cp(&f), sizeof(float));
			features.push_back(f);
		}

		ms.push_back(Measurement(r, features, frame));
	}

	std::vector<long long> cs;
	size_t csSize;
	file.read(rc_cp(&csSize), sizeof(size_t));
	for (int j = 0; j < csSize; j++) {
		long long c;
		file.read(rc_cp(&c), sizeof(long long));
		cs.push_back(c);
	}

	std::vector<long long> ps;
	size_t psSize;
	file.read(rc_cp(&psSize), sizeof(size_t));
	for (int j = 0; j < psSize; j++) {
		long long p;
		file.read(rc_cp(&p), sizeof(long long));
		ps.push_back(p);
	}

	bool active;
	file.read(rc_cp(&active), sizeof(bool));

	int framesToRetire;
	file.read(rc_cp(&framesToRetire), sizeof(int));

	int startFrame;
	file.read(rc_cp(&startFrame), sizeof(int));

	int endFrame;
	file.read(rc_cp(&endFrame), sizeof(int));

	track_ptr track = std::make_shared<std::unique_ptr<Track>>(new Track(id, ms, active, framesToRetire, startFrame, endFrame));
	return std::make_pair(track, std::make_pair(cs, ps));
}

std::vector<track_ptr> TmpFilesReader::readSubgraph() {
	std::vector<track_ptr> tracks;

	std::ifstream file;
	file.open(tracksFilePath, std::ios::in | std::ios::binary);

	file.seekg(tracksPos);

	bool cntn;
	file.read(rc_cp(&cntn), sizeof(bool));
	if (!cntn) {
		return tracks;
	}

	std::map<long long, track_ptr> tracksById;
	std::map<long long, std::pair<std::vector<long long>, std::vector<long long>>> trackFamily;

	int size = 0;

	while (true) {
		std::pair<track_ptr, std::pair<std::vector<long long>, std::vector<long long>>> trackAndFamily = loadTrack(file);
		if (trackAndFamily.first == nullptr) {
			break;
		}
		size++;
		tracks.push_back(trackAndFamily.first);
		long long id = (*trackAndFamily.first)->getId();
		tracksById[id] = trackAndFamily.first;
		trackFamily[id] = trackAndFamily.second;
	}

	for (int i = 0; i < size; i++) {
		long long id = (*tracks[i])->getId();
		std::vector<long long> cs = trackFamily[id].first;
		for (int j = 0; j < cs.size(); j++) {
			(*tracks[i])->addChild(tracksById[cs[j]]);
		}

		std::vector<long long> ps = trackFamily[id].second;
		for (int j = 0; j < ps.size(); j++) {
			(*tracks[i])->addParent(tracksById[ps[j]]);
		}
	}

	tracksPos = file.tellg();

	file.close();

	return tracks;
}

std::map<int, std::pair<cv::Mat, cv::Mat>> TmpFilesReader::readHomographies() {
	std::map<int, std::pair<cv::Mat, cv::Mat>> homographies;

	std::ifstream file;
	file.open(homographiesFilePath, std::ios::in | std::ios::binary);

	while (true) {
		int trackFrame;

		file.read(rc_cp(&trackFrame), sizeof(int));

		if (trackFrame == -1) {
			break;
		}

		double h00;
		double h01;
		double h02;
		double h10;
		double h11;
		double h12;
		double h20;
		double h21;
		double h22;

		file.read(rc_cp(&h00), sizeof(double));
		file.read(rc_cp(&h01), sizeof(double));
		file.read(rc_cp(&h02), sizeof(double));
		file.read(rc_cp(&h10), sizeof(double));
		file.read(rc_cp(&h11), sizeof(double));
		file.read(rc_cp(&h12), sizeof(double));
		file.read(rc_cp(&h20), sizeof(double));
		file.read(rc_cp(&h21), sizeof(double));
		file.read(rc_cp(&h22), sizeof(double));

		cv::Mat homography(3, 3, CV_64FC1);
		homography.at<double>(0, 0) = h00;
		homography.at<double>(0, 1) = h01;
		homography.at<double>(0, 2) = h02;
		homography.at<double>(1, 0) = h10;
		homography.at<double>(1, 1) = h11;
		homography.at<double>(1, 2) = h12;
		homography.at<double>(2, 0) = h20;
		homography.at<double>(2, 1) = h21;
		homography.at<double>(2, 2) = h22;

		homographies[trackFrame] = std::make_pair(homography, homography.inv());
	}

	file.close();

	return homographies;
}
