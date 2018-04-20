#pragma once

#include "preprocessing.h"
#include "tmp_files_write.h"
#include "track.h"
#include "tracks_filtering.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

class PlayerTracker {
	TracksFilter tracks;
	TmpFilesWriter tfw;

	int trackCountdown = initTrackFrames;
	int trackFrame = 0;
public:
	PlayerTracker(cv::VideoCapture &vc);
	void track(FrameData &fd, bool debug = false);
};

void testFindMinimalEdgeCover();
