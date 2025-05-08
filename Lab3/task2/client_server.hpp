#pragma once

#include <thread>
#include <future>
#include <condition_variable>
#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <unordered_map>
#include <functional>

#include <random>
#include <vector>
#include <iomanip>


template <typename T> class server{
private:
    std::queue<std::pair<size_t, std::future<T>>> tasks;
    std::unordered_map<size_t, T> results;
    size_t global_task_id;
    std::mutex mut;
    std::condition_variable cv;
    bool stop_server;

public:
    server(){
        global_task_id = 0;
        stop_server = false;
    }

    ~server(){
        results.clear();
    }

    void start(){
        std::unique_lock<std::mutex> lock(mut, std::defer_lock);

        while(!stop_server){
            lock.lock();
            cv.wait(lock, [this] { return !tasks.empty() || stop_server; });
            if(!tasks.empty()){
                results.insert({tasks.front().first, tasks.front().second.get()});
                tasks.pop();
            }
            lock.unlock();
            cv.notify_all();
        }
    }

    void stop(){
        stop_server = true;
        cv.notify_all();
    }

    size_t add_task(T (*task)(std::vector<T>&), std::vector<T>& task_args){
        std::future<T> res = std::async(std::launch::async, std::bind(task, task_args));
        size_t curr_task_id;
        
        {
            std::lock_guard<std::mutex> lock(mut);
            curr_task_id = ++global_task_id;
            tasks.push({curr_task_id, std::move(res)});
        }
        cv.notify_one();

        return curr_task_id;
    }

    T request_result(int res_id){
        std::unique_lock<std::mutex> lock(mut, std::defer_lock);
        T res;

        lock.lock();
        cv.wait(lock, [this, res_id] { return results.find(res_id) != results.end(); });
        res = results[res_id];
        results.erase(res_id);
        lock.unlock();

        return res;
    }
};


template <typename T> class client{
private:
    static std::ofstream log_file;
    static std::mutex file_mut;
    T (*task)(std::vector<T>&);
    std::string name;
    server<T>* serv;
    size_t num_calls;
    size_t num_args;

public:
    client(T (*task)(std::vector<T>&), size_t num_args, std::string&& name, server<T>* serv, size_t num_calls){
        this->task = task;
        this->num_args = num_args;
        this->name = name;
        this->serv = serv;
        this->num_calls = num_calls;
    }

    ~client(){
        name.clear();
    }

    void start(){
        std::unique_lock<std::mutex> file_lock(file_mut, std::defer_lock);
        size_t task_id;
        T res;
        std::vector<T> args(num_args);
        std::uniform_real_distribution<T> unif(1, 30);
        std::default_random_engine re;

        while(num_calls != 0){
            for(int i = 0; i < num_args; i++){
                args[i] = unif(re);
            }
            task_id = serv->add_task(task, args);
            res = serv->request_result(task_id);

            file_lock.lock();
            log_file << task_id << " " << name << " " << num_args << " ";
            for(const auto& arg : args){
                log_file << arg << " ";
            }
            log_file << res << "\n";
            file_lock.unlock();
            
            num_calls--;
        }

        args.clear();
    }

    static void open_log_file();
    static void close_log_file();
};


template<class T> std::ofstream client<T>::log_file;
template<class T> std::mutex client<T>::file_mut;


template<class T> inline void client<T>::open_log_file(){
    log_file.open("results.txt", std::ios::out);
    log_file << std::fixed << std::setprecision(19);
}


template<class T> inline void client<T>::close_log_file(){
    log_file.close();
}
