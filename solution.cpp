#include <intrin.h>
#include <omp.h>

#include <chrono>
#include <fstream>
#include <iostream>

using namespace std;
using namespace chrono;

const string BIN_DIR = "./data/";
struct Matrix {
  int size;
  int padded_size;
  int* aligned_ints;

  Matrix(int n) : size(n) {
    padded_size = (n + 15) & ~15;
    aligned_ints =
        (int*)_aligned_malloc(padded_size * padded_size * sizeof(int), 32);
    if (!aligned_ints) {
      throw runtime_error("Failed to allocate aligned memory");
    }
    memset(aligned_ints, 0, padded_size * padded_size * sizeof(int));
  }

  // 복사 생성자
  Matrix(const Matrix& other)
      : size(other.size), padded_size(other.padded_size) {
    aligned_ints =
        (int*)_aligned_malloc(padded_size * padded_size * sizeof(int), 32);
    if (!aligned_ints) {
      throw runtime_error("Failed to allocate aligned memory");
    }
    memcpy(aligned_ints, other.aligned_ints,
           padded_size * padded_size * sizeof(int));
  }

  // 이동 생성자
  Matrix(Matrix&& other) noexcept
      : size(other.size),
        padded_size(other.padded_size),
        aligned_ints(other.aligned_ints) {
    other.aligned_ints = nullptr;
    other.size = 0;
    other.padded_size = 0;
  }

  // 대입 연산자
  Matrix& operator=(const Matrix& other) {
    if (this != &other) {
      if (aligned_ints) {
        _aligned_free(aligned_ints);
      }

      size = other.size;
      padded_size = other.padded_size;
      aligned_ints =
          (int*)_aligned_malloc(padded_size * padded_size * sizeof(int), 32);
      if (!aligned_ints) {
        throw runtime_error("Failed to allocate aligned memory");
      }
      memcpy(aligned_ints, other.aligned_ints,
             padded_size * padded_size * sizeof(int));
    }
    return *this;
  }

  // 이동 대입 연산자
  Matrix& operator=(Matrix&& other) noexcept {
    if (this != &other) {
      if (aligned_ints) {
        _aligned_free(aligned_ints);
      }

      size = other.size;
      padded_size = other.padded_size;
      aligned_ints = other.aligned_ints;

      other.aligned_ints = nullptr;
      other.size = 0;
      other.padded_size = 0;
    }
    return *this;
  }

  ~Matrix() {
    if (aligned_ints) {
      _aligned_free(aligned_ints);
    }
  }

  int* data_ptr() { return aligned_ints; }
  const int* data_ptr() const { return aligned_ints; }
};

void init_matrix(Matrix& A);
void transpose_matrix(const Matrix& A, Matrix& B);
void multiply_tile(int* a, int* b, int* c, int y, int x, int n);
void multiply_matrix(const Matrix& A, const Matrix& B, Matrix& C);

Matrix read_matrix_binary(const string& filename) {
  ifstream file(BIN_DIR + filename, ios::binary);
  if (!file) {
    throw runtime_error("Cannot open file: " + BIN_DIR + filename);
  }

  int size;
  file.read(reinterpret_cast<char*>(&size), sizeof(size));

  Matrix mat(size);
  for (int i = 0; i < size; i++) {
    file.read(reinterpret_cast<char*>(mat.data_ptr() + i * mat.padded_size),
              size * sizeof(int));
  }

  return mat;
}

void save_matrix_binary(const Matrix& mat, const string& filename) {
  ofstream file(BIN_DIR + filename, ios::binary);
  if (!file) {
    throw runtime_error("Cannot open file for writing: " + BIN_DIR + filename);
  }

  file.write(reinterpret_cast<const char*>(&mat.size), sizeof(mat.size));
  for (int i = 0; i < mat.size; i++) {
    file.write(
        reinterpret_cast<const char*>(mat.data_ptr() + i * mat.padded_size),
        mat.size * sizeof(int));
  }
}

// void multiply_matrix_naive(const Matrix& A, const Matrix& B, Matrix& C) {
// #pragma omp parallel for num_threads(16)
//   for (int k = 0; k < A.size; k++) {
//     for (int y = 0; y < A.size; y++) {
//       for (int x = 0; x < A.size; x++) {
//         C.aligned_ints[y * A.size + x] +=
//             A.aligned_ints[y * A.size + k] * B.aligned_ints[k * A.size + x];
//       }
//     }
//   }
// }

int main() {
  try {
    // 행렬 읽기
    Matrix A = read_matrix_binary("a.bin");
    Matrix C = read_matrix_binary("b.bin");
    Matrix B = Matrix(A.size);

    // 연산 시간 측정 시작
    cout << "cpp start" << endl;
    auto start = high_resolution_clock::now();

    // init_matrix(B);
    // multiply_matrix_naive(A, C, B);

    // My Solution
    transpose_matrix(C, B);
    init_matrix(C);
    multiply_matrix(A, B, C);

    // 연산 시간 측정 종료
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Time taken: " << duration.count() << " microseconds" << endl;

    // 결과 저장
    save_matrix_binary(C, "c.bin");
    cout << "cpp end" << endl;
  } catch (const exception& e) {
    cout << "Error: " << e.what() << endl;
    return 1;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////

// @param A target matrix
void init_matrix(Matrix& A) {
  memset(A.aligned_ints, 0, A.padded_size * A.padded_size * sizeof(int));
}

// @param A from (leave as original)
// @param B to (transposed A is stored)
void transpose_matrix(const Matrix& A, Matrix& B) {
#pragma omp parallel for num_threads(16)
  for (int y = 0; y < A.padded_size; y += 16) {
    for (int x = 0; x < A.padded_size; x += 16) {
      // size of `cache_line` => 64bytes ( == 16 floats, == 16 ints)
      // watch out `false_sharing`!
      for (int yy = y; yy < y + 16; yy++) {
        for (int xx = x; xx < x + 16; xx++) {
          B.aligned_ints[yy * A.padded_size + xx] =
              A.aligned_ints[xx * A.padded_size + yy];
        }
      }
    }
  }
}

// @param A operand matrix
// @param B operand matrix
// @param C result matrix
void multiply_matrix(const Matrix& A, const Matrix& B, Matrix& C) {
#pragma omp parallel for num_threads(16)
  for (int y = 0; y < A.padded_size; y += 8) {
    for (int x = 0; x < A.padded_size; x += 8) {
      multiply_tile(A.aligned_ints, B.aligned_ints, C.aligned_ints, y, x,
                    A.padded_size);
    }
  }
}

// @param a operand matrix (flatted)
// @param b operand matrix (flatted)
// @param c result matrix (flatted)
// @param y tile location
// @param x tile location
// @param n size of matrix
void multiply_tile(int* a, int* b, int* c, int y, int x, int n) {
  // `__m256` => 8 floats => 64bytes ( == 256bit)
  // `__m256i` => 8 ints => 64bytes ( == 256bit)
  // size of `cache_line` => 64bytes ( == 256bit)
  for (int step = 0; step < n; step += 8) {
    for (int ai = 0; ai < 8; ai++) {
      for (int bi = 0; bi < 8; bi++) {
        // __m256 aa = _mm256_load_ps(a + (y + ai) * n + step);
        // __m256 bb = _mm256_load_ps(b + (x + bi) * n + step);
        // int* v = (int*)&_mm256_mul_ps(aa, bb);

        // overflow will be ignored
        __m256i aa = _mm256_load_si256((__m256i*)(a + (y + ai) * n + step));
        __m256i bb = _mm256_load_si256((__m256i*)(b + (x + bi) * n + step));
        int* v = (int*)&_mm256_mullo_epi32(aa, bb);

        // is there another solution using `avx`?
        v[0] = v[0] + v[1];
        v[2] = v[2] + v[3];
        v[4] = v[4] + v[5];
        v[6] = v[6] + v[7];

        v[0] = v[0] + v[2];
        v[4] = v[4] + v[6];

        c[(y + ai) * n + (x + bi)] += v[0] + v[4];
      }
    }
  }
}