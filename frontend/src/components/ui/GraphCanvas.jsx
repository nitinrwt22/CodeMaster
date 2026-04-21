import { useRef, useState, useEffect, useCallback } from 'react';

// ── Layout: compute initial node positions with a radial / force spread ──────

function buildLayout(functions) {
  const nodes = [];
  const edges = [];

  const total = functions.length;
  const cx = 520;
  const cy = 340;
  const radius = Math.min(220, 60 + total * 28);

  functions.forEach((fn, i) => {
    let x, y;
    if (fn.name === 'main') {
      x = cx;
      y = cy;
    } else {
      const angle = ((i - 1) / (total - 1)) * 2 * Math.PI;
      x = cx + radius * Math.cos(angle);
      y = cy + radius * Math.sin(angle);
    }
    nodes.push({ id: fn.name, x, y, fn });
  });

  functions.forEach((fn) => {
    (fn.calls || []).forEach((callee) => {
      edges.push({ from: fn.name, to: callee });
    });
  });

  return { nodes, edges };
}

// ── Node visual config ────────────────────────────────────────────────────────

function nodeStyle(fn) {
  if (fn.name === 'main')       return { fill: '#0e2a3a', stroke: '#3bbffa', glow: '0 0 18px #3bbffa99', label: '#3bbffa' };
  if (fn.is_recursive)          return { fill: '#2a0d0d', stroke: '#ff6f7e', glow: '0 0 18px #ff6f7e88', label: '#ff6f7e' };
  if (fn.is_dead_code)          return { fill: '#111827', stroke: '#40485d', glow: 'none',               label: '#6d758c' };
  if (fn.is_highly_coupled)     return { fill: '#1a1a0d', stroke: '#ffc843', glow: '0 0 16px #ffc84388', label: '#ffc843' };
  return                               { fill: '#091328', stroke: '#22b1ec', glow: 'none',               label: '#dee5ff' };
}

// ── Bezier control for curved edges ─────────────────────────────────────────

function curvedPath(x1, y1, x2, y2) {
  const mx = (x1 + x2) / 2;
  const my = (y1 + y2) / 2 - 50;
  return `M${x1},${y1} Q${mx},${my} ${x2},${y2}`;
}

// ── Self-loop path ────────────────────────────────────────────────────────────
function selfLoopPath(x, y) {
  return `M${x},${y - 22} C${x + 70},${y - 80} ${x + 70},${y + 80} ${x},${y + 22}`;
}

const NODE_W = 130;
const NODE_H = 38;

export default function GraphCanvas({ functions, selected, onSelect }) {
  const svgRef = useRef(null);
  const [positions, setPositions] = useState(() => buildLayout(functions));

  // Pan / Zoom state
  const [view, setView] = useState({ x: 0, y: 0, scale: 1 });
  const panning = useRef(false);
  const panStart = useRef({ mx: 0, my: 0, vx: 0, vy: 0 });

  // Dragging node
  const dragging = useRef(null);
  const dragOffset = useRef({ dx: 0, dy: 0 });

  // Re-layout when functions change
  useEffect(() => {
    setPositions(buildLayout(functions));
  }, [functions]);

  // ── Convert screen coords → SVG coords ──────────────────────────────────
  const toSVG = useCallback((sx, sy) => {
    return {
      x: (sx - view.x) / view.scale,
      y: (sy - view.y) / view.scale,
    };
  }, [view]);

  // ── Pointer events ───────────────────────────────────────────────────────
  const onSVGMouseDown = (e) => {
    if (e.target === svgRef.current || e.target.classList.contains('bg-rect')) {
      panning.current = true;
      panStart.current = { mx: e.clientX, my: e.clientY, vx: view.x, vy: view.y };
    }
  };

  const onNodeMouseDown = (e, nodeId) => {
    e.stopPropagation();
    const { x, y } = toSVG(e.clientX, e.clientY);
    const node = positions.nodes.find(n => n.id === nodeId);
    dragging.current = nodeId;
    dragOffset.current = { dx: x - node.x, dy: y - node.y };
  };

  const onMouseMove = (e) => {
    if (dragging.current) {
      const { x, y } = toSVG(e.clientX, e.clientY);
      setPositions(prev => ({
        ...prev,
        nodes: prev.nodes.map(n =>
          n.id === dragging.current
            ? { ...n, x: x - dragOffset.current.dx, y: y - dragOffset.current.dy }
            : n
        )
      }));
    } else if (panning.current) {
      setView(v => ({
        ...v,
        x: panStart.current.vx + (e.clientX - panStart.current.mx),
        y: panStart.current.vy + (e.clientY - panStart.current.my),
      }));
    }
  };

  const onMouseUp = () => {
    dragging.current = null;
    panning.current = false;
  };

  const onWheel = (e) => {
    e.preventDefault();
    const delta = e.deltaY > 0 ? 0.9 : 1.1;
    setView(v => ({
      ...v,
      scale: Math.min(3, Math.max(0.3, v.scale * delta)),
    }));
  };

  const onNodeClick = (e, fn) => {
    e.stopPropagation();
    onSelect(fn);
  };

  const resetView = () => setView({ x: 0, y: 0, scale: 1 });

  const zoomIn  = () => setView(v => ({ ...v, scale: Math.min(3, v.scale * 1.2) }));
  const zoomOut = () => setView(v => ({ ...v, scale: Math.max(0.3, v.scale * 0.8) }));

  // ── Build edge map for highlight ─────────────────────────────────────────
  const nodeMap = Object.fromEntries(positions.nodes.map(n => [n.id, n]));

  const transform = `translate(${view.x},${view.y}) scale(${view.scale})`;

  return (
    <div style={{ position: 'relative', width: '100%', height: '100%' }}>
      {/* Zoom controls */}
      <div className="fcg-zoom-controls">
        <button className="fcg-icon-btn" onClick={zoomIn}  title="Zoom in">+</button>
        <button className="fcg-icon-btn" onClick={zoomOut} title="Zoom out">−</button>
        <button className="fcg-icon-btn" onClick={resetView} title="Reset view">⊙</button>
      </div>

      <svg
        ref={svgRef}
        width="100%"
        height="100%"
        style={{ cursor: panning.current ? 'grabbing' : 'grab', display: 'block' }}
        onMouseDown={onSVGMouseDown}
        onMouseMove={onMouseMove}
        onMouseUp={onMouseUp}
        onMouseLeave={onMouseUp}
        onWheel={onWheel}
      >
        <defs>
          {/* Arrowhead marker - normal */}
          <marker id="arrow-normal" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
            <path d="M0,0 L0,6 L8,3 z" fill="#3bbffa55" />
          </marker>
          {/* Arrowhead marker - recursive */}
          <marker id="arrow-recursive" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
            <path d="M0,0 L0,6 L8,3 z" fill="#ff6f7e88" />
          </marker>
          {/* Glow filter */}
          <filter id="glow-blue" x="-40%" y="-40%" width="180%" height="180%">
            <feGaussianBlur stdDeviation="6" result="coloredBlur" />
            <feMerge><feMergeNode in="coloredBlur" /><feMergeNode in="SourceGraphic" /></feMerge>
          </filter>
          <filter id="glow-red" x="-40%" y="-40%" width="180%" height="180%">
            <feGaussianBlur stdDeviation="8" result="coloredBlur" />
            <feMerge><feMergeNode in="coloredBlur" /><feMergeNode in="SourceGraphic" /></feMerge>
          </filter>
          {/* Recursive ring dashes */}
          <filter id="glow-ring" x="-50%" y="-50%" width="200%" height="200%">
            <feGaussianBlur stdDeviation="4" result="coloredBlur" />
            <feMerge><feMergeNode in="coloredBlur" /><feMergeNode in="SourceGraphic" /></feMerge>
          </filter>
        </defs>

        {/* Dark bg rect to capture pan events */}
        <rect className="bg-rect" width="100%" height="100%" fill="transparent" />

        <g transform={transform}>

          {/* ── Draw edges ── */}
          {positions.edges.map((edge, i) => {
            const from = nodeMap[edge.from];
            const to   = nodeMap[edge.to];
            if (!from || !to) return null;

            const isSelf = edge.from === edge.to;
            const fromFn = functions.find(f => f.name === edge.from);
            const isRec  = fromFn?.is_recursive;

            if (isSelf) {
              return (
                <path
                  key={i}
                  d={selfLoopPath(from.x + NODE_W / 2, from.y + NODE_H / 2)}
                  fill="none"
                  stroke={isRec ? '#ff6f7e88' : '#3bbffa55'}
                  strokeWidth="1.5"
                  strokeDasharray={isRec ? '5,4' : '0'}
                  markerEnd={isRec ? 'url(#arrow-recursive)' : 'url(#arrow-normal)'}
                />
              );
            }

            const x1 = from.x + NODE_W / 2;
            const y1 = from.y + NODE_H / 2;
            const x2 = to.x   + NODE_W / 2;
            const y2 = to.y   + NODE_H / 2;

            return (
              <path
                key={i}
                d={curvedPath(x1, y1, x2, y2)}
                fill="none"
                stroke={isRec ? '#ff6f7e66' : '#3bbffa44'}
                strokeWidth={isRec ? 1.5 : 1.2}
                strokeDasharray={isRec ? '6,4' : '0'}
                markerEnd={isRec ? 'url(#arrow-recursive)' : 'url(#arrow-normal)'}
              />
            );
          })}

          {/* ── Draw nodes ── */}
          {positions.nodes.map((node) => {
            const style = nodeStyle(node.fn);
            const isSelected = selected?.name === node.id;
            const isRec = node.fn.is_recursive;
            const glowFilter = node.fn.name === 'main' ? 'url(#glow-blue)' : isRec ? 'url(#glow-red)' : 'none';

            return (
              <g
                key={node.id}
                transform={`translate(${node.x},${node.y})`}
                style={{ cursor: 'pointer' }}
                onMouseDown={(e) => onNodeMouseDown(e, node.id)}
                onClick={(e) => onNodeClick(e, node.fn)}
              >
                {/* Recursive dashed ring */}
                {isRec && (
                  <ellipse
                    cx={NODE_W / 2}
                    cy={NODE_H / 2}
                    rx={NODE_W / 2 + 16}
                    ry={NODE_H / 2 + 16}
                    fill="none"
                    stroke="#ff6f7e"
                    strokeWidth="1.5"
                    strokeDasharray="6,4"
                    opacity="0.55"
                    filter="url(#glow-ring)"
                  />
                )}

                {/* Node rect */}
                <rect
                  x={0} y={0}
                  width={NODE_W}
                  height={NODE_H}
                  rx={8} ry={8}
                  fill={style.fill}
                  stroke={isSelected ? '#69f6b8' : style.stroke}
                  strokeWidth={isSelected ? 2 : 1.5}
                  filter={glowFilter}
                  opacity={node.fn.is_dead_code ? 0.45 : 1}
                />

                {/* Node label */}
                <text
                  x={NODE_W / 2}
                  y={NODE_H / 2 + 1}
                  textAnchor="middle"
                  dominantBaseline="middle"
                  fontSize="12"
                  fontFamily="'Fira Code', monospace"
                  fontWeight={node.fn.name === 'main' ? '700' : '500'}
                  fill={isSelected ? '#69f6b8' : style.label}
                  opacity={node.fn.is_dead_code ? 0.5 : 1}
                >
                  {node.id}
                </text>

                {/* Recursive badge */}
                {isRec && (
                  <g transform={`translate(${NODE_W - 4}, ${NODE_H - 4})`}>
                    <rect x={-44} y={-14} width={44} height={14} rx={4} fill="#9f0519" opacity="0.9" />
                    <text x={-22} y={-4} textAnchor="middle" fontSize="8" fill="#ffa8a3" fontWeight="700" fontFamily="'Fira Code', monospace">RECURSIVE</text>
                  </g>
                )}

                {/* Dead code badge */}
                {node.fn.is_dead_code && (
                  <g transform={`translate(${NODE_W - 4}, ${NODE_H - 4})`}>
                    <rect x={-52} y={-14} width={52} height={14} rx={4} fill="#1a1f2e" opacity="0.9" />
                    <text x={-26} y={-4} textAnchor="middle" fontSize="8" fill="#6d758c" fontWeight="700" fontFamily="'Fira Code', monospace">DEAD CODE</text>
                  </g>
                )}
              </g>
            );
          })}
        </g>
      </svg>
    </div>
  );
}
