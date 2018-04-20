#pragma once

#include "field_model.h"
#include "frame_data.h"
#include "tmp_files_read.h"

class PlayerPaths {
	TmpFilesReader tfr;

public:
	void storePaths(FieldModel &fm, FrameData &fd);
};
