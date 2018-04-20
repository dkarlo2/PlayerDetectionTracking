#include "inputs.h"
#include <queue>
#include "tracks_filtering.h"
#include "util.h"

long long trackIdGenerator = 0;

bool TracksFilter::iIsSubgraphActiveOrProcessed(track_ptr track, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return false;
	}
	used[track] = true;

	if ((*track)->isActive() || (*track)->isProcessed()) {
		return true;
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		if (iIsSubgraphActiveOrProcessed(parents[i], used)) {
			return true;
		}
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		if (iIsSubgraphActiveOrProcessed(children[i], used)) {
			return true;
		}
	}

	return false;
}

void TracksFilter::iMarkSubgraphProcessed(track_ptr track, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return;
	}
	used[track] = true;

	(*track)->setProcessed();

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		iMarkSubgraphProcessed(parents[i], used);
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		iMarkSubgraphProcessed(children[i], used);
	}
}

void TracksFilter::iEraseTrack(track_ptr track) {
	std::vector<track_ptr>::iterator position = std::find(tracks.begin(), tracks.end(), track);
	if (position == tracks.end()) {
		std::cerr << "hohohooo" << std::endl;
		throw "";
	}
	tracks.erase(position);
	track->reset();
}

void TracksFilter::iEraseSubgraph(track_ptr track, std::map<track_ptr, bool> &used) {
	if (!(*track) || used[track]) {
		return;
	}
	used[track] = true;

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		iEraseSubgraph(children[i], used);
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		iEraseSubgraph(parents[i], used);
	}

	iEraseTrack(track);
}

bool TracksFilter::iIsMaxLengthToRootSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo4) {
	if (memo4.find(track) != memo4.end()) {
		return memo4[track];
	}
	int d = (*track)->getMeasurements().size();
	acc += d;
	if (acc > shortSubgraph) {
		memo4[track] = false;
		return false;
	}
	if ((*track)->isRoot()) {
		memo4[track] = true;
		return true;
	}
	std::vector<track_ptr> parents = (*track)->getParents();
	bool b = true;
	for (int i = 0; i < parents.size(); i++) {
		b &= iIsMaxLengthToRootSmall(track, acc, memo4);
		if (!b) {
			break;
		}
	}
	memo4[track] = b;
	return b;
}

bool TracksFilter::iIsMaxLengthToLeafSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo3,
	std::map<track_ptr, bool> &memo4)
{
	if (memo3.find(track) != memo3.end()) {
		return memo3[track];
	}
	int d = (*track)->getMeasurements().size();
	acc += d;
	if (acc > shortSubgraph) {
		memo3[track] = false;
		return false;
	}
	if ((*track)->isLeaf()) {
		bool b = iIsMaxLengthToRootSmall(track, 0, memo4);
		memo3[track] = b;
		return b;
	}
	std::vector<track_ptr> children = (*track)->getChildren();
	bool b = true;
	for (int i = 0; i < children.size(); i++) {
		b &= iIsMaxLengthToLeafSmall(children[i], acc, memo3, memo4);
		if (!b) {
			break;
		}
	}
	memo3[track] = b;
	return b;
}

bool TracksFilter::iIsMaxLengthOfRootsSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo2,
	std::map<track_ptr, bool> &memo3, std::map<track_ptr, bool> &memo4)
{
	if (memo2.find(track) != memo2.end()) {
		return memo2[track];
	}
	int d = (*track)->getMeasurements().size();
	acc += d;
	if (acc > shortSubgraph) {
		memo2[track] = false;
		return false;
	}
	if ((*track)->isRoot()) {
		bool b = iIsMaxLengthToLeafSmall(track, 0, memo3, memo4);
		memo2[track] = b;
		return b;
	}
	std::vector<track_ptr> parents = (*track)->getParents();
	bool b = true;
	for (int i = 0; i < parents.size(); i++) {
		b &= iIsMaxLengthOfRootsSmall(track, acc, memo2, memo3, memo4);
		if (!b) {
			break;
		}
	}
	memo2[track] = b;
	return b;
}

bool TracksFilter::iIsSubgraphLengthSmall(track_ptr track, int acc, std::map<track_ptr, bool> &memo1, std::map<track_ptr, bool> &memo2,
	std::map<track_ptr, bool> &memo3, std::map<track_ptr, bool> &memo4)
{
	if (memo1.find(track) != memo1.end()) {
		return memo1[track];
	}
	int d = (*track)->getMeasurements().size();
	acc += d;
	if (acc > shortSubgraph) {
		memo1[track] = false;
		return false;
	}
	if ((*track)->isLeaf()) {
		bool b = iIsMaxLengthOfRootsSmall(track, 0, memo2, memo3, memo4);
		memo1[track] = b;
		return b;
	}
	std::vector<track_ptr> children = (*track)->getChildren();
	bool b = true;
	for (int i = 0; i < children.size(); i++) {
		b &= iIsSubgraphLengthSmall(children[i], acc, memo1, memo2, memo3, memo4);
		if (!b) {
			break;
		}
	}
	memo1[track] = b;
	return b;
}

int TracksFilter::iGetSubgraphLength(track_ptr track, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return 0;
	}
	used[track] = true;

	int l = (*track)->getMeasurements().size();

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		l += iGetSubgraphLength(parents[i], used);
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		l += iGetSubgraphLength(children[i], used);
	}

	return l;
}

std::pair<double, double> TracksFilter::iGetSubgraphMean(track_ptr track, int length, std::map<track_ptr, bool> &used) {
	std::pair<double, double> mean = std::make_pair(0, 0);
	if (used[track]) {
		return mean;
	}
	used[track] = true;

	std::vector<Measurement> measurements = (*track)->getMeasurements();
	for (int i = 0; i < measurements.size(); i++) {
		cv::Rect r = measurements[i].rect;
		double x = r.x + r.width / 2.;
		double y = r.y + r.height / 2.;
		mean.first += x / length;
		mean.second += y / length;
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		std::pair<double, double> m = iGetSubgraphMean(parents[i], length, used);
		mean.first += m.first;
		mean.second += m.second;
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		std::pair<double, double> m = iGetSubgraphMean(children[i], length, used);
		mean.first += m.first;
		mean.second += m.second;
	}

	return mean;
}

double TracksFilter::iGetSubgraphStdDevSq(track_ptr track, int length, std::pair<double, double> mean, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return 0;
	}
	used[track] = true;

	double stdDev = 0;

	std::vector<Measurement> measurements = (*track)->getMeasurements();
	for (int i = 0; i < measurements.size(); i++) {
		cv::Rect r = measurements[i].rect;
		double x = r.x + r.width / 2.;
		double y = r.y + r.height / 2.;
		stdDev += ((x - mean.first) * (x - mean.first) + (y - mean.second) * (y - mean.second)) / length;
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		stdDev += iGetSubgraphStdDevSq(parents[i], length, mean, used);
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		stdDev += iGetSubgraphStdDevSq(children[i], length, mean, used);
	}

	return stdDev;
}

bool TracksFilter::iIsStaticSubgraph(track_ptr track) {
	std::map<track_ptr, bool> used1;
	int length = iGetSubgraphLength(track, used1);

	std::map<track_ptr, bool> used2;
	std::pair<double, double> mean = iGetSubgraphMean(track, length, used2);

	std::map<track_ptr, bool> used3;
	double stdDev = sqrt(iGetSubgraphStdDevSq(track, length, mean, used3));

	return stdDev <= staticSubgraph;
}

void TracksFilter::iFilterShortExpandsForRoot(track_ptr track, std::map<track_ptr, bool> &used) {
	std::queue<track_ptr> q;
	q.push(track);

	while (!q.empty()) {
		track_ptr t = q.front();
		q.pop();

		if (used[t]) {
			continue;
		}
		used[t] = true;

		std::vector<track_ptr> children = (*t)->getChildren();
		bool b = false;
		track_ptr grandchild = nullptr;
		if (children.size() > 1) { // TODO mozda nesto tipa ipak == 2
			for (int i = 0; i < children.size(); i++) {
				track_ptr c = children[i];
				std::vector<track_ptr> cs = (*c)->getChildren();
				if (cs.size() == 1) {
					if (grandchild == nullptr) {
						grandchild = cs[0];
					}
					else if (grandchild != cs[0]) {
						b = true;
						break;
					}
				}
				else {
					b = true;
					break;
				}
			}
		}
		else {
			b = true;
		}
		if (b) {
			for (int i = 0; i < children.size(); i++) {
				track_ptr c = children[i];
				q.push(c);
			}
		} else {
			int maxMsSize = 0;
			for (int k = 0; k < children.size(); k++) {
				maxMsSize = std::max(maxMsSize, (int)(*children[k])->getMeasurements().size());
			}
			//if (maxMsSize < 10) { // TODO
			// TODO takoðer provjera jesu li djeca meðusobno bliska
			std::vector<Measurement> newMeasurements;
			for (int m = 0; m < maxMsSize; m++) {
				cv::Point2d ps;
				cv::Size ss;
				int s = children.size();
				int measurementFrame = 0;
				for (int k = 0; k < s; k++) {
					std::vector<Measurement> measurements = (*children[k])->getMeasurements();
					int mReal = std::min((int)round(m * measurements.size() / (double)maxMsSize), (int)measurements.size() - 1);
					cv::Point2d p = getRectCenter(measurements[mReal].rect);
					ps += p;
					ss += measurements[mReal].rect.size();
					measurementFrame += measurements[mReal].frame;
				}
				measurementFrame /= s;
				ps /= s;
				ss /= s;
				std::vector<float> emptyFeatures;
				newMeasurements.push_back(Measurement(cv::Rect(ps, ss), emptyFeatures, measurementFrame));
			}
			for (int k = 0; k < children.size(); k++) {
				(*grandchild)->clearParent(children[k]);
				toEraseTrack.push_back(children[k]);
			}
			(*t)->clearChildren();
			if (newMeasurements.size() == 0) {
				std::cerr << "kako je 0" << std::endl;
				system("pause");
			}
			track_ptr newChild = std::make_shared<unique_ptr<Track>>(new Track(newMeasurements, grandchild));
			tracks.push_back(newChild);

			(*t)->addChild(newChild);
			(*newChild)->addParent(t);
			(*grandchild)->addParent(newChild);

			q.push(newChild);
			/*for (int k = 0; k < newMeasurements.size(); k++) {
			t->addMeasurement(newMeasurements[k]);
			}
			t->addChild(grandchild);
			q.push(grandchild);*/
			//}
		}
	}
}

int TracksFilter::iFilterShortExpandsInSubgraph(track_ptr track, std::map<track_ptr, int> &memo, std::map<track_ptr, bool> &used) {
	if (memo.find(track) != memo.end()) {
		return memo[track];
	}
	memo[track] = -1;

	int rootIndex = -1;
	
	if ((*track)->isRoot()) {
		rootIndex = std::find(tracks.begin(), tracks.end(), track) - tracks.begin();
		iFilterShortExpandsForRoot(track, used);
		std::map<track_ptr, bool> usedConnect;
		iConnectTracksForRoot(track, usedConnect);
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	for (int i = 0; i < parents.size(); i++) {
		int ri = iFilterShortExpandsInSubgraph(parents[i], memo, used);
		if (rootIndex == -1) {
			rootIndex = ri;
		}
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		int ri = iFilterShortExpandsInSubgraph(children[i], memo, used);
		if (rootIndex == -1) {
			rootIndex = ri;
		}
	}

	memo[track] = rootIndex;
	return rootIndex;
}

void TracksFilter::iConnectTracksForRoot(track_ptr track, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return;
	}
	used[track] = true;

	std::vector<track_ptr> children = (*track)->getChildren();
	if (children.size() == 1) {
		if ((*children[0])->getParents().size() > 1) {
			iConnectTracksForRoot(children[0], used);
			return;
		}

		std::vector<Measurement> ms = (*children[0])->getMeasurements();
		for (int i = 0; i < ms.size(); i++) {
			(*track)->addMeasurement(ms[i]);
		}

		(*track)->clearChildren();
		std::vector<track_ptr> cs = (*children[0])->getChildren();
		for (int i = 0; i < cs.size(); i++) {
			(*track)->addChild(cs[i]);
			(*cs[i])->clearParent(children[0]);
			(*cs[i])->addParent(track);
		}

		(*track)->setActive((*children[0])->isActive());
		if ((*children[0])->isRetiring()) {
			(*track)->retire((*children[0])->getFramesToRetire());
		}

		toEraseTrack.push_back(children[0]);

		used[track] = false;
		iConnectTracksForRoot(track, used);
	} else {
		for (int i = 0; i < children.size(); i++) {
			iConnectTracksForRoot(children[i], used);
		}
	}
}

int TracksFilter::iCheckTracks(track_ptr track, std::map<track_ptr, int> &memo, std::map<track_ptr, bool> &used) {
	if (used[track]) {
		return 0;
	}
	used[track] = true;

	if (memo.find(track) != memo.end()) {
		return memo[track];
	}

	if (!(*track)) {
		memo[track] = 3;
		return 3;
	}

	if (std::find(tracks.begin(), tracks.end(), track) == tracks.end()) {
		memo[track] = 1;
		return 1;
	}

	std::vector<track_ptr> parents = (*track)->getParents();
	size_t pcs = 0;
	for (int i = 0; i < parents.size(); i++) {
		if (!(*parents[i])) {
			continue;
		}
		pcs = std::max(pcs, (*parents[i])->getChildren().size());
	}

	if (pcs > 1 && parents.size() > 1) {
		memo[track] = 2;
		return 2;
	}

	for (int i = 0; i < parents.size(); i++) {
		int ct = iCheckTracks(parents[i], memo, used);
		if (ct) {
			return ct;
		}
	}

	std::vector<track_ptr> children = (*track)->getChildren();
	for (int i = 0; i < children.size(); i++) {
		int ct = iCheckTracks(children[i], memo, used);
		if (ct) {
			return ct;
		}
	}

	memo[track] = 0;
	return 0;
}

bool TracksFilter::isSubgraphActiveOrProcessed(size_t i) {
	std::map<track_ptr, bool> used;
	return iIsSubgraphActiveOrProcessed(tracks[i], used);
}

void TracksFilter::markSubgraphProcessed(size_t i) {
	std::map<track_ptr, bool> used;
	iMarkSubgraphProcessed(tracks[i], used);
}

void TracksFilter::addEraseSubgraph(size_t i) {
	toEraseSubgraph.push_back(tracks[i]);
}

void TracksFilter::commitErases() {
	for (int i = 0; i < toEraseSubgraph.size(); i++) {
		std::map<track_ptr, bool> used;
		iEraseSubgraph(toEraseSubgraph[i], used);
	}
	toEraseSubgraph.clear();

	for (int i = 0; i < toEraseTrack.size(); i++) {
		iEraseTrack(toEraseTrack[i]);
	}
	toEraseTrack.clear();
}

bool TracksFilter::isSubgraphLengthSmall(size_t i) {
	std::map<track_ptr, bool> memo1;
	std::map<track_ptr, bool> memo2;
	std::map<track_ptr, bool> memo3;
	std::map<track_ptr, bool> memo4;

	return iIsSubgraphLengthSmall(tracks[i], 0, memo1, memo2, memo3, memo4);
}

bool TracksFilter::isStaticSubgraph(size_t i) {
	return iIsStaticSubgraph(tracks[i]);
}

int TracksFilter::filterShortExpands(size_t i) {
	std::map<track_ptr, int> memo;
	std::map<track_ptr, bool> used;
	return iFilterShortExpandsInSubgraph(tracks[i], memo, used);
}

int TracksFilter::checkGraph(bool pauseIfFails) {
	std::map<track_ptr, int> memo;
	for (int i = 0; i < tracks.size(); i++) {
		if ((*tracks[i])->isRoot()) {
			std::map<track_ptr, bool> used;
			int ct = iCheckTracks(tracks[i], memo, used);
			if (ct) {
				if (pauseIfFails) {
					std::cerr << "Graph status is " << ct << std::endl;
				}
				return ct;
			}
		}
	}
	return 0;
}

int TracksFilter::getSubgraphLength(size_t i) {
	std::map<track_ptr, bool> used;
	return iGetSubgraphLength(tracks[i], used);
}
