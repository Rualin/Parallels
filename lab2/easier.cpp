#include <iostream>
#include <cstdlib>


using namespace std;

int main() {
    system("rm res_mat_vec.csv res_integral.csv");
    string threads[8] = {"1", "2", "4", "7", "8", "16", "20", "40"};
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 30; j++) {
            system((string("./mat_vec ") + string("20000 ") + threads[i]).c_str());
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 30; j++) {
            system((string("./mat_vec ") + string("40000 ") + threads[i]).c_str());
        }
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 30; j++) {
            system((string("./integral ") + threads[i]).c_str());
        }
    }
}
