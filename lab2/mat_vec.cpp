#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <fstream>


using namespace std;

double** init_matrix(int m) {
    double** res = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        res[i] = (double*)malloc(m * sizeof(double));
        for (int j = 0; j < m; j++) {
            res[i][j] = i * j + j;
        }
    }
    return res;
}
double* init_vector(int m) {
    double* res = (double*)malloc(m * sizeof(double));
    for (int i = 0; i < m; i++) {
        res[i] = i * 2 + 1;
    }
    return res;
}

double* mul_mat_vec(double** mat, double* vec, int m, int n_threads) {
    int div = m / n_threads, mod = m % n_threads;
    double* res = (double*)calloc(m, sizeof(double));
    #pragma omp parallel num_threads(n_threads)
    {
        int id = omp_get_thread_num();
        int lb = (id < mod) ? ((div + 1) * id) : (mod + div * id);
        int ub = (id < mod) ? ((div + 1) * (id + 1)) : (mod + div * (id + 1));
        for (int i = lb; i < ub; i++) {
            for (int j = 0; j < m; j++) {
                res[i] += mat[i][j] * vec[j];
            }
        }
    }
    return res;
}

int main(int argc, char **argv) {
    int m = atol(argv[1]);
    int n_threads = atol(argv[2]);
    double** mat = init_matrix(m);
    double* vec = init_vector(m);
    const auto start{chrono::steady_clock::now()};
    double* res = mul_mat_vec(mat, vec, m, n_threads);
    const auto end{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds{end - start};
    fstream fout("res_mat_vec.csv", fstream::out | fstream::app);
    // cout << "Multiplication of matrix " << m << " x " << m << " to vector with size " << m << " elapsed " << elapsed_seconds.count() << "\n";
    fout << n_threads << "," << m << "," << elapsed_seconds.count() << "\n";
    fout.close();
    return 0;
}
