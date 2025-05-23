#include <iostream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <boost/program_options.hpp>


// using vd = std::vector<double>;
using vd = double*;
int n, max_iters;
double eps;

#define ind(i, j) ((i) * n + (j))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define abs(a) ((a) < 0 ? (0-(a)) : (a))

vd interpolation(double start, double end) {
    vd res = (vd)malloc(n * sizeof(double));
    double curr = start;
    double dx = (end - start) / (n - 1);
    for (int i = 0; i < n; i++) {
        res[i] = curr;
        curr += dx;
    }
    return res;
}
vd init_grid() {
    vd res = (vd)calloc(n * n, sizeof(double));
    //  10 ... 20
    // ... ... ...
    //  20 ... 30
    vd inter = interpolation(10, 20);
    for (int i = 0; i < n; i++) {
        res[ind(0, i)] = inter[i];
        res[ind(i, 0)] = inter[i];
    }
    inter = interpolation(20, 30);
    for (int i = 0; i < n; i++) {
        res[ind(i, n - 1)] = inter[i];
        res[ind(n - 1, i)] = inter[i];
    }
    free(inter);
    return res;
}

void print_matrix(vd mat, std::ostream& out=std::cout) {
    out << "Matrix " << n << "x" << n << "\n";
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            std::cout << std::fixed << std::setprecision(2) 
                      << std::setw(6) << mat[ind(i, j)] << " ";
        }
        out << "\n";
    }
}

std::pair<int, double> method_Jacobi(vd A, vd A_new) {
    int iter = 0;
    double error = eps + 1; // to enter while loop
    int sub_n = n - 1;
    // double* tmp;
    while(error > eps && iter < max_iters) {
        error = 0;
        for (int i = 1; i < sub_n; i++) {
            for (int j = 1; j < sub_n; j++) {
                A_new[ind(i, j)] = (A[ind(i - 1, j)] + A[ind(i + 1, j)] + 
                                    A[ind(i, j - 1)] + A[ind(i, j + 1)]) * 0.25;
                error = max(error, abs(A_new[ind(i, j)] - A[ind(i, j)]));
            }
        }
        // print_matrix(A);
        // print_matrix(A_new);
        // std::cout << "\n\n";
        std::swap(A, A_new);
        // tmp = A;
        // A = A_new;
        // A_new = tmp;

        iter++;
    }
    return std::make_pair(iter, error);
}

int parse_args(int argc, char** argv) {
    boost::program_options::options_description desc("Heat Equation Solver Options");
    desc.add_options()
        ("help", "help message")
        ("n", boost::program_options::value<int>()->default_value(256), "grid size")
        ("eps", boost::program_options::value<double>()->default_value(1.0e-6), "precision")
        ("iter", boost::program_options::value<int>()->default_value(1000000), "max iterations")
        ("profile", "enable profiling");
    
    boost::program_options::variables_map vm;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);
        
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }
        
        n = vm["n"].as<int>();
        eps = vm["eps"].as<double>();
        max_iters = vm["iter"].as<int>();
        
        if (vm.count("profile")) {
            max_iters = 50;  // Для профилирования
            std::cout << "PROFILING MODE\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        return 2;
    }
    return 0;
}

int main(int argc, char** argv) {
    switch (parse_args(argc, argv)) {
        case 1:
            return 0;
            break;
        case 2:
            return 1;
            break;
        default:
            break;
    }

    vd A = init_grid();
    vd A_new = init_grid();
    // print_matrix(A);
    auto start = std::chrono::steady_clock::now();
    std::pair<int, double> res = method_Jacobi(A, A_new);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> dur = end - start;
    std::cout << "Iters: " << res.first << "\n";
    std::cout << "Error: " << res.second << "\n";
    std::cout << "Elapsed time: " << dur.count() << "\n";
    // print_matrix(A);
    free(A);
    free(A_new);
}
