#include "background_subtraction.h"
#include <direct.h>
#include "field_model.h"
#include "field_tracking.h"
#include "frame_data.h"
#include "globals.h"
#include "inputs.h"
#include <iostream>
#include "player_detection.h"
#include "player_tracking.h"
#include "player_paths.h"
#include "preprocessing.h"
#include "show_paths.h"
#include "video_manager.h"
#include "window_manager.h"

#define USE_TEST_HOMOGRAPHY 1
#define TRACK_FIELD 0
#define SHOW_PATHS_ONLY 1

// TODO dodat ovdje time, fps..
// TODO dodat opise globalsa i kako kontrolirati

int main(int argc, char **argv) {
	if (argc != 2) {
		std::cout << "One argument expected: a path to the config file; (got " << argc << ")." << std::endl;
		system("pause");
		return -1;
	}
	loadInputs(argv[1]);

	_mkdir(tmpDirectory.c_str()); // create temporary directory

#if SHOW_PATHS_ONLY
	finishTracking = true;
#else

	// ###### init system components ######

	VideoManager vm(videoPath);

	FieldModel fm(fieldModelScale);
	FrameData fd;
	BackgroundEliminator be;
	FieldTracker ft;
	PlayerTracker pt(vm.getVideoCapture());

	// ####################################

	bool initialized = false;

	vm.nextFrame();

	while (true) {
		fd.frame = vm.getFrame().clone();

		if (vm.nextFrame()) {
			finishTracking = true;
		}

		preprocess(fd);

		be.detectForeground(fd);

		if (!initialized) {
#if USE_TEST_HOMOGRAPHY
			ft.init(fd, fm, testHomography);
#else
			ft.init(fd, fm);
#endif
		}

#if TRACK_FIELD
		ft.calculateHomography(fd, fm);
#else
		fd.homography = testHomography;
#endif
		
		detectPlayers(fd);

		pt.track(fd);

		initialized = true;
		
		if (finishTracking) {
			break;
		}

		int key = cv::waitKey(1);
		if (key == ESC) break;
		processGlobalKeyInput(key);
	}

	if (finishTracking) {
		fd.teamModel.train();

		PlayerPaths pp;
		pp.storePaths(fm, fd);
	}
#endif

	if (finishTracking) {
		showPaths();
	}

	// TODO delete temporary directory
}
