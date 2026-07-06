# CoreX

CoreX is a kernel-agnostic, AOT-compiled systems programming language. Source files use the `.cx` extension. The compiler is implemented in C++17 using only the standard library.

## Installing CoreX

CoreX is distributed as a single toolchain binary, similar to how `rustup` installs `rustc` or how Go installs the `go` tool. There are two steps: install the toolchain once, then use it to manage everything else.

### Step 1: Install the toolchain

**Linux / macOS**

```
curl -fsSL https://raw.githubusercontent.com/PPeter2/CoreX/main/install/install.sh | sh
```

**Windows (PowerShell)**

```
irm https://raw.githubusercontent.com/PPeter2/CoreX/main/install/install.ps1 | iex
```

This downloads the prebuilt `corex` binary for your platform, places it in `~/.corex/bin` (or `%USERPROFILE%\.corex\bin` on Windows), and adds it to your PATH. Restart your terminal afterward.

### Step 2: Verify

```
corex version
```

### Step 3: Get the standard library

```
corex install std
```

## Using the CLI

```
corex build <file.cx>     compile a CoreX source file
corex run <file.cx>       compile and run a CoreX source file
corex tokens <file.cx>    print the lexer token stream for a file
corex install <package>   install a CoreX package
corex version             print the installed corex version
corex help                show this message
```

`build`, `run`, and `install` are stubs right now since the compiler currently only implements the lexer. They will fill in as the parser, type checker, and code generator are built.

## Building from source

If you are developing CoreX itself, clone the repository and build directly:

```
make
make run
```

`make run` runs `corex tokens` against the test file in `tests/`.

## Folder Structure

```
corex/
├── Makefile
├── README.md
├── .gitignore
├── install/
│   ├── install.sh
│   └── install.ps1
├── .github/workflows/
│   └── release.yml
├── include/corex/
│   ├── lexer/
│   │   ├── Token.h
│   │   ├── TokenType.h
│   │   └── Lexer.h
│   ├── ast/
│   ├── parser/
│   ├── sema/
│   ├── codegen/
│   └── driver/
│       └── Cli.h
├── src/
│   ├── lexer/
│   │   ├── TokenType.cpp
│   │   └── Lexer.cpp
│   ├── ast/
│   ├── parser/
│   ├── sema/
│   ├── codegen/
│   └── driver/
│       ├── Cli.cpp
│       └── main.cpp
├── tests/
│   └── lexer_full_feature.cx
└── examples/
```

Each compiler stage gets its own header directory under `include/corex/` and its own implementation directory under `src/`. As we proceed through the phases, `parser/`, `ast/`, `sema/`, and `codegen/` will fill in following the same pattern.

## Releases

Pushing a tag matching `v*.*.*` triggers `.github/workflows/release.yml`, which builds the toolchain for Windows x64, Linux x64, macOS x64, and macOS arm64, and publishes them as a GitHub Release. The install scripts above always fetch the latest release.

## Phase Status

- [x] Phase 1: Lexer
- [ ] Phase 2: Parser
- [ ] Phase 3: AST
- [ ] Phase 4: Type Checker / Semantic Analysis
- [ ] Phase 5: Code Generation
- [x] CLI scaffold (`build`, `run`, `tokens`, `install`, `version`, `help`)
- [x] Cross-platform release pipeline (GitHub Actions)
- [x] Install scripts (Windows / Linux / macOS)
