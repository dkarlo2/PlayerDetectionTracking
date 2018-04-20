#pragma once

#include "track.h"
#include "util.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/video.hpp>

class TmpFilesWriter {
	std::string videoFilePath;
	std::string tracksFilePath;
	std::string homographiesFilePath;

	cv::VideoWriter videoWriter;

	void initVideoFile(cv::VideoCapture &vc);
	void initTracksFile();
	void initHomographiesFile();

	void saveTrack(track_ptr track, double factor, std::ofstream &file);
	void iSaveSubgraph(track_ptr track, double factor, std::ofstream &file,
		std::map<track_ptr, bool> &used);

	void closeVideoFile();
	void closeTracksFile();
	void closeHomographiesFile();
public:
	TmpFilesWriter() : videoFilePath(makePath(tmpDirectory, tmpVideoFile)),
		tracksFilePath(makePath(tmpDirectory, tmpTrackFile)),
		homographiesFilePath(makePath(tmpDirectory, tmpHomographyFile)) {}

	void initFiles(cv::VideoCapture &vc);

	void saveVideoFrame(cv::Mat frame);
	void saveSubgraph(track_ptr track, double factor);
	void saveHomography(cv::Mat homography, int trackFrame, double factor);

	void closeFiles();
};
