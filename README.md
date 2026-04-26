
<div align="center">

```
  ____          _      __  __           _            
 / ___|___   __| | ___|  \/  | __ _ ___| |_ ___ _ __ 
| |   / _ \ / _` |/ _ \ |\/| |/ _` / __| __/ _ \ '__|
| |__| (_) | (_| |  __/ |  | | (_| \__ \ ||  __/ |   
 \____\___/ \__,_|\___|_|  |_|\__,_|___/\__\___|_|   
```

### ⚡ A high-performance C static analysis engine with a full-stack interactive visualization dashboard

[![Language](https://img.shields.io/badge/Engine-C-blue?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Frontend](https://img.shields.io/badge/Frontend-React%2019-61dafb?style=for-the-badge&logo=react)](https://react.dev/)
[![Build](https://img.shields.io/badge/Build-Makefile%20%2B%20GCC-orange?style=for-the-badge&logo=gnu)](https://www.gnu.org/software/make/)
[![Bundler](https://img.shields.io/badge/Bundler-Vite-646cff?style=for-the-badge&logo=vite)](https://vitejs.dev/)
[![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)]()
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)

</div>

---

## 🧠 What is CodeMaster?

**CodeMaster** is a full-stack C source code analysis tool that parses your C programs at the compiler level — extracting symbols, building an Abstract Syntax Tree (AST), mapping function call graphs, analyzing control flow, and surfacing ranked quality issues — then visualizes everything in a sleek, interactive React dashboard.

> It's not a linter. It's not just a parser. It's a complete static analysis engine built entirely in C, with no external analysis libraries, exposing structured JSON outputs consumed by a modern web frontend.

---

## ✨ Features

### 🔍 Symbol Analysis
- Instant **symbol lookup by name** from a hash table with O(1) average access
- **Prefix autocomplete** powered by a Trie — find every symbol starting with a given string
- Filter symbols by **category** (function / variable / loop / condition) or **data type** (int, float, char, void…)
- **Line-range queries** — retrieve all symbols declared between any two source lines
- Export full symbol table to **CSV** for external processing

### 🌳 AST Analysis
- Full **Abstract Syntax Tree** construction from C source — functions, declarations, control flow, expressions, blocks
- Computes **cyclomatic complexity** and **max nesting depth** per function
- **Data Flow Analysis (DFA)** — tracks read/write variable counts, detects unread writes and unused variables
- Detailed node-level metadata: type, value, data type, line number, parent/children links

### 📊 Function Call Graph
- Builds a directed **adjacency-list graph** of all inter-function calls (up to 200 nodes)
- Detects **cycles** (DFS-based), **dead code**, and **highly-coupled** functions (configurable threshold)
- Computes **in-degree / out-degree** for each node, ranks by popularity
- Exports to **DOT format** (Graphviz) and **CSV** for external rendering

### 🔄 Control Flow Graph (CFG)
- Derives a **Control Flow Graph** from the AST capturing branches, loops, and return paths
- Exports CFG to DOT format (`output/function_cfg.dot`) for visualization

### 🏆 Code Quality Analysis
- Ranked **priority-queue** of issues sorted by severity (1–5 scale)
- Detects: **unused variables**, **deep nesting**, **large functions**, **high fanout**, **too many parameters**, **high cyclomatic complexity**, **dead code**, and **recursive patterns**
- Generates a structured quality report consumable by the frontend (`output/issues.json`)

### 🖥️ Frontend Visualization
- **Dashboard** — metric cards, severity gauge, code churn analytics, system metadata
- **AST Viewer** — interactive tree rendering of the parsed AST
- **Function Graph** — canvas-based interactive call graph with node highlighting
- **Issues Report** — severity-grouped issue cards with filtering
- **Symbol Explorer** — searchable, filterable symbol table browser
- Drag-and-drop **file upload** that triggers backend analysis and auto-refreshes all panels

---

## 🛠️ Tech Stack

### Backend — Pure C Engine

| Module | File(s) | Data Structure |
|---|---|---|
| Symbol Table | `hashtable.c / .h` | Hash Table (chaining, size 1031) |
| Autocomplete | `trie.c / .h` | Prefix Trie |
| Sorted View | `avltree.c / .h` | AVL Tree (self-balancing BST) |
| Lexer | `lexer.c / .h` | Token stream |
| Parser | `parser.c / .h` | Recursive-descent parser |
| AST | `ast.c / .h` | N-ary tree with sibling links |
| Call Graph | `graph.c / .h` | Directed adjacency-list graph |
| Control Flow | `cfg.c / .h` | CFG node/edge graph |
| Quality Engine | `quality_analyzer.c / .h` | Max-heap priority queue |
| JSON Export | `json_export.c / .h` | Structured file I/O |

> **Zero external C libraries.** Every data structure is hand-implemented from scratch.

### Frontend — React + Vite

| Technology | Purpose |
|---|---|
| **React 19** | Component-based UI framework |
| **Vite 8** | Lightning-fast dev server & bundler |
| **React Router v7** | Client-side routing between views |
| **Lucide React** | Icon system |
| **Vanilla CSS** | Custom design system — glassmorphism, dark mode, animations |
| **Canvas API** | Function graph rendering (`GraphCanvas.jsx`) |

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        CodeMaster Pipeline                          │
└─────────────────────────────────────────────────────────────────────┘

  ┌──────────────┐
  │  C Source    │  target_input.c  (uploaded via React frontend)
  │  File (.c)   │
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │   Lexer      │  lexer.c  →  Token stream (identifiers, keywords,
  │              │               operators, literals)
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │   Parser     │  parser.c  →  Recursive-descent parsing
  │              │               Populates: HashTable, Trie, AVL Tree,
  │              │               FunctionGraph simultaneously
  └──────┬───────┘
         │
         ├─────────────────────────────────────────────┐
         │                                             │
         ▼                                             ▼
  ┌──────────────┐                            ┌────────────────┐
  │  Symbol      │  hashtable.c               │  AST Builder   │
  │  Table       │  trie.c                    │  ast.c         │
  │  (HT+Trie    │  avltree.c                 │  parser.c      │
  │  +AVL)       │                            └───────┬────────┘
  └──────┬───────┘                                    │
         │                                            ├──────────────┐
         │                                            ▼              ▼
         │                                  ┌──────────────┐  ┌─────────────┐
         │                                  │  Function    │  │  CFG        │
         │                                  │  Call Graph  │  │  Builder    │
         │                                  │  graph.c     │  │  cfg.c      │
         │                                  └──────┬───────┘  └──────┬──────┘
         │                                         │                 │
         │                                         ▼                 │
         │                                ┌──────────────┐           │
         │                                │  Quality     │           │
         │                                │  Analyzer    │           │
         │                                │  quality_    │           │
         │                                │  analyzer.c  │           │
         │                                └──────┬───────┘           │
         │                                       │                   │
         └───────────────────┬───────────────────┘                   │
                             │◄──────────────────────────────────────┘
                             ▼
                   ┌──────────────────┐
                   │   JSON Export    │  json_export.c
                   │                  │
                   │  output/         │
                   │  ├─ symbols.json │
                   │  ├─ graph.json   │
                   │  ├─ ast.json     │
                   │  └─ issues.json  │
                   └────────┬─────────┘
                            │  HTTP (Vite dev proxy)
                            ▼
                   ┌──────────────────────────────────────┐
                   │         React Frontend               │
                   │                                      │
                   │  ┌──────────┐  ┌───────────────────┐ │
                   │  │Dashboard │  │  AST Viewer       │ │
                   │  └──────────┘  └───────────────────┘ │
                   │  ┌──────────┐  ┌───────────────────┐ │
                   │  │Function  │  │  Issues Report    │ │
                   │  │Graph     │  └───────────────────┘ │
                   │  └──────────┘  ┌───────────────────┐ │
                   │                │  Symbol Explorer  │ │
                   │                └───────────────────┘ │
                   └──────────────────────────────────────┘
```

---

## 🔄 Full System Flowchart

```
                          ┌─────────────┐
                          │  User drops │
                          │  .c file on │
                          │  dashboard  │
                          └──────┬──────┘
                                 │
                                 ▼
                    ┌────────────────────────┐
                    │  Vite backend proxy    │
                    │  triggers master_bin   │
                    │  with --headless flag  │
                    └────────────┬───────────┘
                                 │
              ┌──────────────────┼─────────────────────┐
              ▼                  ▼                     ▼
     ┌────────────────┐  ┌──────────────┐    ┌─────────────────┐
     │  LEXER         │  │  PARSER      │    │  AST BUILDER    │
     │  Tokenize      │  │  Recursive   │    │  N-ary tree     │
     │  source code   │  │  descent     │    │  construction   │
     └────────┬───────┘  └──────┬───────┘    └────────┬────────┘
              │                 │                      │
              │        ┌────────┴──────┐               │
              │        ▼               ▼               │
              │  ┌──────────┐  ┌─────────────┐         │
              │  │ HashTable│  │ Trie + AVL  │         │
              │  │ O(1) sym │  │ autocomplete│         │
              │  │ lookup   │  │ + sorted    │         │
              │  └──────────┘  └─────────────┘         │
              │                                        │
              │                         ┌──────────────┘
              │                         ▼
              │                ┌──────────────────┐
              │                │  CALL GRAPH      │
              │                │  DFS cycle det.  │
              │                │  Dead code detect│
              │                │  Coupling check  │
              │                └────────┬─────────┘
              │                         │
              │                ┌────────┘
              │                ▼
              │       ┌────────────────────┐
              │       │  QUALITY ANALYZER  │
              │       │  Priority Queue    │
              │       │  Severity ranking  │
              │       │  Issue detection   │
              │       └────────┬───────────┘
              │                │
              └────────┬───────┘
                       ▼
              ┌─────────────────────┐
              │    JSON EXPORT      │
              │  symbols / graph /  │
              │  ast / issues .json │
              └──────────┬──────────┘
                         │
                         ▼
              ┌─────────────────────┐
              │   REACT DASHBOARD   │
              │  Live render of all │
              │  analysis results   │
              └─────────────────────┘
```

---

## 🌟 Why CodeMaster is Unique

| Aspect | What makes it stand out |
|---|---|
| **C-native engine** | Parser, AST, graph, and quality engine written entirely in C — no libclang, no LLVM wrappers |
| **5 data structures** | Hash Table, Trie, AVL Tree, Adjacency-list Graph, Max-heap Priority Queue — all hand-implemented |
| **Full-stack architecture** | C backend → JSON API → React frontend. Two completely different technology domains unified |
| **Real compiler pipeline** | Follows Lexer → Parser → AST → Graph → Analysis — the same stages as a real compiler front-end |
| **Headless mode** | Backend supports `--headless` flag for CI/CD and web-server integration |
| **Interactive visualization** | Canvas-rendered call graphs, interactive AST trees, severity-filterable issue panels |
| **Zero dependencies in engine** | Pure C89/C99 standard. No malloc wrappers, no parsing libraries — entirely self-contained |
| **Priority-ranked reporting** | Issues are surfaced by severity using a max-heap — not a flat list dump |

---

## 📊 Comparison with Existing Tools

| Feature | Basic Linters (e.g. clang-tidy) | Typical Student Analyzers | **CodeMaster** |
|---|---|---|---|
| Custom C engine | ❌ (wraps LLVM) | ❌ (usually Python/Java) | ✅ Pure C |
| Hand-built data structures | ❌ | ❌ | ✅ HT + Trie + AVL + Graph + PQ |
| AST construction | ✅ (via libclang) | ⚠️ partial | ✅ Custom recursive-descent |
| Function call graph | ✅ | ❌ | ✅ with cycle & dead-code detection |
| Control flow graph | ✅ | ❌ | ✅ CFG with DOT export |
| Data flow analysis | ✅ | ❌ | ✅ read/write variable tracking |
| Priority-ranked issues | ⚠️ | ❌ | ✅ Max-heap severity queue |
| Web visualization | ❌ | ❌ | ✅ React dashboard |
| JSON export for UI | ❌ | ❌ | ✅ symbols / graph / ast / issues |
| Headless/CI mode | ✅ | ❌ | ✅ `--headless` flag |
| Autocomplete search | ❌ | ❌ | ✅ Trie-based prefix search |

---

## 🎯 Use Cases

| Domain | How CodeMaster Helps |
|---|---|
| **Code Quality Auditing** | Get severity-ranked reports on nesting depth, function complexity, dead code, and coupling issues |
| **CS Education** | Visualize how a C program becomes tokens → AST → call graph — ideal for compilers or DSA courses |
| **Debugging & Refactoring** | Identify highly coupled functions, recursive patterns, and unused variables before refactoring |
| **Academic Projects** | Demonstrate a full compiler front-end + data structure integration with a polished UI |
| **Hackathons** | Complete, running, visually impressive — stands out immediately |
| **Portfolio / Recruitment** | Demonstrates mastery of C, data structures, system design, and full-stack development simultaneously |

---

## ⚙️ Installation & Setup

### Prerequisites

- `gcc` (GCC 11+ recommended)
- `make`
- `node` v18+ and `npm` v9+

---

### 🔧 Backend — Compile the C Engine

```bash
# Clone the repository
git clone https://github.com/nitinrwt22/CodeMaster.git
cd CodeMaster

# Compile all backend modules
make

# Verify the binary was created
ls -lh master_bin
```

**Run in interactive CLI mode:**
```bash
./master_bin path/to/your_code.c
```

**Run in headless mode (for frontend integration):**
```bash
./master_bin path/to/your_code.c --headless
# Outputs: output/symbols.json, output/graph.json, output/ast.json, output/issues.json
```

**Clean build artifacts:**
```bash
make clean
```

---

### 🌐 Frontend — Launch the React Dashboard

```bash
cd frontend

# Install dependencies
npm install

# Start the development server
npm run dev
```

Open **http://localhost:5173** in your browser.

> The Vite dev server proxies `/api` and `/output` routes to the backend. Upload a `.c` file via the dashboard — the backend runs headless, writes JSON outputs, and the frontend auto-refreshes all panels.

---

## 📁 Project Structure

```
CodeMaster/
│
├── backend/                    # C analysis engine
│   ├── main.c                  # Entry point — CLI + headless orchestration
│   ├── lexer.c / lexer.h       # Tokenizer
│   ├── parser.c / parser.h     # Recursive-descent parser
│   ├── ast.c / ast.h           # AST node types, construction, traversal
│   ├── hashtable.c / .h        # Symbol table (hash table with chaining)
│   ├── trie.c / trie.h         # Autocomplete (prefix trie)
│   ├── avltree.c / .h          # Sorted symbol view (AVL tree)
│   ├── graph.c / graph.h       # Function call graph (adjacency list)
│   ├── cfg.c / cfg.h           # Control flow graph
│   ├── quality_analyzer.c / .h # Issue detection + priority queue
│   ├── json_export.c / .h      # Structured JSON file writer
│   └── utils.h                 # Shared constants and macros
│
├── frontend/                   # React + Vite web dashboard
│   ├── src/
│   │   ├── pages/
│   │   │   ├── Dashboard.jsx       # Main analysis overview
│   │   │   ├── ASTViewer.jsx       # Interactive AST tree
│   │   │   ├── FunctionGraph.jsx   # Call graph visualization
│   │   │   ├── IssuesReport.jsx    # Severity-ranked issues panel
│   │   │   └── SymbolExplorer.jsx  # Searchable symbol browser
│   │   ├── components/
│   │   │   ├── ui/
│   │   │   │   ├── FileUpload.jsx  # Drag-and-drop file uploader
│   │   │   │   ├── IssueCard.jsx   # Severity-colored issue card
│   │   │   │   ├── MetricCard.jsx  # Dashboard stat card
│   │   │   │   ├── Table.jsx       # Reusable data table
│   │   │   │   └── GraphCanvas.jsx # Canvas-based graph renderer
│   │   │   └── layout/             # Sidebar, navbar, layout wrappers
│   │   ├── services/               # API call helpers
│   │   ├── App.jsx                 # Router setup
│   │   └── index.css               # Global design system
│   ├── vite.config.js              # Vite + proxy config
│   └── package.json
│
├── output/                     # Generated JSON results (auto-created)
│   ├── symbols.json
│   ├── graph.json
│   ├── ast.json
│   └── issues.json
│
├── Makefile                    # Build system
└── README.md
```

---

## 🖼️ Screenshots

> *Upload your own `.c` file to generate live results — these are representative views.*

| Dashboard Overview | AST Viewer |
|---|---|
| ![Dashboard](screenshots/dashboard.png) | ![AST Viewer](screenshots/ast_viewer.png) |

| Function Call Graph | Issues Report |
|---|---|
| ![Graph](screenshots/function_graph.png) | ![Issues](screenshots/issues_report.png) |

---

## 🚀 Future Improvements

| Category | Planned Feature |
|---|---|
| 🤖 **AI Integration** | LLM-powered fix suggestions for detected quality issues |
| ⚡ **Real-time Analysis** | WebSocket-based live analysis as you type in a built-in editor |
| 🌍 **Multi-language Support** | Extend parser to support C++, Python, and Java source files |
| 📈 **Historical Tracking** | Persist analysis runs and show quality trend graphs over time |
| 🔌 **VS Code Extension** | Surface CodeMaster results directly in the editor sidebar |
| 📦 **Docker Support** | Containerized deployment with a pre-built image |
| 🧪 **Test Coverage Analysis** | Map test files against analyzed functions for coverage insight |
| 🌐 **Online Playground** | Browser-based C code editor with instant analysis — no install needed |


---

## 👤 Author

<div align="center">

Made with 🖤 by **Nitin Rawat**

*Designed, architected, and built from scratch — C engine to React dashboard.*

[![GitHub](https://img.shields.io/badge/GitHub-nitinrwt22-181717?style=for-the-badge&logo=github)](https://github.com/nitinrwt22)

---

**CodeMaster** © 2025 Nitin Rawat — All rights reserved.

*Because real engineers understand what happens below the abstraction.*

</div>

