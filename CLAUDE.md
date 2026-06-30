# gothic-jvm — Codebase Reference (for AI assistants)

> **Maintenance note:** Keep this file up to date. After every major change to the
> codebase (new opcodes, new runtime components, new native classes, build/test
> layout changes, or API renames), update the relevant sections below so this file
> stays an accurate single-source-of-truth and AI tools don't have to re-read every file.

## What this is

A toy/educational **Java Virtual Machine** written in modern **C++23**. It parses
`.class` files, builds a runtime model of classes, and interprets bytecode. The
project is in an early, partial state: only a subset of opcodes and native methods
are implemented. The default workload is a J2ME-style MIDlet (`gothic3thebeginning/HG`).

- **Language:** C++23 (`cxx_std_23`)
- **Build:** CMake (min 3.28), MSVC/Visual Studio solution generated under `build/`
- **Tests:** GoogleTest (`third_party/gtest`), discovered via `gtest_discover_tests`
- **Executable:** `app` (entry `source/main.cpp`); core logic in static lib `app_lib`

## Build & test

```
cmake -S . -B build
cmake --build build
ctest --test-dir build        # or run the app_tests target
```

Test resources live at `resources/test_files/` (`HelloWorld.class`, `Addition.class`,
plus `.java` sources); path injected via `TEST_FILES_DIR` compile definition. The
runnable MIDlet data is at `resources/gothic3thebeginning/` (`HG.class`, `.mid`, `.mdl`,
`.lng`, `META-INF/MANIFEST.MF`, etc.).

## Directory layout

```
source/
  main.cpp                      # entry: VM(main_class); add classpath; run()
  class_loader/                 # .class file parsing + class loading
	class_file.{hpp,cpp}        # ClassFile: raw parse of constant pool, fields, methods, Code attr
	constant_pool_entry.hpp     # ConstantPoolEntry variant (Utf8/Integer/Long/Class/String/refs...)
	class_loader.{hpp,cpp}      # ClassLoader: classpath resolve, cache, native + array registry
	native_class_description.{hpp,cpp}  # NativeClassDescription factories (data_input_stream/object/clazz/string/hashtable/random/stack/vector/midlet/full_canvas)
  native_classes/               # C++ impls of native methods (callbacks: void(VM&, Frame&)); Java-style namespaces
	io/data_input_stream.{hpp,cpp}  # java::io::DataInputStream::init (stub)
	lang/object.{hpp,cpp}       # java::lang::Object::get_class (returns canonical Class mirror)
	lang/clazz.{hpp,cpp}        # java::lang::Class::get_resource_as_stream (throwing stub; file named clazz to avoid class.cpp basename clash)
	lang/string.{hpp,cpp}       # java::lang::String (no native methods registered)
	util/hashtable.{hpp,cpp}    # java::util::Hashtable::init (stub)
	util/random.{hpp,cpp}       # java::util::Random::init (stub)
	util/stack.{hpp,cpp}        # java::util::Stack::init (stub)
	util/vector.{hpp,cpp}       # java::util::Vector::init (stub)
	midlet/midlet.{hpp,cpp}     # javax::microedition::midlet::MIDlet::init (stub)
	ui/full_canvas.{hpp,cpp}    # com::nokia::mid::ui::FullCanvas::init/get_width/get_height (placeholder size)
  runtime/                      # execution model
	vm.{hpp,cpp}                # VM: owns ClassLoader/Heap/Interpreter; load+init+run MIDlet
	class.{hpp,cpp}             # Class (File/Native/Array kinds), Method, Field, ClassInitState
	interpreter.{hpp,cpp}       # bytecode dispatch loop (execute/invoke/run) -> optional<Value>
	frame.{hpp,cpp}             # Frame: locals, operand stack, pc, code readers
	heap.{hpp,cpp}              # Heap: allocates instances/arrays + canonical Class mirrors (owns unique_ptrs)
	runtime_object.{hpp,cpp}    # RuntimeObject = variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>
	value.hpp                   # Value = variant<monostate,int32,int64,float,double,RuntimeObject*>
	runtime_constant_pool_entry.hpp  # resolved CP cache types (mostly unused so far)
  utils/
	binary_reader.{hpp,cpp}     # big-endian u8/u16/u32, string/bytes, bounds-checked
tests/
  test_binary_reader.cpp, test_class_file.cpp
resources/test_files/           # .class fixtures for tests
resources/gothic3thebeginning/  # MIDlet data (HG.class, mid/mdl/lng, META-INF)
third_party/gtest/              # vendored GoogleTest
build/                          # generated VS solution/projects
```

## Architecture & flow

1. `main.cpp` (`args [main_class] [class_path_entry]...`) builds a `VM(main_class)`;
   default main class is `HG`. With no extra args the classpath defaults to
   `<cwd>/gothic3thebeginning`; otherwise each extra arg is added as a classpath entry.
2. `VM` ctor registers native classes via `ClassLoader::load_native`:
   `java/io/DataInputStream`, `java/lang/Object`, `java/lang/Class`, `java/lang/String`,
   `java/util/Hashtable`, `java/util/Random`, `java/util/Stack`, `java/util/Vector`,
   `javax/microedition/midlet/MIDlet`, `com/nokia/mid/ui/FullCanvas`.
3. `VM::run()` -> `load_class` -> `initialize_class`, then `new_instance(main)`,
   invokes `<init>()V`, then invokes the MIDlet lifecycle entry `startApp()V`.
4. `Interpreter` holds a `VM&` (ctor `Interpreter(VM&)`). `execute(owner, method, args)`
   creates a `Frame`, then `run()` dispatches opcodes in a `switch`, returning
   `optional<Value>`. The private `invoke(owner, method, arg_count, frame)` helper
   centralizes the native-vs-bytecode call path: native methods call
   `Method::native_callback(vm, frame)`; bytecode methods pop args and re-enter `execute`.
5. `ClassLoader::load` resolves `binary/name` -> `entry/binary/name.class`, caches
   in `loaded_`; names starting with `[` route to `load_array`. `load_native` uses
   the `native_classes_desc` map.

### Key types
- **Value** (`value.hpp`): JVM slot = `variant<monostate, int32_t, int64_t, float, double, RuntimeObject*>`.
- **Class** (`ClassKind` File/Native/Array): built from a `NativeClassDescription`,
  a parsed `ClassFile`, or synthesized as an array class. Holds static fields,
  methods, init state. Lookup: `find_method(name,desc)`, `find_static_field`,
  `resolve_constant/class_name/field_ref/method_ref`. Names via `this_name()` /
  `super_name()`; arrays expose `is_array()` / `component_type()`. Native classes
  carry a `super_name` (default `java/lang/Object`, empty only for `Object` itself),
  so the superclass chain walk reaches `Object` for virtual dispatch.
- **Method**: `is_native`, name, descriptor, max_stack/locals, `code` span, exception_table,
  `native_callback` (`std::function<void(VM&, Frame&)>`, invoked with the live frame for native methods).
- **Heap**: owns all objects; `new_instance`, `new_primitive_array`, `new_instance_array`,
  and `class_object_for(Class&)` which lazily creates and caches one canonical
  `java/lang/Class` mirror per `Class` (stable identity for `getClass()`).
- **RuntimeObject** (`runtime_object.hpp`): tagged union (not a class hierarchy),
  `variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>`.
  Access payloads with `std::get_if`/`std::holds_alternative` on `.data`, mirroring
  how `Value` is used. `InstanceData` holds the runtime `Class*`; the array data
  types hold element type + elements; `ClassMirrorData` holds the mirrored `Class*`.
- **Frame**: `locals()`, `operand_stack()`, `pc()/set_pc()`; `pop_code_u8/u16`, `push/pop/peek_stack`.

### Implemented opcodes (in `interpreter.cpp`)
`nop` (0x00), `aconst_null` (0x01), `iconst_m1..5` (0x02–0x08), `lconst_0` (0x09),
`lconst_1` (0x0A), `fconst_1` (0x0C), `bipush` (0x10), `sipush` (0x11), `ldc_w` (0x13),
`ldc2_w` (0x14), `aload_0` (0x2A), `aload_1` (0x2B), `astore_0` (0x4B), `astore_1` (0x4C),
`iastore` (0x4F), `aastore` (0x53), `bastore` (0x54), `castore` (0x55), `sastore` (0x56),
`dup` (0x59), `return` (0xB1), `getstatic` (0xB2), `putstatic` (0xB3),
`invokevirtual` (0xB6, virtual dispatch via `select_virtual_method`; resolves a
`ClassMirrorData` receiver to `java/lang/Class`), `invokespecial` (0xB7),
`invokestatic` (0xB8), `new` (0xBB), `newarray` (0xBC), `anewarray` (0xBD),
`arraylength` (0xBE), `checkcast` (0xC0, currently a no-op),
`multianewarray` (0xC5, only the outermost dimension is allocated), `ifnonnull` (0xC7).
Unknown opcodes throw. Argument counts come from `count_descriptor_arguments`.
The three `invoke*` opcodes share the `Interpreter::invoke` helper.

### Native classes
Registry in `class_loader.cpp` (`native_classes_desc`): `java/io/DataInputStream`,
`java/lang/Object`, `java/lang/Class`, `java/lang/String`, `java/util/Hashtable`,
`java/util/Random`, `java/util/Stack`, `java/util/Vector`,
`javax/microedition/midlet/MIDlet`, `com/nokia/mid/ui/FullCanvas`. Method tables
built by factories in `native_class_description.cpp`; impls in `native_classes/`
under Java-style namespaces (e.g. `java::lang::Object`, `com::nokia::mid::ui::FullCanvas`).
Callbacks take `(VM&, Frame&)` and typically pop the receiver (`this`) off the operand stack.
`<init>()V` callbacks (DataInputStream/Hashtable/Random/Stack/Vector/MIDlet/FullCanvas)
are stubs; `FullCanvas` also implements `getWidth()I`/`getHeight()I` returning fixed
placeholder dimensions (176x208). `Object` implements `getClass()Ljava/lang/Class;`
(returns the canonical heap mirror via `Heap::class_object_for`). `Class` registers
`getResourceAsStream(Ljava/lang/String;)Ljava/io/InputStream;` as a throwing
"not implemented" stub. `String` registers no native methods.

## Conventions
- Headers `.hpp`, sources `.cpp`; include via paths rooted at `source/` (e.g. `#include "runtime/vm.hpp"`).
- `snake_case` members with trailing underscore (`class_loader_`); types `PascalCase`.
- Native class impls live under namespaces mirroring the Java package
  (e.g. `java::lang::Object`, `java::util::Vector`, `com::nokia::mid::ui::FullCanvas`);
  static callbacks named `snake_case` (`get_class`, `get_width`).
- The `java/lang/Class` native class is implemented in files named `clazz.{hpp,cpp}`
  to avoid a duplicate-basename clash with `runtime/class.cpp` (MSVC module-scan).
- Copy deleted on owning types; raw pointers are non-owning, `unique_ptr` for ownership.
- big-endian reads in `BinaryReader`; class file magic `0xCAFEBABE`.

## Known gaps / TODO (as of writing)
- Native callbacks (DataInputStream/Hashtable/Random/Stack/Vector/MIDlet, String) are stubs; no real Java SE/J2ME library behavior.
- `checkcast` is a no-op; no NullPointerException/ArrayStoreException/NoSuchMethodError modeling (errors throw `std::runtime_error`).
- `multianewarray` (0xC5) allocates only the outermost dimension.
- String objects are not modeled: `ClassFile::get_constant` returns `nullptr` for `String` constants (so `ldc` of a string pushes null).
- `java/lang/Class.getResourceAsStream` is a throwing stub; real resource loading needs String modeling, a `java/io/InputStream` representation, and classpath resource lookup. There is no `java/io/InputStream` type yet.
- `Object.getClass()` only handles `InstanceData` receivers; array and `ClassMirrorData` (Class-of-a-Class) receivers throw `unsupported receiver kind`.
- Most JVM opcodes, full exception-table handling, and GC are unimplemented; `runtime_constant_pool_entry.hpp` cache is largely unused.
