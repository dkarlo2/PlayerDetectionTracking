#ifndef CORRELATION_H
#define CORRELATION_H

#include <iostream>
#include <math.h>

/*
	Calculates pearson correlation coefficient for the given arrays.
*/
static double corr(int n, double *x, double *y) {
    double xa = 0, ya = 0;
    for (int i = 0; i < n; i++) {
        xa += x[i];
        ya += y[i];
    }
    xa /= n;
    ya /= n;

    double xx = 0, yy = 0, xy = 0;
    for (int i = 0; i < n; i++) {
        double tmpx = x[i] - xa, tmpy = y[i] - ya;
        xx += tmpx * tmpx;
        yy += tmpy * tmpy;
        xy += tmpx * tmpy;
    }

    return (fabs(yy) < 1e-5 || fabs(xx) < 1e-5) ? 1 : xy / sqrt(xx * yy);
}

#endif
