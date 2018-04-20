#include "inputs.h"

std::string videoPath;

std::string tmpDirectory;

int imageCalculationPixelArea;

double fieldWidth;
double fieldLength;

double fieldAreaCannyMin;
double fieldAreaCannyMax;
int fieldAreaCannyAper;
int fieldAreaCannyDilate;

int floodPointsHor;
int floodPointsVer;
int floodPointsMargin;
int fieldFloodDilate;

double splitAndMergeSplitThresh;
double splitAndMergeMergeThresh;

int bgCalcFramePeriod;

int mog2ShadowValue;
double mog2ShadowThreshold;

double fieldTrackScoreLineRes;
double fieldTrackScoreArcRes;

int maxContourRectArea;

double avgHeightMin;
double avgHeightMax;
double avgWidthMin;
double avgWidthMax;
double heightOffset;
double widthOffset;
double areaRatio;
double perspectivePower;

int maxClusters;

double matchDistancesMin;
double matchDistancesMax;

int initTrackFrames;
int framesToRetire;

int shortSubgraph;
double staticSubgraph;

double teamModelProb;
int maxTeamModelSize;

std::string outputFile;

void loadInputs(char* configFilename) {
	ConfigParser config;
	config.parse(configFilename);

	videoPath = config.getString("videoPath");

	tmpDirectory = config.getString("tmpDirectory");

	imageCalculationPixelArea = config.getInt("imageCalculationPixelArea");

	fieldWidth = config.getDouble("fieldWidth");
	fieldLength = config.getDouble("fieldLength");

	fieldAreaCannyMin = config.getDouble("fieldAreaCannyMin");
	fieldAreaCannyMax = config.getDouble("fieldAreaCannyMax");
	fieldAreaCannyAper = config.getInt("fieldAreaCannyAper");
	fieldAreaCannyDilate = config.getInt("fieldAreaCannyDilate");

	floodPointsHor = config.getInt("floodPointsHor");
	floodPointsVer = config.getInt("floodPointsVer");
	floodPointsMargin = config.getInt("floodPointsMargin");
	fieldFloodDilate = config.getInt("fieldFloodDilate");

	splitAndMergeSplitThresh = config.getDouble("splitAndMergeSplitThresh");
	splitAndMergeMergeThresh = config.getDouble("splitAndMergeMergeThresh");

	bgCalcFramePeriod = config.getInt("bgCalcFramePeriod");

	mog2ShadowValue = config.getInt("mog2ShadowValue");
	mog2ShadowThreshold = config.getDouble("mog2ShadowThreshold");

	fieldTrackScoreLineRes = config.getDouble("fieldTrackScoreLineRes");
	fieldTrackScoreArcRes = config.getDouble("fieldTrackScoreArcRes");

	maxContourRectArea = config.getInt("maxContourRectArea");

	avgHeightMin = config.getDouble("avgHeightMin");
	avgHeightMax = config.getDouble("avgHeightMax");
	avgWidthMin = config.getDouble("avgWidthMin");
	avgWidthMax = config.getDouble("avgWidthMax");
	heightOffset = config.getDouble("heightOffset");
	widthOffset = config.getDouble("widthOffset");
	areaRatio = config.getDouble("areaRatio");
	perspectivePower = config.getDouble("perspectivePower");

	maxClusters = config.getInt("maxClusters");

	matchDistancesMin = config.getDouble("matchDistancesMin");
	matchDistancesMax = config.getDouble("matchDistancesMax");

	initTrackFrames = config.getInt("initTrackFrames");
	framesToRetire = config.getInt("framesToRetire");

	shortSubgraph = config.getInt("shortSubgraph");
	staticSubgraph = config.getDouble("staticSubgraph");

	teamModelProb = config.getDouble("teamModelProb");
	maxTeamModelSize = config.getInt("maxTeamModelSize");

	outputFile = config.getString("outputFile");
}
