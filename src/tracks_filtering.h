#pragma once

#include <map>
#include <memory>
#include "track.h"

class TracksFilter {
	std::vector<track_ptr> tracks; // contains only tracks from active subgraphs

	std::vector<track_ptr> toEraseSubgraph;
	std::vector<track_ptr> toEraseTrack;

	bool iIsSubgraphActiveOrProcessed(track_ptr track, std::map<track_ptr, bool> &used);
	void iMarkSubgraphProcessed(track_ptr track, std::map<track_ptr, bool> &used);

	void iEraseTrack(track_ptr track);
	void iEraseSubgraph(track_ptr track, std::map<track_ptr, bool> &used);

	bool iIsMaxLengthToRootSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo4);
	bool iIsMaxLengthToLeafSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo3, std::map<track_ptr, bool> &memo4);
	bool iIsMaxLengthOfRootsSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo2, std::map<track_ptr, bool> &memo3,
		std::map<track_ptr, bool> &memo4);
	bool iIsSubgraphLengthSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo1, std::map<track_ptr, bool> &memo2,
		std::map<track_ptr, bool> &memo3, std::map<track_ptr, bool> &memo4);

	int iGetSubgraphLength(track_ptr track, std::map<track_ptr, bool> &used);
	std::pair<double, double> iGetSubgraphMean(track_ptr track, int length, std::map<track_ptr, bool> &used);
	double iGetSubgraphStdDevSq(track_ptr track, int length, std::pair<double, double> mean,
		std::map<track_ptr, bool> &used);
	bool iIsStaticSubgraph(track_ptr track);

	void iFilterShortExpandsForRoot(track_ptr track, std::map<track_ptr, bool> &used);
	int iFilterShortExpandsInSubgraph(track_ptr track, std::map<track_ptr, int> &memo, std::map<track_ptr, bool> &used);

	void iConnectTracksForRoot(track_ptr track, std::map<track_ptr, bool> &used);

	int iCheckTracks(track_ptr track, std::map<track_ptr, int> &memo, std::map<track_ptr, bool> &used);
public:
	bool empty() {
		return tracks.empty();
	}

	int size() {
		return tracks.size();
	}

	size_t addTrack(cv::Rect boundingRect, std::vector<float> features, int trackFrame) {
		track_ptr t = std::make_shared<unique_ptr<Track>>(new Track(
			Measurement(boundingRect, features, trackFrame)));
		tracks.push_back(t);
		return tracks.size() - 1;
	}

	Track& operator[](size_t i) {
		return *(*tracks[i]);
	}

	auto get(size_t i) {
		return tracks[i];
	}

	bool isSubgraphActiveOrProcessed(size_t i);

	void markSubgraphProcessed(size_t i);

	void addEraseSubgraph(size_t i);

	void commitErases();

	bool isSubgraphLengthSmall(size_t i);

	bool isStaticSubgraph(size_t i);

	/*
		returns index of any root node from that subgraph
		(we need some root because we are sure that it won't be deleted in the filtering process)
	*/
	int filterShortExpands(size_t i);

	int checkGraph(bool pauseIfFails);

	int getSubgraphLength(size_t i);
};
