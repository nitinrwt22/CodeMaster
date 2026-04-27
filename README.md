
<div align="center">

[![header](https://capsule-render.vercel.app/api?type=waving&color=0:0f2027,50:203a43,100:2c5364&height=220&section=header&text=CodeMaster&fontSize=90&fontColor=ffffff&fontAlignY=42&desc=C%20Code%20Analyzer%20%26%20Symbol%20Browser&descFontSize=20&descFontColor=a0c4ff&descAlignY=68&animation=fadeIn)](https://github.com/nitinrwt22/codemaster)

### *Because Your Code Deserves More Than a Linter.*

</div>

**A high-performance, full-stack static analysis engine built in pure C, with a React visualization frontend.**  
Parse → Analyze → Visualize — in under 30ms.

---

![Language](https://img.shields.io/badge/Backend-C%20(GCC)-blue?style=for-the-badge&logo=c)
![Frontend](https://img.shields.io/badge/Frontend-React%2019%20%2B%20Vite-61DAFB?style=for-the-badge&logo=react)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)
![Platform](https://img.shields.io/badge/Platform-macOS%20%7C%20Linux-lightgrey?style=for-the-badge&logo=apple)

---

## 📖 Table of Contents

- [✨ Features](#-features)
- [🧱 Tech Stack](#-tech-stack)
- [🏗️ System Architecture](#%EF%B8%8F-system-architecture)
- [🔄 System Flowchart](#-system-flowchart)
- [💎 Why CodeMaster is Unique](#-why-codemaster-is-unique)
- [⚖️ Comparison with Existing Tools](#%EF%B8%8F-comparison-with-existing-tools)
- [🎯 Use Cases](#-use-cases)
- [🚀 Installation & Setup](#-installation--setup)
- [📁 Project Structure](#-project-structure)
- [📸 Screenshots](#-screenshots)
- [🔭 Future Improvements](#-future-improvements)
- [👨‍💻 Author](#-author)

---

## ✨ Features

### 🔍 Symbol Analysis
- **Hash Table–backed symbol table** with O(1) average lookups across functions, variables, loops, and conditions
- **Prefix autocomplete** via Trie — instant symbol search as-you-type
- **AVL Tree sorted view** — alphabetical traversal with balance stats
- **Category & type filtering** — query by `function`, `variable`, `loop`, `condition`, data type, or line range
- **CSV export** for full symbol table snapshots

### 🌲 AST Analysis
- **Full recursive Abstract Syntax Tree** built from a hand-written C parser with depth-limited traversal
- **Complexity metrics** — cyclomatic complexity, nesting depth, node count per scope
- **Dual-mode display** — compact tree view and detailed structured printout
- **Control Flow Graph (CFG)** extraction from AST, exported to `.dot` format

### 📈 Function Call Graph
- **Directed adjacency-list graph** with up to 200 function nodes and 65+ tracked edges
- **DFS + BFS traversal** for hierarchy walking and call-chain discovery
- **Recursion detection** — identifies and flags 7+ recursive patterns (factorial, fibonacci, etc.)
- **Dead code detection** — isolates unreachable functions
- **High-coupling detection** — flags functions with fanout > 5
- **DOT + CSV export** for external visualization tools

### 🏆 Code Quality Analysis
- **Priority Queue–ranked issue reporting** — issues sorted by severity (1–5)
- **8 issue categories**: unused variables, deep nesting, large functions, high fanout, too many parameters, high complexity, dead code, and recursive patterns
- **Per-function scoring** with line-level diagnostics
- **JSON export** to `output/issues.json` for frontend rendering

### 🖥️ Frontend Visualization
- **Dashboard** — live metrics overview with metric cards, stat summaries, and analysis trigger
- **AST Viewer** — collapsible, color-coded node tree with depth indicators
- **Function Graph** — interactive canvas renderer with force-directed node layout
- **Issues Report** — severity-labeled issue cards with file/function/line context
- **Symbol Explorer** — paginated symbol table with search and filter
- **Analyze Page** — upload `.c` files or paste code directly into a syntax-aware editor; triggers backend in headless mode

---

## 🧱 Tech Stack

### ⚙️ Backend — Pure C Engine

| Module | File(s) | Data Structure | Complexity |
|---|---|---|---|
| Symbol Table | `hashtable.c / .h` | Hash Table (chaining, size 1031) | O(1) avg lookup / insert |
| Autocomplete | `trie.c / .h` | Trie (26-child nodes) | O(L) per prefix query |
| Sorted View | `avltree.c / .h` | AVL Tree (self-balancing BST) | O(log n) insert / search |
| Lexer | `lexer.c / .h` | Token stream (linear scan) | O(n) per file |
| Parser | `parser.c / .h` | Recursive descent parser | O(n) per token stream |
| AST | `ast.c / .h` | N-ary tree (heap-allocated) | O(n) traversal |
| Function Graph | `graph.c / .h` | Directed graph (adjacency list) | O(V + E) DFS/BFS |
| CFG | `cfg.c / .h` | Control Flow Graph (DAG) | O(n) from AST |
| Quality Analyzer | `quality_analyzer.c / .h` | Max-Heap Priority Queue | O(log k) per issue |
| JSON Exporter | `json_export.c / .h` | Sequential serializer | O(n) per structure |

**Compiler:** `gcc` with `-Wall -Wextra`  
**Build System:** `make` (Makefile with incremental compilation via `obj/`)

---

### 🎨 Frontend — React + Vite

| Layer | Technology |
|---|---|
| Framework | React 19 + Vite 8 |
| Routing | React Router DOM v7 |
| Icons | Lucide React |
| Graph Rendering | Custom HTML5 Canvas (`GraphCanvas.jsx`) |
| API Bridge | Vite custom middleware (serves `output/*.json`, handles `/api/analyze`) |
| Styling | Vanilla CSS with CSS variables, glassmorphism & dark theme |
| Dev Server | `vite` (HMR enabled, 30s exec timeout on backend subprocess) |

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        CodeMaster Engine                         │
│                                                                  │
│   .c Source File                                                 │
│        │                                                         │
│        ▼                                                         │
│   ┌──────────┐    Tokens     ┌──────────┐                        │
│   │  Lexer   │ ────────────► │  Parser  │                        │
│   └──────────┘               └──────────┘                        │
│                                    │                             │
│                    ┌───────────────┼───────────────┐             │
│                    ▼               ▼               ▼             │
│              ┌──────────┐  ┌──────────┐  ┌──────────────┐       │
│              │   AST    │  │Hash Table│  │Function Graph│       │
│              │ (N-ary   │  │+ Trie    │  │(Adj. List)   │       │
│              │  tree)   │  │+ AVL Tree│  │              │       │
│              └──────────┘  └──────────┘  └──────────────┘       │
│                    │                             │               │
│                    ▼                             ▼               │
│              ┌──────────┐               ┌──────────────┐         │
│              │   CFG    │               │  Recursion / │         │
│              │ Builder  │               │  Dead Code   │         │
│              └──────────┘               │  Detection   │         │
│                    │                    └──────────────┘         │
│                    └──────────┬──────────────────┘               │
│                               ▼                                  │
│                      ┌─────────────────┐                         │
│                      │ Quality Analyzer│                          │
│                      │ (Priority Queue)│                          │
│                      └─────────────────┘                         │
│                               │                                  │
│                               ▼                                  │
│                      ┌─────────────────┐                         │
│                      │  JSON Exporter  │                          │
│                      │ symbols.json    │                          │
│                      │ ast.json        │                          │
│                      │ graph.json      │                          │
│                      │ issues.json     │                          │
│                      └─────────────────┘                         │
└─────────────────────────────────────────────────────────────────┘
                               │
                               ▼
┌─────────────────────────────────────────────────────────────────┐
│                     React Frontend (Vite)                        │
│                                                                  │
│  /api/analyze ──► master_bin --headless ──► output/*.json        │
│                                                                  │
│  ┌───────────┐ ┌───────────┐ ┌────────────┐ ┌───────────────┐   │
│  │Dashboard  │ │AST Viewer │ │Func. Graph │ │Issues Report  │   │
│  │(metrics)  │ │(tree UI)  │ │(canvas)    │ │(priority list)│   │
│  └───────────┘ └───────────┘ └────────────┘ └───────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 🔄 System Flowchart

```
  ╔══════════════════╗
  ║  User Input:     ║
  ║  Upload .c file  ║
  ║  or paste code   ║
  ╚═════════╤════════╝
            │
            ▼
  ╔══════════════════╗
  ║  Vite Middleware ║  POST /api/analyze
  ║  writes to       ║──────────────────►  target_input.c
  ║  target_input.c  ║
  ╚═════════╤════════╝
            │
            ▼
  ╔══════════════════════════════════════╗
  ║  master_bin target_input.c           ║
  ║  --headless                          ║
  ║                                      ║
  ║  1. Lexer  → token stream            ║
  ║  2. Parser → populates:              ║
  ║       • Hash Table  (symbols)        ║
  ║       • Trie        (autocomplete)   ║
  ║       • AVL Tree    (sorted view)    ║
  ║       • Graph       (call edges)     ║
  ║  3. parse_file_to_ast() → AST tree   ║
  ║  4. CFG built from AST               ║
  ║  5. Quality analysis → PQ ranked     ║
  ║  6. export_all_json() writes:         ║
  ║       • output/symbols.json          ║
  ║       • output/graph.json            ║
  ║       • output/ast.json              ║
  ║       • output/issues.json           ║
  ╚══════════════════╤═══════════════════╝
                     │  < 30 ms
                     ▼
  ╔══════════════════════════════════════╗
  ║  Vite serves output/*.json           ║
  ║  React fetches and renders:          ║
  ║                                      ║
  ║  • Dashboard   → MetricCards         ║
  ║  • AST Viewer  → collapsible tree    ║
  ║  • Graph View  → canvas rendering    ║
  ║  • Issues Page → severity-ranked     ║
  ║  • Symbols     → filterable table    ║
  ╚══════════════════════════════════════╝
```

---

## 💎 Why CodeMaster is Unique

Most static analysis tools are either black boxes or language-specific commercial products. CodeMaster is different:

| Trait | What It Means |
|---|---|
| 🔩 **Pure C Engine** | No runtime, no VM, no interpreter overhead. The entire analysis pipeline compiles to a single binary (`master_bin`) under 200 KB |
| 🌲 **5 Data Structures — by design** | Hash Table, Trie, AVL Tree, Directed Graph, and Max-Heap Priority Queue — each chosen for a specific algorithmic reason, not convenience |
| 📐 **Hand-Written Parser** | Not a third-party library — a custom recursive-descent parser with its own lexer, giving full control over what gets analyzed |
| 🖼️ **Full-Stack Visualization** | Analysis results are serialized to structured JSON and rendered live in a React dashboard — not a CLI dump |
| ⚡ **Sub-30ms Analysis** | On real-world C files (500+ lines, 29 functions, 65 call edges), the entire pipeline completes in under 30ms |
| 🔄 **Two Modes: Interactive & Headless** | Run interactively via the 18-option CLI menu, or trigger via `--headless` for programmatic frontend integration |
| 📦 **Self-Contained** | No external C libraries. The backend compiles with `gcc -Wall -Wextra` and zero dependency warnings |

---

## ⚖️ Comparison with Existing Tools

| Feature | CodeMaster | `cppcheck` | `clang-tidy` | `splint` | Basic Linters |
|---|:---:|:---:|:---:|:---:|:---:|
| Custom C Parser (no LLVM/Clang) | ✅ | ❌ | ❌ | ✅ | ❌ |
| Full AST Construction | ✅ | ⚠️ Partial | ✅ | ⚠️ Partial | ❌ |
| Function Call Graph | ✅ | ⚠️ Limited | ✅ | ❌ | ❌ |
| Recursion Detection | ✅ | ❌ | ✅ | ❌ | ❌ |
| Dead Code Detection | ✅ | ✅ | ✅ | ⚠️ | ❌ |
| Priority-Ranked Issue Queue | ✅ | ❌ | ❌ | ❌ | ❌ |
| Control Flow Graph (CFG) | ✅ | ❌ | ✅ | ❌ | ❌ |
| Prefix Autocomplete (Trie) | ✅ | ❌ | ❌ | ❌ | ❌ |
| Sorted Symbol View (AVL) | ✅ | ❌ | ❌ | ❌ | ❌ |
| React Visualization Frontend | ✅ | ❌ | ❌ | ❌ | ❌ |
| Interactive Canvas Graph | ✅ | ❌ | ❌ | ❌ | ❌ |
| Web Upload / Paste API | ✅ | ❌ | ❌ | ❌ | ❌ |
| JSON-structured output | ✅ | ⚠️ XML | ⚠️ YAML | ❌ | ❌ |
| External dependencies | **Zero** | Medium | Heavy (LLVM) | Low | Low |
| Stability (no crashes on test files) | ✅ 30s timeout | ✅ | ✅ | ⚠️ | ✅ |

---

## 🎯 Use Cases

| Scenario | How CodeMaster Helps |
|---|---|
| 🎓 **Learning C Internals** | Visualize how your own C code forms an AST — understand scopes, declarations, and call hierarchies in real time |
| 🔍 **Debugging Complex Codebases** | The function call graph instantly reveals which functions are tightly coupled or calling each other recursively |
| 📋 **Code Quality Reviews** | The Priority Queue–ranked issue report surfaces the most severe problems first, not a flat wall of warnings |
| 🏆 **Hackathon Submissions** | Upload your `.c` file to the web dashboard — get a professional analysis report in under 30ms |
| 🧪 **CS Education / Viva Demos** | 18 interactive menu options let you demonstrate symbol lookup, trie autocomplete, and graph analysis live |
| 🛠️ **Refactoring Guidance** | Dead code detection and high-coupling flags highlight where to clean up before submission |
| 📊 **Codebase Metrics** | Get exact counts: functions, call edges, recursion depth, nesting levels, and more — all in structured JSON |

---

## 🚀 Installation & Setup

### Prerequisites

- `gcc` (GCC 11+ recommended)
- `make`
- `Node.js` (v18+) and `npm`

---

### 1️⃣ Clone the Repository

```bash
git clone https://github.com/nitinrwt22/codemaster.git
cd CodeMaster
```

---

### 2️⃣ Build the Backend

```bash
make
```

This compiles all `.c` files in `backend/` into object files under `obj/` and links them into the `master_bin` binary.

**Run in interactive mode (CLI):**
```bash
./master_bin path/to/your_file.c
```

**Run in headless mode (exports JSON to `output/`):**
```bash
./master_bin path/to/your_file.c --headless
```

**Clean build artifacts:**
```bash
make clean
```

---

### 3️⃣ Set Up the Frontend

```bash
cd frontend
npm install
npm run dev
```

The Vite dev server will start at **`http://localhost:5173`**.

> **Note:** The Vite server includes a custom middleware that:
> - Serves `output/*.json` files directly to the React app
> - Handles `POST /api/analyze` — writes uploaded code to `target_input.c` and executes `./master_bin --headless` with a 30-second timeout

---

### 4️⃣ Analyze Your Code

Open `http://localhost:5173` → navigate to **Analyze** → upload a `.c` file or paste code → click **Analyze**.

The backend will run, populate `output/`, and the dashboard will auto-refresh with live results.

---

## 📁 Project Structure

```
CodeMaster/
│
├── backend/                      # C analysis engine
│   ├── main.c                    # Entry point: 18-option CLI + headless mode
│   ├── lexer.c / lexer.h         # Tokenizer — scans raw C source
│   ├── parser.c / parser.h       # Recursive-descent parser → symbol extraction
│   ├── ast.c / ast.h             # AST builder, traversal, and analysis
│   ├── hashtable.c / hashtable.h # Symbol table (open chaining, size 1031)
│   ├── trie.c / trie.h           # Prefix autocomplete for symbol names
│   ├── avltree.c / avltree.h     # Self-balancing BST for sorted symbol view
│   ├── graph.c / graph.h         # Directed function call graph (adjacency list)
│   ├── cfg.c / cfg.h             # Control Flow Graph from AST
│   ├── quality_analyzer.c / .h   # Max-heap PQ for ranked issue reporting
│   ├── json_export.c / .h        # Serializes all analysis to JSON
│   └── utils.h                   # Shared constants (MAX_NAME, MAX_TYPE, etc.)
│
├── frontend/                     # React + Vite visualization layer
│   ├── vite.config.js            # Custom middleware: /output files + /api/analyze
│   ├── src/
│   │   ├── App.jsx               # Root router — defines all page routes
│   │   ├── main.jsx              # React DOM entry point
│   │   ├── index.css             # Global design tokens, dark theme, typography
│   │   ├── pages/
│   │   │   ├── Dashboard.jsx/css     # Metrics overview, analysis trigger
│   │   │   ├── ASTViewer.jsx/css     # Collapsible AST tree renderer
│   │   │   ├── FunctionGraph.jsx/css # Canvas-based interactive call graph
│   │   │   ├── IssuesReport.jsx/css  # Priority-ranked issue card list
│   │   │   ├── Analyze.jsx/css       # Upload + code paste + trigger UI
│   │   │   └── SymbolExplorer.jsx/css# Searchable, filterable symbol table
│   │   └── components/
│   │       ├── layout/           # Shared layout wrappers (sidebar, nav)
│   │       └── ui/
│   │           ├── MetricCard.jsx/css  # Stat display cards for dashboard
│   │           ├── GraphCanvas.jsx     # HTML5 canvas graph renderer
│   │           ├── ASTNode.jsx         # Recursive AST node component
│   │           ├── IssueCard.jsx/css   # Individual issue display component
│   │           ├── FileUpload.jsx/css  # Drag-and-drop file uploader
│   │           └── Table.jsx/css       # Reusable data table component
│
├── output/                       # Auto-generated JSON analysis results
│   ├── symbols.json              # All extracted symbols with type, category, line
│   ├── ast.json                  # Full AST as nested JSON (~310 KB for real files)
│   ├── graph.json                # Function graph: 29 nodes, 65 edges, cycle info
│   └── issues.json               # Ranked quality issues with severity + location
│
├── Makefile                      # Build system (gcc, incremental via obj/)
├── master_bin                    # Compiled binary (160 KB, zero external deps)
├── target_input.c                # Temp file written by frontend for analysis
└── README.md                     # This file
```

---

## 📸 Screenshots

> *Replace these placeholders with actual screenshots of your running dashboard.*

### 🖥️ Dashboard — Metrics Overview
```
┌─────────────────────────────────────────────────────────────────┐
│  📊 CodeMaster Dashboard                                         │
│                                                                  │
│  ┌─────────────┐ ┌─────────────┐ ┌─────────────┐ ┌───────────┐ │
│  │ 29 Functions│ │  65 Edges   │ │ 7 Recursive │ │ < 30ms    │ │
│  │  Detected   │ │  in Graph   │ │  Patterns   │ │ Analysis  │ │
│  └─────────────┘ └─────────────┘ └─────────────┘ └───────────┘ │
└─────────────────────────────────────────────────────────────────┘
```
> 📷 `screenshots/dashboard.png`

---

### 🌲 AST Viewer — Tree Structure
```
  Program
  └── FunctionDecl: main (line 640)
      ├── ParamDecl: argc (int)
      ├── ParamDecl: argv (char**)
      └── CompoundStmt
          ├── CallExpr: init_sample_data
          ├── CallExpr: authenticate_admin
          └── IfStmt
              ├── Condition: ...
              └── CallExpr: admin_menu
```
> 📷 `screenshots/ast_viewer.png`

---

### 📈 Function Call Graph — Canvas View
> Interactive force-directed graph showing all 29 functions and 65 call edges.  
> Recursive nodes highlighted. Dead code nodes greyed out.
>
> 📷 `screenshots/function_graph.png`

---

### 🏆 Issues Report — Priority Ranked
| Severity | Issue | Function | Line |
|:---:|---|---|:---:|
| 🔴 Critical | High cyclomatic complexity | `admin_menu` | 578 |
| 🔴 Critical | High fanout (13 callees) | `admin_menu` | 578 |
| 🟠 High | Deep nesting level | `borrow_book` | 408 |
| 🟡 Medium | Recursive without base guard | `fibonacci` | 176 |
| 🔵 Info | Unused function detected | `debug_dump` | 136 |

> 📷 `screenshots/issues_report.png`

---

## 🔭 Future Improvements

| Feature | Description |
|---|---|
| 🤖 **AI-Assisted Suggestions** | Integrate a local LLM (e.g. llama.cpp) to generate fix suggestions for each flagged issue |
| ⚡ **Real-Time Analysis** | File-system watcher triggers re-analysis on save — results update without page reload |
| 🌐 **Multi-Language Support** | Extend the lexer/parser for C++ and Python subsets |
| 🔗 **Inline Code Diff** | Show exactly which lines need changing, highlighted in the editor |
| 📤 **PDF/HTML Report Export** | One-click download of the full analysis as a formatted report |
| 🧪 **Test Coverage Overlay** | Integrate with `gcov` to overlay test coverage data on the AST viewer |
| 🐳 **Docker Container** | Containerize backend + frontend for zero-dependency deployment |
| 🔌 **VS Code Extension** | Surface CodeMaster analysis inline inside the editor as diagnostics |

---

## 👨‍💻 Author

<div align="center">

**Nitin Rawat**  
*Computer Science Student · Systems & Full-Stack Developer*

[![GitHub](https://img.shields.io/badge/GitHub-nitinrwt22-181717?style=for-the-badge&logo=github)](https://github.com/nitinrwt22)

> *"Build systems, not just scripts."*

</div>

---

<div align="center">

**⭐ Star this repo if you found it useful!**

*CodeMaster — Parse deeper. Visualize smarter. Code better.*

[![footer](https://capsule-render.vercel.app/api?type=waving&color=0:2c5364,50:203a43,100:0f2027&height=120&section=footer)](https://github.com/nitinrwt22/codemaster)

</div>
