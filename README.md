# Mercator Series Calculator

A parallel implementation of the Mercator series approximation for calculating logarithmic values using shared memory and semaphores in C.

## Overview

This program calculates an approximation of `ln(1 + x)` using the Mercator series:

```
ln(1 + x) = x - x²/2 + x³/3 - x⁴/4 + ... + (-1)^(n+1) * x^n/n + ...
```

The calculation is distributed across multiple processes using shared memory for inter-process communication and semaphores for synchronization.

## Features

- Parallel computation using multiple processes (default: 4)
- Shared memory implementation for efficient data sharing
- Semaphore-based synchronization between worker processes and master process
- Performance timing for execution analysis
- Comparison with standard library logarithmic function

## Requirements

- GCC compiler
- POSIX-compliant operating system (Linux, macOS, etc.)
- Math library (`libm`)

## Compilation

Compile the program with:

```bash
gcc -o mercator mercator.c -lm
```

The `-lm` flag is required to link the math library.

## Usage

Simply run the compiled executable:

```bash
./mercator
```

The program will:
1. Create worker processes to calculate portions of the Mercator series
2. Sum the partial results from each process
3. Display the calculated result
4. Compare with the standard library's `log()` function
5. Show execution time

## Configuration

You can modify the following parameters in the source code:

- `NPROCS`: Number of worker processes (default: 4)
- `SERIES_MEMBER_COUNT`: Number of terms in the series (default: 200000)
- `x`: The value for which to calculate ln(1 + x) (default: 1.0)

## Example Output

```
Mercator series member count for ln(1 + x) is 200000
The value of argument x is 1.000000
Execution time = 0.123456 seconds
The result is 0.69314718
Calling the function ln(1 + 1.000000) = 0.69314718
```

## How It Works

1. The program divides the series calculation across multiple worker processes
2. Each process calculates terms with indices matching `i % NPROCS == process_id`
3. Results are stored in shared memory
4. Semaphores coordinate the start of calculations and the final summation
5. A master process aggregates results from all worker processes

## Implementation Details

- Shared memory is used for storing partial sums and the final result
- Three semaphores control the execution flow:
  - `sem_start_all`: Signals worker processes to begin calculations
  - `sem_count`: Protects access to the process counter
  - `sem_master`: Signals the master process when all workers are done
