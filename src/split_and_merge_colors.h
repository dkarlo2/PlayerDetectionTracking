/*
	Defines class for executing Split and Merge algorithm, which is used for
	finding the best color background and foreground separation (simulates knee method).
*/

#ifndef SPLIT_AND_MERGE_COLORS_H
#define SPLIT_AND_MERGE_COLORS_H

#include "globals.h"
#include <vector>

template <class T>
using SAMCSimilarityFunc = std::function<double(std::vector<T>)>;

template <class T>
class SplitAndMergeColors {
    SAMCSimilarityFunc<T> similarity;
    double splitThresh;
    double mergeThresh;
    std::vector<T> data;
    std::vector< std::pair<int, int> > intervals;

    void internalSplit(int begin, int end);
    void internalMerge(int maxGroups);
public:
    SplitAndMergeColors(SAMCSimilarityFunc<T> S, double ST, double MT) : similarity(S), splitThresh(ST), mergeThresh(MT) {}
    void addPoint(T value) {
        data.push_back(value);
    }
    std::vector<T> getData() {
        return data;
    }
    std::vector< std::pair<int, int> > execute(int maxGroups = 0);
};

template <class T>
std::vector< std::pair<int, int> > SplitAndMergeColors<T>::execute(int maxGroups) {
    intervals.clear();
    internalSplit(0, data.size()-1);
    internalMerge(maxGroups);
    return intervals;
}

template <class T>
void SplitAndMergeColors<T>::internalSplit(int begin, int end) {
    if (begin == end) {
        intervals.push_back(std::make_pair(begin, end));
        return;
    }
    double s = similarity(std::vector<T>(data.begin() + begin, data.begin() + end));
    if (s >= splitThresh) {
        intervals.push_back(std::make_pair(begin, end));
        return;
    }
    int middle = (begin + end) / 2;
    internalSplit(begin, middle);
    internalSplit(middle+1, end);
}

template <class T>
void SplitAndMergeColors<T>::internalMerge(int maxGroups) {
    /*std::vector< std::pair<int, int> > mergedIntervals;
    std::pair<int, int> merged = intervals[0];
    for (int i = 1; i < intervals.size(); i++) {
        int l = merged.first;
        int d = intervals[i].second;
        if (similarity(std::vector<T>(data.begin() + l, data.begin() + d)) >= similarityThresh) {
            merged.second = d;
        } else {
            mergedIntervals.push_back(merged);
            merged = intervals[i];
        }
    }
    mergedIntervals.push_back(merged);
    intervals = mergedIntervals;*/

    while (maxGroups && intervals.size() > maxGroups) {
        double sim = similarity(std::vector<T>(data.begin() + intervals[0].first, data.begin() + intervals[1].second));
        if (sim < mergeThresh) {
            break;
        }
#if DEBUG_SPLIT_AND_MERGE_COLORS
            std::cerr << "Merging " << intervals[0].first << '-' << intervals[0].second << " and "
                << intervals[1].first << '-' << intervals[1].second << ' ' << sim << std::endl;
#endif
        std::pair<int, int> newInt(intervals[0].first, intervals[1].second);
        intervals.erase(intervals.begin());
        intervals.erase(intervals.begin());
        intervals.insert(intervals.begin(), newInt);
    }
}

#endif
