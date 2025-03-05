#include <iostream>
#include <cstdlib>
#pragma once


using namespace std;

double** init_matrix(int m) {
    double** res = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        res[i] = (double*)malloc(m * sizeof(double));
        for (int j = 0; j < m; j++) {
            if (i == j) {
                res[i][i] = 2.0;
            }
            else {
                res[i][j] = 1.0;
            }
        }
    }
    return res;
}
double* init_vector(int m) {
    double* res = (double*)malloc(m * sizeof(double));
    for (int i = 0; i < m; i++) {
        res[i] = m + 1;
    }
    return res;
}
