#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <utility>
#include <iomanip>

#include "funcs.hpp"


// using TYPE = double;

template <typename T>
std::pair<T, T> test_one_line(std::ifstream& log_file){
    std::string func_name;
    log_file >> func_name;

    std::size_t num_args; 
    log_file >> num_args;

    std::vector<T> args(num_args);
    for(int i = 0; i < num_args; i++){
        log_file >> args[i];
    }

    T logged_res;
    log_file >> logged_res;
    T real_res;
    if(func_name == "sqrt"){
        real_res = t_func::sqrt(args);
    }
    else if(func_name == "sin"){
        real_res = t_func::sin(args);
    }
    else if(func_name == "pow"){
        real_res = t_func::pow(args);
    }

    return {real_res, logged_res};
}


template <typename T>
bool eq(T x, T y, T eps){
    return std::fabs(std::fabs(x) - std::fabs(y)) <= eps;
}


template <typename T>
void test(std::ifstream& log_file){
    std::pair<T, T> res;
    std::size_t id;

    while(log_file >> id){
        res = test_one_line<T>(log_file);
        if(!eq<T>(res.first, res.second, 1e-3)){
            std::cerr << id << ": " << res.first << " != " << res.second << "\n";
        }
    }

    if(log_file.bad()){
        throw std::ios_base::failure("I/O error while reading\n");
    }
}


int main(){
    int code = 0;
    std::ifstream log_file("results.txt", std::ios::in);
    log_file >> std::fixed >> std::setprecision(19);

    try{
        std::cerr << std::fixed << std::setprecision(19);
        test<TYPE>(log_file);
        std::cout << "Test completed\n";
    }
    catch(const std::ios_base::failure& ex){
        std::cerr << ex.what();
        code = 1;
    } 
    
    log_file.close();
    return code;
}