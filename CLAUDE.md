# gothic-jvm — Agent Reference (CLAUDE.md)

> **Audience — agents only.** This file is the technical single-source-of-truth for AI
> coding agents. The human-facing overview, build/run walkthrough, and motivation live in
> [README.md](README.md). Keep onboarding prose in README.md; keep precise, terse technical
> facts (types, signatures, opcode tables, invariants, gaps) here.
>
> **Maintenance note:** Keep this file up to date. After every major change to the
> codebase (new opcodes, new runtime components, new native callbacks, build/test
> layout changes, or API renames), update the relevant sections below so this file
> stays an accurate single-source-of-truth and agents don't have to re-read every file.
> If the change is also user-visible (features, how to run), update README.md too.

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
- **Windowing/rendering:** SDL3 (window + 2D renderer + event loop), pulled in via CMake
  `FetchContent` (`GIT_TAG release-3.4.12`, built as a shared lib; the DLL is copied next to
  `app`/`app_tests` on Windows). Wrapped by the `Display` class (see [Windowing](#windowing)).
- **Tests:** GoogleTest (`third_party/gtest`, a git submodule), discovered via `gtest_discover_tests`
- **Executable:** `app` (entry `source/main.cpp`); core logic in static lib `app_lib`

## Build & test

```
cmake -S . -B build
cmake --build build
ctest --test-dir build        # or run the app_tests target
```

The top-level `CMakeLists.txt` fetches and builds **SDL3** via `FetchContent` (before defining
`app_lib`), links it into `app_lib` as `SDL3::SDL3` (PUBLIC, so `app` and `app_tests` both get
it), and on Windows copies `SDL3.dll` next to the `app` and `app_tests` binaries in a POST_BUILD
step. It also does `add_subdirectory(java_classes)`, which uses
`find_package(Java)` plus a `javac` custom command to compile the hand-maintained `.java`
runtime sources into `build/java_classes/` (target `java_classes`, on which `app` depends).
Building therefore needs a JDK whose `javac` still accepts `-source 1.3 -target 1.1`. Only the
eleven sources listed in `java_classes/CMakeLists.txt` are compiled (see the classpath notes
and gaps for what else `build/java_classes/` must contain at runtime).

The `tests/` target no longer injects a `TEST_FILES_DIR` compile definition (that stale
definition and `resources/test_files/` were removed); the sole remaining test
(`tests/test_binary_reader.cpp`) builds its inputs in memory. There is no `ClassFile`/`Class`
parsing or interpreter test coverage anymore (the former `test_class_file.cpp` was deleted).

### Runtime data / classpath

At startup the classpath is assembled from the **current working directory**, not from
`resources/`:

1. `VM` ctor adds `<cwd>/java_classes` and eagerly loads `java/lang/Class` and
   `java/lang/String` (`java/lang/Object` is pulled in transitively as their superclass).
2. `main.cpp` adds either the CLI-provided classpath entries, or (with no extra args)
   `<cwd>/gothic3thebeginning`.

Relevant on-disk assets:
- `java_classes/` (repo root) — hand-maintained bootstrap runtime classes kept as **`.java`
  sources** (`com/kostu96/gjvm/ResourceInputStream`, `com/nokia/mid/ui/FullCanvas`,
  `java/lang/{Class,String,System}`, `javax/microedition/lcdui/{Canvas,Font,Graphics,Image}`,
  `javax/microedition/midlet/{MIDlet,MIDletStateChangeException}`), plus a prebuilt
  `java/lang/Object.class` (there is no `Object.java`). `Command.java` also lives here but is
  **not** in the compile list. The CMake `java_classes` target compiles these with `javac`
  (see [Build & test](#build--test)); they are not committed as `.class` files.
- `resources/classes/` — a large **vendored J2ME/MIDP/CLDC class library** (real SDK
  `.class` files: `java.lang.*`, `java.io.*`, `java.util.*`, `javax.microedition.*`).
  Not on the default runtime classpath; it's the source pool the extra runtime classes
  (`java/io/*`, `java/util/*`, `StringBuffer`, …) sitting in `build/java_classes/` come from.
- `resources/gothic3thebeginning/` — MIDlet data: `HG.class` (default main class),
  obfuscated single-letter classes (`a`/`b.class`/`c`/`d.class`/…), `.mid`/`.mdl`/`.lng`
  data, `icon.png`, `s00.png`, `META-INF/`.
- `build/java_classes/` — output directory for the CMake javac step; this is what satisfies
  `<cwd>/java_classes` lookups when running from `build/`. Besides the freshly compiled
  sources it currently also holds `java/lang/Object.class` and dependency classes
  (`java/io/*`, `java/util/*`, `StringBuffer`, `Hashtable`, …) that the javac step does **not**
  produce (see gaps). `build/gothic3thebeginning/` mirrors the MIDlet data.

## Directory layout

```
source/
  main.cpp                      # entry: open Display; VM vm; add classpath; run vm.run on a thread; window loop
  class_loader/                 # .class file parsing + classpath-based class loading
    class_file.{hpp,cpp}        # ClassFile: raw parse of constant pool, fields, methods, Code attr, MUTF-8 decode
    constant_pool_entry.hpp     # ConstantPoolEntry variant (Utf8/Integer/Long/Class/String/refs/NameAndType)
    class_loader.{hpp,cpp}      # ClassLoader: classpath resolve + cache; array classes via load_array
  platform/
    display.{hpp,cpp}           # Display: owns SDL_Window + SDL_Renderer; process_events/clear/present; width/height
  runtime/                      # execution model
    vm.{hpp,cpp}                # VM: owns NativeMethods/ClassLoader/Heap/Interpreter; boot Class+String; run+init MIDlet
    class.{hpp,cpp}             # Class/Method/Field; constant-pool resolution; binds ACC_NATIVE methods to callbacks
    native_methods.{hpp,cpp}    # NativeMethods: "<class>.<name><descriptor>" -> C++ callback registry
    interpreter.{hpp,cpp}       # bytecode dispatch loop (execute/invoke/run) -> optional<Value>
    frame.{hpp,cpp}             # Frame: locals, operand stack, pc, code readers, branch()
    heap.{hpp,cpp}              # Heap: instances/arrays + interned Strings + canonical Class mirrors (owns unique_ptrs)
    object.{hpp,cpp}            # Object = variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>
    opcodes.hpp                 # op_* bytecode opcode constants
    value.hpp                   # Value = variant<monostate,int32,int64,float,double,Object*>
    runtime_constant_pool_entry.hpp  # resolved CP cache types (Class/String/Field/Method/InterfaceMethod)
  utils/
    binary_reader.{hpp,cpp}     # big-endian u8/u16/u32, string/bytes, bounds-checked
tests/
  CMakeLists.txt, test_binary_reader.cpp
resources/
  classes/                      # vendored J2ME/MIDP/CLDC standard library (.class)
  gothic3thebeginning/          # MIDlet data (HG.class, obfuscated classes, mid/mdl/lng, images, META-INF)
java_classes/                   # hand-maintained runtime classes (+ some .java sources)
third_party/gtest/              # vendored GoogleTest (submodule)
build/                          # generated VS solution/projects + <cwd> copies of java_classes/gothic3thebeginning
```

## Architecture & flow

1. `main.cpp` (`args <main_class> [<class_path_entry>...]`) opens the SDL3 `Display`
   (240x320 logical size, scale 2 -> 480x640 window + renderer), default-constructs a `VM`, connects the two via
   `vm.set_display(&display)`, and configures the classpath (default main class `HG`; with no
   extra args the classpath defaults to `<cwd>/gothic3thebeginning`, otherwise each extra arg
   is added as an entry). It then launches `vm.run(main_class)` on a **background
   `std::thread`** (SDL must own the thread that created the window/pumps events, and a
   MIDlet's `startApp()` may never return) and runs the window event/render loop on the main
   thread (`process_events` -> `clear` -> `present`). When the user closes the window it calls
   `vm.request_stop()` and joins the JVM thread. The JVM thread swallows `VmStopRequested`
   (clean shutdown) and prints other exceptions to stderr.
2. `VM` ctor wires its owned `NativeMethods` into the `ClassLoader`, adds
   `<cwd>/java_classes` to the classpath, eagerly loads `java/lang/Class` then
   `java/lang/String` (`java/lang/Object` loads transitively), registers the String class
   with the heap (`Heap::set_string_class`) so literals can be interned, and initializes
   `String`.
3. `VM::run(main_class)` -> `load` main class -> `initialize_class`, then
   `new_instance(main)`, invokes `<init>()V`, then invokes the MIDlet lifecycle entry
   `startApp()V` (both via `Interpreter::execute` with the receiver passed as the sole
   argument).
4. `Interpreter` holds a `VM&` (ctor `Interpreter(VM&)`). `execute(method, args = {})`
   takes only the `Method` (the owner comes from `method.owner`), builds a
   `Frame(method.owner, method)`, copies args into locals using `arg_slot_widths`, then
   `run()` dispatches opcodes in a `switch`, returning `optional<Value>`. The private
   `invoke(method, frame)` helper centralizes the native-vs-bytecode call path: native
   methods call the `Method::native_callback` (a `const NativeMethods::Callback*`) with the
   live frame, throwing if it is null; bytecode methods pop `num_args` args and re-enter
   `execute`.
5. `initialize_class` walks the superclass chain first (recursively) and then runs
   `<clinit>()V` if present, tracking `Class::InitState` (Loaded/Initializing/Initialized/Failed).
6. `ClassLoader::load` checks a cache, routes names starting with `[` to `load_array`,
   otherwise resolves `binary/name` -> `<entry>/binary/name.class` across classpath
   entries and constructs a `Class` (which recursively loads its super + interfaces).

### Key types
- **Value** (`value.hpp`): JVM slot = `variant<monostate, int32_t, int64_t, float, double, Object*>`.
  `Object*` is the reference type (`nullptr` == null). Doubles/longs occupy a single slot here.
- **Object** (`object.hpp`): tagged union (not a class hierarchy),
  `variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>` accessed
  via `std::get_if`/`std::holds_alternative` on `.data`.
  - `InstanceData` holds `Class& type` + `vector<Value> fields` (indexed by field slot) + a
    `native_payload` (`NativePayload = variant<monostate, ResourceInputStreamNativeData,
    StringNativeData>`, declared in `object.hpp`). The payload backs C++-side state for objects
    that need it: `String` instances carry a `StringNativeData{value}` (the raw `std::string`),
    and `ResourceInputStream` instances carry a `ResourceInputStreamNativeData{buffer, position}`.
  - `PrimitiveArrayData` holds a `variant` of typed element vectors (boolean/byte→uint8,
    char→char16, short→int16, int→int32, long→int64, float, double) with `ElementType`
    tags matching the JVM `newarray` atype codes (4..11); `get`/`set` widen to `Value`.
  - `InstanceArrayData` holds `Class& element_type` + `vector<Object*>`.
  - `ClassMirrorData` holds the mirrored `Class&` (the `java/lang/Class` object identity).
- **Class** (`Class::Kind`: `Ordinary`, `Array`, `Primitive`): built from a parsed `ClassFile`
  via `Class(const char* filename, ClassLoader&)` (Ordinary), or synthesized via
  `Class(std::string name, Class* component_type = nullptr)` — a non-null `component_type`
  yields an `Array`, null yields a `Primitive`; both are constructed already in
  `InitState::Initialized`. Holds `super_`/`interfaces_` (`Class*`), `treat_super_specially_`
  (`ACC_SUPER`), `is_interface_` (`ACC_INTERFACE`), static + instance `Field`s (instance fields
  get sequential `slot`s continuing the super's `instance_field_count_`), `Method`s, a
  lazily-resolved `runtime_constant_pool_`, and (arrays only) `component_type_`. Key API:
  `find_method`/`find_field` (this class only), `resolve_class/field/method` (cached CP
  resolution; field and method resolution walk the full superclass + superinterface hierarchy),
  `resolve_constant(index, class_loader, heap)` (ints/longs + `String`s interned via
  `Heap::new_interned_string`), plus predicates/accessors `kind()`, `is_interface()`,
  `treat_super_specially()`, `component_type()`, `this_name()`, `super()`/`super_name()`,
  `instance_field_count()`, `init_state()`/`set_init_state()`. (`create_string`,
  `resolve_class_name`, `resolve_method_ref`, and `is_primitive()` no longer exist — test for
  primitive classes with `kind() == Kind::Primitive`; String materialization now lives on the
  heap.)
- **Method**: `owner` (Class&), `is_static`, `is_native`, name, descriptor, `num_args` (argument
  count including `this` for instance methods), `arg_slot_widths` (per-argument local-slot widths
  incl. the leading `this` slot; `long`/`double` = 2, everything else = 1; computed once at parse
  time), `max_stack`/`max_locals`, `code` span, `exception_table` span, `native_callback`
  (`const NativeMethods::Callback*`, i.e. a pointer into the `NativeMethods` registry; `nullptr`
  when the method is not native or has no registered binding). `Interpreter::invoke(method, frame)`
  pops `num_args` operand-stack entries; `execute` lays arguments into locals using `arg_slot_widths`.
- **Field**: `owner`, `is_static`, name, descriptor, `Value value` (static storage), `slot` (instance index).
- **Heap**: owns all `Object`s via `unique_ptr`; `new_instance`, `new_primitive_array`,
  `new_instance_array`, `new_interned_string(str)` (materializes a `java/lang/String`; see
  below), and `class_object_for(Class&)` which lazily creates and caches one canonical
  `java/lang/Class` mirror per `Class` (stable identity for `getClass()`). Requires the String
  class registered via `set_string_class` before any string is interned. `new_interned_string`
  now **deduplicates** via the `string_objects_` map (keyed by the raw string), returning the
  cached `String` for a repeated literal instead of allocating a fresh one.
- **Frame**: `owner()`/`method()`, `locals()`, `operand_stack()`, `get_pc()`, `branch(offset)`,
  `pop_code_u8/u16`, `push_stack`/`pop_stack`/`peek_stack(index = 0)` (peek is depth-indexed
  from the top of the stack). (No `set_pc`; jumps go through `branch`.)
- **Runtime constant pool** (`runtime_constant_pool_entry.hpp`): `RuntimeClassInfo`,
  `RuntimeFieldRefInfo`, `RuntimeMethodRefInfo` and `RuntimeStringInfo` are populated on first
  resolution and cache the resolved pointer (`RuntimeStringInfo.resolved` caches the interned
  `java/lang/String`). `RuntimeIntegerInfo`/`RuntimeLongInfo` are used as tags only (the value
  is re-read from the `ClassFile`). `RuntimeInterfaceMethodRefInfo` exists in the variant but
  is not populated yet.
- **Display** (`platform/display.{hpp,cpp}`): wraps SDL3. Ctor `Display(title, width, height,
  scale)` calls `SDL_Init(SDL_INIT_VIDEO)` + `SDL_CreateWindowAndRenderer` (window sized
  `width*scale` x `height*scale`, then vsync on via `SDL_SetRenderVSync`); dtor tears it all
  down. `width()`/`height()` return the **logical** (unscaled) size. `process_events()` pumps
  SDL events and returns `false` on `SDL_EVENT_QUIT`/`SDL_EVENT_WINDOW_CLOSE_REQUESTED`;
  `clear(r,g,b)`/`present()` drive the 2D renderer; `width()`/`height()`/`renderer()` expose
  the surface. Non-owned by `VM` (a `Display*` set via `VM::set_display`), so the VM can run
  headless in tests.

### Windowing
`main.cpp` constructs one `Display` on the stack (240x320 logical, scale 2 -> 480x640 window,
title `gothic-jvm`) before booting the `VM`, wires it in with `vm.set_display(&display)`, then
runs `vm.run` on a background
`std::thread` while the **main thread** spins the window event/render loop
(`while (display.process_events()) { clear(0,0,0); present(); }`). SDL owns the main thread
(window creation + event pump must live there), so the JVM runs concurrently; the `Display`'s
`width_`/`height_` are set once in the ctor and only read afterwards, so the JVM thread reads
them race-free. On window close, `main` calls `vm.request_stop()` (an atomic the interpreter
loop checks each instruction, throwing `VmStopRequested` to unwind a MIDlet that never returns
from `startApp()`) and joins the JVM thread. Native callbacks reach the screen through
`vm.display()` (currently just Canvas size). Actual MIDP `Graphics`/`Canvas.paint` rendering
into the renderer is still TODO: `Graphics.java` and `Image.java` now model the MIDP API
(`fillRect`/`setColor`/`getGraphics`/`createImage`), but their `init`/`fillRect` natives are
no-op stubs, so nothing is drawn yet. Note the deeper
gap: proper MIDlet execution also needs Java thread support, `Display.setCurrent`, and a
`paint`/`repaint`/`serviceRepaints` event-dispatch model, none of which exist yet — so only
narrow single-threaded, self-rendering MIDlets could run even with the non-blocking loop.

### String modeling
`Heap::new_interned_string(str)` builds a real `java/lang/String` instance: it allocates a
backing `char[]` (one char per byte — ASCII/MUTF-8-ish), then populates the String's
`value:[C`, `offset:I`, `count:I`, `hash:I` fields directly (rather than running an
`<init>`), locating each field by name/descriptor via `find_field`, and also stashes the raw
`std::string` in the instance's `native_payload` as `StringNativeData` (used by natives such
as `ResourceInputStream.init`). Repeated literals are deduplicated via the heap's
`string_objects_` map. `Class::resolve_constant` uses this to intern `String` constants (`ldc`
of a string now yields a live object instead of null) and caches the result in the runtime
constant pool (`RuntimeStringInfo.resolved`). The exact field layout depends on the vendored
`java/lang/String.class` on the classpath, and the String class must have been registered with
the heap (`Heap::set_string_class`, done in the `VM` ctor) first.

### Implemented opcodes (dispatched in `interpreter.cpp`)
Constants/consts: `nop` (0x00), `aconst_null` (0x01), `iconst_m1..5` (0x02–0x08),
`lconst_0/1` (0x09–0x0A), `fconst_0/1/2` (0x0B–0x0D), `dconst_0/1` (0x0E–0x0F),
`bipush` (0x10), `sipush` (0x11), `ldc` (0x12), `ldc_w` (0x13), `ldc2_w` (0x14).
Loads/stores: `iload` (0x15), `aload` (0x19), `iload_0..3` (0x1A–0x1D),
`lload_0..3` (0x1E–0x21), `aload_0..3` (0x2A–0x2D), `iaload` (0x2E), `aaload` (0x32),
`caload` (0x34), `istore` (0x36), `astore` (0x3A), `istore_0..3` (0x3B–0x3E),
`astore_0..3` (0x4B–0x4E), `iastore` (0x4F), `aastore` (0x53), `bastore` (0x54),
`castore` (0x55), `sastore` (0x56).
`aastore` performs a partial `ArrayStoreException`-style check (a stored `InstanceData`'s
type must match the array's `element_type`).
Stack/math: `dup` (0x59), `iadd` (0x60), `isub` (0x64), `imul` (0x68), `idiv` (0x6C),
`ishl` (0x78), `ishr` (0x7A), `iand` (0x7E), `land` (0x7F), `ior` (0x80), `lxor` (0x83),
`iinc` (0x84), `i2s` (0x93).
Branches: `ifne` (0x9A), `iflt` (0x9B), `ifge` (0x9C), `ifle` (0x9E),
`if_icmpeq` (0x9F), `if_icmpne` (0xA0), `if_icmplt` (0xA1), `if_icmpge` (0xA2),
`goto` (0xA7), `ifnull` (0xC6), `ifnonnull` (0xC7).
Returns: `ireturn` (0xAC), `areturn` (0xB0), `return` (0xB1).
Fields/calls: `getstatic` (0xB2), `putstatic` (0xB3), `getfield` (0xB4), `putfield` (0xB5),
`invokevirtual` (0xB6, virtual dispatch by walking `find_method` up the receiver's superclass
chain; a `ClassMirrorData` receiver dispatches against `java/lang/Class`), `invokespecial`
(0xB7, resolved via `resolve_method`; throws for the not-yet-implemented `ACC_SUPER`
super-invoke case), `invokestatic` (0xB8, triggers `initialize_class`).
Object/array: `new` (0xBB), `newarray` (0xBC), `anewarray` (0xBD), `arraylength` (0xBE),
`checkcast` (0xC0, resolves the class but performs no type check), `multianewarray` (0xC5,
recursively allocates every requested dimension via a self-recursive lambda).

Notes:
- `op_lload` (0x16) is declared in `opcodes.hpp` but has no dispatch case yet.
- `ldc`/`ldc_w`/`ldc2_w` resolve `Integer`/`Long`/`String`/`Class` constants through the
  runtime constant pool: a `String` constant interns a live `java/lang/String`, and a `Class`
  constant pushes the heap `java/lang/Class` mirror. (`Float`/`Double` constants still fail to
  parse; see gaps.)
- The interpreter prints the operand stack and each decoded opcode to stdout (verbose trace).
- Unknown opcodes throw `std::runtime_error`. Argument/slot layout comes from
  `Method::arg_slot_widths` (computed by `compute_arg_slot_widths` at parse time); the three
  `invoke*` opcodes share `Interpreter::invoke`.
- Error conditions that a real VM would surface as Java exceptions (NPE, div-by-zero,
  ArrayStore, index OOB, missing field/method) are thrown as `std::runtime_error`.

### Native callbacks
Native bindings live in a dedicated `NativeMethods` class (`native_methods.{hpp,cpp}`). The
`VM` owns one `NativeMethods` instance and hands it to the `ClassLoader` (ctor takes
`const NativeMethods&`, exposed via `ClassLoader::native_methods()`). Its constructor fills an
`unordered_map<std::string, Callback>` keyed by `"<class>.<name><descriptor>"`;
`find(class, name, descriptor)` returns a `const Callback*` (or `nullptr`). When `Class` parses
a method flagged `ACC_NATIVE`, it looks up that key and stores the resulting pointer in
`Method::native_callback`. Currently registered:
- `java/lang/System.currentTimeMillis()J` — real wall-clock millis.
- `javax/microedition/lcdui/Canvas.getWidth()I` / `getHeight()I` — return the connected
  `Display`'s width/height (`vm.display()`), falling back to 240 / 320 when no display is set.
- `java/lang/Object.getClass()Ljava/lang/Class;` — returns the canonical heap `Class` mirror
  (only handles `InstanceData` receivers).
- `java/lang/Class.getName()Ljava/lang/String;` — interns a String of the mirrored name.
- `java/lang/String.charAt(I)C`, `String.indexOf(II)I` and `String.lastIndexOf(I)I` — operate
  on the backing `char[]`.
- `com/kostu96/gjvm/ResourceInputStream.init(Ljava/lang/String;)V` — loads the named resource
  off the classpath (`ClassLoader::load_resource`) into the instance's
  `ResourceInputStreamNativeData` payload; `ResourceInputStream.read()I` returns the next byte
  from that buffer. Both are bound to real **private native** methods (declared `native` in the
  `.java` source and called from a plain constructor), not to `<init>`.
- `javax/microedition/lcdui/Font.init()V`, `Graphics.init(Ljavax/microedition/lcdui/Image;)V`
  and `Image.init(II)V` — no-op stubs, likewise bound to private native `init` methods.

## Conventions
- Headers `.hpp`, sources `.cpp`; include via paths rooted at `source/` (e.g. `#include "runtime/vm.hpp"`).
- `snake_case` members with trailing underscore (`class_loader_`); types `PascalCase`; opcode
  constants `op_snake_case`.
- Native callback functions are free functions in an anonymous namespace in
  `native_methods.cpp`, named after their fully-qualified Java method
  (`java_lang_Object_getClass`, etc.).
- Copy deleted on owning types; raw pointers are non-owning, `unique_ptr` for ownership.
- Big-endian reads in `BinaryReader`; class file magic `0xCAFEBABE`; UTF-8 constants are
  (modified) UTF-8.

## Known gaps / WIP / dead code (as of writing)
- **Array/primitive classes now work:** `Class(std::string name, Class* component_type)` sets
  `kind_` to `Array` (component non-null) or `Primitive` (null) and starts in
  `InitState::Initialized`. `ClassLoader::load_array` synthesizes the component chain
  (nested `[`, `L...;` refs, and cached primitive classes keyed by their Java name). Type
  checks for `checkcast`/`instanceof` are still not enforced, though.
- **Constant pool coverage is partial:** `ClassFile` parses only Utf8/Integer/Long/Class/
  String/Field/Method/InterfaceMethod/NameAndType tags. `Float` (tag 4) and `Double` (tag 6)
  are **not** parsed and will throw "Unsupported constant pool tag". `get_constant` only
  returns Integer/Long (and monostate for String, which `resolve_constant` then interns).
- **`invokespecial` `ACC_SUPER` is unimplemented:** a non-constructor, non-interface call that
  targets the direct superclass of an `ACC_SUPER` class throws
  "invokespecial: ACC_SUPER semantics not implemented yet".
- **Dead / commented code:** `decode_modified_utf8` in `class_file.cpp` is defined but never
  called; a commented-out `java_lang_Class_newInstance` native remains in
  `native_methods.cpp`; `Class::resolve_constant` has an unreachable fallback block after its
  `std::visit` return; several `TODO(Kostu)` markers persist (empty `Font.init`/`Graphics.init`/
  `Image.init` bodies, hardcoded canvas fallback size).
- **Many `.java` runtime methods are declared `native` but have no C++ binding** and would
  throw "Failed to call native" if invoked: `java/lang/System.{arraycopy,identityHashCode,
  getProperty,exit,gc}`, `java/lang/Class.{forName,newInstance,isInstance,isAssignableFrom,
  isInterface,isArray}`, `java/lang/String.{replace,substring,init}`, and
  `javax/microedition/lcdui/Graphics.fillRect(IIIII)V`. Conversely, `String.indexOf(II)I` is
  registered in `NativeMethods` but `java/lang/String.java` declares no such method, so that
  binding never attaches.
- **The CMake javac step only compiles the eleven listed sources.** `java_classes/java/lang/
  Object.class` is a committed prebuilt file (no `Object.java`), and the other runtime classes
  the MIDlet needs (`java/io/*`, `java/util/*`, `StringBuffer`, …) are vendored copies that
  currently sit in `build/java_classes/`; nothing copies them from `resources/classes/`, so a
  clean build tree would be missing both `Object` and those dependencies.
- `checkcast` (0xC0) resolves the target class but is otherwise a no-op; no real exception
  objects/handler-table dispatch (errors throw `std::runtime_error`); no
  `NoSuchMethodError`/`AbstractMethodError` modeling.
- **Runtime constant-pool caching now works, with leftover dead code:** `resolve_constant`
  reads through the runtime constant pool — caching resolved `String` literals in
  `RuntimeStringInfo.resolved`, resolving `Class` constants to a heap mirror, and re-reading
  int/long values from the `ClassFile` (the `RuntimeIntegerInfo`/`RuntimeLongInfo` slots act as
  tags only). An unreachable fallback block remains after the `std::visit` return.
  `RuntimeInterfaceMethodRefInfo` is unused; `invokeinterface` is not implemented.
- **Test coverage shrank:** only `tests/test_binary_reader.cpp` remains (`test_class_file.cpp`
  and `resources/test_files/` were deleted, along with the `TEST_FILES_DIR` compile
  definition), so class-file/`Class` parsing and interpreter behaviour are untested.
- No garbage collection; the heap only grows. Most JVM opcodes remain unimplemented.
