#include <iostream>
#include <vector>
#include <cmath>
// #include <ctime>

#ifndef IS_FLOAT
#define IS_FLOAT false
#endif

#define N 10000000
#define pi 3.1415926535897932

using namespace std;

int main() {
    // cout << IS_FLOATT << "\n";
    if (IS_FLOAT) {
        vector<float> float_mas;
        float sumf = 0;
        float lenf = (2.0 * pi) / N;
        for (int i = 0; i < N; i++) {
            float_mas.push_back(sin(lenf * i));
            sumf += float_mas[i];
        }
        cout << "Float: " << sumf << "\n";
    }
    else {
        vector<double> double_mas;
        double sumd = 0;
        double lend = (2.0 * pi) / N;
        for (int i = 0; i < N; i++) {
            double_mas.push_back(sin(lend * i));
            sumd += double_mas[i];
        }
        cout << "Double: " << sumd << "\n";
    }
    // int t = clock();
    // cout << ((float)t / CLOCKS_PER_SEC * 1000) << " ms\n";
    return 0;
}