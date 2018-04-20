#include "globals.h"
#include "inputs.h"
#include "field_tracking.h"
#include "point_mapping.h"

#include "time_measurement.h"
#include "util.h"
#include "window_manager.h"

#include <iostream>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <random>

// TODO Kod zašumljivanja homografije umjesto dosadašnjeg pristupa koristiti onaj koji "šumi" poziciju kamere,
// u smislu da raèunam koja bi homografija bila ako se kamera pomakne za neki random broj u x, y i z smjeru, te
// zakrene za random stupnjeva oko x, y i z osi.

#undef min
#undef max

#define PARTICLES_N 100
#define INIT_SIGMA 0.01
#define SIGMA 0.001

static std::default_random_engine generator;
static std::normal_distribution<double> init_gauss(1, INIT_SIGMA);
static std::uniform_real_distribution<double> uniform;
static std::normal_distribution<double> gauss(1, SIGMA);

static void noiseHomography(cv::Mat &homography, std::normal_distribution<double> &distribution) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			// TODO ovo mnozenje s gaussom mozda nije ok
			homography.at<double>(i, j) *= distribution(generator);
		}
	}
}

void FieldTracker::initParticles(cv::Mat homography) {
	particles.clear();

	for (int p = 0; p < PARTICLES_N; p++) {
		cv::Mat h = homography.clone();
		noiseHomography(h, init_gauss);
		particles.push_back(FTParticle(h, 1. / PARTICLES_N));
	}
}

cv::Mat FieldTracker::getHomography() {
	/*cv::Mat homography = cv::Mat::zeros(3, 3, CV_64FC1);
	for (int i = 0; i < PARTICLES_N; i++) {
		homography += particles[i].homography * particles[i].weight;
	}
	homography /= PARTICLES_N;
	return homography;*/
	cv::Mat homography = particles[0].homography;
	double score = particles[0].weight;
	for (int i = 1; i < PARTICLES_N; i++) {
		if (particles[i].weight > score) {
			homography = particles[i].homography;
			score = particles[i].weight;
		}
	}
	return homography;
}

void FieldTracker::predict() {
	double accumulator = 0;
	std::vector<double> randoms;
	double minWeight = std::numeric_limits<double>::max();
	int minIndex = -1;
	for (int i = 0; i < PARTICLES_N; i++) {
		accumulator += particles[i].weight;
		particles[i].accumulator = accumulator;
		randoms.push_back(uniform(generator));
		if (particles[i].weight < minWeight) {
			minWeight = particles[i].weight;
			minIndex = i;
		}
	}

	std::sort(randoms.begin(), randoms.end());

	std::vector<FTParticle> newParticles;
	newParticles.push_back(particles[minIndex].clone());

	int i = 0;
	int j = 0;
	while (newParticles.size() < PARTICLES_N) {
		while (particles[i].accumulator <= randoms[j]) {
			i++;
		}
		newParticles.push_back(particles[i].clone());
		j++;
	}
	particles = newParticles;

	for (int i = 0; i < PARTICLES_N; i++) {
		noiseHomography(particles[i].homography, gauss);
	}
}

void FieldTracker::correct(FieldModel &fm, cv::Mat distanceTransform) {
	double minWeight = std::numeric_limits<double>::max();
	MyTime mt;
	mt.start();
	for (int i = 0; i < PARTICLES_N; i++) {
		particles[i].weight = fm.calculateScore(distanceTransform, particles[i].homography, fieldTrackScoreLineRes, fieldTrackScoreArcRes);
		minWeight = std::min(minWeight, particles[i].weight);
	}
#if DEBUG_FIELD_TRACKING
	std::cerr << "Correct calculating scores: " << mt.time() << std::endl;
#endif

	double maxWeight = 0;
	for (int i = 0; i < PARTICLES_N; i++) {
		particles[i].weight -= minWeight;
		maxWeight = std::max(maxWeight, particles[i].weight);
	}

	double weightSum = 0;
	for (int i = 0; i < PARTICLES_N; i++) {
		particles[i].weight = maxWeight - particles[i].weight;
		weightSum += particles[i].weight;
	}

#if DEBUG_FIELD_TRACKING
	std::cerr << "Weights: " << minWeight << ' ' << maxWeight << ' ' << weightSum << std::endl;
#endif

	if (weightSum == 0) {
		std::cerr << "hey! weightSum == 0" << std::endl;
		throw "";
	}

	for (int i = 0; i < PARTICLES_N; i++) {
		particles[i].weight /= weightSum;
	}
}

void FieldTracker::init(FrameData &fd, FieldModel &fm, cv::Mat testHomography) {
	cv::Mat homography;
	if (testHomography.empty()) {
		Mappings mappingPoints = getPointMappings(fd.image.clone(), fm.getImage());
		homography = findHomography(mappingPoints.second, mappingPoints.first, CV_LMEDS);
		fd.homography = homography;
#if DEBUG_FIELD_TRACKING
		std::cerr << "Homography calculated" << std::endl << std::endl;
		std::cerr << mappingPoints.second << std::endl << std::endl;
		std::cerr << mappingPoints.first << std::endl << std::endl;
		std::cerr << homography << std::endl << std::endl;
		std::cerr << "-------------------------" << std::endl;
		cv::Mat combined;
		addWeighted(fm.getImage(-1, fd.homography, fd.image.size()), 0.5, fd.image, 0.5, 0, combined);
		static WindowManager wm("Test Homography");
		wm.showImage(combined);
		cv::waitKey(0);
#endif
	}
	else {
		homography = testHomography;
	}
	initParticles(homography);
}

void FieldTracker::calculateHomography(FrameData &fd, FieldModel &fm) {
	MyTime totalMt;
	totalMt.start();

	cv::Mat modelImage = fm.getImage();

	MyTime mt;

	mt.start();

	cv::Mat dt;
	distanceTransform(fd.bgMask, dt, CV_DIST_L2, CV_DIST_MASK_PRECISE);
	cv::normalize(dt, dt, 1, 0, cv::NORM_INF);

#if DEBUG_FIELD_TRACKING
	std::cerr << "Distance transform: " << mt.time() << std::endl;
	imshow("Distance transform of background mask", dt);
#endif

	mt.start();
	predict();
#if DEBUG_FIELD_TRACKING
	std::cerr << "Predict: " << mt.time() << std::endl;
#endif

	mt.start();
	correct(fm, dt);
#if DEBUG_FIELD_TRACKING
	std::cerr << "Correct: " << mt.time() << std::endl;
#endif

	fd.homography = getHomography();
	
#if DEBUG_FIELD_TRACKING
	mt.start();

	/*cv::Mat warpedImage;
	warpPerspective(modelImage, warpedImage, getHomography(debug), fd.getImage().size());*/
	cv::Mat combined;
	addWeighted(fm.getImage(-1, fd.homography, fd.image.size()), 0.5, fd.image, 0.5, 0, combined);
	static WindowManager wm("Test Homography");
	wm.showImage(combined);

	std::cerr << "Warping image: " << mt.time() << std::endl;

	std::cerr << "Total process frame: " << totalMt.time() << std::endl;
#endif

}
