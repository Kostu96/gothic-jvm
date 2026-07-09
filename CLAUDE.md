# gothic-jvm — Codebase Reference (for AI assistants)

> **Maintenance note:** Keep this file up to date. After every major change to the
> codebase (new opcodes, new runtime components, new native callbacks, build/test
> layout changes, or API renames), update the relevant sections below so this file
> stays an accurate single-source-of-truth and AI tools don't have to re-read every file.

## What this is

A toy/educational **Java Virtual Machine** written in modern **C++23**. It parses
`.class` files, builds a runtime model of classes, and interprets bytecode. The
project is in an early, partial state: only a subset of opcodes is implemented, and
only a handful of methods have C++ "native" implementations. The default workload is
a J2ME-style MIDlet (`gothic3thebeginning/HG`).

Unlike an earlier iteration, the standard/runtime library classes (`java.lang.Object`,
`java.lang.String`, `javax.microedition.*`, etc.) are **real `.class` files loaded off
the classpath**, not hand-written C++ class descriptions. A small set of methods marked
`ACC_NATIVE` in those class files are bound to C++ callbacks (see [Native callbacks](#native-callbacks)).

- **Language:** C++23 (`cxx_std_23`)
- **Build:** CMake (min 3.28), MSVC/Visual Studio solution generated under `build/`
- **Tests:** GoogleTest (`third_party/gtest`, a git submodule), discovered via `gtest_discover_tests`
- **Executable:** `app` (entry `source/main.cpp`); core logic in static lib `app_lib`

## Build & test

```
cmake -S . -B build
cmake --build build
ctest --test-dir build        # or run the app_tests target
```

Test resources live at `resources/test_files/` (`HelloWorld.class`, `Addition.class`,
plus `.java` sources); path injected via `TEST_FILES_DIR` compile definition. Current
tests only cover `BinaryReader` and `ClassFile`/`Class` parsing (no interpreter tests).

### Runtime data / classpath

At startup the classpath is assembled from the **current working directory**, not from
`resources/`:

1. `VM` ctor adds `<cwd>/java_classes` and eagerly loads `java/lang/Object` and
   `java/lang/String`.
2. `main.cpp` adds either the CLI-provided classpath entries, or (with no extra args)
   `<cwd>/gothic3thebeginning`.

Relevant on-disk assets:
- `java_classes/` (repo root) — small hand-maintained set of runtime classes, several
  with `.java` sources (`FullCanvas`, `ResourceInputStream`, `Object`, `Canvas`,
  `Command`, `MIDlet`, `MIDletStateChangeException`). Note the root copy's `java/lang`
  only contains `Object.class`.
- `resources/classes/` — a large **vendored J2ME/MIDP/CLDC class library** (real SDK
  `.class` files: `java.lang.*`, `java.io.*`, `java.util.*`, `javax.microedition.*`).
  Not on the default runtime classpath; it's the source pool these bootstrap classes
  come from.
- `resources/gothic3thebeginning/` — MIDlet data: `HG.class` (default main class),
  obfuscated single-letter classes (`a`/`b.class`/`c`/`d.class`/…), `.mid`/`.mdl`/`.lng`
  data, `icon.png`, `s00.png`, `META-INF/`.
- `build/java_classes/` and `build/gothic3thebeginning/` — build-tree copies that
  actually satisfy `<cwd>` lookups when running from `build/` (e.g. `build/java_classes`
  additionally contains `String`, `System`, `Class`, `StringBuffer`, `Hashtable`, …).

## Directory layout

```
source/
  main.cpp                      # entry: VM(main_class); add classpath entries; run()
  class_loader/                 # .class file parsing + classpath-based class loading
    class_file.{hpp,cpp}        # ClassFile: raw parse of constant pool, fields, methods, Code attr, MUTF-8 decode
    constant_pool_entry.hpp     # ConstantPoolEntry variant (Utf8/Integer/Long/Class/String/refs/NameAndType)
    class_loader.{hpp,cpp}      # ClassLoader: classpath resolve + cache; array classes via load_array
  runtime/                      # execution model
    vm.{hpp,cpp}                # VM: owns ClassLoader/Heap/Interpreter; boot Object+String; run+init MIDlet
    class.{hpp,cpp}             # Class/Method/Field; constant-pool resolution; String materialization; native callback table
    interpreter.{hpp,cpp}       # bytecode dispatch loop (execute/invoke/run) -> optional<Value>
    frame.{hpp,cpp}             # Frame: locals, operand stack, pc, code readers, branch()
    heap.{hpp,cpp}              # Heap: allocates instances/arrays + canonical Class mirrors (owns unique_ptrs)
    object.{hpp,cpp}            # Object = variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>
    opcodes.hpp                 # op_* bytecode opcode constants
    value.hpp                   # Value = variant<monostate,int32,int64,float,double,Object*>
    runtime_constant_pool_entry.hpp  # resolved CP cache types (Class/String/Field/Method/InterfaceMethod)
  utils/
    binary_reader.{hpp,cpp}     # big-endian u8/u16/u32, string/bytes, bounds-checked
tests/
  CMakeLists.txt, test_binary_reader.cpp, test_class_file.cpp
resources/
  test_files/                   # .class fixtures for tests (HelloWorld, Addition + .java)
  classes/                      # vendored J2ME/MIDP/CLDC standard library (.class)
  gothic3thebeginning/          # MIDlet data (HG.class, obfuscated classes, mid/mdl/lng, images, META-INF)
java_classes/                   # hand-maintained runtime classes (+ some .java sources)
third_party/gtest/              # vendored GoogleTest (submodule)
build/                          # generated VS solution/projects + <cwd> copies of java_classes/gothic3thebeginning
```

## Architecture & flow

1. `main.cpp` (`args <main_class> [<class_path_entry>...]`) builds a `VM(main_class)`;
   default main class is `HG`. With no extra args the classpath defaults to
   `<cwd>/gothic3thebeginning`; otherwise each extra arg is added as a classpath entry.
2. `VM` ctor adds `<cwd>/java_classes` to the classpath and eagerly loads
   `java/lang/Object` then `java/lang/String`.
3. `VM::run()` -> `load` main class -> `initialize_class`, then `new_instance(main)`,
   invokes `<init>()V`, then invokes the MIDlet lifecycle entry `startApp()V` (both via
   `Interpreter::execute` with the receiver passed as the sole argument).
4. `Interpreter` holds a `VM&` (ctor `Interpreter(VM&)`). `execute(method, args)`
   builds a `Frame(method.owner, method)`, copies args into locals, then `run()`
   dispatches opcodes in a `switch`, returning `optional<Value>`. The private
   `invoke(owner, method, arg_count, frame)` helper centralizes the native-vs-bytecode
   call path: native methods call `Method::native_callback(vm, frame)` with the live
   frame; bytecode methods pop args and re-enter `execute`.
5. `initialize_class` walks the superclass chain first (recursively) and then runs
   `<clinit>()V` if present, tracking `ClassInitState` (Loaded/Initializing/Initialized/Failed).
6. `ClassLoader::load` checks a cache, routes names starting with `[` to `load_array`,
   otherwise resolves `binary/name` -> `<entry>/binary/name.class` across classpath
   entries and constructs a `Class` (which recursively loads its super + interfaces).

### Key types
- **Value** (`value.hpp`): JVM slot = `variant<monostate, int32_t, int64_t, float, double, Object*>`.
  `Object*` is the reference type (`nullptr` == null). Doubles/longs occupy a single slot here.
- **Object** (`object.hpp`): tagged union (not a class hierarchy),
  `variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>` accessed
  via `std::get_if`/`std::holds_alternative` on `.data`.
  - `InstanceData` holds `Class& type` + `vector<Value> fields` (indexed by field slot).
  - `PrimitiveArrayData` holds a `variant` of typed element vectors (boolean/byte→uint8,
    char→char16, short→int16, int→int32, long→int64, float, double) with `ElementType`
    tags matching the JVM `newarray` atype codes (4..11); `get`/`set` widen to `Value`.
  - `InstanceArrayData` holds `Class& element_type` + `vector<Object*>`.
  - `ClassMirrorData` holds the mirrored `Class&` (the `java/lang/Class` object identity).
- **Class** (`ClassKind`: `File`, plus per-primitive/ref array kinds): built from a parsed
  `ClassFile` (File kind) or synthesized for arrays. Holds `super_`/`interfaces_` (as `Class*`),
  static + instance `Field`s (instance fields get sequential `slot`s continuing the super's
  count), `Method`s, and a lazily-resolved `runtime_constant_pool_`. Key API:
  `find_method`, `find_field`, `resolve_class/field/method` (cached CP resolution),
  `resolve_constant` (ints/longs + interned `String`s), `resolve_class_name`,
  `resolve_field_ref`/`resolve_method_ref` (string-triples for late binding), and
  `create_string` (materializes a `java/lang/String` instance — see below).
- **Method**: `owner` (Class&), `is_native`, name, descriptor, `max_stack`/`max_locals`,
  `code` span, `exception_table` span, `native_callback` (`std::function<void(VM&, Frame&)>`).
- **Field**: `owner`, `is_static`, name, descriptor, `Value value` (static storage), `slot` (instance index).
- **Heap**: owns all `Object`s via `unique_ptr`; `new_instance`, `new_primitive_array`,
  `new_instance_array`, and `class_object_for(Class&)` which lazily creates and caches one
  canonical `java/lang/Class` mirror per `Class` (stable identity for `getClass()`).
- **Frame**: `owner()`/`method()`, `locals()`, `operand_stack()`, `get_pc()`, `branch(offset)`,
  `pop_code_u8/u16`, `push/pop/peek_stack`. (No `set_pc`; jumps go through `branch`.)
- **Runtime constant pool** (`runtime_constant_pool_entry.hpp`): `RuntimeClassInfo`,
  `RuntimeFieldRefInfo`, `RuntimeMethodRefInfo` are populated on first resolution and
  cache the resolved pointer. `RuntimeStringInfo`/`RuntimeInterfaceMethodRefInfo` exist in
  the variant but are not populated yet.

### String modeling
`Class::create_string(vm, str)` builds a real `java/lang/String` instance: it allocates a
backing `char[]` (one char per byte — ASCII/MUTF-8-ish), then populates the String's
`value:[C`, `offset:I`, `count:I`, `hash:I` fields directly (rather than running an
`<init>`). `resolve_constant` uses this to intern `String` constants (`ldc` of a string now
yields a live object instead of null). The exact field layout depends on the vendored
`java/lang/String.class` on the classpath.

### Implemented opcodes (dispatched in `interpreter.cpp`)
Constants/consts: `nop` (0x00), `aconst_null` (0x01), `iconst_m1..5` (0x02–0x08),
`lconst_0/1` (0x09–0x0A), `fconst_0/1/2` (0x0B–0x0D), `dconst_0/1` (0x0E–0x0F),
`bipush` (0x10), `sipush` (0x11), `ldc` (0x12), `ldc_w` (0x13), `ldc2_w` (0x14).
Loads/stores: `iload` (0x15), `aload` (0x19), `iload_0..3` (0x1A–0x1D),
`lload_0..3` (0x1E–0x21), `aload_0..3` (0x2A–0x2D), `caload` (0x34), `istore` (0x36),
`astore` (0x3A), `istore_0..3` (0x3B–0x3E), `astore_0..3` (0x4B–0x4E),
`iastore` (0x4F), `aastore` (0x53), `bastore` (0x54), `castore` (0x55), `sastore` (0x56).
Stack/math: `dup` (0x59), `iadd` (0x60), `isub` (0x64), `imul` (0x68), `idiv` (0x6C),
`land` (0x7F), `lxor` (0x83), `iinc` (0x84).
Branches: `ifne` (0x9A), `iflt` (0x9B), `ifge` (0x9C), `ifle` (0x9E),
`if_icmpeq` (0x9F), `if_icmpne` (0xA0), `if_icmplt` (0xA1), `if_icmpge` (0xA2),
`goto` (0xA7), `ifnull` (0xC6), `ifnonnull` (0xC7).
Returns: `ireturn` (0xAC), `areturn` (0xB0), `return` (0xB1).
Fields/calls: `getstatic` (0xB2), `putstatic` (0xB3), `getfield` (0xB4), `putfield` (0xB5),
`invokevirtual` (0xB6, virtual dispatch via `select_virtual_method` up the superclass chain;
a `ClassMirrorData` receiver dispatches against `java/lang/Class`), `invokespecial` (0xB7),
`invokestatic` (0xB8, triggers `initialize_class`).
Object/array: `new` (0xBB), `newarray` (0xBC), `anewarray` (0xBD), `arraylength` (0xBE),
`checkcast` (0xC0, currently a no-op), `multianewarray` (0xC5, only the outermost
dimension is allocated).

Notes:
- `op_lload` (0x16) is declared in `opcodes.hpp` but has no dispatch case yet.
- The interpreter prints the operand stack and each decoded opcode to stdout (verbose trace).
- Unknown opcodes throw `std::runtime_error`; argument counts come from
  `count_descriptor_arguments`. The three `invoke*` opcodes share `Interpreter::invoke`.
- Error conditions that a real VM would surface as Java exceptions (NPE, div-by-zero,
  ArrayStore, index OOB, missing field/method) are thrown as `std::runtime_error`.

### Native callbacks
There is no separate native-class machinery anymore. Methods flagged `ACC_NATIVE` in a
loaded `.class` file are matched, by `"<class>.<name><descriptor>"` key, against a static
`native_method_callbacks` map in `class.cpp`; a match wires up `Method::native_callback`.
Currently registered:
- `java/lang/System.currentTimeMillis()J` — real wall-clock millis.
- `javax/microedition/lcdui/Canvas.getWidth()I` / `getHeight()I` — hardcoded 240 / 320.
- `java/lang/Object.getClass()Ljava/lang/Class;` — returns the canonical heap `Class` mirror
  (only handles `InstanceData` receivers).
- `java/lang/Class.getName()Ljava/lang/String;` — materializes a String of the mirrored name.
- `java/lang/String.charAt(I)C` and `String.indexOf(II)I` — operate on the backing `char[]`.

## Conventions
- Headers `.hpp`, sources `.cpp`; include via paths rooted at `source/` (e.g. `#include "runtime/vm.hpp"`).
- `snake_case` members with trailing underscore (`class_loader_`); types `PascalCase`; opcode
  constants `op_snake_case`.
- Native callback functions are free functions in an anonymous namespace in `class.cpp`, named
  after their fully-qualified Java method (`java_lang_object_get_class`, etc.).
- Copy deleted on owning types; raw pointers are non-owning, `unique_ptr` for ownership.
- Big-endian reads in `BinaryReader`; class file magic `0xCAFEBABE`; UTF-8 constants are
  (modified) UTF-8.

## Known gaps / WIP / dead code (as of writing)
- **Array classes are incomplete/likely broken:** `Class(std::string array_name)` never sets
  `kind_` (stays default `ClassKind::File`), so `this_name()` would dereference the null
  `class_file_` for an array class. The per-primitive/ref `ClassKind` array enumerators are
  defined but unused.
- **Constant pool coverage is partial:** `ClassFile` parses only Utf8/Integer/Long/Class/
  String/Field/Method/InterfaceMethod/NameAndType tags. `Float` (tag 4) and `Double` (tag 6)
  are **not** parsed and will throw "Unsupported constant pool tag". `get_constant` only
  returns Integer/Long (and monostate for String, which `resolve_constant` then interns).
- **Dead / commented code:** `decode_modified_utf8` in `class_file.cpp` is defined but never
  called; a commented-out `java_lang_class_new_instance` native and a commented-out
  `Class::find_field_slot` remain; `frame.cpp` has a redundant self-include; several
  `TODO(Kostu)` markers (e.g. `frame.hpp` include for the `currentTimeMillis` native).
- `checkcast` is a no-op; no real exception objects/handler-table dispatch (errors throw
  `std::runtime_error`); no `NoSuchMethodError`/`AbstractMethodError` modeling.
- `multianewarray` (0xC5) allocates only the outermost dimension.
- `RuntimeStringInfo` / `RuntimeInterfaceMethodRefInfo` cache slots are declared but not used;
  interface method invocation (`invokeinterface`) is not implemented.
- No garbage collection; the heap only grows. Most JVM opcodes remain unimplemented.
