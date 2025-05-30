#include <iostream>
#include <boost/program_options.hpp>
#include <new>
#include <nvtx3/nvToolsExt.h>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <fstream>
#include <cuda_runtime.h>
#include <cub/cub.cuh>
#include <cub/block/block_load.cuh>
#include <cub/block/block_reduce.cuh>
#include <cub/block/block_store.cuh>


using vd = double*;
int n, max_iters;
double eps;

#define ind(i, j) ((i) * n + (j))
#define cind(i, j, m) ((i) * (m) + (j))
#define max(a, b) ((a) > (b) ? (a) : (b))
#define abs(a) ((a) < 0 ? (0-(a)) : (a))
#define graph_step 512

// указатель для управления памятью на устройстве
template<typename T>
using cuda_unique_ptr = std::unique_ptr<T, std::function<void(T*)>>;

void f_exception(std::string message) {
    printf("%s!\n", message.c_str());
    exit(2);
}

// выделение памяти на устройстве
template<typename T>
T* cuda_new(size_t size) {
    T *d_ptr;
    cudaError_t status;
    status = cudaMalloc((void **)&d_ptr, sizeof(T) * size);
    if (status != cudaSuccess) f_exception(std::string("cudaMalloc error"));
    return d_ptr;
}
// освобождение ресурсов
template<typename T>
void cuda_free(T *dev_ptr) {
    cudaError_t status;
    status = cudaFree(dev_ptr);
    if (status != cudaSuccess) f_exception(std::string("cudaFree error"));
}

__global__ void sub_mats(const double *A, const double *Anew, double *subtr_res, int m) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    int j = blockDim.y * blockIdx.y + threadIdx.y;
    if ((i >= 0) && (i < m) && (j >= 0) && (j < m))
        subtr_res[cind(i, j, m)] = fabs(A[cind(i, j, m)] - Anew[cind(i, j, m)]);
}

__global__ void calc_mean(double *A, double *Anew, int m, bool flag) {
    int i = blockDim.x * blockIdx.x + threadIdx.x;
    int j = blockDim.y * blockIdx.y + threadIdx.y;
    if (flag) {
        if ((i > 0) && (i < m - 1) && (j > 0) && (j < m - 1))
            A[cind(j, i, m)] = 0.25 * (Anew[cind(j, i + 1, m)] + \
            Anew[cind(j, i - 1, m)] + Anew[cind(j - 1, i, m)] + Anew[cind(j + 1, i, m)]);
    }
    else {
        if ((i > 0) && (i < m - 1) && (j > 0) && (j < m - 1))
            Anew[cind(j, i, m)] = 0.25 * (A[cind(j, i + 1, m)] + \
            A[cind(j, i - 1, m)] + A[cind(j - 1, i, m)] + A[cind(j + 1, i, m)]);
    }
}

void init_grids(std::unique_ptr<double[]> &A, std::unique_ptr<double[]> &Anew) {
    nvtxRangePushA("init");
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
    nvtxRangePop();
}

void create_graph(cudaStream_t& stream, cudaGraph_t& graph, cudaGraphExec_t& instance, double* d_A, double* d_Anew) {
    cudaError_t cudaErr = cudaSuccess;
    dim3 grid(32, 32);
    dim3 block(32, 32);
    nvtxRangePushA("createGraph");
    // начало захвата операций на потоке stream
    cudaErr = cudaStreamBeginCapture(stream, cudaStreamCaptureModeGlobal);
    if (cudaErr != cudaSuccess) f_exception(std::string("cudaStreamBeginCapture error"));
    for (int i = 0; i < graph_step; i++)
        calc_mean<<<grid, block, 0, stream>>>(d_A, d_Anew, n, (i % 2 == 1));
    // завершение захвата операций
    cudaErr = cudaStreamEndCapture(stream, &graph);
    if (cudaErr != cudaSuccess) f_exception(std::string("cudaStreamEndCapture error"));
    nvtxRangePop();
    // создаем исполняемый граф
    cudaErr = cudaGraphInstantiate(&instance, graph, NULL, NULL, 0);
    if (cudaErr != cudaSuccess) f_exception(std::string("cudaGraphInstantiate error"));
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

int main(int argc, char **argv) {
    switch (parse_args(argc, argv)) {
        case 1:
            return 0;
        case 2:
            return 1;
        default:
            break;
    }

    std::unique_ptr<double[]> A_ptr(new double[n * n]);
    std::unique_ptr<double[]> Anew_ptr(new double[n * n]);

    init_grids(A_ptr, Anew_ptr);
    double* A = A_ptr.get();
    double* Anew = Anew_ptr.get();

    std::string cudaMalloc_err = "cudaMalloc error";
    std::string cudaMemcpy_err = "cudaMemcpy error";
    std::string cudaGraphLaunch_err = "cudaGraphLaunch error";
    std::string cudaStreamCreate_err = "cudaStreamCreate error";

    dim3 grid(32, 32);
    dim3 block(32, 32);

    cudaError_t cudaErr = cudaSuccess;
    cudaStream_t stream;
    
    cudaErr = cudaStreamCreate(&stream);
    if (cudaErr != cudaSuccess) f_exception(cudaStreamCreate_err);  

    cuda_unique_ptr<double> d_unique_ptr_error(cuda_new<double>(1), cuda_free<double>);
    cuda_unique_ptr<void> d_unique_ptr_temp_storage(cuda_new<void>(0), cuda_free<void>);

    cuda_unique_ptr<double> d_unique_ptr_A(cuda_new<double>(n*n), cuda_free<double>);
    cuda_unique_ptr<double> d_unique_ptr_Anew(cuda_new<double>(n*n), cuda_free<double>);
    cuda_unique_ptr<double> d_unique_ptr_subtr_temp(cuda_new<double>(n*n), cuda_free<double>);

    // выделение памяти и перенос на устройство
	double *d_error_ptr = d_unique_ptr_error.get();
    double *d_A = d_unique_ptr_A.get();
	double *d_Anew = d_unique_ptr_Anew.get();
    double *d_subtr_temp = d_unique_ptr_subtr_temp.get();

    // копирование матриц с хоста на gpu
    cudaErr = cudaMemcpy(d_A, A, n * n * sizeof(double), cudaMemcpyHostToDevice);
    if (cudaErr != cudaSuccess) f_exception(cudaMemcpy_err);
    cudaErr = cudaMemcpy(d_Anew, Anew, n * n * sizeof(double), cudaMemcpyHostToDevice);
    if (cudaErr != cudaSuccess) f_exception(cudaMemcpy_err);

    // проверка памяти для редукции
    void *d_temp_storage = d_unique_ptr_temp_storage.get();
    size_t temp_storage_bytes = 0;
    cub::DeviceReduce::Max(d_temp_storage, temp_storage_bytes, d_Anew, d_error_ptr, n * n, stream);
    cudaErr = cudaMalloc((void**)&d_temp_storage, temp_storage_bytes);
    if (cudaErr != cudaSuccess) f_exception(cudaMalloc_err);

    // printf("Jacobi relaxation Calculation: %d x %d mesh\n", n, n);

    cudaGraph_t graph;
    cudaGraphExec_t instance;

    int iter = 0;
    double error = eps + 1.0;
    create_graph(stream, graph, instance, d_A, d_Anew);

    nvtxRangePushA("while");
    auto start_time = std::chrono::steady_clock::now();
    while (error > eps && iter < max_iters) {
        // старт графа
        nvtxRangePushA("startGraph");
        cudaErr = cudaGraphLaunch(instance, stream);
        if (cudaErr != cudaSuccess) f_exception(cudaGraphLaunch_err);
        nvtxRangePop();

        iter += graph_step;
        if (iter % graph_step == 0) {
            nvtxRangePushA("calcError");
            sub_mats<<<grid, block, 0, stream>>>(d_A, d_Anew, d_subtr_temp, n);
            cub::DeviceReduce::Max(d_temp_storage, temp_storage_bytes, d_subtr_temp, d_error_ptr, n * n, stream);
            cudaErr = cudaMemcpy(&error, d_error_ptr, sizeof(double), cudaMemcpyDeviceToHost);
            if (cudaErr != cudaSuccess) f_exception(cudaMemcpy_err);
            nvtxRangePop();
        }
    }
    nvtxRangePop();
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> dur = end_time - start_time;

    cudaErr = cudaMemcpy(A, d_A, n * n * sizeof(double), cudaMemcpyDeviceToHost);
    if (cudaErr != cudaSuccess) f_exception(cudaMemcpy_err);

    // освобождение ресурсов
    cudaStreamDestroy(stream);
    cudaGraphDestroy(graph);
    cudaGraphExecDestroy(instance);

    std::cout << "Iters: " << iter << "\n";
    std::cout << "Error: " << error << "\n";
    std::cout << "Elapsed time: " << dur.count() << "\n";
    return 0;
}