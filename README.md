# gothic-jvm

A small, educational **Java Virtual Machine** written in modern **C++23**. It parses
`.class` files, builds a runtime model of classes, and interprets JVM bytecode. The default
workload is a J2ME-style MIDlet: the `HG` main class shipped under `gothic3thebeginning/`.
The goal is to implement *just enough* of a VM to bring that MIDlet to life.

> ⚠️ **Status: early and partial.** Only a subset of JVM opcodes is implemented, a handful
> of standard-library methods are backed by C++ "native" callbacks, there is no garbage
> collector, and runtime errors surface as C++ exceptions rather than Java ones. This is a
> learning project, not a production VM.

## What works today

- Parsing `.class` files (constant pool, fields, methods, the `Code` attribute, modified UTF-8).
- Classpath-based class loading with caching, plus synthesized array and primitive classes.
- Lazy, superclass-first class initialization (`<clinit>`) driven off an explicit per-thread
  frame stack rather than C++ recursion.
- A stackless, budgeted bytecode interpreter — `run(thread, n)` executes up to `n` instructions
  against a thread's explicit frame stack (groundwork for cooperative green threads) — covering
  constants, loads/stores, integer/long math, branches, field access, object/array creation,
  monitors, and `invokevirtual`/`invokespecial`/`invokestatic`.
- Real `java.lang.String`/`StringBuffer` objects backed by native C++ payloads; string constants
  are interned and deduplicated.
- A set of native methods (timing, canvas size, `getClass`, `String`/`StringBuffer` operations,
  resource streaming, MIDP `Graphics`/`Image` drawing).
- An SDL3 window with a 2D renderer that opens on startup. The MIDP drawing primitives
  (`Graphics.fillRect`/`drawString`, `Image` surfaces) do real work but render to **offscreen**
  surfaces that are not yet blitted to the window (which just clears each frame and handles the
  window-close event).

## Getting started

### Prerequisites

- A C++23-capable compiler (developed with MSVC / Visual Studio 2022).
- CMake ≥ 3.28.
- A JDK providing `javac` (old enough to accept `-source 1.3 -target 1.1`) — CMake compiles the
  bootstrap `java_classes/*.java` sources at build time.
- The GoogleTest submodule under `third_party/gtest`.
- SDL3 is fetched and built automatically by CMake (`FetchContent`), so no manual install is
  needed — but the first configure clones and builds SDL from source, which takes a while.

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

The build also drives `javac` (the `java_classes` CMake target) to compile the hand-maintained
runtime classes under `java_classes/` into `build/java_classes/`.

### Test

```
ctest --test-dir build
```

Tests currently cover only the binary reader (see `tests/`).

### Run

The `app` executable takes an optional main class followed by optional classpath entries:

```
app <main-class> [<classpath-entry> ...]
```

With no arguments it defaults to the `HG` MIDlet and looks for classes under
`<cwd>/gothic3thebeginning`. Because the classpath is assembled from the **current working
directory**, run it from a directory that contains `java_classes/` and `gothic3thebeginning/`;
the build compiles `java_classes/` into `build/java_classes/` for you.

## How it works

1. `main` opens the SDL3 `Display` (window + renderer), constructs a `VM`, sets the chosen main
   class, connects the display, and configures the classpath.
2. `VM::run` (on a background thread) walks a small bootstrap state machine: it initializes
   `java/lang/String`, then the main class, then instantiates the MIDlet, runs its `<init>`, and
   calls `startApp()` — each step scheduled as frames on the VM's main `Thread`.
3. A `Thread` is an explicit JVM call stack (a stack of `Frame`s). The `Interpreter` runs a
   bounded number of instructions against the top frame and returns, so threads can eventually be
   scheduled cooperatively (green threads); method calls and class `<clinit>`s push frames instead
   of recursing in C++.
4. `main` runs the window event/render loop until the user closes the window, then asks the VM to
   stop and joins the background thread.

```mermaid
flowchart LR
    A[main.cpp] --> B[VM]
    B --> C[ClassLoader]
    B --> D[Heap]
    B --> E[Interpreter]
    B --> H[Thread]
    C -->|parses| F[ClassFile]
    H -->|stack of| G[Frame]
    E -->|runs| H
    E -->|allocates via| D
```

Core components:

- **ClassLoader / ClassFile** — resolve and parse `.class` files off the classpath.
- **Class / Method / Field** — the runtime model, including lazy constant-pool resolution.
- **Interpreter / Thread / Frame** — the bytecode engine, a green thread's explicit call stack,
  and per-call activation records.
- **Heap / Object** — object and array allocation; `Object` is a tagged union of instance,
  primitive-array, instance-array, and `java.lang.Class`-mirror data.

## Project layout

```
source/
  main.cpp            # entry point
  class_loader/       # .class parsing + classpath class loading
  platform/           # SDL3 window + 2D renderer wrapper (Display)
  runtime/            # VM, interpreter, threads, classes, heap, objects, frames
  utils/              # big-endian binary reader
tests/                # GoogleTest unit tests
resources/            # vendored J2ME stdlib + MIDlet data
java_classes/         # hand-maintained bootstrap runtime classes (.java, compiled by CMake)
third_party/gtest/    # GoogleTest (submodule)
```

## Limitations & roadmap

Known gaps:

- **Green threads, no scheduler yet.** The interpreter is stackless (runs an instruction budget
  against a thread's frame stack), but the VM drives a single main thread — there is no
  scheduler, cooperative yielding, or blocking. `java.lang.Thread.start` and the monitor opcodes
  are non-blocking stubs.
- **JVM errors surface as C++ exceptions.** Null-pointer, divide-by-zero, array-store, and
  out-of-bounds conditions throw `std::runtime_error` instead of Java exceptions. The parsed
  exception table is never consulted, so `try`/`catch`/`finally` does not work.
- **No garbage collection** — the heap only grows.
- **Partial type/opcode coverage.** Many opcodes are unimplemented (the interpreter throws on
  anything it doesn't know); `checkcast` performs no type check and there is no `instanceof`;
  `invokespecial` `super`-dispatch (`ACC_SUPER`) and `invokeinterface` are missing; `Float`/`Double`
  constant-pool entries fail to load.
- **Unbound `native` methods.** Some methods declared `native` in the bootstrap sources have no
  C++ binding yet (e.g. `System.arraycopy`, `Class.isInterface`, `String.substring`,
  `String.replace`), so calling them throws.
- **Runtime dependency classes are vendored.** The build compiles only the `java_classes/*.java`
  sources; other classes the MIDlet needs (`java/io/*`, `java/util/*`, `javax/microedition/media/*`)
  come from vendored copies under `resources/classes/` and must be present in
  `build/java_classes/` at runtime.
- **MIDP graphics are offscreen.** `Graphics`/`Image` natives draw onto SDL surfaces that are
  never blitted to the window, so nothing is visible on screen yet.
- **Test coverage** is limited to the binary reader.

Roadmap:

- Build a cooperative scheduler over multiple green threads and wire up `java.lang.Thread`.
- Introduce real exception objects and exception-handler-table dispatch.
- Implement `invokeinterface`, `super`-calls, and `checkcast`/`instanceof` checks.
- Parse `Float`/`Double` constants and widen opcode coverage.
- Blit the offscreen MIDP surfaces to the window and wire up the `Displayable`/`paint` model.
- Bind the remaining `native` methods, start on garbage collection, and restore parser/interpreter
  test coverage.

## For contributors and AI agents

A detailed, always-current technical reference — architecture, type signatures, the exact list of
implemented opcodes, conventions, and known gaps — lives in [CLAUDE.md](CLAUDE.md). It is written
for AI coding agents but is equally useful as a deep map of the codebase for human contributors.
