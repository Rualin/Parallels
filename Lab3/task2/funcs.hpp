#pragma once

#include <iostream>
#include <cmath>
#include <vector>


namespace t_func{
    template <typename T>
    T sin(std::vector<T>& args){
        return std::sin(args[0]);
    }

    template <typename T>
    T sqrt(std::vector<T>& args){
        return std::sqrt(args[0]);
    }

    template <typename T>
    T pow(std::vector<T>& args){
        return std::pow(args[0], args[1]);
    }
}
