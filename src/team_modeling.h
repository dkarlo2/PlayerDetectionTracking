#pragma once

#include "inputs.h"
#include "svm\svm.h"
#include <vector>

class TeamModel {
	std::vector<std::vector<float>> featureVector;

	svm_model *model;

public:
	void addFeatures(std::vector<float> features) {
		if (featureVector.size() < maxTeamModelSize) {
			featureVector.push_back(features);
		}
	}
	void train();
	double predict(std::vector<float> features);
};
