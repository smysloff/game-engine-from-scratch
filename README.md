# Game Engine From Scratch

A cross-platform game engine implementation with minimized external dependencies. Built from fundamental components to explore the complete graphics software stack, from memory management to rendering pipelines.

## 🎯 Project Direction

Long-term goal: Implementation of a complete game engine with:
- **Custom core systems** — Memory management, mathematics, containers
- **Direct platform APIs** — Native system interfaces without heavy abstraction layers
- **Software rendering pipeline** — CPU-based graphics from first principles

## 📍 Current Status

**Early prototype phase** — Foundation work in progress

### What Exists Right Now:

#### Core Foundation (`src/core/`)
- **Basic type definitions** — Platform-independent types and constants
- **Initial container prototypes** — Early array and buffer implementations
- **Utility functions** — Basic operations and helpers

#### Experimental Code (`examples/`)
- **Platform exploration** — Testing XCB and Win32 APIs
- **Graphics experiments** — Early rendering algorithm tests
- **Architecture prototypes** — Proof-of-concept implementations

### What's Being Worked On:
1. **Memory allocator system** — Arena, pool, and stack allocators
2. **Mathematical library** — Vectors, matrices, transformation functions  
3. **Platform abstraction layer** — Unified API for X11 and Windows
4. **Software rasterizer** — Basic triangle rendering algorithms

## 🏗️ Project Structure (Planned)

```bash
src/ (under construction)
├── core/ # Foundation systems (in design)
│ ├── memory/ # Custom allocators (planned)
│ ├── math/ # Mathematical library (planned)
│ └── containers/ # Data structures (early prototypes)
│
├── platform/ # Platform layer (in exploration)
│ ├── xcb/ # Linux/X11 interface (experimental)
│ └── win32/ # Windows interface (experimental)
│
├── graphics/ # Rendering (research phase)
│ └── software/ # CPU renderer (algorithm testing)
│
└── examples/ # Development sandbox
├── xcb_test/ # X11/XCB experimentation
└── render_test/# Rendering algorithm tests
```


## 🔬 Development Approach

The project follows an experimental, bottom-up approach:

1. **Research and prototyping** in `examples/` directory
2. **Algorithm validation** through isolated test programs
3. **Gradual integration** into engine architecture
4. **Iterative refinement** based on implementation experience

### Current Experiments:
- XCB window creation and event handling
- Win32 GDI graphics operations  
- Basic line and triangle rasterization algorithms
- Memory allocation patterns and performance

## 🚧 Getting Started (For Developers)

### Prerequisites
- **Linux**: GCC/Clang, XCB development libraries
- **Windows**: MinGW-w64 or Visual Studio
- **Basic C development environment**

### Building Examples
```bash
# Clone repository
git clone https://github.com/smysloff/game-engine-from-scratch.git
cd game-engine-from-scratch

# List of examples
cd examples
make

# Build specific examples
cd examples
make xcb/window_management
```

## 📝 License

GPL-3.0 — See [LICENSE](LICENSE) for details.
