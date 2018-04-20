#include "time_measurement.h"

static std::chrono::milliseconds timeNow() {
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
}

void MyTime::start() {
	timeStart = timeNow();
}

long MyTime::time() {
	std::chrono::milliseconds timePast = timeNow() - timeStart;
	return timePast.count();
}