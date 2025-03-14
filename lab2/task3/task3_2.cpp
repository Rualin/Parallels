#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <fstream>
#include "functions.cpp"


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

void inline mul_mat_vec(double** mat, double* vec, double* res, int m) {
    #pragma omp for schedule(dynamic, 100)
    for (int i = 0; i < m; i++) {
        res[i] = 0;
        for (int j = 0; j < m; j++) {
            res[i] += mat[i][j] * vec[j];
        }
    }
    // return res;
}
void inline sub_vect(double* a, double* b, double* res, int m) {
    #pragma omp for schedule(dynamic, 100)
    for (int i = 0; i < m; i++) {
        res[i] = a[i] - b[i];
    }
    // return res;
}
void inline vec_to_scal(double* a, double val, double* res, int m) {
    #pragma omp for schedule(dynamic, 100)
    for (int i = 0; i < m; i++) {
        res[i] = a[i] * val;
    }
    // return res;
}

double norm(double* vec, int m) {
    double res = 0.0;
    // #pragma omp for schedule(dynamic, 100) reduction(+:res)
    for(int i = 0; i < m; i++) {
        res += vec[i] * vec[i];
    }
    res = sqrt(res);
    return res;
}
bool criterion(double** A, double* b, double* x, double eps, int m) {
    double* sub = (double*)calloc(m, sizeof(double));
    mul_mat_vec(A, x, sub, m);
    sub_vect(sub, b, sub, m); // must work with two "sub"
    double numerat = norm(sub, m);
    // double numerat = norm(sub_vect(mul_mat_vec(A, x, m, n_threads), b, m, n_threads), m, n_threads);
    double denomin = norm(b, m);
    free(sub);
    return (numerat / denomin) > eps;
}

double* easy_iter(double** A, double* b, double t, int m, int n_threads) {
    double* x = (double*)calloc(m, sizeof(double));
    double* sub = (double*)malloc(m * sizeof(double));
    bool criter = true;
    double numerat, denomin;
    while(criter) {
        #pragma omp parallel num_threads(n_threads) shared(x, sub, criter, numerat, denomin)
        {
            mul_mat_vec(A, x, sub, m);

            sub_vect(sub, b, sub, m); // must work with two "sub"
            vec_to_scal(sub, t, sub, m);
            sub_vect(x, sub, x, m);
            // #pragma omp barrier
            // #pragma omp single
            // {
            // cout << x[0] << " " << x[1] << " " << x[2] << " ... ";
            // cout << x[m - 3] << " " << x[m - 2] << " " << x[m - 1] << "\n";
            // }
            // if (epoch == 10000) {
            //     cout << "The sequence diverges: " << epoch << "\n";
            //     exit(1);
            // }
            // criter = criterion(A, b, x, 1.0e-5, m);
            
            //criterion calc start
            mul_mat_vec(A, x, sub, m);
            sub_vect(sub, b, sub, m); // must work with two "sub"
            #pragma omp single
            numerat = norm(sub, m);
            #pragma omp single
            denomin = norm(b, m);
            #pragma omp barrier
            criter = (numerat / denomin) > 1.0e-5;
            //criterion calc end
        }
    }
    free(sub);
    return x;
}

int main(int argc, char** argv) {
    int m = atol(argv[1]);
    int n_threads = atol(argv[2]);
    double** A = init_matrix(m, n_threads);
    double* b = init_vector(m, n_threads);
    double t = 1.0 / m;
    const auto start{chrono::steady_clock::now()};
    double* res = easy_iter(A, b, t, m, n_threads);
    const auto end{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds{end - start};
    // cout << "Result:\n";
    // cout << res[0] << " " << res[1] << " " << res[2] << " ... ";
    // cout << res[m - 3] << " " << res[m - 2] << " " << res[m - 1] << "\n";
    // cout << elapsed_seconds.count() << "\n";
    fstream fout("result2.csv", fstream::out | fstream::app);
    fout << n_threads << "," << elapsed_seconds.count() << "\n";
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
    return 0;
}
