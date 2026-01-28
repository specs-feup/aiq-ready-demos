# ONNX-Flow Demo

This document describes how to install the **ONNX-Flow demo release**, obtain the example ONNX models, and reproduce the demo:

- Run the **full** compatibility test suite (“all” set)
- Run the **core** subset of tests (the full models the tool is able to decompose end-to-end)
- Run **feature examples**:
  - `add_chain_standard.onnx` / `matmuladd_test.onnx` with and without loop fusion
  - `softmax_standard.onnx` / `avgpool_standard.onnx` with and without loop lowering
- Run **any ONNX model** with arbitrary settings and optional equivalence checking

---

## 1. Requirements

- Node.js ≥ 16
- npm (bundled with Node.js)
- Terminal / shell:
  - Linux/macOS: bash or zsh
  - Windows: PowerShell or cmd

No TypeScript, build tools, or source repository are required on the demo machine.

---

## 2. Install the ONNX-Flow demo package

Install the demo-tagged version of the package globally:

```bash
npm install -g @specs-feup/onnx-flow@demo
```

This installs the following CLI commands:

- `onnx-flow` – main CLI for running transformations
- `onnx-flow-testcomp` – compatibility test runner

Check that they are available:

```bash
onnx-flow --help
onnx-flow-testcomp --help
```

---

## 3. Download the example ONNX models

The ONNX models used in the demo are provided separately.

1. Download the archive from Google Drive:

   **Google Drive link:**
   `https://drive.google.com/drive/folders/17JFcR5ElsBJr9U48uBXiH0VU-k9js0uC?usp=drive_link`

2. Extract the archive. After extraction, you should have:

   ```text
   examples/
     onnx/
       add_chain_standard.onnx
       matmuladd_test.onnx
       softmax_standard.onnx
       avgpool_standard.onnx
       … (other models used in tests)
   ```

---

## 4. Prepare the demo environment

The demo assumes the `examples` folder sits in the **same directory** as the installed `onnx-flow` package.

1. Find the global `node_modules` directory:

   ```bash
   npm root -g
   ```

   Typical values:

   - Linux/macOS: `/usr/local/lib/node_modules`
   - Windows: `C:\Users\<username>\AppData\Roaming\npm\node_modules`

2. Change into the installed package directory:

   ```bash
   cd "$(npm root -g)/@specs-feup/onnx-flow"
   ```

   On Windows PowerShell:

   ```powershell
   cd (npm root -g)'\@specs-feup\onnx-flow'
   ```

3. Copy the `examples` folder you downloaded into this directory:

   ```bash
   # From inside the onnx-flow package directory:
   cp -r /path/to/downloaded/examples .
   ```

   After this step, the structure should be:

   ```text
   <global-npm-root>/@specs-feup/onnx-flow/
     out/
     examples/
       onnx/
         add_chain_standard.onnx
         matmuladd_test.onnx
         softmax_standard.onnx
         avgpool_standard.onnx
         ...
   ```

4. Keep the working directory set to the package root for the rest of the demo:

   ```bash
   cd "$(npm root -g)/@specs-feup/onnx-flow"
   ```

---

## 5. Running the test suites

### 5.1 Full test suite (“all” set)

This runs all configured compatibility tests. Each test:

- Decomposes and reconverts the ONNX model
- Runs ONNX Runtime on original and reconverted models
- Compares outputs within a defined tolerance

Run:

```bash
onnx-flow-testcomp
# or explicitly:
COMPAT_MODE=all onnx-flow-testcomp
```

Windows PowerShell:

```powershell
$env:COMPAT_MODE = "all"
onnx-flow-testcomp
```

A summary of passed/failed tests and any non-equivalent models is printed at the end.

---

### 5.2 Core tests (core set)

The core set comprises selected tests corresponding to the full models the tool is currently able to decompose end-to-end.

Run:

```bash
COMPAT_MODE=core onnx-flow-testcomp
```

Windows PowerShell:

```powershell
$env:COMPAT_MODE = "core"
onnx-flow-testcomp
```

---

## 6. Feature examples

All commands in this section are intended to be run from:

```bash
cd "$(npm root -g)/@specs-feup/onnx-flow"
```

General CLI pattern:

```bash
onnx-flow <input.onnx> [options]
```

Key options:

- `--fuse` / `--no-fuse`
  Enable/disable loop fusion (fuse supported ops into a single Loop).
- `--coalesce` / `--no-coalesce`
  Enable/disable coalesced scalar MAC for MatMul inside Loop bodies.
- `--loopLowering` / `--no-loop-lowering`
  Enable/disable loop lowering (explicit Loop nodes vs high-level ops).
- `--decomposeForCgra`
  Enable CGRA-oriented decomposition.
- `--formatter default|cgra`
  Select DOT formatter (default graph view vs CGRA-oriented output).
- `--checkEquivalence` or `-qe`
  Run ONNX Runtime equivalence check using test inputs (when available).
- `--visualization` or `-vz`
  Visualization mode:
  - `0` – none
  - `1` – Graphviz online link
  - `2` – Graphviz HTTP server

For scripted demo runs, `--vz 0` is recommended to avoid blocking visualization servers.

---

### 6.1 Add chain and MatMul+Add – loop fusion

#### Add chain: fusion and coalescing enabled

```bash
onnx-flow examples/onnx/add_chain_standard.onnx   --fuse   --coalesce   --loopLowering   --qe   --vz 0
```

This runs:

- Decomposition with loop fusion and coalesced MAC
- ONNX reconversion
- Equivalence check against the original model

#### Add chain: fusion disabled

```bash
onnx-flow examples/onnx/add_chain_standard.onnx   --no-fuse   --coalesce   --loopLowering   --qe   --vz 0
```

Use this to compare behaviour and structure with and without fusion.
The equivalence check confirms that outputs remain identical within tolerance.

#### MatMul+Add: fusion and coalescing enabled

```bash
onnx-flow examples/onnx/matmuladd_test.onnx   --fuse   --coalesce   --loopLowering   --qe   --vz 0
```

Optional comparison with fusion disabled:

```bash
onnx-flow examples/onnx/matmuladd_test.onnx   --no-fuse   --coalesce   --loopLowering   --qe   --vz 0
```

---

### 6.2 Softmax and AvgPool – loop lowering

These examples show the effect of enabling or disabling loop lowering.

#### Softmax: with loop lowering (default behaviour)

```bash
onnx-flow examples/onnx/softmax_standard.onnx   --loopLowering   --qe   --vz 0
```

#### Softmax: without loop lowering

```bash
onnx-flow examples/onnx/softmax_standard.onnx   --no-loop-lowering   --qe   --vz 0
```

#### AvgPool: with loop lowering

```bash
onnx-flow examples/onnx/avgpool_standard.onnx   --loopLowering   --qe   --vz 0
```

#### AvgPool: without loop lowering

```bash
onnx-flow examples/onnx/avgpool_standard.onnx   --no-loop-lowering   --qe   --vz 0
```

In each pair, the structural decomposition differs (explicit loops vs higher-level ops) while the equivalence check validates functional correctness.

---

### 6.3 Matmul CGRA-oriented decomposition

These examples illustrate CGRA-oriented decomposition and formatting.

```bash
onnx-flow examples/onnx/matmul_simple.onnx   --decomposeForCgra   --format dot --formatter cgra   --output output.dot   --vz 0
```

This produces a DOT file (`output.dot`) with CGRA-oriented structure. You can also visualize the structure of the transformed graph in the browser by adding `--vz 1` or `--vz 2`.

## 7. Running arbitrary models (Q&A mode)

During questions, any ONNX model can be explored with custom settings.

General pattern:

```bash
onnx-flow path/to/your_model.onnx   --fuse <true|false>   --coalesce <true|false>   --loopLowering <true|false>   --recurse <true|false>   --decomposeForCgra <true|false>   --format <json|dot>   --formatter <default|cgra>   --output <output_file>   --qe <true|false>   --vz 0
```

Examples:

Disable fusion and coalescing, keep loop lowering:

```bash
onnx-flow path/to/your_model.onnx   --no-fuse   --no-coalesce   --loopLowering   --qe   --vz 0
```

CGRA-oriented decomposition and formatter:

```bash
onnx-flow path/to/your_model.onnx   --decomposeForCgra   --format dot   --formatter cgra   --output output.dot   --vz 0
```

### Behaviour when no test configuration exists

If `--qe` is used on a model that does not have a test entry:

- The tool prints a message indicating that no test input specification is available for that model.
- The equivalence step is skipped.
- All other behaviour (decomposition, reconversion, visualization) proceeds normally.

This makes `--qe` safe to use generically: equivalence is performed when possible and skipped otherwise.

---

## 8. Optional: interactive visualization

For a visual inspection of the decomposed graph:

```bash
onnx-flow examples/onnx/add_chain_standard.onnx   --vz 2   --fuse   --coalesce   --loopLowering   --noReconversion
```

- `--visualization 2` (`-vz 2`) starts a Graphviz HTTP server.
- A URL is printed in the terminal; open it in a browser to explore the graph.

For the main scripted demo, it is usually easier to use `--vz 0` and reserve visualization for follow-up exploration.
