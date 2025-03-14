#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <fstream>
// #include "functions.cpp"


using namespace std;


double** init_matrix(int m, int n_threads) {
    double** res = (double**)malloc(m * sizeof(double*));
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
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
    }
    return res;
}
double* init_vector(int m, int n_threads) {
    double* res = (double*)malloc(m * sizeof(double));
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = m + 1;
        }
    }
    return res;
}

void mul_mat_vec(double** mat, double* vec, double* res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = 0;
            for (int j = 0; j < m; j++) {
                res[i] += mat[i][j] * vec[j];
            }
        }
    }
    // return res;
}
void sub_vect(double* a, double* b, double* res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = a[i] - b[i];
        }
    }    // return res;
}
void vec_to_scal(double* a, double val, double* res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = a[i] * val;
        }
    }
    // return res;
}

double norm(double* vec, int m, int n_threads) {
    double res = 0.0;
    #pragma omp parallel num_threads(n_threads) 
    {
        #pragma omp for schedule(dynamic, 100) reduction(+:res)
        for(int i = 0; i < m; i++) {
            res += vec[i] * vec[i];
        }
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
    free(sub);
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
        if (epoch == 100000) {
            cout << "The sequence diverges: " << epoch << "\n";
            exit(1);
        }
    }
    return x;
}

int main(int argc, char** argv) {
    int m = atol(argv[1]);
    // double t = atof(argv[2]);
    int n_threads = atol(argv[2]);
    double** A = init_matrix(m, n_threads);
    double* b = init_vector(m, n_threads);
    double t = 1.0 / m;
    // cout << t << "\n";
    // cout << "Max threds: " << omp_get_max_threads() << "\n";
    const auto start{chrono::steady_clock::now()};
    double* res = easy_iter(A, b, t, m, n_threads);
    const auto end{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds{end - start};
    fstream fout("result.csv", fstream::out | fstream::app);
    fout << n_threads << "," << elapsed_seconds.count() << "\n";
    // cout << "Result:\n";
    // cout << res[0] << " " << res[1] << " " << res[2] << " ... ";
    // cout << res[m - 3] << " " << res[m - 2] << " " << res[m - 1] << "\n";
    // cout << "Duration: " << elapsed_seconds.count() << "\n";
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            free(A[i]);
        }
    }
    free(A);
    free(b);
    free(res);
    fout.close();
    // cout << "Max threds: " << omp_get_max_threads() << "\n";
    return 0;
}
