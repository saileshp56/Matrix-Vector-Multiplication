#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

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

bool compareArraysWithTolerance(const std::vector<double>& b, const std::vector<double>& seq_result, double epsilon = 1e-9) {
    for (size_t i = 0; i < b.size(); ++i) {
        if (std::abs(b[i] - seq_result[i]) > epsilon) {
            cout << "Mismatch at index " << i << ": b[i] = " << b[i] << ", seq_result[i] = " << seq_result[i] << endl;
            return false;
        }
    }
    return true;
}

auto benchmark(auto func) -> double {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    duration<double> durationSec = end - start;

    auto nano = duration_cast<nanoseconds>(end - start);

    return durationSec.count();
}

int main() {
    constexpr size_t N = 8000;

    auto A = genRandMatrixDouble(N);
    auto x = genRandVectorDouble(N);

    std::vector<double> seq_result(N, 0.0);

    double totalRegularTime = 0.0;
    double totalBlockedTime = 0.0;

    for (int i = 0; i < 100; ++i) {
        totalRegularTime += benchmark([&]() {
            std::fill(seq_result.begin(), seq_result.end(), 0.0);
            for (size_t Arow = 0; Arow < N; ++Arow) {
                for (size_t Acol = 0; Acol < N; ++Acol) {
                    seq_result[Arow] += A[Arow][Acol] * x[Acol];
                }
            }
        });
    }

    std::vector<double> b(N, 0.0);
    constexpr size_t SM = 16;  

    for (int i = 0; i < 100; ++i) {
        totalBlockedTime += benchmark([&]() {
            std::fill(b.begin(), b.end(), 0.0);
            for (size_t iBlock = 0; iBlock < N; iBlock += SM) {
                for (size_t jBlock = 0; jBlock < N; jBlock += SM) {
                    for (size_t i = iBlock; i < std::min(iBlock + SM, N); i++) {
                        double temp = 0.0;
                        for (size_t j = jBlock; j < std::min(jBlock + SM, N); j++) {
                            temp += A[i][j] * x[j];
                        }
                        b[i] += temp;
                    }
                }
            }
        });
    }

    std::cout << "Average Regular Time: " << totalRegularTime / 100.0 << " seconds" << std::endl;
    std::cout << "Average Block Multiplication Time: " << totalBlockedTime / 100.0 << " seconds" << std::endl;

    if (!compareArraysWithTolerance(b, seq_result)) {
        std::cout << "Result vectors are not identical." << std::endl;
    } else {
        std::cout << "Result vectors are the same." << std::endl;
    }

    return 0;
}
