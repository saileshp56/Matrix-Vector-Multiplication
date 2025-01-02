#include <iostream>
#include <chrono>
#include <emmintrin.h>
#include <xmmintrin.h>
#include <vector>
#include <random>
#include <cstdlib> 
#include <unistd.h>

using namespace std;
using namespace std::chrono;

// Generate vector of doubles
std::vector<double> genRandVectorDouble(size_t N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib(0.0, 1.0);
    std::vector<double> data(N);
    for (size_t i = 0; i < N; ++i) {
        data[i] = distrib(gen);
    }
    return data;
}
// Generate NxN matrix of doubles
std::vector<std::vector<double>> genRandMatrixDouble(size_t N) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distrib(0.0, 1.0);
    std::vector<std::vector<double>> matrix(N, std::vector<double>(N));
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            matrix[i][j] = distrib(gen);
        }
    }

    return matrix;
}

bool compareArrays(const double* b, const double* seq_result, size_t N) {
    if (!std::equal(b, b + N, seq_result)) {
        return false;
    }

    return true;
}

auto benchmark(const string& name, auto func) -> double {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    duration<double> durationSec = end - start;

    auto nano = duration_cast<nanoseconds>(end - start);

    cout << name << ": " << nano.count() << " nanoseconds" << endl;

    cout << name << ": " << durationSec.count() << " seconds" << endl;
    return durationSec.count();
}

int main() {
    const size_t N = 32768;

    auto A = genRandMatrixDouble(N);
    auto x = genRandVectorDouble(N);

    double seq_result[N] = {0.0};

    double countRegular = benchmark("Regular", [&]() {
        for (int Arow = 0; Arow < N; ++Arow) {
            for (int Acol = 0; Acol < N; ++Acol) {
                seq_result[Arow] += A[Arow][Acol] * x[Acol];
            }
        }
    });

    // This is all sequential access


    double b[N] = {0.0};


    int SM = sysconf (_SC_LEVEL1_DCACHE_LINESIZE) / sizeof(double);

    double countBlocked = benchmark("Block Multiplication", [&]() {
        for (int jBlock = 0; jBlock < N; jBlock += SM) {
            for (int iBlock = 0; iBlock < N; iBlock += SM) { // reuse for vector
                for (int i = iBlock; i < iBlock + SM; i++) {
                    for (int j = jBlock; j < jBlock + SM; j++) {
                        b[i] += A[i][j] * x[j];
                    
                    }
                }                
            }
        }
    });

    if (!compareArrays(b, seq_result, N)) {
        std::cout << "Result vectors are not identical." << std::endl;
    } else {
        std::cout << "Result vectors are the same." << std::endl;


    }

    return 0;
}
