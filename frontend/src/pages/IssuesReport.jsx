import { useState, useEffect, useMemo } from 'react';
import { getIssues } from '../services/api';
import IssueCard from '../components/ui/IssueCard';
import './IssuesReport.css';

// ── Constants ──────────────────────────────────────────────────────────────
const SEVERITY_GROUPS = [
  {
    key: 'critical',
    label: 'Critical',
    severities: [5],
    icon: '✖',
    color: 'critical',
    description: 'Immediate action required',
  },
  {
    key: 'high',
    label: 'High',
    severities: [4],
    icon: '!',
    color: 'high',
    description: 'Should be fixed soon',
  },
  {
    key: 'medium',
    label: 'Medium',
    severities: [3],
    icon: '~',
    color: 'medium',
    description: 'Moderate risk — plan a fix',
  },
  {
    key: 'low',
    label: 'Low',
    severities: [2, 1],
    icon: '·',
    color: 'low',
    description: 'Minor / informational',
  },
];

// ── Summary Card ───────────────────────────────────────────────────────────
function SummaryCard({ summary, total }) {
  const score = summary?.stability_score ?? 100;
  const scoreColor =
    score >= 80 ? 'score--good' :
    score >= 50 ? 'score--warn' :
                  'score--bad';

  const scoreLabel =
    score >= 80 ? 'Stable' :
    score >= 50 ? 'Needs Attention' :
                  'Critical Risk';

  const arc = Math.max(0, Math.min(100, score));
  // SVG circle: r=38, circumference≈239
  const circumference = 2 * Math.PI * 38;
  const dashOffset = circumference - (arc / 100) * circumference;

  return (
    <div className="ir-summary-card">
      {/* Left: counts */}
      <div className="ir-summary-left">
        <div className="ir-summary-heading">
          <span className="ir-eyebrow">Static Analysis</span>
          <h1>Code Quality<br />Dashboard</h1>
          <p className="ir-sub">Ranked analysis issues requiring developer attention.</p>
        </div>

        <div className="ir-summary-counts">
          <div className="ir-count-item ir-count--total">
            <span className="ir-count-num">{total}</span>
            <span className="ir-count-label">Total Issues</span>
          </div>
          <div className="ir-count-item ir-count--critical">
            <span className="ir-count-num">{summary?.critical ?? 0}</span>
            <span className="ir-count-label">Critical</span>
          </div>
          <div className="ir-count-item ir-count--high">
            <span className="ir-count-num">{summary?.high ?? 0}</span>
            <span className="ir-count-label">High</span>
          </div>
          <div className="ir-count-item ir-count--medium">
            <span className="ir-count-num">{summary?.medium ?? 0}</span>
            <span className="ir-count-label">Medium</span>
          </div>
          <div className="ir-count-item ir-count--low">
            <span className="ir-count-num">{summary?.low ?? 0}</span>
            <span className="ir-count-label">Low</span>
          </div>
        </div>
      </div>

      {/* Right: score gauge */}
      <div className="ir-score-wrap">
        <div className={`ir-score-gauge ${scoreColor}`}>
          <svg viewBox="0 0 90 90" className="ir-score-svg">
            {/* Track */}
            <circle cx="45" cy="45" r="38" fill="none"
              strokeWidth="7" stroke="rgba(255,255,255,0.06)" />
            {/* Fill */}
            <circle cx="45" cy="45" r="38" fill="none"
              strokeWidth="7"
              strokeLinecap="round"
              strokeDasharray={circumference}
              strokeDashoffset={dashOffset}
              transform="rotate(-90 45 45)"
              className="ir-score-arc"
            />
          </svg>
          <div className="ir-score-inner">
            <span className="ir-score-num">{score}</span>
            <span className="ir-score-pct">%</span>
          </div>
        </div>
        <div className="ir-score-label">Stability Score</div>
        <div className={`ir-score-status ${scoreColor}`}>{scoreLabel}</div>
      </div>
    </div>
  );
}

// ── Filter Bar ──────────────────────────────────────────────────────────────
function FilterBar({ active, onChange, counts }) {
  const filters = [
    { key: 'all',      label: 'All',      count: counts.all },
    { key: 'critical', label: 'Critical', count: counts.critical },
    { key: 'high',     label: 'High',     count: counts.high },
    { key: 'medium',   label: 'Medium',   count: counts.medium },
    { key: 'low',      label: 'Low',      count: counts.low },
  ];

  return (
    <div className="ir-filter-bar">
      {filters.map(f => (
        <button
          key={f.key}
          className={`ir-filter-btn ir-filter-btn--${f.key} ${active === f.key ? 'active' : ''}`}
          onClick={() => onChange(f.key)}
        >
          {f.label}
          <span className="ir-filter-count">{f.count}</span>
        </button>
      ))}
    </div>
  );
}

// ── Section Header ──────────────────────────────────────────────────────────
function SectionHeader({ group, count }) {
  return (
    <div className={`ir-section-header ir-section-header--${group.color}`}>
      <div className="ir-section-badge">
        <span className="ir-section-icon">{group.icon}</span>
        <span className="ir-section-label">{group.label.toUpperCase()}</span>
        <span className="ir-section-count">{count}</span>
      </div>
      <span className="ir-section-desc">{group.description}</span>
    </div>
  );
}

// ── Main Page ────────────────────────────────────────────────────────────────
export default function IssuesReport() {
  const [issues, setIssues]   = useState([]);
  const [summary, setSummary] = useState(null);
  const [loading, setLoading] = useState(true);
  const [filter, setFilter]   = useState('all');

  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getIssues();
        setSummary(result.summary || null);
        const sorted = (result.issues || []).sort((a, b) => b.severity - a.severity);
        setIssues(sorted);
      } catch (err) {
        console.error('Failed to fetch issues', err);
      } finally {
        setLoading(false);
      }
    }
    fetchData();
  }, []);

  // Derived counts
  const counts = useMemo(() => ({
    all:      issues.length,
    critical: issues.filter(i => i.severity_label === 'critical').length,
    high:     issues.filter(i => i.severity_label === 'high').length,
    medium:   issues.filter(i => i.severity_label === 'medium').length,
    low:      issues.filter(i => i.severity_label === 'low' || i.severity_label === 'info').length,
  }), [issues]);

  // Filtered set
  const filtered = useMemo(() => {
    if (filter === 'all') return issues;
    if (filter === 'low')
      return issues.filter(i => i.severity_label === 'low' || i.severity_label === 'info');
    return issues.filter(i => i.severity_label === filter);
  }, [issues, filter]);

  if (loading) {
    return (
      <div className="issues-report">
        <div className="ir-loading">
          <div className="ir-loading-spinner" />
          <span>Loading analysis…</span>
        </div>
      </div>
    );
  }

  return (
    <div className="issues-report">
      {/* Summary Banner */}
      <SummaryCard summary={summary} total={issues.length} />

      {/* Filter Bar */}
      <FilterBar active={filter} onChange={setFilter} counts={counts} />

      {/* Issues grouped by severity */}
      {issues.length === 0 ? (
        <div className="ir-no-issues">
          <span className="ir-no-issues-icon">✓</span>
          <h3>No issues detected!</h3>
          <p>Your code passed all quality checks. Well done.</p>
        </div>
      ) : (
        <div className="ir-groups">
          {SEVERITY_GROUPS.map(group => {
            const groupIssues = filtered.filter(i =>
              group.severities.includes(i.severity)
            );
            if (groupIssues.length === 0) return null;

            return (
              <div key={group.key} className={`ir-group ir-group--${group.color}`}>
                <SectionHeader group={group} count={groupIssues.length} />
                <div className="ir-cards">
                  {groupIssues.map((issue, idx) => (
                    <IssueCard key={`${group.key}-${idx}`} issue={issue} />
                  ))}
                </div>
              </div>
            );
          })}
        </div>
      )}
    </div>
  );
}
