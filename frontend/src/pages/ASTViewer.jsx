import { useState, useEffect, useMemo } from 'react';
import { getAST } from '../services/api';
import ASTNode from '../components/ui/ASTNode';
import './ASTViewer.css';

// ── Helpers ────────────────────────────────────────────────────────────────

/** Recursively count all nodes in the tree */
function countNodes(node) {
  if (!node) return 0;
  let total = 1;
  if (node.children) {
    for (const child of node.children) total += countNodes(child);
  }
  return total;
}

/** Find max nesting depth */
function maxDepth(node, depth = 0) {
  if (!node || !node.children || node.children.length === 0) return depth;
  return Math.max(...node.children.map((c) => maxDepth(c, depth + 1)));
}

/** Get a display color class string for the inspector */
const INSPECTOR_COLORS = {
  PROGRAM:       { tag: 'ast-tag--primary',   label: 'ROOT' },
  FUNCTION_DECL: { tag: 'ast-tag--secondary', label: 'FUNCTION' },
  VAR_DECL:      { tag: 'ast-tag--primary',   label: 'VARIABLE' },
  RETURN:        { tag: 'ast-tag--error',      label: 'RETURN' },
  IF:            { tag: 'ast-tag--warn',       label: 'BRANCH' },
  FOR:           { tag: 'ast-tag--warn',       label: 'LOOP' },
  WHILE:         { tag: 'ast-tag--warn',       label: 'LOOP' },
  ASSIGNMENT:    { tag: 'ast-tag--neutral',    label: 'ASSIGN' },
  EXPRESSION:    { tag: 'ast-tag--neutral',    label: 'EXPR' },
};

function getInspectorMeta(type) {
  return INSPECTOR_COLORS[type] || { tag: 'ast-tag--neutral', label: type };
}

// ── Component ──────────────────────────────────────────────────────────────

export default function ASTViewer() {
  const [data, setData]       = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError]     = useState(null);
  const [selected, setSelected] = useState(null);
  const [searchQuery, setSearchQuery] = useState('');
  const [allExpanded, setAllExpanded] = useState(false); // hint key to force re-mount

  // Fetch AST data
  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getAST();
        setData(result);
        setSelected(result); // auto-select root
      } catch (err) {
        console.error('Failed to fetch AST data', err);
        setError(err.message);
      } finally {
        setLoading(false);
      }
    }
    fetchData();
  }, []);

  // Computed stats
  const stats = useMemo(() => {
    if (!data) return { nodes: 0, depth: 0 };
    return {
      nodes: countNodes(data),
      depth: maxDepth(data),
    };
  }, [data]);

  // Inspector meta for selected node
  const inspectorMeta = selected ? getInspectorMeta(selected.type) : null;

  // Source info (derive from selected node)
  const sourceLocation = selected
    ? `target_input.c:${selected.line ?? '?'}`
    : '—';

  return (
    <div className="ast-page">
      {/* ── Breadcrumb ── */}
      <nav className="ast-breadcrumb" aria-label="breadcrumb">
        <span>SRC</span>
        <span>›</span>
        <span>CORE</span>
        <span>›</span>
        <strong>TARGET_INPUT.C</strong>
      </nav>

      {/* ── Page header + stats ── */}
      <div className="ast-header-row">
        <div className="ast-header-title">
          <h1>Abstract Syntax Tree</h1>
        </div>
        <div className="ast-stats-row">
          <div className="ast-stat-chip">
            <span className="ast-stat-label">Nesting Depth</span>
            <span className="ast-stat-value">{stats.depth}</span>
          </div>
          <div className="ast-stat-chip">
            <span className="ast-stat-label">Nodes Analyzed</span>
            <span className="ast-stat-value">{stats.nodes.toLocaleString()}</span>
          </div>
        </div>
      </div>

      {/* ── Main body (tree + inspector) ── */}
      <div className="ast-body">

        {/* ── Left: Tree panel ── */}
        <div className="ast-tree-panel">
          {/* Panel top bar */}
          <div className="ast-panel-header">
            <span className="ast-panel-title">Tree Visualization</span>
            <div className="ast-panel-dots" aria-hidden="true">
              <span /><span /><span />
            </div>
            <div className="ast-panel-actions">
              <button
                className="ast-icon-btn"
                title="Collapse all"
                onClick={() => setAllExpanded((k) => !k)}
                aria-label="Toggle expand all"
              >
                ⊟
              </button>
              <button
                className="ast-icon-btn"
                title="Search"
                onClick={() => document.getElementById('ast-search')?.focus()}
                aria-label="Focus search"
              >
                ⌕
              </button>
            </div>
          </div>

          {/* Search bar */}
          <div style={{ padding: '0.5rem 1rem 0', flexShrink: 0 }}>
            <input
              id="ast-search"
              type="text"
              placeholder="Filter nodes by type or value…"
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              style={{
                width: '100%',
                background: 'rgba(6,14,32,0.7)',
                border: '1px solid rgba(64,72,93,0.35)',
                borderRadius: '6px',
                padding: '0.4rem 0.75rem',
                color: 'var(--color-on-surface)',
                fontFamily: 'var(--font-mono)',
                fontSize: '0.78rem',
                outline: 'none',
              }}
              aria-label="Search AST nodes"
            />
          </div>

          {/* Scrollable tree area */}
          <div className="ast-tree-scroll">
            {loading && (
              <div className="ast-loading">
                <div className="ast-spinner" />
                <span>Parsing AST…</span>
              </div>
            )}

            {error && (
              <div className="ast-loading" style={{ color: 'var(--color-error)' }}>
                ⚠ Failed to load AST: {error}
              </div>
            )}

            {!loading && !error && data && (
              <ASTNode
                key={String(allExpanded)} /* re-mount on toggle to reset all expanded states */
                node={data}
                depth={0}
                onSelect={setSelected}
                selectedNode={selected}
              />
            )}
          </div>
        </div>

        {/* ── Right: Inspector col ── */}
        <div className="ast-inspector-col">

          {/* Node Inspector */}
          <div className="ast-inspector-panel">
            <div className="ast-inspector-header">
              <span className="ast-inspector-title">Node Inspector</span>
              <div className="ast-inspector-info-btn" title="Click a tree node to inspect">ℹ</div>
            </div>

            {selected ? (
              <div className="ast-inspector-body">
                {/* Big node type heading */}
                <div className="ast-inspector-node-type">{selected.type}</div>

                {/* Tag row */}
                <div className="ast-inspector-tags">
                  <span className={`ast-tag ${inspectorMeta.tag}`}>
                    {inspectorMeta.label}
                  </span>
                  {selected.value && selected.value !== 'expr' && (
                    <span className="ast-tag ast-tag--neutral">
                      VALUE: {selected.value}
                    </span>
                  )}
                  {selected.children && selected.children.length > 0 && (
                    <span className="ast-tag ast-tag--primary">
                      {selected.children.length} CHILDREN
                    </span>
                  )}
                </div>

                {/* Source location field */}
                <div className="ast-inspector-field">
                  <span className="ast-inspector-field-label">Source Location</span>
                  <div className="ast-inspector-field-value">{sourceLocation}</div>
                </div>

                {/* Value field */}
                {selected.value && (
                  <div className="ast-inspector-field">
                    <span className="ast-inspector-field-label">Identifier / Value</span>
                    <div className="ast-inspector-field-value">{selected.value}</div>
                  </div>
                )}

                {/* Children list */}
                {selected.children && selected.children.length > 0 && (
                  <div className="ast-inspector-field">
                    <span className="ast-inspector-field-label">
                      Direct Children ({selected.children.length})
                    </span>
                    <div className="ast-inspector-children-list">
                      {selected.children.map((child, idx) => (
                        <div
                          key={idx}
                          className="ast-inspector-child-item"
                          role="button"
                          tabIndex={0}
                          onClick={() => setSelected(child)}
                          onKeyDown={(e) => e.key === 'Enter' && setSelected(child)}
                          style={{ cursor: 'pointer' }}
                        >
                          <span
                            className="ast-inspector-child-type"
                            style={{ color: 'var(--color-primary)' }}
                          >
                            {child.type}
                          </span>
                          {child.value && child.value !== 'expr' && (
                            <span className="ast-inspector-child-value">
                              '{child.value}'
                            </span>
                          )}
                          <span
                            className="ast-inspector-child-value"
                            style={{ marginLeft: 'auto' }}
                          >
                            L{child.line}
                          </span>
                        </div>
                      ))}
                    </div>
                  </div>
                )}
              </div>
            ) : (
              <div className="ast-inspector-empty">
                Click a node in the tree<br />to inspect its properties
              </div>
            )}
          </div>

          {/* Legend */}
          <div className="ast-legend-panel">
            <div className="ast-legend-title">Node Type Legend</div>
            <div className="ast-legend-grid">
              {[
                { color: 'var(--color-primary)',   label: 'Program / Root' },
                { color: 'var(--color-secondary)', label: 'Function Decl' },
                { color: '#82d4f7',                label: 'Variable Decl' },
                { color: '#ffc843',                label: 'Control Flow' },
                { color: 'var(--color-tertiary)',  label: 'Return Stmt' },
                { color: 'var(--color-on-surface-variant)', label: 'Expression' },
              ].map(({ color, label }) => (
                <div key={label} className="ast-legend-item">
                  <span
                    className="ast-legend-dot"
                    style={{ background: color }}
                  />
                  <span className="ast-legend-name">{label}</span>
                </div>
              ))}
            </div>
          </div>

        </div>
      </div>
    </div>
  );
}
