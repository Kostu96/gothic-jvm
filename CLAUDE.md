# gothic-jvm — Agent Reference (CLAUDE.md)

> **Audience — agents only.** This file is the technical single-source-of-truth for AI
> coding agents. The human-facing overview and build/run walkthrough live in
> [README.md](README.md). Keep onboarding prose there; keep precise, terse technical facts
> (types, signatures, opcode tables, invariants, gaps) here.
>
> **Maintenance note:** Keep this file current. After any major change (new opcodes, runtime
> components, native callbacks, build/test layout, API renames), update the relevant sections
> so agents don't have to re-read every file. If the change is user-visible, update README.md too.

## What this is

A toy/educational **Java Virtual Machine** in modern **C++23**. It parses `.class` files,
builds a runtime model of classes, and interprets bytecode. Only a subset of opcodes and a
handful of native methods are implemented. The default workload is a J2ME-style MIDlet
(`gothic3thebeginning/HG`).

Standard/runtime library classes (`java.lang.Object`, `java.lang.String`, `javax.microedition.*`,
…) are **real `.class` files loaded off the classpath**. Methods marked `ACC_NATIVE` in those
class files are bound to C++ callbacks (see [Native callbacks](#native-callbacks)).

- **Language:** C++23 (`cxx_std_23`)
- **Build:** CMake (min 3.28), MSVC/Visual Studio solution generated under `build/`
- **Windowing/rendering:** SDL3 (window + 2D renderer + event loop), via CMake `FetchContent`
  (`GIT_TAG release-3.4.12`, shared lib; DLL copied next to `app`/`app_tests` on Windows).
  Wrapped by `Display` (see [Windowing](#windowing)).
- **Image decoding:** stb_image (`third_party/stb`, vendored header + `stb_impl.cpp` in
  `app_lib`) — decodes PNG resources for the `Image.createImage` native.
- **Tests:** GoogleTest (`third_party/gtest`, submodule), via `gtest_discover_tests`
- **Executable:** `app` (entry `source/main.cpp`); core logic in static lib `app_lib`

## Build & test

```
cmake -S . -B build
cmake --build build
ctest --test-dir build        # or run the app_tests target
```

`CMakeLists.txt` fetches/builds **SDL3** (before defining `app_lib`), links it into `app_lib`
as `SDL3::SDL3` (PUBLIC, so `app` and `app_tests` inherit it), and on Windows copies `SDL3.dll`
next to both binaries (POST_BUILD). It also `add_subdirectory(java_classes)`, which uses
`find_package(Java)` + a `javac` custom command (`-source 1.3 -target 1.1`) to compile the
runtime `.java` sources into `build/java_classes/` (target `java_classes`, on which `app`
depends). The only test, `tests/test_binary_reader.cpp`, builds its inputs in memory.

### Runtime data / classpath

The classpath is assembled from the **current working directory**, not `resources/`:

1. `VM` ctor adds `<cwd>/java_classes` only (no eager class load; loading is deferred to `run`).
2. `main.cpp` adds either the CLI-provided classpath entries, or (no extra args)
   `<cwd>/gothic3thebeginning`.

On-disk assets:
- `java_classes/` (repo root) — runtime bootstrap classes kept as **`.java` sources**, compiled
  by CMake. The 39 explicitly listed sources: `com/kostu96/gjvm/ResourceInputStream`,
  `com/nokia/mid/ui/FullCanvas`, `java/io/{ByteArrayInputStream,IOException}`,
  `java/lang/{Class,ClassNotFoundException,Exception,Integer,NullPointerException,Object,Runnable,Runtime,RuntimeException,String,StringBuffer,System,Thread,Throwable}`,
  `javax/microedition/lcdui/{Canvas,Display,Command,CommandListener,Displayable,Font,Graphics,Image}`,
  `javax/microedition/media/{Control,Controllable,MediaException,Player,PlayerListener}`,
  `javax/microedition/midlet/{MIDlet,MIDletStateChangeException}`,
  `javax/microedition/rms/{InvalidRecordIDException,RecordStore,RecordStoreException,RecordStoreFullException,RecordStoreNotFoundException,RecordStoreNotOpenException}`.
  (`ByteArrayInputStream`, `NullPointerException`, and `RuntimeException` are the newest additions;
  `NullPointerException` backs the VM-synthesized NPE on a null virtual-call receiver.)
- `resources/classes/` — vendored J2ME/MIDP/CLDC class library (real SDK `.class` files:
  `java.lang.*`, `java.io.*`, `java.util.*`, `javax.microedition.*`, `com.sun.*`). Not on the
  default runtime classpath; it is the source pool for the extra dependency classes that must
  sit in `build/java_classes/` at runtime (see gaps).
- `resources/gothic3thebeginning/` — MIDlet data: `HG.class` (default main class), obfuscated
  single-letter classes (`a`, `a.class`, `b.class`, `c`, `d`, …), `.mid`/`.mdl`/`.lng` data,
  `icon.png`/`s00.png`/`s01.png`, `META-INF/`.
- `build/java_classes/` — javac output for `<cwd>/java_classes` lookups when running from
  `build/`. Besides the compiled sources it also holds vendored dependency classes that the javac
  step does **not** produce: the rest of `java/io/*` (e.g. `InputStream`, the superclass of the
  now-compiled `ByteArrayInputStream`) and `java/util/*`. (`javax/microedition/media/*` and
  `java/io/{IOException,ByteArrayInputStream}` are now compiled from `java_classes/`.)
  `build/gothic3thebeginning/` mirrors the MIDlet data.

## Directory layout

```
source/
  main.cpp                      # entry: open Display; VM vm; add classpath; run vm.run on a thread; window loop
  class_loader/
    class_file.{hpp,cpp}        # ClassFile: raw parse of constant pool, fields, methods, Code attr, MUTF-8 decode
    constant_pool_entry.hpp     # ConstantPoolEntry variant (Utf8/Integer/Long/Class/String/refs/NameAndType)
    class_loader.{hpp,cpp}      # ClassLoader: classpath resolve + cache; array classes via load_array
  platform/
    display.{hpp,cpp}           # Display: owns SDL_Window + SDL_Renderer; process_events/clear/present; width/height
    record_store.{hpp,cpp}      # RecordStore: named in-memory record list (add_record/size) backing MIDP RMS
    record_store_manager.{hpp,cpp} # RecordStoreManager: name -> RecordStore map; open_record_store(name, create)
  runtime/
    vm.{hpp,cpp}                # VM: owns NativeMethods/ClassLoader/Heap/Interpreter + threads_; bootstrap state machine
    thread.{hpp,cpp}            # Thread: a green thread = an explicit call stack (vector<Frame>)
    class.{hpp,cpp}             # Class/Method/Field; CP resolution; ensure_initialized; binds ACC_NATIVE callbacks
    native_methods.{hpp,cpp}    # NativeMethods: "<class>.<name><descriptor>" -> C++ callback registry
    interpreter.{hpp,cpp}       # budgeted dispatch loop: run(Thread&, num_instructions) / invoke(Thread&, Method)
    frame.{hpp,cpp}             # Frame: locals, operand stack, pc, code readers; movable
    heap.{hpp,cpp}              # Heap: instances/arrays + interned Strings + canonical Class mirrors
    object.{hpp,cpp}            # Object = variant<InstanceData, PrimitiveArrayData, InstanceArrayData, ClassMirrorData> + Monitor
    opcodes.hpp                 # op_* bytecode opcode constants
    value.hpp                   # Value = variant<monostate,int32,int64,float,double,Object*>
    runtime_constant_pool_entry.hpp  # resolved CP cache types (Class/String/Field/Method/InterfaceMethod)
  utils/
    binary_reader.{hpp,cpp}     # big-endian u8/u16/u32, string/bytes, bounds-checked
tests/                          # CMakeLists.txt, test_binary_reader.cpp
resources/
  classes/                      # vendored J2ME/MIDP/CLDC standard library (.class)
  gothic3thebeginning/          # MIDlet data
java_classes/                   # runtime bootstrap classes (.java, compiled by CMake)
third_party/gtest/              # vendored GoogleTest (submodule)
third_party/stb/                # vendored stb_image (stb_image.h + stb_impl.cpp) for PNG decode
build/                          # generated VS solution/projects + <cwd> copies of java_classes/gothic3thebeginning
```

## Architecture & flow

> **Execution model: green threads on an explicit frame stack.** The interpreter is *stackless
> and budgeted*: each `Thread` owns its own call stack (`std::vector<Frame>`), and
> `Interpreter::run` executes a bounded number of instructions against the top frame and returns.
> `VM::run` round-robins **every** live thread in `threads_`, giving each a 500-instruction quantum
> per loop iteration — a rudimentary cooperative scheduler. `Thread.start()` spawns additional
> green threads (see [Native callbacks](#native-callbacks)); there is still no yielding, blocking,
> priorities, or thread-state model.

1. `main.cpp` (`args <main_class> [<class_path_entry>...]`) opens the SDL3 `Display` (240x320
   logical, scale 2 -> 480x640 window + renderer), default-constructs a `VM`, calls
   `set_main_class` and `set_display`, and configures the classpath (default main class `HG`;
   no extra args -> classpath `<cwd>/gothic3thebeginning`, else each extra arg is an entry). It
   runs `vm.run()` on a **background `std::thread`** (SDL must own the thread that created the
   window/pumps events, and a MIDlet's `startApp()` may never return) and spins the window
   event/render loop on the main thread (`process_events` -> `clear` -> `present`). On window
   close it calls `vm.request_stop()` and joins the JVM thread, which swallows `VmStopRequested`
   and prints other exceptions to stderr.
2. `VM` ctor wires its owned `NativeMethods` into the `ClassLoader` and adds `<cwd>/java_classes`.
   It does **not** eagerly load or initialize any class.
3. `VM::run()` is a **bootstrap state machine** driving a `while (true)` loop. It emplaces one
   `Thread` into `threads_`, loads `java/lang/String` (registered with the heap via
   `Heap::set_string_class` so literals can be interned) and the main class, then each iteration
   runs *every* non-terminated thread for 500 instructions (`interpreter_.run(thread, 500)`) while
   advancing phases on the main thread, each guarded by `Thread::is_terminated()` (frame stack
   drained empty): `Boot` inits `String`; `Phase1` inits the main class; `Phase2` `new_instance`s
   the MIDlet and pushes its `<init>()V`; `Phase3` pushes `startApp()V`; `Phase4` is terminal (its
   `return` is commented out, so the loop keeps spinning to service any threads spawned via
   `Thread.start`). Class init and lifecycle entries are scheduled as pushed frames, not
   synchronous C++ calls.
4. `Interpreter` holds a `VM&`. `run(Thread&, num_instructions)` executes up to that many opcodes
   against `thread.current_frame()`, checking `vm.stop_requested()` each step and stopping early
   if the thread terminates. Method calls do **not** recurse in C++: `invoke(Thread&, const
   Method&)` runs a native (`Method::native_callback` with `thread.current_frame()`), or for
   bytecode pops `arg_slot_widths.size()` operand entries and `Thread::push_frame`s a new `Frame`.
   `invokevirtual`/`invokeinterface` route through `virtual_dispatch`, which peeks the receiver,
   walks its superclass chain via `find_method`, and calls `invoke`. Returns
   (`ireturn`/`areturn`/`return`) `pop_frame` and push any result onto the caller's stack. After
   each instruction it dispatches any pending exception (see
   [Exception handling](#exception-handling)).
5. Class init is stackless and lazy (see [Class initialization](#class-initialization)).
6. `ClassLoader::load` checks a cache, routes `[`-prefixed names to `load_array`, otherwise
   resolves `binary/name` -> `<entry>/binary/name.class` across classpath entries and constructs
   a `Class` (which recursively loads its super + interfaces).

### Key types
- **Value** (`value.hpp`): JVM slot = `variant<monostate, int32_t, int64_t, float, double, Object*>`.
  `Object*` is the reference type (`nullptr` == null); doubles/longs occupy a single slot.
- **Object** (`object.hpp`): tagged union `variant<InstanceData, PrimitiveArrayData,
  InstanceArrayData, ClassMirrorData>` in `.data`, plus a `Monitor{Thread* owner; uint32_t
  recursion_count}` (used by `monitorenter`/`monitorexit`).
  - `InstanceData`: `Class& type` + `vector<Value> fields` (by field slot) + `native_payload`
    (`NativePayload = variant<monostate, ResourceInputStreamNativeData, StringNativeData,
    ImageNativeData, GraphicsNativeData, RecordStoreNativeData>`). `String`/`StringBuffer` carry
    `StringNativeData{value}` (raw `std::string`), `ResourceInputStream` carries `{buffer,
    position}`, `Image` carries an `SDL_Surface*`, `Graphics` carries an `SDL_Renderer*` +
    `SDL_Surface*`, `RecordStore` carries a non-owning `RecordStore*` (into the
    `RecordStoreManager`).
  - `PrimitiveArrayData`: variant of typed element vectors (boolean/byte→uint8, char→char16,
    short→int16, int→int32, long→int64, float, double) with `ElementType` tags matching JVM
    `newarray` atype codes (4..11); `get`/`set` widen to `Value`.
  - `InstanceArrayData`: `Class& element_type` + `vector<Object*>`.
  - `ClassMirrorData`: the mirrored `Class&` (the `java/lang/Class` object identity).
- **Class** (`Kind`: `Ordinary`, `Array`, `Primitive`): built from a parsed `ClassFile` via
  `Class(const char*, ClassLoader&)`, or synthesized via `Class(std::string, Class* component
  = nullptr)` — non-null component -> `Array`, null -> `Primitive`; both start `Initialized`.
  Holds `super_`/`interfaces_`, `treat_super_specially_` (`ACC_SUPER`), `is_interface_`
  (`ACC_INTERFACE`), static + instance `Field`s (instance fields get sequential `slot`s
  continuing the super's `instance_field_count_`), `Method`s, a lazily-resolved
  `runtime_constant_pool_`, and (arrays only) `component_type_`. API: `find_method`/`find_field`
  (this class only), `resolve_class/field/method` (cached CP resolution; field/method resolution
  walk the full superclass + superinterface hierarchy), `resolve_interface_method` (cached; walks
  the target interface's superinterface hierarchy, backing `invokeinterface`), `resolve_constant(index, class_loader,
  heap)` (ints/longs re-read from the `ClassFile`; `String`s interned via
  `Heap::new_interned_string`; `Class` constants -> heap mirror). Predicates/accessors: `kind()`,
  `is_interface()`, `is_subclass_of(other)` (walks the `super_` chain; used by exception handler
  matching), `treat_super_specially()`, `component_type()`, `this_name()`,
  `super()`/`super_name()`, `instance_field_count()`. Init API: `needs_initialization()` (true
  only when `Loaded`), `ensure_initialized(Thread&)`, `set_initialized()`.
- **Method**: `owner`, name, descriptor, `arg_slot_widths` (per-argument local-slot widths incl.
  leading `this`; long/double = 2, else 1; computed at parse time — its `.size()` is the operand
  count `invoke` pops), `max_stack`/`max_locals`, `is_static`, `is_native`,
  `is_class_initializer` (`<clinit>`/`()V`/static), `code` span, `exception_table` span,
  `native_callback` (`const NativeMethods::Callback*`; `nullptr` when not native or unbound).
- **Field**: `owner`, `is_static`, name, descriptor, `Value value` (static storage), `slot`
  (instance index).
- **Heap**: owns all `Object`s via `unique_ptr`; `new_instance`, `new_primitive_array`,
  `new_instance_array`, `new_interned_string(str)` (see [Strings](#string-modeling); dedupes via
  `string_objects_`), `class_object_for(Class&)` (lazily creates/caches one canonical
  `java/lang/Class` mirror per `Class`). Requires the String class registered via
  `set_string_class` before any string is interned.
- **Frame** (movable): `owner()`/`method()`, `locals()`, `operand_stack()`, `pc()`/`set_pc(pc)`
  (jumps use `branch(offset)`; `set_pc` also used to jump to an exception `handler_pc`),
  `record_last_pc()`/`last_pc()` (the interpreter snapshots the current pc before each instruction;
  used by lazy class init and exception dispatch to locate the faulting instruction) and
  `rewind_pc()` (rewind pc to `last_pc_`, i.e. the start of the current instruction, for lazy class
  init), `pop_code_u8/u16/i32` (the `i32` reader backs `tableswitch`/`lookupswitch`),
  `push_stack`/`pop_stack`/`peek_stack(index = 0)` (depth-indexed from
  top). Ctor sizes `locals` to `max_locals`, reserves `max_stack`.
- **Thread** (`thread.{hpp,cpp}`): a green thread = explicit JVM call stack. Owns
  `std::vector<Frame> frames_`; `current_frame()`, `push_frame(method, span<const Value> args)`
  (lays `args` into locals via `arg_slot_widths`), `pop_frame()`, `is_terminated()`
  (`frames_.empty()`). Also carries a single **pending exception** slot (`Object*`):
  `set_pending_exception`/`pending_exception`/`has_pending_exception`/`clear_pending_exception`
  (see [Exception handling](#exception-handling)). Because `Frame`s live in a `vector`,
  `push_frame` can reallocate and invalidate a held `Frame&` — the interpreter re-fetches
  `current_frame()` each instruction, and init-triggering opcodes rewind pc *before* calling
  `ensure_initialized`. The `VM` owns `threads_` (a `vector<unique_ptr<Thread>>`) and round-robins
  all live threads 500 instructions at a time; `Thread.start` appends new threads via
  `VM::create_thread` (which `emplace_back`s into `threads_` — a latent reallocation hazard while
  `VM::run` iterates that vector).
- **Runtime constant pool** (`runtime_constant_pool_entry.hpp`): `RuntimeClassInfo`,
  `RuntimeFieldRefInfo`, `RuntimeMethodRefInfo`, `RuntimeStringInfo` cache the resolved pointer on
  first resolution. `RuntimeIntegerInfo`/`RuntimeLongInfo` are tags only (value re-read from the
  `ClassFile`). `RuntimeInterfaceMethodRefInfo` caches the resolved interface `Method*`, backing
  `invokeinterface`.
- **Display** (`platform/display.{hpp,cpp}`): wraps SDL3. Ctor `Display(title, width, height,
  scale)` -> `SDL_Init(SDL_INIT_VIDEO)` + `SDL_CreateWindowAndRenderer` (window `width*scale` x
  `height*scale`, vsync on); dtor tears it down. `width()`/`height()` return the **logical**
  size. `process_events()` returns `false` on quit/close; `clear(r,g,b)`/`present()` drive the 2D
  renderer; `renderer()` exposes it. Non-owned by `VM` (a `Display*` via `set_display`), so the
  VM can run headless in tests.
- **RecordStore / RecordStoreManager** (`platform/record_store.{hpp,cpp}`,
  `record_store_manager.{hpp,cpp}`): the MIDP RMS backend, owned by `VM`
  (`record_store_manager()`). `RecordStore(name)` holds a `vector<Record>` (each `Record` = a
  `vector<uint8_t>`); `add_record(span<const uint8_t>)` appends and returns the new 1-based record
  id, `set_record(record_id, span<const uint8_t>)` overwrites an existing record, `size()` the
  count. `RecordStoreManager` maps `name -> RecordStore` (`unordered_map`);
  `open_record_store(name, create_if_necessary)` returns the existing store, creates one when
  missing and `create_if_necessary`, or `nullptr` otherwise. All in-memory; nothing is persisted.

### Windowing
`main.cpp` constructs one stack `Display` (240x320 logical, scale 2 -> 480x640 window, title
`gothic-jvm`), wires it in with `set_display`, runs `vm.run` on a background `std::thread`, and
spins the window event/render loop on the **main thread** (`while (process_events()) { clear(0,0,0);
present(); }`). SDL owns the main thread; the JVM runs concurrently and reads `width_`/`height_`
(set once in the ctor) race-free. On close, `main` calls `vm.request_stop()` (an atomic the
interpreter loop checks each instruction, throwing `VmStopRequested` to unwind a MIDlet stuck in
`startApp()`) and joins the JVM thread. Native callbacks reach the screen through `vm.display()`
(Canvas size) and the MIDP `Graphics`/`Image` natives, which do **real** 2D drawing — but onto
**offscreen** SDL surfaces, not the on-screen window: `Image.init` allocates an `SDL_Surface`
(ARGB8888); `Graphics.init` wraps a surface in an `SDL_CreateSoftwareRenderer`;
`Graphics.fillRect`/`drawStringNative` render into it; `Image.getRGB` reads pixels back. The
window's own renderer is only cleared to black and presented each frame — nothing blits those
offscreen surfaces to the window. `Display.java` models `getDisplay`/`setCurrent`, but
`setCurrent` is a no-op and there is no `paint`/`repaint`/`serviceRepaints` dispatch model, so
full MIDlet execution is not wired up.

### String modeling
`java/lang/String` has a single `size:I` field plus a `StringNativeData` payload holding the raw
`std::string`. `Heap::new_interned_string(str)` allocates a String instance, sets `size`, stashes
the payload directly (no `<init>`), and dedupes via the heap's `string_objects_` map.
`Class::resolve_constant` uses this to intern `String` constants (`ldc` of a string yields a live
object) and caches the result (`RuntimeStringInfo.resolved`). Java-level constructors
`String(char[], offset, size)` and `String(StringBuffer)` route to native `init` methods that
fill the payload; `StringBuffer` is likewise backed by `StringNativeData`, and `Integer.toString`
builds a `char[]` then a `String`. The String class must be registered with the heap
(`Heap::set_string_class`, done in `VM::run`'s bootstrap) before any string is interned.

### Class initialization
Class init is **stackless and lazy**, driven off the running `Thread`'s frame stack:
- `needs_initialization()` is true only in `InitState::Loaded`. `Initializing`/`Initialized` both
  mean "proceed"; `Failed` throws.
- The init-triggering opcodes (`getstatic`, `putstatic`, `invokestatic`, `new`) check it and, when
  true, `frame.rewind_pc()` to rewind to the start of the current instruction, call
  `ensure_initialized(thread)`, and fall through. The instruction re-executes after init
  completes; checking before any effect keeps the retry idempotent.
- `ensure_initialized` sets `Initializing`, pushes the `<clinit>()V` frame (or flips straight to
  `Initialized` when there is none), then recurses into `super_` — pushing *this* class's clinit
  **before** the super's so the super runs first (super-class-first).
- When a `<clinit>` frame's `return` executes, `op_return` sees `Method::is_class_initializer` and
  calls `set_initialized()`.
- Caveats: `Failed` is effectively unreachable (a throwing `<clinit>` becomes a
  `std::runtime_error` that kills the thread and leaves the class stuck in `Initializing`);
  interfaces are not walked (only `super_`); no cross-thread init locking.

### Implemented opcodes (dispatched in `interpreter.cpp`)
Constants: `nop` (0x00), `aconst_null` (0x01), `iconst_m1..5` (0x02–0x08), `lconst_0/1`
(0x09–0x0A), `fconst_0/1/2` (0x0B–0x0D), `dconst_0/1` (0x0E–0x0F), `bipush` (0x10), `sipush`
(0x11), `ldc` (0x12), `ldc_w` (0x13), `ldc2_w` (0x14).
Loads: `iload` (0x15), `aload` (0x19), `iload_0..3` (0x1A–0x1D), `lload_0..3` (0x1E–0x21),
`aload_0..3` (0x2A–0x2D), `iaload` (0x2E), `aaload` (0x32), `baload` (0x33), `caload` (0x34),
`saload` (0x35).
Stores: `istore` (0x36), `lstore` (0x37), `astore` (0x3A), `istore_0..3` (0x3B–0x3E), `lstore_0..3`
(0x3F–0x42), `astore_0..3` (0x4B–0x4E), `iastore` (0x4F), `aastore` (0x53, partial
`ArrayStoreException`-style check), `bastore` (0x54), `castore` (0x55), `sastore` (0x56).
Stack/math: `pop` (0x57), `pop2` (0x58; pops one slot for a category-2 long/double top, else two),
`dup` (0x59), `dup_x1` (0x5A; TODO
comment about arbitrary-position insert), `dup2` (0x5C; both the category-2 long/double top and the
two-slot category-1 forms), `iadd` (0x60), `ladd` (0x61), `isub` (0x64), `lsub` (0x65), `imul`
(0x68), `idiv` (0x6C), `irem` (0x70), `ineg` (0x74), `ishl` (0x78), `ishr` (0x7A), `iushr` (0x7C),
`iand` (0x7E), `land` (0x7F), `ior` (0x80), `ixor` (0x82), `lxor` (0x83), `iinc` (0x84), `i2l`
(0x85), `i2b` (0x91), `i2c` (0x92), `i2s` (0x93), `lcmp` (0x94).
Branches: `ifeq` (0x99), `ifne` (0x9A), `iflt` (0x9B), `ifge` (0x9C), `ifgt` (0x9D), `ifle` (0x9E),
`if_icmpeq` (0x9F), `if_icmpne` (0xA0), `if_icmplt` (0xA1), `if_icmpge` (0xA2), `if_icmpgt` (0xA3),
`if_icmple` (0xA4), `goto` (0xA7), `ifnull` (0xC6), `ifnonnull` (0xC7).
Switches: `tableswitch` (0xAA), `lookupswitch` (0xAB) — both skip the 4-byte-alignment padding,
read their table off the code stream, and jump relative to `last_pc()`.
Returns: `ireturn` (0xAC), `lreturn` (0xAD), `areturn` (0xB0), `return` (0xB1). Returns are
stackless: `pop_frame` then (for `ireturn`/`lreturn`/`areturn`) push the result onto the caller's
stack; a `return` from a `<clinit>` frame marks its owner class `Initialized`. Returning from the
outermost frame empties the thread's stack, terminating it.
Fields/calls: `getstatic` (0xB2), `putstatic` (0xB3), `getfield` (0xB4), `putfield` (0xB5),
`invokevirtual` (0xB6) and `invokeinterface` (0xB9), both routed through the shared
`virtual_dispatch` (walks `find_method` up the receiver's superclass chain; a null receiver raises a
VM-synthesized `java/lang/NullPointerException` via the pending-exception path; a `ClassMirrorData`
receiver dispatches against `java/lang/Class`; `invokeinterface` first resolves the interface
method via `resolve_interface_method` and skips its `count`/reserved operand bytes),
`invokespecial` (0xB7, resolved via `resolve_method`; throws for the not-yet-implemented
`ACC_SUPER` super-invoke), `invokestatic` (0xB8). `getstatic`/`putstatic`/`invokestatic`/`new`
guard on `needs_initialization()` and, when true, rewind pc + `ensure_initialized` instead of
acting; the `invoke*` opcodes push a frame via the shared `Interpreter::invoke`.
Object/array/monitor: `new` (0xBB), `newarray` (0xBC), `anewarray` (0xBD), `arraylength` (0xBE),
`athrow` (0xBF, sets the thread's pending exception — see [Exception handling](#exception-handling)),
`checkcast` (0xC0, resolves the class but performs no type check), `monitorenter` (0xC2)/
`monitorexit` (0xC3) (a simple non-blocking monitor: owner `Thread*` + recursion count; throws if
held by another thread or on null), `multianewarray` (0xC5, recursive allocation via a
self-recursive lambda).

Notes:
- `op_lload` (0x16) is declared in `opcodes.hpp` but has no dispatch case.
- `ldc`/`ldc_w`/`ldc2_w` resolve `Integer`/`Long`/`String`/`Class` constants through the runtime
  constant pool; a `String` constant interns a live `java/lang/String` and a `Class` constant
  pushes the heap `java/lang/Class` mirror.
- The interpreter traces class loads (`ClassLoader`), frame pushes (`Thread`), and native calls
  (`invoke`) to stdout; it does **not** print a per-opcode/operand-stack trace.
- Unknown opcodes throw `std::runtime_error`. VM-internal errors a real VM would surface as Java
  exceptions (NPE, div-by-zero, ArrayStore, index OOB, missing field/method) are still thrown as
  `std::runtime_error`; only explicit `athrow` (and a few natives) route through the Java exception
  machinery below.

### Exception handling
A bytecode-visible exception path exists (thrown values, handler-table dispatch), but VM-internal
faults do **not** use it:
- `Thread` carries one **pending exception** slot (`Object*`). `athrow` pops a reference and calls
  `set_pending_exception` (null reference -> `std::runtime_error`); the interpreter itself raises a
  VM-synthesized `java/lang/NullPointerException` on a null `invokevirtual`/`invokeinterface`
  receiver; and some natives set it directly (e.g. `RecordStore.openRecordStore` raises
  `RecordStoreNotFoundException`; `ResourceInputStream.init` raises `java/io/IOException` when the
  resource fails to load).
- After **every** dispatched instruction `Interpreter::run` checks `thread.has_pending_exception()`
  and, if set, calls `dispatch_pending_exception(thread)`.
- `dispatch_pending_exception` walks the frame stack from the top: for the current frame it scans
  `method().exception_table` for an entry whose `[start_pc, end_pc)` covers `frame.last_pc()` and
  whose `catch_type` matches (`catch_type == 0` is catch-all/`finally`; otherwise the entry's
  class is resolved and matched via `exc_class.is_subclass_of(handler_type)`). On a match it clears
  the operand stack, pushes the exception, `set_pc(handler_pc)`, clears the pending slot, and
  returns. On no match it `pop_frame`s and continues outward.
- If the stack drains with no handler, it throws `std::runtime_error("Uncaught exception: <name>")`,
  which unwinds the C++ interpreter (killing the thread).
- Caveats: no exception *object* state is populated (message/stack trace are ignored; `Throwable`'s
  constructors are TODO stubs); superinterface handler types are matched only through `super_`
  (`is_subclass_of` walks the superclass chain, not interfaces); there is no `NoClassDefFoundError`
  or VM-synthesized `ArithmeticException` (only the null virtual-call receiver NPE above is
  synthesized).

### Native callbacks
Native bindings live in `NativeMethods` (`native_methods.{hpp,cpp}`). The `VM` owns one instance
and hands it to the `ClassLoader` (`native_methods()`). Its ctor fills an
`unordered_map<std::string, Callback>` keyed by `"<class>.<name><descriptor>"`; `find(class, name,
descriptor)` returns a `const Callback*` (or `nullptr`). When `Class` parses an `ACC_NATIVE`
method it stores that pointer in `Method::native_callback`. Callback functions are free functions
in an anonymous namespace, named after their fully-qualified Java method. Registered:
- `com/kostu96/gjvm/ResourceInputStream.init(Ljava/lang/String;)V` — loads the named resource off
  the classpath (`ClassLoader::load_resource`) into the instance payload; on failure sets a pending
  `java/io/IOException`. `.read()I` returns the next byte. Bound to private `native` methods called
  from a plain constructor.
- `java/lang/Class.forName(Ljava/lang/String;)Ljava/lang/Class;` — replaces `.` with `/`, loads
  the class, and pushes its heap `Class` mirror.
- `java/lang/Class.getName()Ljava/lang/String;` — interns a String of the mirrored name.
- `java/lang/Object.getClass()Ljava/lang/Class;` — canonical heap `Class` mirror (InstanceData
  receiver only).
- `java/lang/String.charAt(I)C`, `String.lastIndexOf(I)I`, `String.indexOf(II)I` — operate on the
  payload string. (`String.indexOf` is registered but `String.java` declares no such method, so
  that binding never attaches.)
- `java/lang/String.init([BII)V` / `String.init([CII)V` / `String.init(Ljava/lang/StringBuffer;)V`
  — fill the payload from a `byte[]` / `char[]` / `StringBuffer`.
- `java/lang/String.replace(CC)Ljava/lang/String;` / `String.toUpperCase()Ljava/lang/String;` /
  `String.substringNative(II)Ljava/lang/String;` — return a fresh interned String (Java-level
  `substring(...)` delegates to `substringNative`).
- `java/lang/StringBuffer.init()V` / `StringBuffer.append(Ljava/lang/String;)Ljava/lang/StringBuffer;`
  — initialize/append the payload string and update `size:I`.
- `java/lang/System.currentTimeMillis()J` — real wall-clock millis.
- `java/lang/System.arraycopy(Ljava/lang/Object;ILjava/lang/Object;II)V` — bounds-checked element
  copy between two **primitive** arrays.
- `java/lang/System.getProperty(Ljava/lang/String;)Ljava/lang/String;` — returns `null` for
  `microedition.locale` (the previous `"en-US"` return is commented out); throws for any other key.
- `java/lang/Thread.start()V` — spawns a new green thread via `VM::create_thread` and pushes the
  receiver's `run()V` frame onto it.
- `javax/microedition/lcdui/Canvas.getWidth()I` / `getHeight()I` — return the connected
  `Display`'s size (no null-display fallback; dereferences `vm.display()`).
- `javax/microedition/lcdui/Canvas.repaint(IIII)V` / `serviceRepaints()V` — no-op stubs (bodies
  commented out; no repaint dispatch model yet).
- `javax/microedition/lcdui/Font.init()V` — no-op stub.
- `javax/microedition/lcdui/Image.init(II)V` — allocates an offscreen `SDL_Surface` (ARGB8888);
  `Image.getWidth()I` / `Image.getHeight()I` return the surface's width/height;
  `Image.getRGB([IIIIIII)V` reads pixels back into an `int[]`;
  `Image.createImage(Ljava/io/InputStream;)Ljavax/microedition/lcdui/Image;` decodes a PNG from a
  `ResourceInputStream` buffer via stb_image (RGBA->ARGB) and wraps it in an `SDL_Surface`, and
  `Image.createImage([BII)Ljavax/microedition/lcdui/Image;` does the same from a `byte[]` slice.
- `javax/microedition/lcdui/Graphics.init(Ljavax/microedition/lcdui/Image;)V` — wraps the Image's
  surface in an `SDL_CreateSoftwareRenderer`.
- `javax/microedition/lcdui/Graphics.fillRect(IIII)V` / `drawStringNative(Ljava/lang/String;II)V`
  — set the draw color from the Graphics `color:I` field, then `SDL_RenderFillRect` /
  `SDL_RenderDebugText`. The public `Graphics.drawString(...)` computes anchor offsets in Java and
  delegates to `drawStringNative`.
- `javax/microedition/rms/RecordStore.openRecordStore(Ljava/lang/String;Z)Ljavax/microedition/rms/RecordStore;`
  (static) — creates a `RecordStore` instance whose payload points at the
  `RecordStoreManager`-owned store for that name; when the store is missing and `createIfNecessary`
  is false it sets a pending `RecordStoreNotFoundException` (see [Exception handling](#exception-handling)).
- `javax/microedition/rms/RecordStore.addRecord([BII)I` / `setRecord(I[BII)V` — append
  `data[offset, offset+size)` as a new 1-based record (returned) / overwrite an existing record id;
  `getNumRecords()I` returns the count.

## Conventions
- Headers `.hpp`, sources `.cpp`; include via paths rooted at `source/` (e.g.
  `#include "runtime/vm.hpp"`).
- `snake_case` members with trailing underscore (`class_loader_`); types `PascalCase`; opcode
  constants `op_snake_case`.
- Native callbacks named after their Java method (`java_lang_Object_getClass`, etc.).
- Copy deleted on owning types; raw pointers are non-owning, `unique_ptr` for ownership.
- Big-endian reads in `BinaryReader`; class file magic `0xCAFEBABE`; UTF-8 constants are
  (modified) UTF-8.

## Known gaps / WIP
- **Rudimentary scheduler, no real concurrency model.** The interpreter is stackless (runs an
  instruction budget against a thread's frame stack) and `VM::run` round-robins every live thread
  in `threads_` for a 500-instruction quantum each. `Thread.start` spawns green threads via
  `VM::create_thread`, but there are no `Thread` id/state enums, ready/blocked queues, yield
  points, priorities, or `java/lang/Thread` scheduling, and `monitorenter`/`monitorexit` never
  block. `VM::create_thread` `emplace_back`s into `threads_` while `VM::run` iterates it — a latent
  vector-reallocation hazard — and `Phase4`'s `return` is commented out, so `run()` spins forever
  (never returning on its own even after `startApp` unwinds).
- **Lazy class init caveats:** `InitState::Failed` is effectively unreachable; interfaces are not
  initialized (only the `super_` chain); no cross-thread init locking. An interface that carries a
  `<clinit>` is rejected at parse time (`Class` throws). `VM::run`'s bootstrap dereferences
  `find_method("<init>"/"startApp", "()V")` without a null check.
- **Type checks not enforced:** `checkcast` resolves its target but performs no check; there is no
  `instanceof`. `aastore` does a partial element-type check.
- **`invokespecial` `ACC_SUPER` unimplemented** (throws). `invokeinterface` *is* implemented
  (backed by `resolve_interface_method` + `RuntimeInterfaceMethodRefInfo`).
- **Constant pool coverage is partial:** `Float` (tag 4) and `Double` (tag 6) are not parsed and
  throw "Unsupported constant pool tag". `get_constant` only returns Integer/Long.
- **Exceptions are partial:** `athrow` and `Code` `exception_table` handler dispatch work (see
  [Exception handling](#exception-handling)), but exception *objects* carry no state (message/stack
  trace unset; `Throwable` ctors are stubs), handler matching walks only the superclass chain (not
  interfaces), and most VM-internal faults (div-by-zero, ArrayStore, index OOB, missing
  method/field) still throw `std::runtime_error` instead of Java exceptions — the one exception is a
  null `invokevirtual`/`invokeinterface` receiver, which raises a real `java/lang/NullPointerException`.
  No `NoSuchMethodError`/`AbstractMethodError` modeling.
- **No garbage collection;** the heap only grows.
- **Unbound `native` methods** (throw "Failed to call native" if invoked):
  `java/lang/System.{identityHashCode,exit}`, `java/lang/Class.isInterface`. The
  `java/lang/String.indexOf(II)I` binding is registered but `String.java` declares no matching
  method, so it is dead. (`System.gc` is a Java-level no-op stub, not native.)
- **The javac step compiles the 39 listed sources.** Remaining runtime dependency classes
  (the rest of `java/io/*` such as `InputStream`, and `java/util/*`) are vendored copies from
  `resources/classes/` that must sit in `build/java_classes/`; nothing in CMake copies them, so a
  clean build tree would be missing them.
- **Dead / TODO code:** `decode_modified_utf8` in `class_file.cpp` is defined but unused;
  `Class::resolve_constant` has an unreachable fallback block after its `std::visit` return; the
  `overloaded` visitor TODO, empty `Font.init` body, commented `Graphics` translation offsets, a
  shared "set color once" note in `fillRect`/`drawStringNative`, and a `Heap::new_interned_string`
  interning TODO persist.
- **Test coverage:** only `tests/test_binary_reader.cpp` (builds inputs in memory); no
  class-file/`Class` parsing or interpreter coverage.
