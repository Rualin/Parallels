#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <cmath>
#include "functions.cpp"


using namespace std;

void mul_mat_vec(double** mat, double* vec, double* res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads) for schedule(dynamic, 100)
    for (int i = 0; i < m; i++) {
        res[i] = 0;
        for (int j = 0; j < m; j++) {
            res[i] += mat[i][j] * vec[j];
        }
    }
    // return res;
}
void sub_vect(double* a, double* b, double* res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads) for schedule(dynamic, 100)
    for (int i = 0; i < m; i++) {
        res[i] = a[i] - b[i];
    }
    // return res;
}
void vec_to_scal(double* a, double val, double* res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads) for schedule(dynamic, 100)
    for (int i = 0; i < m; i++) {
        res[i] = a[i] * val;
    }
    // return res;
}

double norm(double* vec, int m, int n_threads) {
    double res = 0.0;
    #pragma omp parallel num_threads(n_threads) for schedule(dynamic, 100) reduction(+:res)
    for(int i = 0; i < m; i++) {
        res += vec[i] * vec[i];
    }
    res = sqrt(res);
    return res;
}
bool criterion(double** A, double* b, double* x, double eps, int m, int n_threads) {
    double* sub = (double*)calloc(m, sizeof(double));
    mul_mat_vec(A, x, sub, m, n_threads);
    sub_vect(sub, b, sub, m, n_threads); // must work with two "sub"
    double numerat = norm(sub, m, n_threads);
    // double numerat = norm(sub_vect(mul_mat_vec(A, x, m, n_threads), b, m, n_threads), m, n_threads);
    double denomin = norm(b, m, n_threads);
    return (numerat / denomin) > eps;
}

double* easy_iter(double** A, double* b, double t, int m, int n_threads) {
    double* x = (double*)calloc(m, sizeof(double));
    double* sub = (double*)malloc(m * sizeof(double));
    int epoch = 0;
    while(criterion(A, b, x, 1.0e-5, m, n_threads)) {
        epoch++;
        mul_mat_vec(A, x, sub, m, n_threads);
        sub_vect(sub, b, sub, m, n_threads); // must work with two "sub"
        vec_to_scal(sub, t, sub, m, n_threads);
        sub_vect(x, sub, x, m, n_threads);
        if (epoch == 10000) {
            cout << "The sequence diverges: " << epoch << "\n";
            exit(1);
        }
    }
    return x;
}

int main(int argc, char** argv) {
    int m = atol(argv[1]);
    int n_threads = atol(argv[2]);
    double** A = init_matrix(m);
    double* b = init_vector(m);
    const auto start{chrono::steady_clock::now()};
    double* res = easy_iter(A, b, 0.01, m, n_threads);
    const auto end{chrono::steady_clock::now()};
    cout << "Result:\n";
    cout << res[0] << " " << res[1] << " " << res[2] << " ... ";
    cout << res[m - 3] << " " << res[m - 2] << " " << res[m - 1] << "\n";
    return 0;
}
