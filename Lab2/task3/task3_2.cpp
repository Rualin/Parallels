#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <fstream>
#include <vector>

#define type dynamic
#define K 500
using vec_doub = std::vector<double>;

std::vector<vec_doub> init_matrix(int m, int n_threads) {
    std::vector<vec_doub> res(m, vec_doub(m, 1.0));
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(type, K)
        for (int i = 0; i < m; i++) {
            res[i][i] = 2.0;
        }
    }
    return res;
}
vec_doub init_vector(int m, int n_threads) {
    vec_doub res(m, m + 1);
    return res;
}

void inline mul_mat_vec(std::vector<vec_doub>& mat, vec_doub& vec, vec_doub& res, int m) {
    #pragma omp for schedule(type, K)
    for (int i = 0; i < m; i++) {
        res[i] = 0;
        for (int j = 0; j < m; j++) {
            res[i] += mat[i][j] * vec[j];
        }
    }
}
void inline sub_vect(vec_doub& a, vec_doub& b, vec_doub& res, int m) {
    #pragma omp for schedule(type, K)
    for (int i = 0; i < m; i++) {
        res[i] = a[i] - b[i];
    }
}
void inline vec_to_scal(vec_doub& a, double val, vec_doub& res, int m) {
    #pragma omp for schedule(type, K)
    for (int i = 0; i < m; i++) {
        res[i] = a[i] * val;
    }
}

double norm(vec_doub& vec, int m) {
    double res = 0.0;
    for(int i = 0; i < m; i++) {
        res += vec[i] * vec[i];
    }
    res = sqrt(res);
    return res;
}
bool criterion(std::vector<vec_doub>& A, vec_doub& b, vec_doub& x, double eps, int m) {
    vec_doub sub(m, 0.0);
    mul_mat_vec(A, x, sub, m);
    sub_vect(sub, b, sub, m); // must work with two "sub"
    double numerat = norm(sub, m);
    double denomin = norm(b, m);
    return (numerat / denomin) > eps;
}

vec_doub easy_iter(std::vector<vec_doub>& A, vec_doub& b, double t, int m, int n_threads) {
    vec_doub x(m, 0.0);
    vec_doub sub(m, 0.0);
    bool criter = true;
    double numerat, denomin;
    while(criter) {
        #pragma omp parallel num_threads(n_threads) shared(x, sub, criter, numerat, denomin)
        {
            mul_mat_vec(A, x, sub, m);
            sub_vect(sub, b, sub, m); // must work with two "sub"
            vec_to_scal(sub, t, sub, m);
            sub_vect(x, sub, x, m);
            mul_mat_vec(A, x, sub, m);
            sub_vect(sub, b, sub, m); // must work with two "sub"
            #pragma omp single
            numerat = norm(sub, m);
            #pragma omp single
            denomin = norm(b, m);
            #pragma omp barrier
            criter = (numerat / denomin) > 1.0e-5;
        }
    }
    return x;
}

int main(int argc, char** argv) {
    int m = atol(argv[1]);
    int n_threads = atol(argv[2]);
    std::vector<vec_doub> A = init_matrix(m, n_threads);
    vec_doub b = init_vector(m, n_threads);
    double t = 1.0 / m;
    const auto start{std::chrono::steady_clock::now()};
    vec_doub res = easy_iter(A, b, t, m, n_threads);
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    std::fstream fout("res_dynamic.csv", std::fstream::out | std::fstream::app);
    fout << n_threads << "," << elapsed_seconds.count() << "," << K << "\n";
    fout.close();
    return 0;
}
