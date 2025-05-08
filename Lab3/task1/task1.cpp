#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <mutex>


using vd = std::vector<double>;
std::mutex mut;

std::vector<vd> init_mat(int m, int n_threads, int id) {
    // std::cout << "Start init_mat\n";
    int div = m / n_threads, mod = m % n_threads;
    int lb = (id < mod) ? ((div + 1) * id) : (mod + div * id);
    int ub = (id < mod) ? ((div + 1) * (id + 1)) : (mod + div * (id + 1));
    // mut.lock();
    // std::cout << lb << " " << ub << "\n";
    // mut.unlock();
    std::vector<vd> res(m, vd(m, 5.0));
    // std::cout << "After vector vd\n";
    for (int i = lb; i < ub; i++) {
        res[i - lb][i] = 2.0;
    }
    // std::cout << "End init_mat\n";
    return res;
}

void mul_mat_vec(std::vector<vd>& mat, vd& vec, vd& res, int m, int n_threads, int id, std::unique_lock<std::mutex>& u_lock) {
    int div = m / n_threads, mod = m % n_threads;
    int lb = (id < mod) ? ((div + 1) * id) : (mod + div * id);
    int ub = (id < mod) ? ((div + 1) * (id + 1)) : (mod + div * (id + 1));
    double curr;
    for (int i = lb; i < ub; i++) {
        curr = 0.0;
        for (int j = 0; j < m; j++) {
            curr += mat[i][j] * vec[j];
        }
        u_lock.lock();
        res[i] = curr;
        u_lock.unlock();
    }
}

void thread_work(vd& res, int m, int n_threads, int id, double* init_times) {
    std::unique_lock<std::mutex> u_lock{mut, std::defer_lock};
    const auto start{std::chrono::steady_clock::now()};
    std::vector<vd> mat = init_mat(m, n_threads, id);
    std::vector<double> b(m, m - 1234);
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> init_time{end - start};
    mul_mat_vec(mat, b, res, m, n_threads, id, u_lock);
    u_lock.lock();
    // std::cout << id << "\n";
    init_times[id] = init_time.count();
    u_lock.unlock();
}

int main(int argc, char** argv) {
    int m = atol(argv[1]);
    int n_threads = atol(argv[2]);
    std::vector<std::thread> threads;
    // std::cout << "Start initialising threads\n";
    double* init_times = (double*)malloc(n_threads * sizeof(double));
    vd res(m, 0.0);
    const auto start{std::chrono::steady_clock::now()};
    for (int i = 0; i < n_threads; i++) {
        threads.push_back(std::thread(thread_work, std::ref(res), m, n_threads, i, init_times));
    }
    // std::cout << "Finish initialising threads\n";

    // std::cout << "\nStart joining threads\n";
    for (int i = 0; i < n_threads; i++) {
        threads[i].join();
    }
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    double max_init = 0;
    for (int i = 0; i < n_threads; i++) {
        max_init = max_init > init_times[i] ? max_init : init_times[i];
    }
    double work_time = elapsed_seconds.count() - max_init;
    std::fstream fout("result.csv", std::fstream::out | std::fstream::app);
    fout << n_threads << "," << m << "," << work_time << "\n";
    fout.close();
    // std::cout << "Finish joining threads\n";
    // std::cout << "Total time: " << elapsed_seconds.count() << "\n";
    // std::cout << "Working time: " << work_time << "\n";
}
