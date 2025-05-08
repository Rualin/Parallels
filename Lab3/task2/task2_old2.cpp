#include <iostream>
#include <thread>
#include <cmath>
#include <queue>
#include <future>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <random>
#include <fstream>


#define num_tasks 5
std::mutex mut;

template<typename T>
T fun_sin(std::vector<T>& arg) {
    if (arg.size() != 1) throw std::range_error("Incorrect number of arguments!\n");
    return std::sin(arg[0]);
}
template<typename T>
T fun_sqrt(std::vector<T>& arg) {
    if (arg.size() != 1) throw std::range_error("Incorrect number of arguments!\n");
    return std::sqrt(arg[0]);
}
template<typename T>
T fun_pow(std::vector<T>& arg) {
    if (arg.size() != 2) throw std::range_error("Incorrect number of arguments!\n");
    return std::pow(arg[0], arg[1]);
}

template<typename T>
T gen_random(T lower_bound=0, T upper_bound=10000) {
    const long max_rand = 1000000L;
    T random_val = lower_bound + (upper_bound - lower_bound) * (random() % max_rand) / max_rand;
    return random_val;
}

// std::queue<std::pair<size_t, std::future<std::string>>> tasks;
// std::unordered_map<int, int> results;
// template<typename T>
// using arg_type = std::pair<int, std::pair<T, T>>;

// template<typename T>
// // using queue_elem = std::tuple<size_t, std::packaged_task<T()>, std::vector<T>>;
// using queue_elem = std::pair<size_t, std::future<T>>;
template<typename T>
class Server {
    private:
        size_t last_task_id;
        // bool is_working;
        std::queue<std::pair<size_t, std::future<T>>> tasks;
        // std::queue<queue_elem> tasks;
        std::unordered_map<size_t, T> results;
        std::condition_variable stopVar;
    public:
        Server() {
            last_task_id = 1;
        }
        ~Server() {
            results.clear();
        }

        void start() {
            std::unique_lock<std::mutex> u_lock{mut, std::defer_lock};
            u_lock.lock();
            std::cout << "Server has started\n";
            u_lock.unlock();
            // is_working = true;
            while (stopVar.wait_for(u_lock, std::chrono::milliseconds(50)) == std::cv_status::timeout) {
                u_lock.lock();
                if (!(tasks.empty())) {
                    // std::pair<size_t, std::future<T>> task = tasks.front();
                    // queue_elem task = tasks.front();
                    // tasks.pop();
                    // u_lock.unlock();
                    // size_t id_task = std::get<0>(task);
                    // // std::packaged_task<T()> func = std::move(std::get<1>(task));
                    // std::function<T(std::vector<T>)&> func = std::get<1>(task);
                    // std::vector<T> args = std::get<2>(task);
                    // size_t id_task = task.first;
                    // T result = (task.second).get();
                    // u_lock.unlock();
                    // T res;
                    // // func();
                    // if (args.size() == 1) {
                    //     res = func(args[0]);
                    // }
                    // else if (args.size() == 2) {
                    //     res = func(args[0], args[1]);
                    // }
                    // else throw std::range_error("Unsupported number of arguments!\n");
                    u_lock.lock();
                    results.insert({tasks.front().first, tasks.front().second.get()});
                    tasks.pop();
                    u_lock.unlock();
                }
                else u_lock.unlock();
            }
            u_lock.lock();
            std::cout << "Server has stopped\n";
            u_lock.unlock();
        }
        void stop() {
            // std::unique_lock<std::mutex> u_lock{mut, std::defer_lock};
            // is_working = false;
            stopVar.notify_one();
        }
        size_t add_task(T (*func)(std::vector<T>&), std::vector<T> args) {
        // size_t add_task(std::packaged_task<T()>& task) {
            std::unique_lock<std::mutex> u_lock{mut, std::defer_lock};
            std::future<T> fut = std::async(std::launch::async, std::bind(func, args));
            u_lock.lock();
            size_t curr_id = last_task_id;
            last_task_id++;
            tasks.push(std::make_pair<size_t, std::future<T>>(std::move(curr_id), fut));
            u_lock.unlock();
            // tasks.push(std::tuple<size_t, std::packaged_task<T()>, std::vector<T>>(last_task_id, task.first, task.second));
            // tasks.push(std::pair<size_t, std::packaged_task<T()>&>(last_task_id, task));
            //cond_var to warn?
            return curr_id;
        }
        T request_result(size_t id_res) {
            // std::unique_lock<std::mutex> u_lock{mut, std::defer_lock};
            //fut.get()?
            if (results.count(id_res)) {
                return results[id_res];
            }
            else throw std::range_error("There is no element with that id!\n");
        }
};

using work_type = double;
Server<work_type> servak;
// Server servak;

template<typename T>
void client_work(T (*func)(std::vector<T>&), std::fstream& fout, int cnt_of_arguments) {
// void client_work(std::function<T(std::vector<T>&)>& func, std::fstream& fout, int cnt_of_arguments) {
    for (int r = 0; r < num_tasks; r++) {
        //adding task:
        std::vector<T> argument;
        for (int i = 0; i < cnt_of_arguments; i++) {
            // argument.push_back(gen_random<T>());
            argument.push_back(gen_random<T>());
        }
        std::unique_lock<std::mutex> u_lock(mut, std::defer_lock);
        u_lock.lock();
        // std::pair<T (*)(std::vector<T>&), std::vector<T>> sub = std::make_pair(func, argument);
        size_t id_task = servak.add_task(func, argument);
        u_lock.unlock();
        // size_t id_task;
        // std::packaged_task<T()> task;
        // if (cnt_of_arguments == 1) {
        //     task = std::packaged_task<T()>(std::bind(func, argument[0]));
        // }
        // else if (cnt_of_arguments == 2) {
        //     task = std::packaged_task<T()>(std::bind(func, argument[0], argument[1]));
        // }
        // else throw std::range_error("Unsupported number of arguments!\n");
        // std::future<T> fut = task.get_future(); //why unlighted?
        // u_lock.lock();
        // id_task = servak.add_task(task);
        // u_lock.unlock();
        // T res = fut.get();

        //getting result:
        bool is_problem = false;
        T result;
        do {
            u_lock.lock();
            try {
                result = servak.request_result(id_task);
                is_problem = false;
            }
            catch (...) {
                is_problem = true;
            }
            u_lock.unlock();
            if (is_problem) std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } while (is_problem);

        //writing result in file:
        u_lock.lock();
        fout << cnt_of_arguments;
        for (int i = 0; i < cnt_of_arguments; i++) {
            fout << "," << argument[i];
        }
        fout << "," << result << "\n";
        u_lock.unlock();
        //iomanip?
    }
}


int main() {
    std::cout << "Start program\n";

    std::fstream fsin("result_sin.csv", std::fstream::out | std::fstream::app);
    std::fstream fsqrt("result_sqrt.csv", std::fstream::out | std::fstream::app);
    std::fstream fpow("result_pow.csv", std::fstream::out | std::fstream::app);
    srandom(time(NULL));
    std::thread server_thread(std::bind(&Server<work_type>::start, &servak));
    std::vector<std::thread> clients;
    // clients.push_back(std::thread(std::bind(&Server<work_type>::add_task, &servak, std::bind(fun_sin<work_type>, std::placeholders::_1))));
    // clients.push_back(std::thread(std::bind(&Server<work_type>::add_task, &servak, std::bind(fun_sqrt<work_type>, std::placeholders::_1))));
    // clients.push_back(std::thread(std::bind(&Server<work_type>::add_task, &servak, std::bind(fun_pow<work_type>, std::placeholders::_1, std::placeholders::_2))));

    clients.push_back(std::thread(client_work<work_type>, fun_sin<work_type>, fsin, 1));
    clients.push_back(std::thread(client_work<work_type>, fun_sqrt<work_type>, fsqrt, 1));
    clients.push_back(std::thread(client_work<work_type>, fun_pow<work_type>, fpow, 2));
    // clients.push_back(std::thread(std::bind(&Server<T>::add_task, &servak, std::bind(fun_sin<T>, std::rand()))));
    // clients.push_back(std::thread(std::bind(&Server<T>::add_task, &servak, std::bind(fun_sqrt<T>, std::placeholders::_1))));
    // clients.push_back(std::thread(std::bind(&Server<T>::add_task, &servak, std::bind(fun_pow<T>, std::placeholders::_1, std::placeholders::_2))));

    server_thread.join();
    for (int i = 0; i < 3; i++) {
        clients[i].join();
    }
    std::cout << "End program\n";
    return 0;
}
