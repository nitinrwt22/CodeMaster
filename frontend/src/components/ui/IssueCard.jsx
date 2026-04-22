import { AlertTriangle, AlertCircle, Info, Bug, Code2, Zap } from 'lucide-react';
import './IssueCard.css';

// ── Severity config ──────────────────────────────────────────────────────
const SEVERITY_CONFIG = {
  critical: {
    Icon:       AlertTriangle,
    colorClass: 'ic--critical',
    badge:      'badge-critical',
    borderVar:  'rgba(255, 113, 108, 0.22)',
  },
  high: {
    Icon:       AlertCircle,
    colorClass: 'ic--high',
    badge:      'badge-high',
    borderVar:  'rgba(255, 200, 67, 0.18)',
  },
  medium: {
    Icon:       Bug,
    colorClass: 'ic--medium',
    badge:      'badge-medium',
    borderVar:  'rgba(59, 191, 250, 0.15)',
  },
  low: {
    Icon:       Info,
    colorClass: 'ic--low',
    badge:      'badge-low',
    borderVar:  'rgba(105, 246, 184, 0.12)',
  },
  info: {
    Icon:       Info,
    colorClass: 'ic--low',
    badge:      'badge-info',
    borderVar:  'rgba(105, 246, 184, 0.1)',
  },
};

// Map issue type to friendly display titles (fallback if backend title missing)
const TYPE_TITLES = {
  unused_variable:    'Unused Variable',
  deep_nesting:       'Deep Nesting',
  large_function:     'Large Function',
  high_fanout:        'High Fan-out',
  too_many_parameters:'Excessive Parameters',
  high_complexity:    'High Complexity',
  dead_code:          'Dead Code',
  recursive_function: 'Recursive Function',
};

export default function IssueCard({ issue }) {
  const {
    severity_label = 'medium',
    type           = 'unknown',
    title,
    function: funcName = '—',
    line       = 0,
    message    = '',
  } = issue;

  const cfg   = SEVERITY_CONFIG[severity_label] || SEVERITY_CONFIG.medium;
  const { Icon, colorClass, badge } = cfg;

  // Prefer backend-provided title, fall back to type mapping, then raw type
  const displayTitle = title || TYPE_TITLES[type] || type.replace(/_/g, ' ');

  return (
    <div className={`ic ${colorClass}`} style={{ '--ic-border': cfg.borderVar }}>
      {/* Left accent bar */}
      <div className="ic-accent" />

      {/* Main content */}
      <div className="ic-content">
        {/* Header row */}
        <div className="ic-header">
          <div className="ic-title-row">
            <Icon size={16} className="ic-icon" />
            <span className={`badge ${badge} ic-badge`}>{severity_label}</span>
            <span className="ic-title">{displayTitle}</span>
          </div>
          <div className="ic-location">
            <Code2 size={12} className="ic-loc-icon" />
            <span className="ic-func">{funcName}</span>
            <span className="ic-line-sep">·</span>
            <span className="ic-line">Line {line}</span>
          </div>
        </div>

        {/* Body: detail message */}
        <div className="ic-body">
          <p className="ic-message">{message}</p>
        </div>
      </div>
    </div>
  );
}
