#include "globals.h"
#include "inputs.h"
#include "tmp_files_write.h"

#include <iostream>
#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/tracking.hpp>

#define rc_ccp reinterpret_cast<const char*>

void TmpFilesWriter::initFiles(cv::VideoCapture &vc) {
	initVideoFile(vc);
	initTracksFile();
	initHomographiesFile();
}

void TmpFilesWriter::initVideoFile(cv::VideoCapture &vc) {
	// int ex = static_cast<int>(vc.get(CV_CAP_PROP_FOURCC));
	int ex = 0;
	int fps = vc.get(CV_CAP_PROP_FPS);
	cv::Size size = cv::Size((int)vc.get(CV_CAP_PROP_FRAME_WIDTH), (int)vc.get(CV_CAP_PROP_FRAME_HEIGHT));
	videoWriter.open(videoFilePath, ex, fps, size, true);

	if (!videoWriter.isOpened()) {
		std::cerr << "Could not open the temporary video for write." << std::endl;
		throw "";
	}
}

void TmpFilesWriter::initTracksFile() {
	std::ofstream file;
	file.open(tracksFilePath, std::ios::out | std::ios::binary);
	file.close();
}

void TmpFilesWriter::initHomographiesFile() {
	std::ofstream file;
	file.open(homographiesFilePath, std::ios::out | std::ios::binary);
	file.close();
}

void TmpFilesWriter::saveVideoFrame(cv::Mat frame) {
	videoWriter << frame;
}

void TmpFilesWriter::saveTrack(track_ptr track, double factor, std::ofstream &file) {
	long long id = (*track)->getId();
	file.write(rc_ccp(&id), sizeof(long long));

	std::vector<Measurement> ms = (*track)->getMeasurements();
	size_t msSize = ms.size();
	file.write(rc_ccp(&msSize), sizeof(size_t));
	for (int j = 0; j < ms.size(); j++) {
		int frame = ms[j].frame;
		int x = ms[j].rect.x / factor;
		int y = ms[j].rect.y / factor;
		int w = ms[j].rect.width / factor;
		int h = ms[j].rect.height / factor;

		file.write(rc_ccp(&frame), sizeof(int));
		file.write(rc_ccp(&x), sizeof(int));
		file.write(rc_ccp(&y), sizeof(int));
		file.write(rc_ccp(&w), sizeof(int));
		file.write(rc_ccp(&h), sizeof(int));

		size_t fsSize = ms[j].features.size();
		file.write(rc_ccp(&fsSize), sizeof(size_t));
		for (int k = 0; k < fsSize; k++) {
			float f = ms[j].features[k];
			file.write(rc_ccp(&f), sizeof(float));
		}
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	size_t childrenSize = children.size();
	file.write(rc_ccp(&childrenSize), sizeof(size_t));
	for (int j = 0; j < children.size(); j++) {
		long long id = (*children[j])->getId();
		file.write(rc_ccp(&id), sizeof(long long));
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	size_t parentsSize = parents.size();
	file.write(rc_ccp(&parentsSize), sizeof(size_t));
	for (int j = 0; j < parents.size(); j++) {
		long long id = (*parents[j])->getId();
		file.write(rc_ccp(&id), sizeof(long long));
	}

	bool active = (*track)->isActive();
	int framesToRetire = (*track)->getFramesToRetire();
	int startFrame = (*track)->getStartFrame();
	int endFrame = (*track)->getEndFrame();

	file.write(rc_ccp(&active), sizeof(bool));
	file.write(rc_ccp(&framesToRetire), sizeof(int));
	file.write(rc_ccp(&startFrame), sizeof(int));
	file.write(rc_ccp(&endFrame), sizeof(int));
}

void TmpFilesWriter::iSaveSubgraph(track_ptr track, double factor, std::ofstream &file,
	std::map<track_ptr, bool> &used)
{
	if (used[track]) {
		return;
	}

	used[track] = true;

	saveTrack(track, factor, file);

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		iSaveSubgraph(parents[i], factor, file, used);
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		iSaveSubgraph(children[i], factor, file, used);
	}
}

void TmpFilesWriter::saveSubgraph(track_ptr track, double factor) {
	std::ofstream file;
	file.open(tracksFilePath, std::ios::out | std::ios::app | std::ios::binary);

	bool cntn = true;
	file.write(rc_ccp(&cntn), sizeof(bool));

	std::map<track_ptr, bool> used;
	iSaveSubgraph(track, factor, file, used);

	long long id = -1;
	file.write(rc_ccp(&id), sizeof(long long));

	file.close();
}

void TmpFilesWriter::saveHomography(cv::Mat homography, int trackFrame, double factor) {
	std::ofstream file;
	file.open(homographiesFilePath, std::ios::out | std::ios::binary | std::ios::app);

	double h00 = homography.at<double>(0, 0);
	double h01 = homography.at<double>(0, 1);
	double h02 = homography.at<double>(0, 2);
	double h10 = homography.at<double>(1, 0);
	double h11 = homography.at<double>(1, 1);
	double h12 = homography.at<double>(1, 2);
	double h20 = homography.at<double>(2, 0) * factor;
	double h21 = homography.at<double>(2, 1) * factor;
	double h22 = homography.at<double>(2, 2) * factor;

	file.write(rc_ccp(&trackFrame), sizeof(int));

	file.write(rc_ccp(&h00), sizeof(double));
	file.write(rc_ccp(&h01), sizeof(double));
	file.write(rc_ccp(&h02), sizeof(double));
	file.write(rc_ccp(&h10), sizeof(double));
	file.write(rc_ccp(&h11), sizeof(double));
	file.write(rc_ccp(&h12), sizeof(double));
	file.write(rc_ccp(&h20), sizeof(double));
	file.write(rc_ccp(&h21), sizeof(double));
	file.write(rc_ccp(&h22), sizeof(double));

	file.close();
}

void TmpFilesWriter::closeFiles() {
	closeVideoFile();
	closeTracksFile();
	closeHomographiesFile();
}

void TmpFilesWriter::closeVideoFile() {
	videoWriter.release();
}

void TmpFilesWriter::closeTracksFile() {
	std::ofstream file;
	file.open(tracksFilePath, std::ios::out | ::ios::app | std::ios::binary);

	bool cntn = false;
	file.write(rc_ccp(&cntn), sizeof(bool));

	file.close();
}

void TmpFilesWriter::closeHomographiesFile() {
	std::ofstream file;
	file.open(homographiesFilePath, std::ios::out | std::ios::binary | std::ios::app);

	int trackFrame = -1;
	file.write(rc_ccp(&trackFrame), sizeof(int));

	file.close();
}
