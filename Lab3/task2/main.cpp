#include <iostream>
#include <cmath>
#include <vector>

#include "client_server.hpp"
#include "funcs.hpp"


// using TYPE = double;
int num_calls;

bool parse_args(int argc, char** argv){
    if(argc != 2){
        return false;
    }
    num_calls = atol(argv[1]);
    return true;
}


int main(int argc, char** argv){
    if(parse_args(argc, argv) == false){
        std::cerr << "Wrong num of args\n";
        return 1;
    }

    server<TYPE> serv;
    std::thread server_thread(&server<TYPE>::start, &serv);
    std::cout << "Started server\n";

    client<TYPE>::open_log_file();
    std::cout << "Opened results.txt\n";

    client<TYPE> client_sin(t_func::sin, 1, "sin", &serv, num_calls);
    std::thread client_sin_thread(&client<TYPE>::start, &client_sin);
    std::cout << "Started client 1\n";

    client<TYPE> client_sqrt(t_func::sqrt, 1, "sqrt", &serv, num_calls);
    std::thread client_sqrt_thread(&client<TYPE>::start, &client_sqrt);
    std::cout << "Started client 2\n";

    client<TYPE> client_pow(t_func::pow, 2, "pow", &serv, num_calls);
    std::thread client_pow_thread(&client<TYPE>::start, &client_pow);
    std::cout << "Started client 3\n";

    client_sin_thread.join();
    client_sqrt_thread.join();
    client_pow_thread.join();
    std::cout << "Joined all clients\n";

    client<TYPE>::close_log_file();
    std::cout << "Closed results.txt\n";

    serv.stop();
    std::cout << "Stopped server\n";
    server_thread.join();
    std::cout << "Joined server\n";
    
    std::cout << "End of programm\n";
    return 0;
}
