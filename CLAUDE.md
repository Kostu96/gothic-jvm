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
thirteen sources listed in `java_classes/CMakeLists.txt` are compiled (see the classpath notes
and gaps for what else `build/java_classes/` must contain at runtime).

The `tests/` target no longer injects a `TEST_FILES_DIR` compile definition (that stale
definition and `resources/test_files/` were removed); the sole remaining test
(`tests/test_binary_reader.cpp`) builds its inputs in memory. There is no `ClassFile`/`Class`
parsing or interpreter test coverage anymore (the former `test_class_file.cpp` was deleted).

### Runtime data / classpath

At startup the classpath is assembled from the **current working directory**, not from
`resources/`:

1. `VM` ctor adds `<cwd>/java_classes` only (it no longer eagerly loads any class; loading
   and initialization are deferred to `VM::run`'s bootstrap).
2. `main.cpp` adds either the CLI-provided classpath entries, or (with no extra args)
   `<cwd>/gothic3thebeginning`.

Relevant on-disk assets:
- `java_classes/` (repo root) — hand-maintained bootstrap runtime classes kept as **`.java`
  sources** (`com/kostu96/gjvm/ResourceInputStream`, `com/nokia/mid/ui/FullCanvas`,
  `java/lang/{Class,String,System}`,
  `javax/microedition/lcdui/{Canvas,Display,Displayable,Font,Graphics,Image}`,
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
    vm.{hpp,cpp}                # VM: owns NativeMethods/ClassLoader/Heap/Interpreter + the main Thread; bootstrap state machine
    thread.{hpp,cpp}            # Thread: a green thread = an explicit call stack (vector<Frame>); push/pop/current frame
    class.{hpp,cpp}             # Class/Method/Field; constant-pool resolution; ensure_initialized; binds ACC_NATIVE callbacks
    native_methods.{hpp,cpp}    # NativeMethods: "<class>.<name><descriptor>" -> C++ callback registry
    interpreter.{hpp,cpp}       # budgeted bytecode dispatch loop: run(Thread&, num_instructions) / invoke(Thread&, Method)
    frame.{hpp,cpp}             # Frame: locals, operand stack, pc (pc()/set_pc()/branch()), code readers; movable
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

> **Execution model (WIP): green threads on an explicit frame stack.** The interpreter is being
> converted from a C++-recursive design to a *stackless, budgeted* one so multiple Java threads
> can eventually be scheduled cooperatively. Each `Thread` owns its own call stack
> (`std::vector<Frame>`); `Interpreter::run` executes a bounded number of instructions against
> the top frame and returns, so a scheduler could round-robin threads by quantum. There is **no
> scheduler yet** — the `VM` drives a single `main_thread_`.

1. `main.cpp` (`args <main_class> [<class_path_entry>...]`) opens the SDL3 `Display`
   (240x320 logical, scale 2 -> 480x640 window + renderer), default-constructs a `VM`, calls
   `vm.set_main_class(...)` and `vm.set_display(&display)`, and configures the classpath
   (default main class `HG`; with no extra args the classpath defaults to
   `<cwd>/gothic3thebeginning`, otherwise each extra arg is added as an entry). It then runs
   `vm.run()` on a **background `std::thread`** (SDL must own the thread that created the
   window/pumps events, and a MIDlet's `startApp()` may never return) and spins the window
   event/render loop on the main thread (`process_events` -> `clear` -> `present`). On window
   close it calls `vm.request_stop()` and joins the JVM thread, which swallows
   `VmStopRequested` (clean shutdown) and prints other exceptions to stderr.
2. `VM` ctor only wires its owned `NativeMethods` into the `ClassLoader` and adds
   `<cwd>/java_classes` to the classpath. It does **not** eagerly load or initialize any class.
3. `VM::run()` is a **bootstrap state machine** driving `main_thread_`. It loads
   `java/lang/String` (registering it with the heap via `Heap::set_string_class` so literals can
   be interned) and the main class, then loops `interpreter_.run(main_thread_, 500)` while
   advancing through phases, each guarded by `Thread::is_terminated()` (the frame stack drained
   empty) so the previous phase's work finishes before the next is scheduled: `Boot` initializes
   `String`; `Phase1` initializes the main class; `Phase2` `new_instance`s the MIDlet and pushes
   its `<init>()V`; `Phase3` pushes `startApp()V`; `Phase4` returns once `startApp` unwinds.
   Class init and lifecycle entries are scheduled as pushed frames, not synchronous C++ calls.
4. `Interpreter` holds a `VM&` (ctor `Interpreter(VM&)`). `run(Thread& thread, size_t
   num_instructions)` executes up to `num_instructions` opcodes against `thread.current_frame()`
   in a `switch`, checking `vm.stop_requested()` each step and stopping early if the thread
   terminates. Method calls do **not** recurse in C++: the private
   `invoke(Thread&, const Method&)` either runs a native (calls `Method::native_callback` with
   `thread.current_frame()`, throwing if null) or, for bytecode, pops `arg_slot_widths.size()`
   operand-stack entries and `Thread::push_frame`es a new `Frame`. Returns (`ireturn`/`areturn`/
   `return`) `pop_frame` and push any result onto the caller's operand stack.
5. Class initialization is stackless and lazy (see [Class initialization](#class-initialization)):
   the init-triggering opcodes (`getstatic`/`putstatic`/`invokestatic`/`new`) check
   `Class::needs_initialization()`, and if so rewind the frame's pc to the current instruction
   and call `Class::ensure_initialized(thread)`, which pushes the `<clinit>` frame(s)
   super-class-first and returns to the loop; the instruction re-executes once the class is
   initialized.
6. `ClassLoader::load` checks a cache, routes names starting with `[` to `load_array`,
   otherwise resolves `binary/name` -> `<entry>/binary/name.class` across classpath entries and
   constructs a `Class` (which recursively loads its super + interfaces).

### Key types
- **Value** (`value.hpp`): JVM slot = `variant<monostate, int32_t, int64_t, float, double, Object*>`.
  `Object*` is the reference type (`nullptr` == null). Doubles/longs occupy a single slot here.
- **Object** (`object.hpp`): tagged union (not a class hierarchy),
  `variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData>` accessed
  via `std::get_if`/`std::holds_alternative` on `.data`.
  - `InstanceData` holds `Class& type` + `vector<Value> fields` (indexed by field slot) + a
    `native_payload` (`NativePayload = variant<monostate, ResourceInputStreamNativeData,
    StringNativeData, ImageNativeData, GraphicsNativeData>`, declared in `object.hpp`). The
    payload backs C++-side state for objects that need it: `String` instances carry a
    `StringNativeData{value}` (the raw `std::string`), `ResourceInputStream` instances carry a
    `ResourceInputStreamNativeData{buffer, position}`, `Image` instances carry an
    `ImageNativeData{sdl_surface}`, and `Graphics` instances carry a
    `GraphicsNativeData{sdl_renderer, sdl_surface}`.
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
  `instance_field_count()`. Initialization API: `needs_initialization()` (true only when
  `init_state_ == Loaded`), `ensure_initialized(Thread&)` (schedules `<clinit>` frames — see
  [Class initialization](#class-initialization)), and `set_initialized()` (flips to
  `Initialized`; called from `op_return` when a `<clinit>` frame unwinds). `InitState` is now
  effectively private — there is no general `init_state()`/`set_init_state()` accessor.
  (`create_string`, `resolve_class_name`, `resolve_method_ref`, and `is_primitive()` no longer
  exist — test for primitive classes with `kind() == Kind::Primitive`; String materialization
  now lives on the heap.)
- **Method**: `owner` (Class&), name, descriptor, `arg_slot_widths` (per-argument local-slot
  widths incl. the leading `this` slot; `long`/`double` = 2, everything else = 1; computed once
  at parse time — its `.size()` is the operand count `invoke` pops), `max_stack`/`max_locals`,
  `is_static`, `is_native`, `is_class_initializer` (name `<clinit>`, descriptor `()V`, static;
  used by `op_return` to mark the owner `Initialized`), `code` span, `exception_table` span,
  `native_callback` (`const NativeMethods::Callback*` into the `NativeMethods` registry;
  `nullptr` when not native or unbound). `Interpreter::invoke(thread, method)` pops
  `arg_slot_widths.size()` operand-stack entries and `Thread::push_frame` lays them into locals.
  (There is no longer a `num_args` field.)
- **Field**: `owner`, `is_static`, name, descriptor, `Value value` (static storage), `slot` (instance index).
- **Heap**: owns all `Object`s via `unique_ptr`; `new_instance`, `new_primitive_array`,
  `new_instance_array`, `new_interned_string(str)` (materializes a `java/lang/String`; see
  below), and `class_object_for(Class&)` which lazily creates and caches one canonical
  `java/lang/Class` mirror per `Class` (stable identity for `getClass()`). Requires the String
  class registered via `set_string_class` before any string is interned. `new_interned_string`
  now **deduplicates** via the `string_objects_` map (keyed by the raw string), returning the
  cached `String` for a repeated literal instead of allocating a fresh one.
- **Frame** (movable so it can live in `Thread::frames_`): `owner()`/`method()`, `locals()`,
  `operand_stack()`, `pc()`/`set_pc(pc)` (jumps use `branch(offset)`; `set_pc` rewinds a whole
  instruction for lazy class init), `pop_code_u8/u16`, `push_stack`/`pop_stack`/
  `peek_stack(index = 0)` (peek is depth-indexed from the top). Ctor sizes `locals` to
  `max_locals` and reserves `max_stack`.
- **Thread** (`thread.{hpp,cpp}`): a green thread = an explicit JVM call stack. Owns
  `std::vector<Frame> frames_`; `current_frame()` (top), `push_frame(method, span<const Value>
  args)` (constructs a `Frame` and lays `args` into locals via `arg_slot_widths`), `pop_frame()`,
  `is_terminated()` (`frames_.empty()`). The `VM` owns one `main_thread_`; there is no scheduler
  or second thread yet. Because `Frame`s live in a `vector`, `push_frame` can reallocate and
  invalidate a held `Frame&` — the interpreter re-fetches `current_frame()` each instruction, and
  init-triggering opcodes rewind pc *before* calling `ensure_initialized`.
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
`vm.display()` (Canvas size) and through the MIDP `Graphics`/`Image` natives, which now do
**real** 2D drawing — but onto **offscreen** SDL surfaces, not the on-screen window.
`Image.init` allocates an `SDL_Surface` (ARGB8888); `Graphics.init` wraps a surface in an
`SDL_CreateSoftwareRenderer`; `Graphics.fillRect`/`drawStringNative` render into it; and
`Image.getRGB` reads pixels back. The window's own renderer is still only cleared to black and
presented each frame — nothing blits those offscreen surfaces to the window yet. `Display.java`
models `getDisplay`/`setCurrent`, but `setCurrent` is a no-op and there is no
`paint`/`repaint`/`serviceRepaints` event-dispatch model, nor Java thread support — so full
MIDlet execution is still not wired up.

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
the heap (`Heap::set_string_class`, done in `VM::run`'s bootstrap) first.

### Class initialization
Class init is **stackless and lazy**, driven off the running `Thread`'s frame stack rather than
a recursive C++ call:
- `Class::needs_initialization()` is true only in `InitState::Loaded`. `Initializing`/
  `Initialized` both mean "proceed" (so a `<clinit>` may touch its own class); `Failed` throws.
- The init-triggering opcodes (`getstatic`, `putstatic`, `invokestatic`, `new`) check it and,
  when true, `frame.set_pc(last_pc)` to rewind to the *start* of the current instruction, then
  call `Class::ensure_initialized(thread)` and fall through to the next dispatch iteration. The
  instruction re-executes after init completes; checking before any effect keeps the retry
  idempotent (no double allocation for `new`, args stay on the stack for `invokestatic`).
- `ensure_initialized` sets the class `Initializing`, pushes its `<clinit>()V` frame (or flips
  straight to `Initialized` when there is none), then recurses into `super_` — pushing *this*
  class's clinit **before** the super's so the super ends up on top of the LIFO stack and runs
  first (super-class-first ordering).
- When a `<clinit>` frame's `return` executes, `op_return` sees `Method::is_class_initializer`
  and calls `Class::set_initialized()`, closing the `Initializing -> Initialized` transition.
- Caveats: the `Failed` state is effectively unreachable (a throwing clinit becomes a
  `std::runtime_error` that kills the thread and leaves the class stuck in `Initializing`),
  interfaces are not walked (only the `super_` chain), and there is no cross-thread init locking.

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
Stack/math: `dup` (0x59), `dup2` (0x5C, single-value for a long/double top),
`iadd` (0x60), `isub` (0x64), `imul` (0x68), `idiv` (0x6C), `irem` (0x70), `ishl` (0x78),
`ishr` (0x7A), `iand` (0x7E), `land` (0x7F), `ior` (0x80), `lxor` (0x83), `iinc` (0x84),
`i2b` (0x91), `i2s` (0x93).
Branches: `ifeq` (0x99), `ifne` (0x9A), `iflt` (0x9B), `ifge` (0x9C), `ifle` (0x9E),
`if_icmpeq` (0x9F), `if_icmpne` (0xA0), `if_icmplt` (0xA1), `if_icmpge` (0xA2),
`goto` (0xA7), `ifnull` (0xC6), `ifnonnull` (0xC7).
Returns: `ireturn` (0xAC), `areturn` (0xB0), `return` (0xB1). Returns are stackless: they
`Thread::pop_frame` the current activation and (for `ireturn`/`areturn`) push the result onto
the caller's operand stack; a `return` from a `<clinit>` frame marks its owner class
`Initialized`. Returning from the outermost frame empties the thread's stack, terminating it.
Fields/calls: `getstatic` (0xB2), `putstatic` (0xB3), `getfield` (0xB4), `putfield` (0xB5),
`invokevirtual` (0xB6, virtual dispatch by walking `find_method` up the receiver's superclass
chain; a `ClassMirrorData` receiver dispatches against `java/lang/Class`), `invokespecial`
(0xB7, resolved via `resolve_method`; throws for the not-yet-implemented `ACC_SUPER`
super-invoke case), `invokestatic` (0xB8). `getstatic`/`putstatic`/`invokestatic`/`new` each
guard on `Class::needs_initialization()` and, when true, rewind pc + `ensure_initialized`
instead of performing their effect (lazy stackless init — see
[Class initialization](#class-initialization)); the `invoke*` opcodes push a frame via the
shared `Interpreter::invoke` rather than recursing.
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
  `Display`'s width/height; there is no null-display fallback, so they dereference
  `vm.display()` directly.
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
- `javax/microedition/lcdui/Font.init()V` — still a no-op stub (bound to a private native `init`).
- `javax/microedition/lcdui/Image.init(II)V` — allocates an offscreen `SDL_Surface`
  (`SDL_CreateSurface`, ARGB8888) into the instance's `ImageNativeData`;
  `Image.getRGB([IIIIIII)V` reads pixels back from that surface into an `int[]`.
- `javax/microedition/lcdui/Graphics.init(Ljavax/microedition/lcdui/Image;)V` — wraps the
  backing `Image`'s surface in an `SDL_CreateSoftwareRenderer`, stored in `GraphicsNativeData`.
- `javax/microedition/lcdui/Graphics.fillRect(IIII)V` — sets the draw color from the Graphics
  `color:I` field and fills an `SDL_FRect` via `SDL_RenderFillRect`.
- `javax/microedition/lcdui/Graphics.drawStringNative(Ljava/lang/String;II)V` — sets the color
  then draws text via `SDL_RenderDebugText` (the public `Graphics.drawString(...)` computes the
  anchor offsets in Java and delegates here).

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
- **Green threads / stackless interpreter (in progress).** The interpreter no longer recurses in
  C++: `Interpreter::run(Thread&, num_instructions)` executes a bounded budget against a
  `Thread`'s explicit `vector<Frame>` stack, and `invoke`/returns push/pop frames. This is the
  groundwork for cooperative green threads, but **there is no scheduler yet** — the `VM` owns a
  single `main_thread_` and `VM::run` drains it with a `while (!is_terminated())` loop, so the
  500-instruction quantum is currently moot. No `Thread` id/state enum, no ready/blocked queues,
  no yield points for blocking natives, and no `java/lang/Thread` binding. The old recursive
  `Interpreter::execute` entry point is gone (its body remains only as a commented block).
- **Lazy stackless class init has sharp edges** (mechanism in [Class initialization](#class-initialization)):
  because `<clinit>` now runs asynchronously in the dispatch loop, the `InitState::Failed`
  transition is effectively unreachable — a throwing `<clinit>` surfaces as a `std::runtime_error`
  that kills the thread and leaves the class stuck in `Initializing`. Interfaces are not
  initialized (only the `super_` chain is walked). `VM::run`'s bootstrap dereferences
  `find_method("<init>"/"startApp", "()V")` without a null check, and `ireturn`/`areturn` from
  the outermost frame would call `current_frame()` on an empty stack (currently unreachable
  because outermost frames are void-returning).
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
  `native_methods.cpp`; the old recursive `Interpreter::execute` (interpreter.cpp) and the old
  synchronous `VM::initialize_class` (vm.cpp) survive only as commented-out blocks;
  `Class::resolve_constant` has an unreachable fallback block after its
  `std::visit` return; several `TODO(Kostu)` markers persist (empty `Font.init` body, the
  `overloaded` visitor "move this to utils", the commented-out `Graphics` translation offsets,
  and a "set color once" note shared by `fillRect`/`drawStringNative`).
- **Many `.java` runtime methods are declared `native` but have no C++ binding** and would
  throw "Failed to call native" if invoked: `java/lang/System.{arraycopy,identityHashCode,
  getProperty,exit,gc}`, `java/lang/Class.{forName,newInstance,isInstance,isAssignableFrom,
  isInterface,isArray}`, and `java/lang/String.{replace,substring,init}`. Conversely,
  `String.indexOf(II)I` is registered in `NativeMethods` but `java/lang/String.java` declares no
  such method, so that binding never attaches.
- **The CMake javac step only compiles the thirteen listed sources.** `java_classes/java/lang/
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
