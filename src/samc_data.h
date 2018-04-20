#ifndef SAMC_DATA_H
#define SAMC_DATA_H

#include "correlation.h"
#include "gnuplot_i.hpp"

#include <thread>
#include <windows.h>

/*
	Represents data for Split and Merge that is used for color separation.
*/
class SAMCData {
    friend void plotSAMCData(std::vector<SAMCData>);
    friend double scoreSAMCData(std::vector<SAMCData>);
    int index;
    double score;
public:
    SAMCData(int I, double S) : index(I), score(S) {}
};

void plotSAMCData(std::vector<SAMCData> data) {
    Gnuplot *gpp = new Gnuplot("lines");
    gpp->set_title("Group accumulated scores");
    gpp->set_xlabel();
    gpp->set_ylabel();
    std::vector<int> xs;
    std::vector<double> ys;
    for (size_t i = 0; i < data.size(); i++) {
        xs.push_back(data[i].index);
        ys.push_back(data[i].score);
    }
    gpp->plot_xy(xs, ys, "");
    gpp->showonscreen();
    system("pause");
    gpp->remove_tmpfiles();
    delete gpp;
}

double scoreSAMCData(std::vector<SAMCData> data) {
    double *xs = new double[data.size()];
    double *ys = new double[data.size()];

    for (size_t i = 0; i < data.size(); i++) {
        xs[i] = data[i].index;
        ys[i] = data[i].score;
    }

    double c = corr(data.size(), xs, ys);

    delete xs;
    delete ys;

    return c;
}

#endif
