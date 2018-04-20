#include "field_background.h"

#include "globals.h"
#include "inputs.h"
#include "samc_data.h"
#include "split_and_merge_colors.h"
#include "util.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <queue>
#include <typeinfo>
#include <vector>

const std::vector<int> hueGroups {1, 6, 6, 12, 12, 12, 12, 12, 12, 6, 6, 1};
const std::vector<double> luminanceIntervals {0, 0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 0.95, 1};
const std::vector<int> saturationGroups {1, 2, 4, 4, 8, 8, 8, 8, 4, 4, 2, 1};

static int getTotalNumOfGroups() {
    int n = 0;
    for (size_t i = 0; i < hueGroups.size(); i++) {
        n += saturationGroups[i] * hueGroups[i];
    }
    return n;
}

static int calcGroup(int hueGroup, int lumGroup, int satGroup) {
    int group = 0;

    for (int i = 0; i < lumGroup; i++) {
        group += saturationGroups[i] * hueGroups[i];
    }

    group += hueGroup * saturationGroups[lumGroup] + satGroup;

    return group;
}

static int getGroupHLS(uchar hue, uchar luminance, uchar saturation) {
    int luminanceGroup = luminanceIntervals.size() - 2;
    for (size_t i = 0; i < luminanceIntervals.size()-1; i++) {
        if (luminance >= luminanceIntervals[i] * LUM_MAX && luminance < luminanceIntervals[i+1] * LUM_MAX) {
            luminanceGroup = i;
            break;
        }
    }

    int sgs = saturationGroups[luminanceGroup];
    int hgs = hueGroups[luminanceGroup];

    int saturationGroup = sgs-1;
    for (int i = 0; i < sgs-1; i++) {
        if (saturation >= SAT_MAX / sgs * i && saturation < SAT_MAX / sgs * (i+1)) {
            saturationGroup = i;
            break;
        }
    }

    int hueGroup = hgs-1;
    for (int i = 0; i < hgs-1; i++) {
        if (hue >= HUE_MAX / hgs * i && hue < HUE_MAX / hgs * (i+1)) {
            hueGroup = i;
            break;
        }
    }

    return calcGroup(hueGroup, luminanceGroup, saturationGroup);
}

static int getGroupBGR(uchar b, uchar g, uchar r) {
    cv::Vec3b v = bgr2hls(b, g, r);
    return getGroupHLS(v[0], v[1], v[2]);
}

static cv::Vec3b getGroupCenterHLS(int group) {
    int gCnt = 0;
    for (size_t lg = 0; lg < luminanceIntervals.size()-1; lg++) {
        int sgs = saturationGroups[lg];
        int hgs = hueGroups[lg];
        if (group < gCnt + sgs * hgs) {
            int hg = (group - gCnt) / sgs;
            int sg = (group - gCnt) % sgs;

            uchar l = (luminanceIntervals[lg] + luminanceIntervals[lg+1]) / 2 * LUM_MAX;
            uchar h = HUE_MAX / hgs * (hg + 0.5);
            uchar s = SAT_MAX / sgs * (sg + 0.5);
            return cv::Vec3b(h, l, s);
        } else {
            gCnt += sgs * hgs;
        }
    }

    std::cerr << "Invalid group number." << std::endl;
    return cv::Vec3b();
}

static cv::Vec3b getGroupCenterBGR(int group) {
    cv::Vec3b hls = getGroupCenterHLS(group);
    return hls2bgr(hls[0], hls[1], hls[2]);
}

static std::pair<cv::Vec3b, cv::Vec3b> getGroupIntervalHLS(int group) {
    int gCnt = 0;
    for (size_t lg = 0; lg < luminanceIntervals.size()-1; lg++) {
        int sgs = saturationGroups[lg];
        int hgs = hueGroups[lg];
        if (group < gCnt + sgs * hgs) {
            int hg = (group - gCnt) / sgs;
            int sg = (group - gCnt) % sgs;

            uchar ld = luminanceIntervals[lg] * LUM_MAX;
            uchar hd = HUE_MAX / hgs * hg;
            uchar sd = SAT_MAX / sgs * sg;

            uchar lu = luminanceIntervals[lg+1] * LUM_MAX;
            uchar hu = HUE_MAX / hgs * (hg+1);
            uchar su = SAT_MAX / sgs * (sg+1);

            return std::make_pair(cv::Vec3b(hd, ld, sd), cv::Vec3b(hu, lu, su));
        } else {
            gCnt += sgs * hgs;
        }
    }

    std::cerr << "Invalid group number." << std::endl;
    return std::make_pair(cv::Vec3b(), cv::Vec3b());
}

/*static std::pair<cv::Vec3b, cv::Vec3b> getGroupIntervalBGR(int group) {
    std::pair<cv::Vec3b, cv::Vec3b> hls = getGroupIntervalHLS(group);
    return std::make_pair(hls2bgr(hls.first[0], hls.first[1], hls.first[2]),
                            hls2bgr(hls.second[0], hls.second[1], hls.second[2]));
}*/

void FieldBackground::calculateBgColors(cv::Mat origImage) {
    if (origImage.rows == 0 || origImage.cols == 0) {
        std::cerr << "Invalid image size" << std::endl;
        return;
    }

#if DEBUG_FIELD_BACKGROUND
    std::cerr << "Image size for background calculation: " << origImage.size() << std::endl;
#endif

    cv::Mat image = origImage.clone();
    resizeImageArea(image, imageCalculationPixelArea);

    cvtColor(image, image, CV_BGR2HLS); // TODO

    image.convertTo(image, CV_8UC3);

    int numOfGroups = getTotalNumOfGroups();

	// ###### calculate group frequences ######
    std::vector< std::pair<int, int> > groupFreqs(numOfGroups);

    for (int i = 0; i < numOfGroups; i++) {
        groupFreqs[i].second = i;
    }

    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b v = image.at<cv::Vec3b>(i, j);
            if (v[1] != 0) {
                int g = getGroupHLS(v[0], v[1], v[2]);
                groupFreqs[g].first++;
            }
        }
    }

	// #############

    std::sort(groupFreqs.begin(), groupFreqs.end(), std::greater< std::pair<int, int> >());

#if DEBUG_FIELD_BACKGROUND
    for (int i = 0; i < 10; i++) {
        int g = groupFreqs[i].second;
        cv::Vec3b gbgr = getGroupCenterBGR(g);
        std::cerr << "Debug GS " << g << ' ' << groupFreqs[i].first << ' ' << pr(gbgr[0]) << ' '
            << pr(gbgr[1]) << ' ' << pr(gbgr[2]) << std::endl;
    }

    std::vector<int> uniGroups;
    for (int i = 0; i < image.rows; i++) {
        for (int j = 0; j < image.cols; j++) {
            cv::Vec3b v = image.at<cv::Vec3b>(i, j);
            int g = getGroupBGR(v[0], v[1], v[2]);
            uniGroups.push_back(g);
        }
	}

    std::sort(uniGroups.begin(), uniGroups.end());
    uniGroups.erase(std::unique(uniGroups.begin(), uniGroups.end()), uniGroups.end());
    std::cerr << "Unique groups: " << uniGroups.size() << std::endl;
#endif

	// ###### perform Split and Merge ######

    SplitAndMergeColors<SAMCData> sam(scoreSAMCData, splitAndMergeSplitThresh, splitAndMergeMergeThresh);

    double score = 0;
    for (int g = 0; g < numOfGroups; g++) {
        score += groupFreqs[g].first;
        sam.addPoint(SAMCData(g, score));
    }

#if DEBUG_FIELD_BACKGROUND
	// plotSAMCData(sam.getData());
#endif

    std::vector< std::pair<int, int> > intervals = sam.execute(2);
    int nColorGroups = intervals[0].second;
#if DEBUG_FIELD_BACKGROUND
	std::cerr << "nColorGroups: " << nColorGroups << std::endl;
#endif

	// ############

    useGroup = std::vector<bool>(numOfGroups, false);

    for (int i = 0; i < nColorGroups; i++) {
        useGroup[groupFreqs[i].second] = true;
#if DEBUG_FIELD_BACKGROUND
		cv::Vec3b gbgr = getGroupCenterBGR(groupFreqs[i].second);
        std::cerr << "Using group " << groupFreqs[i].second << ' ' << pr(gbgr[0]) << ' ' << pr(gbgr[1]) << ' ' << pr(gbgr[2]) << std::endl;
        std::cerr << groupFreqs[i].first << std::endl;
        cv::Vec3b ghls = getGroupCenterHLS(groupFreqs[i].second);
        std::cerr << pr(ghls[0]) << ' ' << pr(ghls[1]) << ' ' << pr(ghls[2]) << std::endl;
#endif
    }
}

cv::Mat FieldBackground::extractBgMask(cv::Mat image) {
    cv::Size imgSize = image.size();
    resizeImageArea(image, imageCalculationPixelArea);
    cvtColor(image, image, CV_BGR2HLS);

    cv::Mat mask = cv::Mat::zeros(image.size(), 0);

    int nColorGroups = getTotalNumOfGroups();
    for (int g = 0; g < nColorGroups; g++) {
        if (!useGroup[g]) {
            continue;
        }
        std::pair<cv::Vec3b, cv::Vec3b> interval = getGroupIntervalHLS(g);
        cv::Scalar down(interval.first[0], interval.first[1], interval.first[2]);
        cv::Scalar up(interval.second[0], interval.second[1], interval.second[2]);
        cv::Mat maskPart;
        inRange(image, down, up, maskPart);
        mask |= maskPart;
    }

    resize(mask, mask, imgSize);

    return mask;
}
