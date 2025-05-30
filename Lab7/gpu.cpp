#include <iostream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <boost/program_options.hpp>
#include <cublas_v2.h>


using vd = double*;
int n, max_iters;
double eps;

#define ind(i, j) ((i) * n + (j))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define abs(a) ((a) < 0 ? (0-(a)) : (a))


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

void init_grids(std::unique_ptr<double[]> &A, std::unique_ptr<double[]> &Anew) {
    memset(A.get(), 0, n * n * sizeof(double));
    //  10 ... 20
    // ... ... ...
    //  20 ... 30
    A[ind(0, 0)] = 10;
    A[ind(0, n - 1)] = 20;
    A[ind(n - 1, n - 1)] = 30;
    A[ind(n - 1, 0)] = 20;
    double dx = 10.0 / (n - 1);
    for (int i = 1; i < n - 1; i++) {
        A[ind(i, 0)] = 10.0 + dx * (double)i;
        A[ind(i, n - 1)] = 20.0 + dx * (double)i;
        A[ind(0, i)] = 10.0 + dx * (double)i;
        A[ind(n - 1, i)] = 20.0 + dx * (double)i;
    }
    std::memcpy(Anew.get(), A.get(), n * n * sizeof(double));
}

std::pair<int, double> method_Jacobi(vd A, vd Anew, cublasHandle_t& handler, cublasStatus_t& status) {
    int iter = 0, idx;
    double error = eps + 1; // to enter while loop
    double alpha = -1.0;
    #pragma acc enter data copyin(A[0:(n * n)], Anew[0:(n * n)], error, idx, alpha)

    while(error > eps && iter < max_iters) {
        #pragma acc parallel loop independent collapse(2) vector_length(n) num_gangs(n) present(A,Anew)
        for (int i = 1; i < n - 1; i++) {
            for (int j = 1; j < n - 1; j++) {
                Anew[ind(i, j)] = (A[ind(i - 1, j)] + A[ind(i + 1, j)] + 
                                    A[ind(i, j - 1)] + A[ind(i, j + 1)]) * 0.25;
            }
        }
        if (iter % 999 == 0) {
            #pragma acc data present (A, Anew) wait // ожидания завершения асинхронных операций
            #pragma acc host_data use_device(A, Anew) // host_data - следующий блок кода выполняется на хосте, но будет использовать указатели на данные с устройства, use_device(A, Anew) - для A и Anew должны быть использованы указатели с устройства
            {
                status = cublasDaxpy(handler, n * n, &alpha, Anew, 1, A, 1); // alpha*Anew + A, записывается в A
                if (status != CUBLAS_STATUS_SUCCESS) {
                    std::cerr << "cublasDaxpy failed with err code: " << status << std::endl;
                    exit (3);
                }
                status = cublasIdamax(handler, n * n, A, 1, &idx); // в idx - индекс максимального элемента из A, шаг 1, кол-во элементов n*n
                if (status != CUBLAS_STATUS_SUCCESS) {
                    std::cerr << "cublasIdamax failed with err code: " << status << std::endl;
                    exit (3);
                }
            }
            #pragma acc update host(A[idx - 1]) // переносит на хост
            error = abs(A[idx - 1]);

            #pragma acc host_data use_device(A, Anew)
            status = cublasDcopy(handler, n * n, Anew, 1, A, 1); // копирование из Anew в A
            if (status != CUBLAS_STATUS_SUCCESS) {
                std::cerr << "cublasDcopy failed with err code: " << status << std::endl;
                exit (3);
            }
            // std::swap(A, Anew);
        }

        std::swap(A, Anew);
        iter++;
    }
    std::swap(A, Anew);
    #pragma acc update self(A[0:(n * n)])
    #pragma acc exit data delete(A[0:(n * n)], Anew[0:(n * n)])
    
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
            max_iters = 50;  // for profiling
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
        case 2:
            return 1;
        default:
            break;
    }

    std::unique_ptr<double[]> A_p(new double[n * n]);
    std::unique_ptr<double[]> A_new_p(new double[n * n]);
    init_grids(std::ref(A_p), std::ref(A_new_p));
    double* A = A_p.get();
    double* A_new = A_new_p.get();

    cublasHandle_t handler;
	cublasStatus_t status;
    status = cublasCreate(&handler);
    if (status != CUBLAS_STATUS_SUCCESS) {
        std::cout << "cublasCreate failed with err code: " << status << std::endl;
        return 3;
    }

    auto start = std::chrono::steady_clock::now();
    std::pair<int, double> res = method_Jacobi(A, A_new, handler, status);
    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> dur = end - start;
    std::cout << "Iters: " << res.first << "\n";
    std::cout << "Error: " << res.second << "\n";
    std::cout << "Elapsed time: " << dur.count() << "\n";
    // print_matrix(A);
    // print_matrix(A_new);
    // free(A);
    // free(A_new);
}
