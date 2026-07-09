# gothic-jvm

A small, educational **Java Virtual Machine** written in modern **C++23**. It parses
`.class` files, builds a runtime model of classes, and interprets JVM bytecode. The
default workload is a J2ME-style MIDlet: the `HG` main class shipped under
`gothic3thebeginning/`. The goal is to implement *just enough* of a VM to bring that
MIDlet to life.

> ⚠️ **Status: early and partial.** Only a subset of JVM opcodes is implemented, a
> handful of standard-library methods are backed by C++ "native" callbacks, there is no
> garbage collector, and runtime errors surface as C++ exceptions rather than Java ones.
> This is a learning project, not a production VM.

## What works today

- Parsing `.class` files (constant pool, fields, methods, the `Code` attribute, MUTF-8).
- Classpath-based class loading with caching, plus synthesized array and primitive classes.
- Class initialization (`<clinit>`) with superclass-first ordering and state tracking.
- A tree-walking bytecode interpreter covering constants, loads/stores, integer/long math,
  branches, field access, object/array creation, and `invokevirtual` / `invokespecial` /
  `invokestatic`.
- Real `java.lang.String` objects materialized from string constants.
- A small set of native methods (timing, canvas size, `getClass`, `String.charAt`, …).

## Getting started

### Prerequisites

- A C++23-capable compiler (developed with MSVC / Visual Studio 2022).
- CMake ≥ 3.28.
- The GoogleTest submodule under `third_party/gtest`.

### Clone

```
git clone --recurse-submodules <repo-url>
```

If you already cloned without submodules:

```
git submodule update --init --recursive
```

### Build

```
cmake -S . -B build
cmake --build build
```

### Test

```
ctest --test-dir build
```

Tests currently cover the binary reader and class-file parsing (see `tests/`).

### Run

The `app` executable takes an optional main class followed by optional classpath entries:

```
app <main-class> [<classpath-entry> ...]
```

With no arguments it defaults to the `HG` MIDlet and looks for classes under
`<cwd>/gothic3thebeginning`. Because the classpath is assembled from the **current working
directory**, run it from a directory that contains `java_classes/` and
`gothic3thebeginning/`. The build copies `java_classes/` into the build tree for you.

## How it works

At a high level:

1. `main` constructs a `VM` for the chosen main class and configures the classpath.
2. The `VM` eagerly loads a few bootstrap classes, then loads and initializes the main class.
3. It instantiates the MIDlet, runs its `<init>`, and calls `startApp()`.
4. The `Interpreter` executes bytecode frame by frame, dispatching opcodes in a large switch.

```mermaid
flowchart LR
    A[main.cpp] --> B[VM]
    B --> C[ClassLoader]
    B --> D[Heap]
    B --> E[Interpreter]
    C -->|parses| F[ClassFile]
    E -->|builds| G[Frame]
    E -->|allocates via| D
```

Core components:

- **ClassLoader / ClassFile** — resolve and parse `.class` files off the classpath.
- **Class / Method / Field** — the runtime model, including lazy constant-pool resolution.
- **Interpreter / Frame** — the bytecode execution engine and per-call activation records.
- **Heap / Object** — object and array allocation; `Object` is a tagged union of instance,
  primitive-array, instance-array, and `java.lang.Class`-mirror data.

## Project layout

```
source/
  main.cpp            # entry point
  class_loader/       # .class parsing + classpath class loading
  runtime/            # VM, interpreter, classes, heap, objects, frames
  utils/              # big-endian binary reader
tests/                # GoogleTest unit tests
resources/            # test fixtures, vendored J2ME stdlib, MIDlet data
java_classes/         # hand-maintained bootstrap runtime classes
third_party/gtest/    # GoogleTest (submodule)
```

## Limitations & roadmap

- Many opcodes are still unimplemented; the interpreter throws on anything it doesn't know.
- No garbage collection — the heap only grows.
- No real Java exception objects or exception-handler dispatch yet.
- `Float` / `Double` constant-pool entries and `invokeinterface` are not yet supported.
- `checkcast` currently performs no type check.

## For contributors and AI agents

A detailed, always-current technical reference — architecture, type signatures, the exact
list of implemented opcodes, conventions, and known gaps — lives in
[CLAUDE.md](CLAUDE.md). It is written for AI coding agents but is equally useful as a deep
map of the codebase for human contributors.
