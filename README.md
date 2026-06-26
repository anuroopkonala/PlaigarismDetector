# PlagiarismDetector

A C++20 plagiarism detection engine for C/C++ source code, built around Clang LibTooling.

## Features

| Feature | Details |
|---|---|
| **Lexer** | Clang `libclang` C API — correct tokenisation of any valid C/C++ with C++20 standard flags |
| **Normaliser** | Strips comments, maps all variable identifiers to a uniform `VAR` placeholder to prevent renaming bypasses, and normalises all literals to `NUM` |
| **TF-IDF cosine** | Bag-of-tokens IDF-weighted similarity, catches vocabulary overlap |
| **LCS** | Space-optimised $O(\|A\| \cdot \|B\|)$ DP, catches reordered and aligned code |
| **AST Normalisation** | Direct recursive n-ary tree construction from Clang AST cursor hierarchy |
| **Tree Edit Distance** | Zhang-Shasha algorithm on real AST structure |
| **Thread Pool** | Fixed-size `std::thread` pool with `std::future` exception propagation |
| **Reports** | Self-contained HTML (sortable, filterable) + structured JSON |

## Architecture

```
FileScanner
    │ (parallel, one task per file)
    ▼
Lexer ──► Normalizer (renaming & comment strip)
                 │
                 ▼
         SimilarityEngine (addDocument)
                 │
                 │ (parallel, one task per candidate pair)
                 ▼
    ┌──────────────────────────────────┐
    │  cosineSimilarity (TF-IDF)       │
    │  lcsSimilarity (LCS DP)          │
    │  ASTNormalizer + APTEDDistance   │
    └──────────────┬───────────────────┘
                   │ combined score
                   ▼
            ReportGenerator
          report.json  report.html
```

## Build

Requires: CMake ≥ 3.20, GCC ≥ 13, libclang, llvm.

### Windows (MSYS2 UCRT64)
```bash
# 1. Install dependencies
pacman -S mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-llvm mingw-w64-ucrt-x86_64-clang-tools-extra mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-gcc cmake

# 2. Configure build
cmake -S . -B build -G "Ninja"

# 3. Build project
cmake --build build
```

### Linux (Ubuntu 24.04+)
```bash
# 1. Install dependencies
sudo apt install libclang-18-dev llvm-18-dev cmake g++ ninja-build

# 2. Configure build
cmake -S . -B build \
    -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm \
    -DClang_DIR=/usr/lib/llvm-18/lib/cmake/clang \
    -G "Ninja"

# 3. Build project
cmake --build build
```

## Usage

```bash
./build/detector <input_dir> [options]

Options:
  --output    <dir>   Output directory for reports (default: .)
  --threshold <f>     Minimum combined score [0-1] (default: 0.3)
  --threads   <n>     Worker threads (default: hw_concurrency)
  --verbose           Per-file processing info
```

### Example

```bash
./build/detector ./submissions --output ./reports --threshold 0.4 --verbose
```

```
[INFO] Files found: 2
[INFO] Thread pool: 20 threads
[INFO]   alice.cpp — 47 tokens
[INFO]   bob.cpp — 47 tokens
[INFO] Processed: 2 files (0 skipped)
[INFO] Candidate pairs: 1
[INFO] Computing AST similarity for 1 pairs...
[INFO] Flagged pairs: 1

┌─ Plagiarism Report (1 pairs above threshold 0.1) ─
│
│  alice.cpp  ↔  bob.cpp
│    LCS=0.830  Cosine=0.973  AST=0.841  Combined=0.889
└──────────────────────────────────────────

[INFO] Reports written to: ./reports
```

## Tests

```bash
# Run unit tests
./build/tests
# Results: 25 passed
```

## Combined Score Weights

| Metric | Weight | Rationale |
|---|---|---|
| LCS Similarity | 40% | Sequential structural similarity |
| TF-IDF Cosine | 40% | Vocabulary/content overlap |
| AST Tree Similarity | 20% | Language-level structural similarity |

## Score Interpretation

| Combined Score | Risk Level |
|---|---|
| ≥ 0.8 | HIGH |
| 0.5 – 0.8 | MEDIUM |
| < 0.5 | LOW |
