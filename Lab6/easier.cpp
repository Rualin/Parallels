#include <iostream>
#include <cstdlib>
// #include <omp.h>


using namespace std;

#define NUM_TESTS 5

int main() {
    system("rm -f result.csv");
    // string threads[8] = {"1", "2", "4", "7", "8", "16", "20", "40"};
    // cout << "First 20000:\n";
    // for (int i = 0; i < 8; i++) {
    //     cout << "\t" << threads[i] << "\n";
    //     for (int j = 0; j < NUM_TESTS; j++) {
    //         cout << "\t\t" << j << "\n";
    //         system((string("./task1 ") + string("20000 ") + threads[i]).c_str());
    //     }
    // }
    string ns[4] = {"128", "256", "512", "1024"};
    string names[3] = {"cpu", "cpu_mult"};
    for (int j = 0; j < 2; j++) {
        cout << "\t" << names[j] << "\n";
        for (int i = 0; i < 4; i++) {
            for (int k = 0; k < NUM_TESTS; k++) {
                cout << ns[i] << ":\n";
                system((string("./") + names[j] + string(" --n ") + ns[i]).c_str());
            }
        }
    }
    return 0;
}
