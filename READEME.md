# Matrix Multiplication with AVX

This project implements matrix multiplication using Advanced Vector Extensions (AVX). AVX is a SIMD (Single Instruction Multiple Data) instruction set extension that allows processing multiple data points simultaneously, optimizing the performance of matrix multiplication operations.

## Development Environment

- **OS**: Windows 11
- **IDE**: Visual Studio Code (launched through Developer Command Prompt for VS)
- **Compiler**: MSVC (Visual Studio's built-in C compiler)

### System Specifications

> Hardware information can be checked by running `/util/check_hw.cpp`

```sh
Processor Information:
==================================================
Physical Cores: 12
Logical Processors: 16
Hyper-Threading: Enabled
Maximum Parallel Threads: 16

CPU Cache Information:
==================================================

CPU Vendor: GenuineIntel

L1 Cache Information:
------------------------------
Type: Instruction Cache
Size: 32 KB
Line Size: 64 bytes
Instances: 12
Each instance shared by 2 logical processor(s)
------------------------------
Type: Data Cache
Size: 48 KB
Line Size: 64 bytes
Instances: 12
Each instance shared by 2 logical processor(s)
------------------------------

L2 Cache Information:
------------------------------
Type: Unified Cache
Size: 1280 KB
Line Size: 64 bytes
Instances: 6
Each instance shared by 2 logical processor(s)
------------------------------

L3 Cache Information:
------------------------------
Type: Unified Cache
Size: 12288 KB
Line Size: 64 bytes
Instances: 1
Each instance shared by 16 logical processor(s)
------------------------------
```

## Acknowledgements

This project was developed with the assistance of Claude AI by [Anthropic](https://www.anthropic.com).
