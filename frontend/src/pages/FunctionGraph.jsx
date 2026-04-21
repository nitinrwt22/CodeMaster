import { useState, useEffect, useMemo } from 'react';
import { getGraph } from '../services/api';
import GraphCanvas from '../components/ui/GraphCanvas';
import './FunctionGraph.css';

// ── Filter definitions ────────────────────────────────────────────────────────
const FILTERS = [
  { id: 'all',       label: 'All Nodes' },
  { id: 'recursive', label: 'Recursive' },
  { id: 'dead',      label: 'Dead Code' },
  { id: 'coupled',   label: 'Highly Coupled' },
];

export default function FunctionGraph() {
  const [rawData,   setRawData]   = useState(null);
  const [loading,   setLoading]   = useState(true);
  const [error,     setError]     = useState(null);
  const [selected,  setSelected]  = useState(null);
  const [filter,    setFilter]    = useState('all');

  // ── Fetch graph data ────────────────────────────────────────────────────────
  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getGraph();
        setRawData(result);
      } catch (err) {
        console.error('Failed to fetch graph data', err);
        setError(err.message);
      } finally {
        setLoading(false);
      }
    }
    fetchData();
  }, []);

  // ── Compute stats ───────────────────────────────────────────────────────────
  const stats = useMemo(() => {
    if (!rawData) return { functions: 0, calls: 0, recursive: 0, dead: 0 };
    return {
      functions: rawData.total_functions || 0,
      calls:     rawData.total_calls     || 0,
      recursive: (rawData.functions || []).filter(f => f.is_recursive).length,
      dead:      (rawData.functions || []).filter(f => f.is_dead_code).length,
    };
  }, [rawData]);

  // ── Filter functions list ───────────────────────────────────────────────────
  const filteredFunctions = useMemo(() => {
    if (!rawData?.functions) return [];
    switch (filter) {
      case 'recursive': return rawData.functions.filter(f => f.is_recursive);
      case 'dead':      return rawData.functions.filter(f => f.is_dead_code);
      case 'coupled':   return rawData.functions.filter(f => f.is_highly_coupled);
      default:          return rawData.functions;
    }
  }, [rawData, filter]);

  // ── Build "incoming calls" map for inspector ──────────────────────────────
  const incomingMap = useMemo(() => {
    const map = {};
    (rawData?.functions || []).forEach(fn => {
      (fn.calls || []).forEach(callee => {
        if (!map[callee]) map[callee] = [];
        if (!map[callee].includes(fn.name)) map[callee].push(fn.name);
      });
    });
    return map;
  }, [rawData]);

  const incomingCalls = selected ? (incomingMap[selected.name] || []) : [];

  return (
    <div className="fcg-page">

      {/* ── Header ── */}
      <div className="fcg-header">
        <div className="fcg-header-left">
          <h1>Function Call Graph</h1>
          <p>Interactive execution flow · target_input.c</p>
        </div>
        <div className="fcg-header-stats">
          <div className="fcg-stat-chip">
            <span className="fcg-stat-label">Functions</span>
            <span className="fcg-stat-value">{stats.functions}</span>
          </div>
          <div className="fcg-stat-chip">
            <span className="fcg-stat-label">Total Calls</span>
            <span className="fcg-stat-value">{stats.calls}</span>
          </div>
          {stats.recursive > 0 && (
            <div className="fcg-stat-chip">
              <span className="fcg-stat-label">Recursive</span>
              <span className="fcg-stat-value" style={{ color: 'var(--color-error)' }}>{stats.recursive}</span>
            </div>
          )}
        </div>
      </div>

      {/* ── Filter toolbar ── */}
      <div className="fcg-toolbar">
        <span className="fcg-toolbar-label">Filter:</span>
        {FILTERS.map((f, i) => (
          <button
            key={f.id}
            className={`fcg-filter-btn ${filter === f.id ? 'active' : ''}`}
            onClick={() => setFilter(f.id)}
          >
            {f.label}
          </button>
        ))}
        <div className="fcg-toolbar-sep" />
        <span style={{ fontSize: '0.65rem', fontFamily: 'var(--font-mono)', color: 'var(--color-on-surface-variant)' }}>
          Drag nodes · Scroll to zoom · Click to inspect
        </span>
      </div>

      {/* ── Main body ── */}
      <div className="fcg-body">

        {/* ── Left: SVG canvas ── */}
        <div className="fcg-canvas-panel">
          {loading && (
            <div className="fcg-loading">
              <div className="fcg-spinner" />
              <span>Building call graph…</span>
            </div>
          )}

          {error && (
            <div className="fcg-loading" style={{ color: 'var(--color-error)' }}>
              ⚠ Failed to load graph: {error}
            </div>
          )}

          {!loading && !error && filteredFunctions.length > 0 && (
            <GraphCanvas
              key={filter}
              functions={filteredFunctions}
              selected={selected}
              onSelect={setSelected}
            />
          )}

          {!loading && !error && filteredFunctions.length === 0 && (
            <div className="fcg-loading">No nodes match this filter</div>
          )}

          {/* Legend */}
          {!loading && !error && (
            <div className="fcg-legend">
              <div className="fcg-legend-title">Graph Legend</div>
              <div className="fcg-legend-items">
                {[
                  { color: '#3bbffa', border: '#3bbffa', label: 'Active Path' },
                  { color: '#2a0d0d', border: '#ff6f7e', label: 'Recursion Cycle' },
                  { color: '#111827', border: '#40485d', label: 'Dead Code' },
                  { color: '#1a1a0d', border: '#ffc843', label: 'Highly Coupled' },
                ].map(item => (
                  <div key={item.label} className="fcg-legend-row">
                    <span
                      className="fcg-legend-dot"
                      style={{ background: item.color, border: `1.5px solid ${item.border}` }}
                    />
                    <span className="fcg-legend-text">{item.label}</span>
                  </div>
                ))}
                <div className="fcg-legend-row" style={{ marginTop: '0.25rem', borderTop: '1px solid rgba(64,72,93,0.3)', paddingTop: '0.25rem' }}>
                  <span style={{ width: 10, height: 1, background: '#3bbffa44', display: 'inline-block', flexShrink: 0 }} />
                  <span className="fcg-legend-text">Standard Call</span>
                </div>
                <div className="fcg-legend-row">
                  <span style={{ width: 10, height: 1, background: '#ff6f7e66', display: 'inline-block', flexShrink: 0 }} />
                  <span className="fcg-legend-text">Recursive Call</span>
                </div>
              </div>
            </div>
          )}

          {/* Mini NAV-MAP overlay */}
          <div className="fcg-minimap">
            {/* A simplified bird's-eye representation */}
            <svg width="100%" height="100%" viewBox="0 0 200 120">
              {filteredFunctions.map((fn, i) => {
                const cols = 4;
                const cx = 15 + (i % cols) * 47;
                const cy = 15 + Math.floor(i / cols) * 38;
                const color =
                  fn.name === 'main'      ? '#3bbffa' :
                  fn.is_recursive         ? '#ff6f7e' :
                  fn.is_dead_code         ? '#40485d' :
                  fn.is_highly_coupled    ? '#ffc843' :
                                            '#22b1ec';
                return (
                  <rect key={fn.name} x={cx} y={cy} width={38} height={14}
                    rx={3} fill={color} opacity={fn.is_dead_code ? 0.25 : 0.35} />
                );
              })}
            </svg>
            <div className="fcg-minimap-label">NAV-MAP</div>
          </div>
        </div>

        {/* ── Right: Inspector panel ── */}
        <div className="fcg-inspector-col">
          <div className="fcg-inspector-panel">
            <div className="fcg-inspector-header">
              <span className="fcg-inspector-tag">
                {selected ? 'Selected Node' : 'Node Inspector'}
              </span>
              {selected && (
                <button className="fcg-inspector-close" onClick={() => setSelected(null)} title="Close">✕</button>
              )}
            </div>

            {!selected ? (
              <div className="fcg-inspector-empty">
                Click any function node<br />in the graph to inspect it
              </div>
            ) : (
              <div className="fcg-inspector-body">

                {/* Name */}
                <div>
                  <div className="fcg-inspector-fn-name">{selected.name}</div>
                  <div className="fcg-inspector-fn-sub">
                    {selected.return_type ? `Returns: ${selected.return_type}` : 'Function'}
                  </div>
                </div>

                {/* Static Analysis */}
                <div>
                  <div className="fcg-section-label">Static Analysis</div>
                  <div className="fcg-stats-grid">
                    <div className="fcg-stats-block">
                      <div className="fcg-stats-block-label">Call Count</div>
                      <div className={`fcg-stats-block-value ${selected.call_count > 5 ? 'danger' : ''}`}>
                        {selected.call_count ?? selected.out_degree ?? 0}
                      </div>
                    </div>
                    <div className="fcg-stats-block">
                      <div className="fcg-stats-block-label">In-Degree</div>
                      <div className="fcg-stats-block-value">{selected.in_degree ?? 0}</div>
                    </div>
                    <div className="fcg-stats-block">
                      <div className="fcg-stats-block-label">Out-Degree</div>
                      <div className="fcg-stats-block-value">{selected.out_degree ?? 0}</div>
                    </div>
                    <div className="fcg-stats-block">
                      <div className="fcg-stats-block-label">Callees</div>
                      <div className="fcg-stats-block-value">{(selected.calls || []).length}</div>
                    </div>
                  </div>
                </div>

                {/* Location */}
                <div>
                  <div className="fcg-section-label">Location</div>
                  <div className="fcg-location-block">
                    <span className="fcg-location-icon">📄</span>
                    <div className="fcg-location-text">
                      <span className="fcg-location-file">target_input.c</span>
                      <span className="fcg-location-line">Line {selected.line ?? '—'}</span>
                    </div>
                  </div>
                </div>

                {/* Recursion warning */}
                {selected.is_recursive && (
                  <div className="fcg-warning-block">
                    <div className="fcg-warning-title">
                      <span>⚠</span> REF WARNING
                    </div>
                    <div className="fcg-warning-body">
                      <strong style={{ color: '#ff6f7e' }}>{selected.name}</strong> calls itself recursively.
                      Ensure a base case is reached to avoid stack overflow.
                    </div>
                  </div>
                )}

                {/* Dead code warning */}
                {selected.is_dead_code && (
                  <div className="fcg-warning-block" style={{ background: 'rgba(64,72,93,0.1)', borderColor: 'rgba(109,117,140,0.3)' }}>
                    <div className="fcg-warning-title" style={{ color: '#6d758c' }}>
                      <span>⚬</span> DEAD CODE
                    </div>
                    <div className="fcg-warning-body" style={{ color: 'rgba(163,170,196,0.7)' }}>
                      This function is never called. Consider removing it to reduce binary size.
                    </div>
                  </div>
                )}

                {/* Calls made by this function */}
                {selected.calls && selected.calls.length > 0 && (
                  <div>
                    <div className="fcg-section-label">Calls Made ({selected.calls.length})</div>
                    <div className="fcg-calls-list">
                      {selected.calls.map(callee => (
                        <div
                          key={callee}
                          className="fcg-call-row"
                          onClick={() => {
                            const fn = rawData?.functions?.find(f => f.name === callee);
                            if (fn) setSelected(fn);
                          }}
                        >
                          <span className="fcg-call-name">{callee}()</span>
                          <span className="fcg-call-jump">Jump to →</span>
                        </div>
                      ))}
                    </div>
                  </div>
                )}

                {/* Incoming calls */}
                {incomingCalls.length > 0 && (
                  <div>
                    <div className="fcg-section-label">Incoming Calls ({incomingCalls.length})</div>
                    <div className="fcg-calls-list">
                      {incomingCalls.map(caller => (
                        <div
                          key={caller}
                          className="fcg-call-row"
                          onClick={() => {
                            const fn = rawData?.functions?.find(f => f.name === caller);
                            if (fn) setSelected(fn);
                          }}
                        >
                          <span className="fcg-call-name">{caller}()</span>
                          <span className="fcg-call-jump">Jump to →</span>
                        </div>
                      ))}
                    </div>
                  </div>
                )}

              </div>
            )}
          </div>
        </div>

      </div>
    </div>
  );
}
