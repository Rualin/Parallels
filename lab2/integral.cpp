#include <iostream>
// #include <math.h>
#include <cstdlib>
#include <chrono>
#include <cmath>
#include <fstream>
#include <omp.h>

using namespace std;
#define pi 3.14159265358979323846
const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

double func(double x) {
    return exp(-x * x);
}

double integrate_omp(double a, double b, int n, int n_threads) {
    double h = (b - a) / n;
    double sum = 0.0;
    #pragma omp parallel num_threads(n_threads)
    {
        double sumloc = 0.0;
        #pragma omp for
        for (int i = 0; i < n; i++) {
            sumloc += func(a + h * ((double)i + 0.5));
        }
        #pragma omp atomic
        sum += sumloc;
    }
    sum *= h;
    return sum;
}

int main(int argc, char** argv) {
    int n_threads = atol(argv[1]);
    const auto start{chrono::steady_clock::now()};
    double res = integrate_omp(a, b, nsteps, n_threads);
    const auto end{chrono::steady_clock::now()};
    const chrono::duration<double> elapsed_seconds{end - start};
    fstream fout("res_integral.csv", fstream::out | fstream::app);
    // cout << "Error: " << abs(res - sqrt(pi)) << "\n";
    // cout << "Duration: " << elapsed_seconds.count() << "\n";
    fout << n_threads << " " << elapsed_seconds.count() << "\n";
    fout.close();
    return 0;
}
