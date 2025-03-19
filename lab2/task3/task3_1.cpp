#include <iostream>
#include <cstdlib>
#include <omp.h>
#include <chrono>
#include <cmath>
#include <fstream>
#include <vector>
// #include "functions.cpp"


using namespace std;
typedef vector<double> vd;

vector<vd> init_matrix(int m, int n_threads) {
    vector<vd> res(m, vd(m, 1.0));
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i][i] = 2.0;
        }
    }
    return res;
}
vd init_vector(int m, int n_threads) {
    vd res(m, m + 1);
    return res;
}

void mul_mat_vec(vector<vd>& mat, vd& vec, vd& res, int m, int n_threads) {
    // cout << "\tmul_mat_vec\n";
    // cout << "mat: " << mat.size() << " " << mat.capacity() << "\n";
    // cout << "mat[0]: " << mat[0].size() << " " << mat[0].capacity() << "\n";
    // cout << "vec: " << vec.size() << " " << vec.capacity() << "\n";
    // cout << "res: " << res.size() << " " << res.capacity() << "\n";
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            // cout << "\t\tBefore assigning 0\n";
            res[i] = 0;
            for (int j = 0; j < m; j++) {
                // cout << "\t\t\tBefore adding\n";
                res[i] = res[i] + mat[i][j] * vec[j];
            }
        }
    }
    // return res;
}
void sub_vect(vd& a, vd& b, vd& res, int m, int n_threads) {
    // cout << "\tsub_vect\n";
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = a[i] - b[i];
        }
    }    // return res;
}
void vec_to_scal(vd& a, double val, vd& res, int m, int n_threads) {
    // cout << "\tvec_to_scal\n";
    #pragma omp parallel num_threads(n_threads)
    {
        #pragma omp for schedule(dynamic, 100)
        for (int i = 0; i < m; i++) {
            res[i] = a[i] * val;
        }
    }
    // return res;
}

double norm(vd& vec, int m, int n_threads) {
    // cout << "\tnorm\n";
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
bool criterion(vector<vd>& A, vd& b, vd& x, double eps, int m, int n_threads) {
    // cout << "\tcriterion\n";
    vd sub(m, 0.0);
    mul_mat_vec(A, x, sub, m, n_threads);
    sub_vect(sub, b, sub, m, n_threads); // must work with two "sub"
    double numerat = norm(sub, m, n_threads);
    // double numerat = norm(sub_vect(mul_mat_vec(A, x, m, n_threads), b, m, n_threads), m, n_threads);
    double denomin = norm(b, m, n_threads);
    // free(sub);
    return (numerat / denomin) > eps;
}

vd easy_iter(vector<vd>& A, vd& b, double t, int m, int n_threads) {
    // cout << "\teasy_iter\n";
    // double* x = (double*)calloc(m, sizeof(double));
    // double* sub = (double*)malloc(m * sizeof(double));
    vd x(m, 0.0);
    vd sub(m, 0.0);
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
    cout << "Start\n";
    int m = atol(argv[1]);
    // double t = atof(argv[2]);
    int n_threads = atol(argv[2]);
    cout << "Before inits\n";
    vector<vd> A = init_matrix(m, n_threads);
    vd b = init_vector(m, n_threads);
    double t = 1.0 / m;
    // cout << t << "\n";
    // cout << "Max threds: " << omp_get_max_threads() << "\n";
    cout << "Before timer\n";
    const auto start{chrono::steady_clock::now()};
    vd res = easy_iter(A, b, t, m, n_threads);
    const auto end{chrono::steady_clock::now()};
    cout << "After timer\n";
    const chrono::duration<double> elapsed_seconds{end - start};
    cout << "Before file\n";
    fstream fout("result.csv", fstream::out | fstream::app);
    fout << n_threads << "," << elapsed_seconds.count() << "\n";
    // cout << "Result:\n";
    // cout << res[0] << " " << res[1] << " " << res[2] << " ... ";
    // cout << res[m - 3] << " " << res[m - 2] << " " << res[m - 1] << "\n";
    // cout << "Duration: " << elapsed_seconds.count() << "\n";
    // #pragma omp parallel num_threads(n_threads)
    // {
    //     #pragma omp for schedule(dynamic, 100)
    //     for (int i = 0; i < m; i++) {
    //         free(A[i]);
    //     }
    // }
    // free(A);
    // free(b);
    // free(res);
    fout.close();
    cout << "End\n";
    // cout << "Max threds: " << omp_get_max_threads() << "\n";
    return 0;
}
