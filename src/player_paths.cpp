#include <fstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include "player_paths.h"
#include "video_manager.h"
#include "window_manager.h"

#undef min
#undef max

#define filter_ptr std::shared_ptr<PlayerFilter>

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

long long playerIdGenerator = 0;

class PlayerFilter {
	long long id;

	cv::KalmanFilter kalman = cv::KalmanFilter(4, 2);
	float posX;
	float posY;
	double team;
	bool initialized = false;
public:
	PlayerFilter(double t) : id(playerIdGenerator++), team(t) {}

	long long getId() {
		return id;
	}

	void update(float x, float y) {
		if (!initialized) {
			kalman.transitionMatrix = (cv::Mat_<float>(4, 4) <<
				1, 0, 1, 0,
				0, 1, 0, 1,
				0, 0, 1, 0,
				0, 0, 0, 1);

			cv::setIdentity(kalman.measurementMatrix);
			cv::setIdentity(kalman.processNoiseCov, cv::Scalar::all(1e-5));
			cv::setIdentity(kalman.measurementNoiseCov, cv::Scalar::all(1e-2));
			cv::setIdentity(kalman.errorCovPost, cv::Scalar::all(1));

			randn(kalman.statePost, cv::Scalar::all(0), cv::Scalar::all(0.1));
			kalman.statePost.at<float>(0) = kalman.statePost.at<float>(0) + x;
			kalman.statePost.at<float>(1) = kalman.statePost.at<float>(1) + y;

			posX = x;
			posY = y;

			initialized = true;
			return;
		}

		cv::Mat prediction = kalman.predict();

		cv::Mat measurement = (cv::Mat_<float>(2, 1) << x, y);
		cv::Mat state = kalman.correct(measurement);

		posX = state.at<float>(0);
		posY = state.at<float>(1);
	}

	float getX() {
		return posX;
	}

	float getY() {
		return posY;
	}

	double getTeam() {
		return team;
	}
};

static void addNewActiveTrack(std::vector<std::shared_ptr<std::unique_ptr<Track>>> &activeTracks,
	std::map<std::shared_ptr<std::unique_ptr<Track>>, std::vector<filter_ptr>> &filters,
	std::map<std::shared_ptr<std::unique_ptr<Track>>, int> &emptyFramesNum,
	std::shared_ptr<std::unique_ptr<Track>> track,
	filter_ptr filter)
{
	if (std::find(activeTracks.begin(), activeTracks.end(), track) == activeTracks.end()) {
		activeTracks.push_back(track);
		emptyFramesNum[track] = 0;
	}
	if (filters.find(track) == filters.end()) {
		filters[track] = std::vector<filter_ptr>();
	}
	if (std::find(filters[track].begin(), filters[track].end(), filter) == filters[track].end()) {
		filters[track].push_back(filter);
	}
}

static filter_ptr addNewActiveTrack(std::vector<std::shared_ptr<std::unique_ptr<Track>>> &activeTracks,
	std::map<std::shared_ptr<std::unique_ptr<Track>>, std::vector<filter_ptr>> &filters,
	std::map<std::shared_ptr<std::unique_ptr<Track>>, int> &emptyFramesNum,
	std::shared_ptr<std::unique_ptr<Track>> track,
	double team) {
	filter_ptr filter = std::make_shared<PlayerFilter>(team);
	addNewActiveTrack(activeTracks, filters, emptyFramesNum, track, filter);
	return filter;
}

void PlayerPaths::storePaths(FieldModel &fm, FrameData &fd) {
	std::ofstream file;
	file.open(outputFile, std::ios::out);

	int subgraphs = 0;
	while (true) {
		std::vector<track_ptr> tracks = tfr.readSubgraph();
		if (tracks.empty()) {
			std::cout << "No more subgraphs" << std::endl;
			break;
		} else {
			std::cout << "Subgraph " << (++subgraphs) << " of " << fd.numOfSubgraphs << std::endl;
		}

		int minTrackFrame = (*tracks[0])->getStartFrame();
		int maxTrackFrame = (*tracks[0])->getEndFrame();
		for (int i = 2; i < tracks.size(); i++) {
			minTrackFrame = std::min(minTrackFrame, (*tracks[i])->getStartFrame());
			maxTrackFrame = std::max(maxTrackFrame, (*tracks[i])->getEndFrame());
		}

		std::vector<track_ptr> rootTracks;
		for (int i = 0; i < tracks.size(); i++) {
			if ((*tracks[i])->isRoot()) {
				rootTracks.push_back(tracks[i]);
			}
		}
		std::sort(rootTracks.begin(), rootTracks.end(), [](track_ptr t1, track_ptr t2) {
			return (*t1)->getStartFrame() < (*t2)->getStartFrame();
		});

		std::map<int, std::pair<cv::Mat, cv::Mat>> homographies = tfr.readHomographies();

		std::vector<track_ptr> activeTracks;
		int index = 0;

		std::map<track_ptr, std::vector<filter_ptr>> filters;

		std::map<track_ptr, int> emptyFramesNum;

		std::map<track_ptr, double> trackTeams;
		for (int i = 0; i < tracks.size(); i++) {
			std::vector<Measurement> ms = (*tracks[i])->getMeasurements();
			double team = 0;
			for (int j = 0; j < ms.size(); j++) {
				double t = 0;
				if (ms[j].features.size() > 0) {
					t = fd.teamModel.predict(ms[j].features);
				}
				team += t;
			}
			trackTeams[tracks[i]] = team / ms.size();
		}

		int trackFrame = minTrackFrame - 1;

		while (trackFrame <= maxTrackFrame) {
			trackFrame++;

			cv::Mat homography = homographies[trackFrame].first;
			// std::cerr << homography << std::endl;

			cv::Mat field = fm.getImage();
			cv::Mat invHom = homographies[trackFrame].second;

			std::vector<track_ptr> toRemove;
			for (int i = 0; i < activeTracks.size(); i++) {
				if ((*activeTracks[i])->getEndFrame() < trackFrame) {
					toRemove.push_back(activeTracks[i]);
					std::vector<track_ptr> cs = (*activeTracks[i])->getChildren();
					std::vector<filter_ptr> fs = filters[activeTracks[i]];

					if (cs.size() == 1) { // spajanje
						for (int i = 0; i < fs.size(); i++) {
							addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[0], fs[i]);
							// TODO zapis?
						}
					}
					else { // odvajanje
						if (cs.size() == fs.size() && cs.size() == 2) {
							// pokusaj rasporedivanja filtera djeci
							if (sgn(trackTeams[cs[0]]) == sgn(fs[0]->getTeam()) && sgn(trackTeams[cs[1]]) == sgn(fs[1]->getTeam())) {
								addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[0], fs[0]);
								addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[1], fs[1]);
								// TODO zapis?
							}
							else if (sgn(trackTeams[cs[0]]) == sgn(fs[1]->getTeam()) && sgn(trackTeams[cs[1]]) == sgn(fs[0]->getTeam())) {
								addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[0], fs[1]);
								addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[1], fs[0]);
								// TODO zapis?
							}
							else {
								filter_ptr f0 = addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[0], trackTeams[cs[0]]);
								filter_ptr f1 = addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[1], trackTeams[cs[1]]);

								file << trackFrame << " new " << f0->getId() << "\n";
								file << trackFrame << " new " << f1->getId() << "\n";
							}
						}
						else {
							for (int i = 0; i < cs.size(); i++) {
								filter_ptr f = addNewActiveTrack(activeTracks, filters, emptyFramesNum, cs[i], trackTeams[cs[i]]);
								file << trackFrame << " new " << f->getId() << "\n";
							}
						}
					}
				}
			}
			for (int i = 0; i < toRemove.size(); i++) {
				activeTracks.erase(std::find(activeTracks.begin(), activeTracks.end(), toRemove[i]));
			}

			while (index < rootTracks.size() && (*rootTracks[index])->getStartFrame() == trackFrame) {
				activeTracks.push_back(rootTracks[index]);

				filter_ptr filter =
					addNewActiveTrack(activeTracks, filters, emptyFramesNum, rootTracks[index], trackTeams[rootTracks[index]]);

				index++;

				file << trackFrame << " new " << filter->getId() << "\n";
			}

			for (int i = 0; i < activeTracks.size(); i++) {
				std::vector<Measurement> ms = (*activeTracks[i])->getMeasurements();
				std::vector<filter_ptr> fs = filters[activeTracks[i]];
				int sf = (*activeTracks[i])->getStartFrame();
				int efn = emptyFramesNum[activeTracks[i]];
				for (int j = trackFrame - sf - efn; j >= 0; j--) {
					if (ms[j].frame == trackFrame) {
						float x = ms[j].rect.x + ms[j].rect.width / 2.f;
						float y = ms[j].rect.y + ms[j].rect.height;
						for (int f = 0; f < fs.size(); f++) {
							fs[f]->update(x, y);
							file << trackFrame << " pos " << fs[f]->getId() << " " << fs[f]->getX() << " " << fs[f]->getY() << " " << fs[f]->getTeam() << "\n";
						}

						break;
					}
					else if (ms[j].frame < trackFrame) {
						break;
					}
					else {
						emptyFramesNum[activeTracks[i]]++;
					}
				}
			}
		}
	}

	file << -1;

	file.close();

	std::cout << "OVER" << std::endl;
	// system("pause");
}
