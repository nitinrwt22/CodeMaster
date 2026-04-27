
<div align="center">

```
   ██████╗ ██████╗ ██████╗ ███████╗███╗   ███╗ █████╗ ███████╗████████╗███████╗██████╗
  ██╔════╝██╔═══██╗██╔══██╗██╔════╝████╗ ████║██╔══██╗██╔════╝╚══██╔══╝██╔════╝██╔══██╗
  ██║     ██║   ██║██║  ██║█████╗  ██╔████╔██║███████║███████╗   ██║   █████╗  ██████╔╝
  ██║     ██║   ██║██║  ██║██╔══╝  ██║╚██╔╝██║██╔══██║╚════██║   ██║   ██╔══╝  ██╔══██╗
  ╚██████╗╚██████╔╝██████╔╝███████╗██║ ╚═╝ ██║██║  ██║███████║   ██║   ███████╗██║  ██║
   ╚═════╝ ╚═════╝ ╚═════╝ ╚══════╝╚═╝     ╚═╝╚═╝  ╚═╝╚══════╝   ╚═╝   ╚══════╝╚═╝  ╚═╝
```

### ⚡ A high-performance C static analysis engine with a full-stack interactive visualization dashboard

[![Language](https://img.shields.io/badge/Engine-C99-blue?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Frontend](https://img.shields.io/badge/Frontend-React%2019-61dafb?style=for-the-badge&logo=react)](https://react.dev/)
[![Build](https://img.shields.io/badge/Build-GCC%20%2B%20Makefile-orange?style=for-the-badge&logo=gnu)](https://www.gnu.org/software/make/)
[![Bundler](https://img.shields.io/badge/Bundler-Vite%208-646cff?style=for-the-badge&logo=vite)](https://vitejs.dev/)
[![Router](https://img.shields.io/badge/Router-React%20Router%20v7-ca4245?style=for-the-badge&logo=reactrouter)](https://reactrouter.com/)
[![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)]()
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)

</div>

---

## 🧠 What is CodeMaster?

**CodeMaster** is a full-stack static analysis platform that processes C source code at the **compiler level** — tokenizing, parsing, constructing an Abstract Syntax Tree, mapping the function call graph, running Data Flow Analysis, and detecting ranked code quality issues — then streams all results as structured JSON into a **live React dashboard**.

> It's not a linter. It's not a wrapper around LLVM or libclang. It is a complete **compiler front-end** built from scratch in pure C, with hand-implemented data structures and a modern visualization layer — two completely different technology stacks unified into one tool.

**Real numbers from a live analysis run:**
- 🔢 **29 functions** tracked across a single source file
- 🔗 **65 call edges** mapped in the function call graph  
- 🔄 **7 recursive functions** detected automatically (factorial, fibonacci, recursive_search, …)
- ⚠️ Issues surfaced across **8 distinct code smell categories** — severity-ranked by a max-heap priority queue
- ⚡ Analysis completes in **< 30ms** on a typical laptop — O(log N) symbol retrieval via AVL Tree

---

## ✨ Features

### 🔍 Symbol Analysis
- **O(1) average symbol lookup** via a hash table with chaining collision resolution (size 1031)
- **Trie-based prefix autocomplete** — retrieve all symbols starting with a given string in O(L) time
- Filter symbols by **category** (`function`, `variable`, `loop`, `condition`) or **data type** (`int`, `float`, `char`, `void`, …)
- **Line-range queries** — pull every symbol declared between any two source lines
- **O(log N) sorted view** of all symbols using a self-balancing AVL Tree
- Export full symbol table to **CSV** for external processing

### 🌳 AST Analysis
- Complete **Abstract Syntax Tree** constructed via recursive-descent parsing — functions, declarations, control flow (`if`, `for`, `while`, `return`), expressions, and blocks
- Per-function **cyclomatic complexity** and **maximum nesting depth** calculation
- **Data Flow Analysis (DFA)** — tracks read and write counts per variable, flags unread writes and completely unused declarations
- Node-level metadata: type, identifier value, data type, line number, parent/children links

### 📊 Function Call Graph
- Directed **adjacency-list graph** supporting up to 200 nodes and 65+ edges per analysis run
- **DFS-based cycle detection** — identifies recursive patterns (e.g., `factorial`, `fibonacci`, `recursive_search`)
- **Dead code detection** — flags functions defined but never reachable from any call path
- Computes **in-degree and out-degree** per node; ranks functions by coupling and popularity
- Detects **highly-coupled functions** (configurable out-degree threshold)
- Exports to **DOT format** (`output/function_graph.dot`) and **CSV** for Graphviz or external rendering

### 🔄 Control Flow Graph (CFG)
- Derives a **Control Flow Graph** from the AST, capturing all branch, loop, and return paths
- Exports to DOT format (`output/function_cfg.dot`) for visualization

### 🏆 Code Quality Analysis
- **8 categories of code smells detected** and ranked by severity:

  | Severity | Label | Issue Type |
  |:---:|---|---|
  | 🔴 5 — Critical | `CRITICAL` | Unused Variables (DFA-detected) |
  | 🟠 4 — High | `HIGH` | Deep Nesting (threshold: 4 levels) |
  | 🟡 3 — Medium | `MEDIUM` | Large Functions, High Complexity, High Fan-out, Recursive Patterns |
  | 🔵 2 — Low | `LOW` | Excessive Parameters, Dead Code |

- Issues ranked and drained via a **max-heap priority queue** — highest severity always surfaces first
- Computes a **weighted Stability Score** (Critical = −10, High = −5, Medium = −2, Low = −1) to give an at-a-glance code health metric
- Full report exported to `output/issues.json` for frontend consumption

### 🖥️ Frontend Visualization
- **Dashboard** — metric cards, severity gauge, code churn analytics, and file metadata
- **AST Viewer** — interactive collapsible tree with node inspector panel, source location mapping, expand/collapse all, and node-type search filtering
- **Function Graph** — Canvas API-rendered interactive call graph with node highlighting and call-edge tracing
- **Issues Report** — severity-grouped issue cards with label filtering and Stability Score gauge
- **Symbol Explorer** — live-search, filterable table of all extracted symbols with category badges
- Drag-and-drop **glassmorphism file uploader** — triggers backend headless analysis and auto-refreshes all panels via Vite middleware

---

## 🛠️ Tech Stack

### ⚙️ Backend — Pure C Engine

| Module | File | Data Structure | Complexity |
|---|---|---|---|
| Symbol Table | `hashtable.c / .h` | Hash Table (chaining, prime size 1031) | O(1) avg lookup |
| Autocomplete | `trie.c / .h` | Prefix Trie | O(L) per prefix |
| Sorted View | `avltree.c / .h` | AVL Tree (self-balancing BST) | O(log N) insert/search |
| Tokenizer | `lexer.c / .h` | Linear token stream | O(N) |
| Parser | `parser.c / .h` | Recursive-descent | O(N) |
| AST | `ast.c / .h` | N-ary tree with sibling links | O(N) traversal |
| Call Graph | `graph.c / .h` | Directed adjacency-list graph (200 nodes) | O(V + E) DFS |
| Control Flow | `cfg.c / .h` | CFG node/edge graph | O(V + E) |
| Quality Engine | `quality_analyzer.c / .h` | Max-heap priority queue | O(K log K) |
| JSON Export | `json_export.c / .h` | Structured file I/O | O(N) |

> **Zero external C libraries.** Every data structure — the hash table, trie, AVL tree, call graph, priority queue — is hand-implemented from scratch in standard C99.

### 🌐 Frontend — React + Vite

| Technology | Version | Purpose |
|---|---|---|
| **React** | 19 | Component-based UI, hooks, state management |
| **Vite** | 8 | Lightning-fast dev server, HMR, proxy middleware |
| **React Router** | v7 | Client-side routing between 5 analysis views |
| **Lucide React** | latest | Icon system |
| **Canvas API** | native | Function call graph rendering (`GraphCanvas.jsx`) |
| **Vanilla CSS** | — | Custom design system — glassmorphism, dark mode, animations |

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        CodeMaster Pipeline                          │
└─────────────────────────────────────────────────────────────────────┘

  ┌──────────────┐
  │  C Source    │  your_code.c  (drag-and-drop via React frontend)
  │  File (.c)   │
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │   Lexer      │  lexer.c  →  Token stream
  │              │              (identifiers, keywords, operators, literals)
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │   Parser     │  parser.c  →  Recursive-descent parsing
  │              │               Simultaneously populates:
  │              │               HashTable · Trie · AVL Tree · FunctionGraph
  └──────┬───────┘
         │
         ├───────────────────────────────────────┐
         │                                       │
         ▼                                       ▼
  ┌──────────────┐                      ┌────────────────┐
  │  Symbol      │  hashtable.c         │  AST Builder   │
  │  Table       │  trie.c              │  ast.c         │
  │  (HT + Trie  │  avltree.c           │  parser.c      │
  │  + AVL)      │                      └───────┬────────┘
  └──────┬───────┘                              │
         │                               ┌──────┴──────────┐
         │                               ▼                  ▼
         │                     ┌──────────────┐    ┌─────────────┐
         │                     │  Function    │    │  CFG        │
         │                     │  Call Graph  │    │  Builder    │
         │                     │  graph.c     │    │  cfg.c      │
         │                     └──────┬───────┘    └──────┬──────┘
         │                            │                   │
         │                            ▼                   │
         │                   ┌──────────────┐             │
         │                   │  Quality     │             │
         │                   │  Analyzer    │             │
         │                   │  Max-Heap PQ │             │
         │                   └──────┬───────┘             │
         │                          │                     │
         └──────────┬───────────────┘                     │
                    │◄────────────────────────────────────┘
                    ▼
          ┌──────────────────┐
          │   JSON Export    │  json_export.c
          │                  │
          │  output/         │
          │  ├─ symbols.json │  ← all symbols, types, lines
          │  ├─ graph.json   │  ← 29 functions, 65 edges, cycles
          │  ├─ ast.json     │  ← full N-ary tree structure
          │  └─ issues.json  │  ← severity-ranked quality issues
          └────────┬─────────┘
                   │  Vite middleware (no separate server needed)
                   ▼
          ┌──────────────────────────────────────┐
          │         React Frontend               │
          │  localhost:5173                      │
          │                                      │
          │  ┌───────────┐  ┌────────────────┐  │
          │  │ Dashboard │  │  AST Viewer    │  │
          │  └───────────┘  └────────────────┘  │
          │  ┌───────────┐  ┌────────────────┐  │
          │  │ Function  │  │ Issues Report  │  │
          │  │ Graph     │  └────────────────┘  │
          │  └───────────┘  ┌────────────────┐  │
          │                 │ Symbol Explorer│  │
          │                 └────────────────┘  │
          └──────────────────────────────────────┘
```

---

## 🔄 Full System Flowchart

```
                        ┌──────────────────────┐
                        │  User drops .c file  │
                        │  on dashboard        │
                        └──────────┬───────────┘
                                   │
                                   ▼
                     ┌─────────────────────────┐
                     │  Vite custom middleware  │
                     │  writes file to disk    │
                     │  executes master_bin    │
                     │  with --headless flag   │
                     └────────────┬────────────┘
                                  │
              ┌───────────────────┼──────────────────┐
              ▼                   ▼                   ▼
   ┌─────────────────┐  ┌──────────────┐   ┌─────────────────┐
   │  LEXER          │  │  PARSER      │   │  AST BUILDER    │
   │  Tokenize src   │  │  Recursive   │   │  N-ary tree     │
   │  → token stream │  │  descent     │   │  construction   │
   └────────┬────────┘  └──────┬───────┘   └────────┬────────┘
            │                  │                     │
            │         ┌────────┴──────┐              │
            │         ▼               ▼              │
            │   ┌──────────┐  ┌─────────────┐       │
            │   │HashTable │  │ Trie + AVL  │       │
            │   │ O(1) sym │  │ O(L) auto-  │       │
            │   │ lookup   │  │ complete    │       │
            │   └──────────┘  └─────────────┘       │
            │                                        │
            │                          ┌─────────────┘
            │                          ▼
            │                 ┌──────────────────┐
            │                 │  CALL GRAPH      │
            │                 │  DFS cycle det.  │
            │                 │  Dead code flag  │
            │                 │  Coupling check  │
            │                 │  29 nodes tracked│
            │                 └────────┬─────────┘
            │                          │
            │                 ┌────────┘
            │                 ▼
            │        ┌────────────────────┐
            │        │  QUALITY ANALYZER  │
            │        │  Max-heap PQ       │
            │        │  8 issue categories│
            │        │  Stability Score   │
            │        └────────┬───────────┘
            │                 │
            └─────────┬───────┘
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
             │  5 live views       │
             │  auto-refresh       │
             │  on analysis done   │
             └─────────────────────┘
```

---

## 🌟 Why CodeMaster is Unique

| Aspect | Detail |
|---|---|
| 🔬 **C-native engine** | Parser, AST, graph, and quality engine written entirely in C99 — no libclang, no LLVM wrappers, no Python glue |
| 🧩 **5 data structures** | Hash Table, Trie, AVL Tree, Adjacency-list Graph, Max-heap Priority Queue — all hand-implemented |
| 🖥️ **Full-stack architecture** | C backend → JSON files → React frontend. Two completely different technology stacks, unified |
| ⚙️ **Real compiler pipeline** | Lexer → Parser → AST → Graph → Analysis — the same stages as a production compiler front-end |
| 🚀 **Headless mode** | Backend supports `--headless` flag enabling CI/CD and web server integration without a TTY |
| 📈 **Interactive visualization** | Canvas-rendered call graph, collapsible AST tree with node inspector, severity-filtered issues panel |
| 🛡️ **Zero C dependencies** | Pure C99 standard library only. No malloc wrappers, no parsing libraries — fully self-contained |
| 🏆 **Priority-ranked reporting** | Issues surfaced by severity via a max-heap — not a flat unsorted dump |
| 🔢 **Real metrics** | 29-function graphs, 65 call edges, 7 recursive patterns — tracked live on actual codebases |

---

## 📊 Comparison with Existing Tools

| Feature | Basic Linters (clang-tidy) | Typical Student Analyzers | **CodeMaster** |
|---|:---:|:---:|:---:|
| Custom C engine | ❌ Wraps LLVM | ❌ Python / Java | ✅ Pure C99 |
| Hand-built data structures | ❌ | ❌ | ✅ HT + Trie + AVL + Graph + PQ |
| AST construction | ✅ via libclang | ⚠️ Partial | ✅ Custom recursive-descent |
| Function call graph | ✅ | ❌ | ✅ With cycle & dead-code detection |
| Control flow graph | ✅ | ❌ | ✅ CFG with DOT export |
| Data flow analysis | ✅ | ❌ | ✅ Read/write variable tracking |
| Priority-ranked issues | ⚠️ Flat list | ❌ | ✅ Max-heap severity queue |
| Stability Score | ❌ | ❌ | ✅ Weighted composite metric |
| Web visualization | ❌ | ❌ | ✅ React dashboard, 5 views |
| JSON export for UI | ❌ | ❌ | ✅ symbols / graph / ast / issues |
| Headless / CI mode | ✅ | ❌ | ✅ `--headless` flag |
| Trie autocomplete | ❌ | ❌ | ✅ O(L) prefix search |
| External dependencies | Heavy (LLVM) | Moderate | ✅ **Zero** |

---

## 🎯 Use Cases

| Domain | How CodeMaster Helps |
|---|---|
| **Code Quality Auditing** | Get a severity-ranked report on nesting depth, function complexity, dead code, and coupling — in seconds |
| **CS Education** | Watch exactly how a C program transforms: tokens → AST → call graph → quality report. Perfect for compilers or DSA courses |
| **Debugging & Refactoring** | Identify highly-coupled functions, deep call chains, and unused variables before making structural changes |
| **Academic Projects** | Demonstrates a complete compiler front-end + data structure portfolio with a polished UI — stands out immediately |
| **Hackathons** | Complete, running, and visually impressive — two tech stacks in one project |
| **Portfolio & Recruitment** | Simultaneously demonstrates mastery of C, algorithms, system design, and full-stack development |

---

## ⚙️ Installation & Setup

### Prerequisites

```
gcc (GCC 11+ recommended)
make
node v18+
npm v9+
```

---

### 🔧 Step 1 — Clone & Build the C Engine

```bash
# Clone the repository
git clone https://github.com/nitinrwt22/CodeMaster.git
cd CodeMaster

# Compile all backend modules (lexer, parser, AST, graph, quality engine, JSON export)
make

# Verify the binary was created
ls -lh master_bin
```

**Interactive CLI mode:**
```bash
./master_bin path/to/your_code.c
```

**Headless mode (for frontend integration):**
```bash
./master_bin path/to/your_code.c --headless
# Writes: output/symbols.json  output/graph.json  output/ast.json  output/issues.json
```

**Clean build:**
```bash
make clean
```

---

### 🌐 Step 2 — Launch the React Dashboard

```bash
cd frontend

# Install dependencies
npm install

# Start the Vite dev server
npm run dev
```

Open **http://localhost:5173** in your browser.

> The Vite dev server includes a **custom middleware plugin** that intercepts `/api/analyze` POST requests — it writes the uploaded source to `target_input.c`, spawns `master_bin --headless`, and serves the resulting JSON files from `/output/` directly to the frontend. No separate Express/Node server needed.

---

## 📁 Project Structure

```
CodeMaster/
│
├── backend/                       # C analysis engine (pure C99, zero external deps)
│   ├── main.c                     # Entry point — interactive CLI + headless mode
│   ├── lexer.c / lexer.h          # Tokenizer → token stream
│   ├── parser.c / parser.h        # Recursive-descent parser → populates all structures
│   ├── ast.c / ast.h              # AST node types, N-ary tree, traversal, analysis
│   ├── hashtable.c / hashtable.h  # Symbol table — hash table with chaining
│   ├── trie.c / trie.h            # Autocomplete — prefix trie
│   ├── avltree.c / avltree.h      # Sorted symbol view — self-balancing AVL tree
│   ├── graph.c / graph.h          # Function call graph — directed adjacency list
│   ├── cfg.c / cfg.h              # Control flow graph
│   ├── quality_analyzer.c / .h    # Issue detection — max-heap priority queue
│   ├── json_export.c / json_export.h  # Structured JSON writer (manual fprintf)
│   └── utils.h                    # Shared constants and macros
│
├── frontend/                      # React + Vite dashboard
│   ├── src/
│   │   ├── pages/
│   │   │   ├── Dashboard.jsx          # Analysis overview — metrics, severity gauge
│   │   │   ├── ASTViewer.jsx          # Interactive collapsible AST + node inspector
│   │   │   ├── FunctionGraph.jsx      # Canvas-rendered call graph visualization
│   │   │   ├── IssuesReport.jsx       # Severity-ranked issue cards + filter
│   │   │   └── SymbolExplorer.jsx     # Live-search symbol table browser
│   │   ├── components/
│   │   │   ├── ui/
│   │   │   │   ├── FileUpload.jsx     # Drag-and-drop glassmorphism uploader
│   │   │   │   ├── IssueCard.jsx      # Severity-colored issue card
│   │   │   │   ├── MetricCard.jsx     # Dashboard stat card
│   │   │   │   ├── Table.jsx          # Reusable data table
│   │   │   │   ├── ASTNode.jsx        # Recursive collapsible AST tree node
│   │   │   │   └── GraphCanvas.jsx    # Canvas API graph renderer
│   │   │   └── layout/               # Sidebar, navbar, layout wrappers
│   │   ├── services/
│   │   │   └── api.js                # Fetch helpers for all JSON endpoints
│   │   ├── App.jsx                   # React Router setup
│   │   └── index.css                 # Global design system — tokens, dark mode
│   ├── vite.config.js                # Vite + custom middleware (file serving + exec)
│   └── package.json
│
├── output/                        # Generated JSON (auto-created on analysis)
│   ├── symbols.json               # All extracted symbols with type, category, line
│   ├── graph.json                 # Function call graph — nodes, edges, metadata
│   ├── ast.json                   # Full AST — N-ary tree structure
│   └── issues.json                # Quality issues — severity-ranked
│
├── Makefile                       # Build system — gcc, -Wall -Wextra, obj/ staging
└── README.md
```

---

## 🖼️ Screenshots

> Upload your own `.c` file to generate live results.

| 📊 Dashboard Overview | 🌳 AST Viewer |
|---|---|
| Metric cards, severity gauge, file metadata | Collapsible tree with node inspector & source map |

| 🔗 Function Call Graph | ⚠️ Issues Report |
|---|---|
| Canvas-rendered call graph with 29 nodes | Severity-grouped cards with Stability Score |

| 🔍 Symbol Explorer | |
|---|---|
| Live-search filterable symbol table | |

---

## 🚀 Future Improvements

| Category | Planned Feature |
|---|---|
| 🤖 **AI Integration** | LLM-powered fix suggestions for each detected quality issue |
| ⚡ **Real-time Analysis** | WebSocket-based live analysis as you type in a built-in code editor |
| 🌍 **Multi-language Support** | Extend the parser to handle C++, Python, and Java source files |
| 📈 **Historical Tracking** | Persist analysis runs and visualize Stability Score trends over time |
| 🔌 **VS Code Extension** | Surface CodeMaster results directly in the editor as inline diagnostics |
| 📦 **Docker Support** | Containerized deployment — single `docker run` to launch the full stack |
| 🧪 **Test Coverage Analysis** | Map test files against analyzed functions to compute test coverage |
| 🌐 **Online Playground** | Browser-based C editor with instant analysis — zero install required |

---

## 👤 Author

<div align="center">

Made with 🖤 by **Nitin Rawat**

*Designed, architected, and built from scratch — C engine to React dashboard.*

[![GitHub](https://img.shields.io/badge/GitHub-nitinrwt22-181717?style=for-the-badge&logo=github)](https://github.com/nitinrwt22)

---

> *Because real engineers understand what happens below the abstraction.*

**CodeMaster** © 2025 Nitin Rawat — All rights reserved.

</div>
