#include <algorithm>
#include "inputs.h"
#include <iostream>
#include "team_modeling.h"

#undef min
#undef max

void TeamModel::train() {
	std::random_shuffle(featureVector.begin(), featureVector.end());

	int s = featureVector.size();

	cv::Mat data = cv::Mat::zeros(s, 3, CV_32F);
	for (int i = 0; i < s; i++) {
		data.at<float>(i, 0) = featureVector[i][0];
		data.at<float>(i, 1) = featureVector[i][1];
		data.at<float>(i, 2) = featureVector[i][2];
	}

	cv::Mat labels, centers;
	double compactness = cv::kmeans(data, std::min(2, s), labels, cv::TermCriteria(cv::TermCriteria::COUNT, 100, 0.1),
		3, cv::KMEANS_PP_CENTERS, centers);

	/*for (int i = 0; i < s; i++) {
	if (labels.at<int>(i) == 1) {
	std::cerr << data.at<float>(i, 0) << data.at<float>(i, 1) << data.at<float>(i, 2) << std::endl;
	system("pause");
	}
	}*/

	struct svm_parameter param;
	param.svm_type = EPSILON_SVR;
	param.kernel_type = LINEAR;
	param.C = 1.5;
	param.p = 0.5;
	param.eps = 1e-5;

	struct svm_problem problem;
	problem.l = s;
	problem.x = new svm_node*[s];
	problem.y = new double[s];
	for (int i = 0; i < s; i++) {
		problem.x[i] = new svm_node[4];
		problem.x[i][0].index = 0;
		problem.x[i][0].value = data.at<float>(i, 0);
		problem.x[i][1].index = 1;
		problem.x[i][1].value = data.at<float>(i, 1);
		problem.x[i][2].index = 2;
		problem.x[i][2].value = data.at<float>(i, 2);
		problem.x[i][3].index = -1;

		problem.y[i] = labels.at<int>(i) == 0 ? -1 : 1;
	}

	model = svm_train(&problem, &param);

	int correct = 0;
	for (int i = 0; i < s; i++) {
		svm_node *testNode = new svm_node[4];
		testNode[0].index = 0;
		testNode[0].value = data.at<float>(i, 0);
		testNode[1].index = 1;
		testNode[1].value = data.at<float>(i, 1);
		testNode[2].index = 2;
		testNode[2].value = data.at<float>(i, 2);
		testNode[3].index = -1;

		double pred = svm_predict(model, testNode);

		int l = labels.at<int>(i);
		if (pred < 0 && l == 0 || pred > 0 && l == 1) {
			correct++;
		}
		else {
			/*std::cerr << pred << " " << labels.at<int>(i) << std::endl;
			system("pause");*/
		}
	}
	std::cerr << "SVM training set accuracy: " << correct << " of " << s << std::endl;
	// system("pause");
}

double TeamModel::predict(std::vector<float> features) {
	svm_node *testNode = new svm_node[4];
	testNode[0].index = 0;
	testNode[0].value = features[0];
	testNode[1].index = 1;
	testNode[1].value = features[1];
	testNode[2].index = 2;
	testNode[2].value = features[2];
	testNode[3].index = -1;

	return svm_predict(model, testNode);
}

