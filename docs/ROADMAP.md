# Roadmap

## Phase 1  — Infrastructure
- CLI argument parsing (`--threshold`, `--threads`, `--output`, `--verbose`)
- Recursive folder scanner (`.c`, `.cpp`, `.cc`, `.h`, `.hpp`)
- Structured logger (`[INFO]`, `[WARN]`, `[ERROR]`)

## Phase 2  — Lexical Analysis (Clang LibTooling)
- `Lexer`: tokenises C/C++ via `libclang` C API with standard-compliant C++20 and C11 compilation arguments.
- `Normalizer`: strips comments, maps all variable identifiers to a uniform `VAR` placeholder to prevent renaming bypasses, and maps all literals to `NUM`.

## Phase 3  — Similarity Metrics
- **TF-IDF cosine** (`SimilarityEngine::cosineSimilarity`): bag-of-tokens weighted by IDF; catches vocabulary overlap.
- **LCS** (`SimilarityEngine::lcsSimilarity`): space-optimised $O(\|A\| \cdot \|B\|)$ DP with $O(\min)$ space; catches sequential structure, insertions, and reordered code blocks.

## Phase 4  — AST Normalisation & Tree Edit Distance
- `ASTNormalizer`: walks AST recursively via `clang_visitChildren` to construct a real hierarchical AST `TreeNode` structure, filtering out system headers and folding unmapped nodes.
- `APTEDDistance`: Zhang-Shasha (1989) tree edit distance on the real AST tree structures, normalised to [0,1].

## Phase 5  — Multithreaded Pipeline
- `ThreadPool`: fixed-size pool with `std::packaged_task` + `std::future`; exception propagation.
- `Pipeline`: parallelizes file lexical analysis and pairwise candidate scoring.

## Phase 6  — Reports
- **JSON** (`report.json`): ISO-8601 timestamp, all scores + risk level per pair.
- **HTML** (`report.html`): self-contained, sortable table, colour-coded HIGH/MEDIUM/LOW badges, per-metric score bars, live score/risk filter.

