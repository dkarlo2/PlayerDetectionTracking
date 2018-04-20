#include "globals.h"

bool showFieldModels = false;
bool showClusters = false;
bool showMatches = false;
bool showTracks = false;
bool finishTracking = false;

void processGlobalKeyInput(int key) {
	switch (key) {
	case 'f':
		showFieldModels = true;
		break;
	case 't':
		showClusters = true;
		break;
	case 'r':
		showMatches = true;
		break;
	case 's':
		showTracks = true;
		break;
	case 'p':
		finishTracking = true;
		break;
	}
}
