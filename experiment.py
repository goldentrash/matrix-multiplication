import numpy as np
import subprocess

NUM_UPPER = 100
NUM_LOWER = -100
BIN_DIR = "data"
EXEC_DIR = "out"
CPP_EXEC = "solution.exe"

def save_matrix_binary(matrix, filename):
    # 크기 정보와 행렬 데이터를 하나의 바이너리 파일로 저장
    with open(filename, 'wb') as f:
        np.array([matrix.shape[0]], dtype=np.int32).tofile(f)  # 크기 저장
        matrix.astype(np.int32).tofile(f)

def read_matrix_binary(filename):
    with open(filename, 'rb') as f:
        # 크기 읽기
        n = np.fromfile(f, dtype=np.int32, count=1)[0]
        # 행렬 데이터 읽기
        return np.fromfile(f, dtype=np.int32).reshape((n, n))

def main():
    # x = int(input("Enter x for matrix size (2^x): "))
    n = 2 ** 11 # x
    
    # 난수 생성기 초기화 (재현성을 위해)
    np.random.seed(42)
    
    # 행렬 생성
    A = np.random.randint(NUM_LOWER, NUM_UPPER, (n, n), dtype=np.int32)
    B = np.random.randint(NUM_LOWER, NUM_UPPER, (n, n), dtype=np.int32)
    
    # 바이너리 형식으로 저장
    save_matrix_binary(A, f'./{BIN_DIR}/a.bin')
    save_matrix_binary(B, f'./{BIN_DIR}/b.bin')
    
    # C++ 프로그램 실행
    subprocess.run([f'./{EXEC_DIR}/{CPP_EXEC}'])
    
    # 결과 검증
    C_cpp = read_matrix_binary(f'./{BIN_DIR}/c.bin')
    C_numpy = A @ B

    if np.array_equal(C_cpp, C_numpy):
        print("Success")
    else:
        print("Fail")
        # print(A)
        # print(B)
        print(C_cpp)
        print(C_numpy)

if __name__ == "__main__":
    main()
