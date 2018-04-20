/*
	Time measuring helper class.
*/

#ifndef TIME_MEASUREMENT_H
#define TIME_MEASUREMENT_H

#include <chrono>

class MyTime {
    std::chrono::milliseconds timeStart;
public:
    void start();
    long time();
};

#endif
