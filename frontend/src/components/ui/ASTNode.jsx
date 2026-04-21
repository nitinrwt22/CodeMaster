import { useState } from 'react';

// Maps AST node types to their display color class
const NODE_TYPE_COLOR = {
  PROGRAM: 'ast-node--program',
  FUNCTION_DECL: 'ast-node--function',
  VAR_DECL: 'ast-node--var',
  RETURN: 'ast-node--return',
  IF: 'ast-node--control',
  FOR: 'ast-node--control',
  WHILE: 'ast-node--control',
  ASSIGNMENT: 'ast-node--assign',
  EXPRESSION: 'ast-node--expr',
};

const getNodeColorClass = (type) =>
  NODE_TYPE_COLOR[type] || 'ast-node--default';

const NODE_ICONS = {
  PROGRAM: '◈',
  FUNCTION_DECL: 'ƒ',
  VAR_DECL: 'v',
  RETURN: '↩',
  IF: '⎇',
  FOR: '↺',
  WHILE: '↺',
  ASSIGNMENT: '=',
  EXPRESSION: '~',
};

const getNodeIcon = (type) => NODE_ICONS[type] || '•';

export default function ASTNode({ node, depth = 0, onSelect, selectedNode }) {
  const hasChildren = node.children && node.children.length > 0;
  // Root and top-level function decls start expanded, rest collapsed
  const [expanded, setExpanded] = useState(depth < 2);

  const isSelected = selectedNode && selectedNode === node;
  const colorClass = getNodeColorClass(node.type);
  const icon = getNodeIcon(node.type);

  const handleToggle = (e) => {
    e.stopPropagation();
    if (hasChildren) setExpanded((prev) => !prev);
  };

  const handleSelect = (e) => {
    e.stopPropagation();
    onSelect && onSelect(node);
  };

  return (
    <div className="ast-node-wrapper" style={{ '--depth': depth }}>
      {/* Node Row */}
      <div
        className={`ast-node-row ${colorClass} ${isSelected ? 'ast-node-row--selected' : ''}`}
        onClick={handleSelect}
        role="button"
        tabIndex={0}
        onKeyDown={(e) => e.key === 'Enter' && handleSelect(e)}
        aria-expanded={hasChildren ? expanded : undefined}
        aria-selected={isSelected}
      >
        {/* Indentation connector lines */}
        {depth > 0 && (
          <span className="ast-indent" style={{ width: `${depth * 20}px` }} aria-hidden="true" />
        )}

        {/* Expand/Collapse toggle */}
        <button
          className={`ast-toggle ${!hasChildren ? 'ast-toggle--leaf' : ''}`}
          onClick={handleToggle}
          tabIndex={-1}
          aria-label={expanded ? 'Collapse' : 'Expand'}
        >
          {hasChildren ? (
            <span className={`ast-chevron ${expanded ? 'ast-chevron--open' : ''}`}>▶</span>
          ) : (
            <span className="ast-leaf-dot">·</span>
          )}
        </button>

        {/* Node type icon badge */}
        <span className={`ast-type-icon ${colorClass}--icon`} aria-hidden="true">
          {icon}
        </span>

        {/* Node type label */}
        <span className={`ast-type-label ${colorClass}--label`}>
          {node.type}
        </span>

        {/* Node value (if meaningful) */}
        {node.value && node.value !== 'expr' && (
          <span className="ast-value">'{node.value}'</span>
        )}

        {/* Line number badge */}
        {node.line != null && node.line > 0 && (
          <span className="ast-line-badge">line {node.line}</span>
        )}

        {/* Children count hint */}
        {hasChildren && (
          <span className="ast-children-count">{node.children.length}</span>
        )}
      </div>

      {/* Children */}
      {hasChildren && expanded && (
        <div className="ast-children" role="group">
          {node.children.map((child, idx) => (
            <ASTNode
              key={`${child.type}-${child.line}-${idx}`}
              node={child}
              depth={depth + 1}
              onSelect={onSelect}
              selectedNode={selectedNode}
            />
          ))}
        </div>
      )}
    </div>
  );
}
