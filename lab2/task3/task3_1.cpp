#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <fstream>
#include <vector>


using vec_doub = std::vector<double>;

std::vector<vec_doub> init_matrix(int m, int n_threads) {
    std::vector<vec_doub> res(m, vec_doub(m, 1.0));
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
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

void mul_mat_vec(std::vector<vec_doub>& mat, vec_doub& vec, vec_doub& res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = 0;
            for (int j = 0; j < m; j++) {
                res[i] = res[i] + mat[i][j] * vec[j];
            }
        }
    }
}
void sub_vect(vec_doub& a, vec_doub& b, vec_doub& res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = a[i] - b[i];
        }
    }
}
void vec_to_scal(vec_doub& a, double val, vec_doub& res, int m, int n_threads) {
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = a[i] * val;
        }
    }
}

double norm(vec_doub& vec, int m, int n_threads) {
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
bool criterion(std::vector<vec_doub>& A, vec_doub& b, vec_doub& x, double eps, int m, int n_threads) {
    vec_doub sub(m, 0.0);
    mul_mat_vec(A, x, sub, m, n_threads);
    sub_vect(sub, b, sub, m, n_threads); // must work with two "sub"
    double numerat = norm(sub, m, n_threads);
    double denomin = norm(b, m, n_threads);
    return (numerat / denomin) > eps;
}

vec_doub easy_iter(std::vector<vec_doub>& A, vec_doub& b, double t, int m, int n_threads) {
    vec_doub x(m, 0.0);
    vec_doub sub(m, 0.0);
    int epoch = 0;
    while(criterion(A, b, x, 1.0e-5, m, n_threads)) {
        epoch++;
        mul_mat_vec(A, x, sub, m, n_threads);
        sub_vect(sub, b, sub, m, n_threads); // must work with two "sub"
        vec_to_scal(sub, t, sub, m, n_threads);
        sub_vect(x, sub, x, m, n_threads);
        if (epoch == 100000) {
            std::cout << "The sequence diverges: " << epoch << "\n";
            exit(1);
        }
    }
    return x;
}

int main(int argc, char** argv) {
    int m = atol(argv[1]);
    // double t = atof(argv[2]);
    int n_threads = atol(argv[2]);
    std::vector<vec_doub> A = init_matrix(m, n_threads);
    vec_doub b = init_vector(m, n_threads);
    double t = 1.0 / m;
    const auto start{std::chrono::steady_clock::now()};
    vec_doub res = easy_iter(A, b, t, m, n_threads);
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    std::fstream fout("result.csv", std::fstream::out | std::fstream::app);
    fout << n_threads << "," << elapsed_seconds.count() << "\n";
    fout.close();
    return 0;
}
