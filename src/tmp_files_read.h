#pragma once

#include "globals.h"
#include "inputs.h"
#include "track.h"
#include "util.h"

#include <fstream>

class TmpFilesReader {
	std::string tracksFilePath;
	std::string homographiesFilePath;

	streampos tracksPos = 0;

	std::pair<track_ptr, std::pair<std::vector<long long>, std::vector<long long>>> loadTrack(std::ifstream &file);

public:
	TmpFilesReader() : tracksFilePath(makePath(tmpDirectory, tmpTrackFile)),
		homographiesFilePath(makePath(tmpDirectory, tmpHomographyFile)) {

		std::ifstream file;
		file.open(tracksFilePath, std::ios::in | std::ios::binary);
		file.seekg(0, std::ios::beg);
		tracksPos = file.tellg();
		file.close();
	}

	std::vector<track_ptr> readSubgraph();
	std::map<int, std::pair<cv::Mat, cv::Mat>> readHomographies();
};
