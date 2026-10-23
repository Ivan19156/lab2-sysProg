#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cassert>

// Розмір матриці (змінюй якщо треба)
static const int N = 2048;

// Розмір блоку для cache-friendly транспонування
static const int BLOCK = 64;

// ─────────────────────────────────────────────
// Наївне транспонування: A[i][j] → B[j][i]
// Проблема: при читанні рядка A ми пишемо в
// різні рядки B → багато cache misses
// ─────────────────────────────────────────────
void transpose_naive(const float* __restrict__ A,
                           float* __restrict__ B,
                           int n)
{
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            B[j * n + i] = A[i * n + j];
}

// ─────────────────────────────────────────────
// Блочне (cache-friendly) транспонування
// Обробляємо матрицю блоками BLOCK×BLOCK
// Блок повністю поміщається в L1/L2 кеш →
// набагато менше cache misses
// ─────────────────────────────────────────────
void transpose_blocked(const float* __restrict__ A,
                              float* __restrict__ B,
                              int n)
{
    for (int i = 0; i < n; i += BLOCK)
        for (int j = 0; j < n; j += BLOCK)
            for (int ii = i; ii < i + BLOCK && ii < n; ++ii)
                for (int jj = j; jj < j + BLOCK && jj < n; ++jj)
                    B[jj * n + ii] = A[ii * n + jj];
}

// ─────────────────────────────────────────────
// Заповнення матриці псевдовипадковими числами
// ─────────────────────────────────────────────
void fill_matrix(float* M, int n)
{
    for (int i = 0; i < n * n; ++i)
        M[i] = static_cast<float>(rand()) / RAND_MAX;
}

// ─────────────────────────────────────────────
// Перевірка коректності: результати мають збігатись
// ─────────────────────────────────────────────
bool verify(const float* B1, const float* B2, int n)
{
    for (int i = 0; i < n * n; ++i)
        if (B1[i] != B2[i]) return false;
    return true;
}

int main()
{
    srand(42);

    const long long total = (long long)N * N;

    float* A       = new float[total];
    float* B_naive = new float[total];
    float* B_block = new float[total];

    fill_matrix(A, N);

    std::cout << "Matrix size: " << N << "x" << N
              << "  (" << (total * 4 / 1024 / 1024) << " MB per matrix)\n";
    std::cout << "Block size:  " << BLOCK << "x" << BLOCK << "\n\n";

    // ── Наївне транспонування ──────────────────
    {
        memset(B_naive, 0, total * sizeof(float));

        auto t0 = std::chrono::high_resolution_clock::now();

        // Повторюємо кілька разів щоб perf мав що вимірювати
        for (int rep = 0; rep < 10; ++rep)
            transpose_naive(A, B_naive, N);

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << "[NAIVE]   10 runs: " << ms      << " ms"
                  << "  |  avg: " << ms / 10.0       << " ms/run\n";
    }

    // ── Блочне транспонування ─────────────────
    {
        memset(B_block, 0, total * sizeof(float));

        auto t0 = std::chrono::high_resolution_clock::now();

        for (int rep = 0; rep < 10; ++rep)
            transpose_blocked(A, B_block, N);

        auto t1 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        std::cout << "[BLOCKED] 10 runs: " << ms      << " ms"
                  << "  |  avg: " << ms / 10.0       << " ms/run\n";
    }

    // ── Перевірка ─────────────────────────────
    if (verify(B_naive, B_block, N))
        std::cout << "\n[OK] Results match.\n";
    else
        std::cout << "\n[ERROR] Results differ!\n";

    delete[] A;
    delete[] B_naive;
    delete[] B_block;

    return 0;
}