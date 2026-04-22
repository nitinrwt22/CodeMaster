import { useState, useCallback, useEffect, useRef } from 'react';
import { getSymbols, getIssues, getGraph, getAST, analyzeCode } from '../services/api';
import { UploadCloud, FileType, CheckCircle, AlertCircle, Share2 } from 'lucide-react';
import './Dashboard.css';

// ── Helpers ───────────────────────────────────────────────────────────────────

/** Compute a simple complexity grade from issue/function ratio */
function computeGrade(issues, functions) {
  if (!functions) return 'N/A';
  const ratio = issues / functions;
  if (ratio === 0)      return 'A+';
  if (ratio < 0.2)     return 'A';
  if (ratio < 0.5)     return 'B+';
  if (ratio < 0.8)     return 'B';
  if (ratio < 1.2)     return 'C+';
  return 'C';
}

function gradeStatus(grade) {
  if (grade.startsWith('A')) return 'stable';
  if (grade.startsWith('B')) return 'warning';
  return 'critical';
}

/** Map issue type to friendly display */
const INSIGHT_META = {
  unused_variable: { title: 'Unused Variable Detected',        tags: ['critical', 'memory'],  cat: 'memory' },
  deep_nesting:    { title: 'Deep Nesting Warning',            tags: ['warn', 'refactor'],    cat: 'refactor' },
  high_fanout:     { title: 'High Fan-Out Function',           tags: ['warn', 'coupling'],    cat: 'coupling' },
  high_complexity: { title: 'Recursive Function Detected',     tags: ['opt', 'memory'],       cat: 'recursion' },
  dead_code:       { title: 'Dead Code Elimination Candidate', tags: ['opt', 'opt'],          cat: 'optimization' },
};

/** Simple sparkbar heights array (0-1 scale, last = current) */
const SPARK_HEIGHTS = [0.25, 0.4, 0.3, 0.55, 0.45, 0.7, 1.0];

// ── Mini Sparkbar ─────────────────────────────────────────────────────────────
function SparkBar({ colorClass }) {
  return (
    <div className="dmc-sparkbar">
      {SPARK_HEIGHTS.map((h, i) => (
        <div
          key={i}
          className={`dmc-bar ${i === SPARK_HEIGHTS.length - 1 ? 'active' : ''}`}
          style={{ height: `${h * 100}%` }}
        />
      ))}
    </div>
  );
}

// ── Enhanced Metric Card ──────────────────────────────────────────────────────
function DashMetricCard({ icon, title, value, status, colorClass }) {
  const statusClass = {
    stable:   'dmc-status--stable',
    warning:  'dmc-status--warning',
    critical: 'dmc-status--critical',
    info:     'dmc-status--info',
  }[status] || 'dmc-status--info';

  const statusLabel = { stable: 'Stable', warning: 'Warning', critical: 'Critical', info: 'Info' }[status] || status;

  return (
    <div className={`dmc dmc--${colorClass}`}>
      <div className="dmc-top">
        <div className="dmc-icon-wrap">{icon}</div>
        <span className={`dmc-status ${statusClass}`}>{statusLabel}</span>
      </div>
      <SparkBar />
      <div className="dmc-title">{title}</div>
      <div className="dmc-value">{value}</div>
    </div>
  );
}

// ── Insight Item ──────────────────────────────────────────────────────────────
const TAG_CLASS = {
  critical: 'dash-insight-tag--critical',
  memory:   'dash-insight-tag--memory',
  opt:      'dash-insight-tag--opt',
  info:     'dash-insight-tag--info',
  warn:     'dash-insight-tag--warn',
  refactor: 'dash-insight-tag--opt',
  coupling: 'dash-insight-tag--info',
};

function InsightItem({ issue, index }) {
  const meta = INSIGHT_META[issue.type] || {
    title: issue.message,
    tags:  ['info'],
    cat:   'general',
  };
  const dotClass = `dash-insight-dot--${issue.severity_label}`;
  const timeLabels = ['just now', '2m ago', '5m ago', '14m ago', '1h ago', '2h ago'];
  const timeLabel  = timeLabels[index % timeLabels.length];

  return (
    <div className="dash-insight-item">
      <div className={`dash-insight-dot ${dotClass}`} />
      <div className="dash-insight-body">
        <div className="dash-insight-toprow">
          <div className="dash-insight-title">
            {meta.title}
            {issue.function && ` in ${issue.function}`}
          </div>
          <span className="dash-insight-time">{timeLabel}</span>
        </div>
        <div className="dash-insight-desc">{issue.message} (line {issue.line})</div>
        <div className="dash-insight-tags">
          {meta.tags.map((t, i) => (
            <span key={i} className={`dash-insight-tag ${TAG_CLASS[t] || 'dash-insight-tag--info'}`}>
              {t.toUpperCase()}
            </span>
          ))}
        </div>
      </div>
    </div>
  );
}

// ── Compact Upload Strip ──────────────────────────────────────────────────────
function UploadStrip({ onDone }) {
  const [isDragging, setIsDragging] = useState(false);
  const [status, setStatus] = useState('idle');
  const fileRef = useRef(null);

  const handleFile = async (file) => {
    if (!file) return;
    setStatus('loading');
    try {
      const text = await file.text();
      await analyzeCode(text);
      setStatus('success');
      onDone?.();
      setTimeout(() => setStatus('idle'), 3000);
    } catch {
      setStatus('error');
      setTimeout(() => setStatus('idle'), 3000);
    }
  };

  const icon = status === 'loading' ? <FileType size={18} />
             : status === 'success' ? <CheckCircle size={18} style={{ color: 'var(--color-secondary)' }} />
             : status === 'error'   ? <AlertCircle size={18} style={{ color: 'var(--color-error)' }} />
             : <UploadCloud size={18} />;

  const headingMap = { idle: 'Upload Source File', loading: 'Analyzing…', success: 'Analysis Complete', error: 'Analysis Failed' };
  const subMap     = { idle: 'Drag & drop a .c / .cpp file or click to browse', loading: 'Running CodeMaster pipeline…', success: 'Dashboard refreshed.', error: 'Error processing file.' };

  return (
    <div
      className={`dash-upload-strip ${isDragging ? 'dragging' : ''}`}
      onClick={() => fileRef.current?.click()}
      onDragOver={(e) => { e.preventDefault(); setIsDragging(true); }}
      onDragLeave={() => setIsDragging(false)}
      onDrop={(e) => { e.preventDefault(); setIsDragging(false); handleFile(e.dataTransfer.files[0]); }}
    >
      <input type="file" accept=".c,.cpp,.h" ref={fileRef} style={{ display: 'none' }}
        onChange={(e) => handleFile(e.target.files[0])} />

      <div className="dash-upload-icon">{icon}</div>
      <div className="dash-upload-text">
        <h4>{headingMap[status]}</h4>
        <p>{subMap[status]}</p>
      </div>

      {status !== 'idle' && (
        <span className={`dash-upload-status dash-upload-status--${status}`}>
          {status === 'loading' ? 'Processing…' : status === 'success' ? '✓ Done' : '✗ Failed'}
        </span>
      )}
    </div>
  );
}

// ── Main Dashboard ────────────────────────────────────────────────────────────
export default function Dashboard() {
  const [metrics, setMetrics] = useState(null);
  const [loading, setLoading] = useState(true);

  const fetchAll = useCallback(async () => {
    try {
      setLoading(true);
      const [symData, issueData, graphData] = await Promise.all([
        getSymbols(),
        getIssues(),
        getGraph(),
      ]);

      const functions  = (graphData.functions || []);
      const symbols    = (symData.symbols || []);
      const issues     = (issueData.issues || []);
      const summary    = issueData.summary || {};
      const fnCount    = graphData.total_functions || functions.length;
      const varCount   = symbols.filter(s => s.category === 'variable').length;
      const issueCount = issueData.total_issues || issues.length;
      const recCount   = functions.filter(f => f.is_recursive).length;
      const deadCount  = functions.filter(f => f.is_dead_code).length;
      const totalCalls = graphData.total_calls || 0;
      const grade      = computeGrade(issueCount, fnCount);
      const stabilityScore = summary.stability_score ?? null;

      setMetrics({ functions, symbols, issues, fnCount, varCount, issueCount, recCount, deadCount, totalCalls, grade, stabilityScore });
    } catch (err) {
      console.error('Dashboard fetch error', err);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => { fetchAll(); }, [fetchAll]);

  const grade          = metrics?.grade ?? '…';
  const gradeS         = gradeStatus(grade);
  const fnCount        = metrics?.fnCount        ?? 0;
  const varCount       = metrics?.varCount       ?? 0;
  const issCount       = metrics?.issueCount     ?? 0;
  const recCount       = metrics?.recCount       ?? 0;
  const deadCount      = metrics?.deadCount      ?? 0;
  const totalCall      = metrics?.totalCalls     ?? 0;
  const symTotal       = metrics?.symbols?.length ?? 0;
  const issues         = metrics?.issues         ?? [];
  const stabilityScore = metrics?.stabilityScore  ?? null;

  // Issue-type distribution for mini bar chart
  const issueDist = {
    critical: issues.filter(i => i.severity_label === 'critical').length,
    high:     issues.filter(i => i.severity_label === 'high').length,
    medium:   issues.filter(i => i.severity_label === 'medium').length,
    low:      issues.filter(i => i.severity_label === 'low').length,
  };
  const maxIssue = Math.max(...Object.values(issueDist), 1);

  // Progress percents
  const nestingPct  = Math.min(100, (deadCount / Math.max(fnCount, 1)) * 100 * 4);
  const complexityPct = Math.min(100, (recCount / Math.max(fnCount, 1)) * 200);
  const coveragePct = Math.min(100, Math.max(0, 100 - (issCount / Math.max(fnCount, 1)) * 20));

  return (
    <div className="dash">

      {/* ── Header ── */}
      <div className="dash-header">
        <div className="dash-header-left">
          <span className="dash-eyebrow">Analysis Overview</span>
          <h1 className="dash-title">Static Analysis <span>Engine</span></h1>
        </div>
        <div className="dash-header-actions">
          <button className="dash-export-btn">
            <Share2 size={13} /> Export
          </button>
        </div>
      </div>

      {/* ── Metrics Row ── */}
      <div className="dash-metrics-row">
        <DashMetricCard
          icon="ƒ"
          title="Total Functions"
          value={loading ? '…' : fnCount}
          status="stable"
          colorClass="primary"
        />
        <DashMetricCard
          icon="[ ]"
          title="Total Variables"
          value={loading ? '…' : varCount}
          status="stable"
          colorClass="secondary"
        />
        <DashMetricCard
          icon="⊕"
          title="Complexity Score"
          value={loading ? '…' : grade}
          status={gradeS}
          colorClass="warning"
        />
        <DashMetricCard
          icon="⚑"
          title="Detected Issues"
          value={loading ? '…' : issCount}
          status={issCount > 4 ? 'critical' : issCount > 1 ? 'warning' : 'stable'}
          colorClass="danger"
        />
      </div>

      {/* ── Body: insights left, analytics right ── */}
      <div className="dash-body">

        {/* ── Left: Summary Insights ── */}
        <div className="dash-insights-panel">
          <div className="dash-insights-header">
            <div className="dash-insights-headings">
              <h2>Summary Insights</h2>
              <p>Real-time heuristics and pattern recognition</p>
            </div>
            <div className="dash-live-badge">
              <span className="dash-live-dot" />
              Live Analysis
            </div>
          </div>

          <div className="dash-insights-list">
            {loading && (
              <div style={{ padding: '2rem', textAlign: 'center', color: 'var(--color-on-surface-variant)', fontSize: '0.8rem' }}>
                Loading insights…
              </div>
            )}
            {!loading && issues.length === 0 && (
              <div style={{ padding: '2rem', textAlign: 'center', color: 'var(--color-on-surface-variant)', fontSize: '0.8rem' }}>
                No issues detected — code looks clean ✓
              </div>
            )}
            {!loading && issues.map((issue, i) => (
              <InsightItem key={i} issue={issue} index={i} />
            ))}
          </div>

          <div className="dash-insights-footer">
            <button className="dash-view-all-btn" onClick={() => window.location.href = '/issues'}>
              View All Insights →
            </button>
          </div>
        </div>

        {/* ── Right column ── */}
        <div className="dash-right-col">

          {/* Code Churn / Analytics */}
          <div className="dash-analytics-panel">
            <div className="dash-analytics-header">
              <div className="dash-analytics-icon">⬡</div>
              <span className="dash-analytics-title">Code Churn</span>
            </div>
            <div className="dash-analytics-body">

              <div className="dash-metric-row">
                <div className="dash-metric-row-header">
                  <span className="dash-metric-row-label">Dead Code</span>
                  <span className="dash-metric-row-value">{deadCount} fn</span>
                </div>
                <div className="dash-progress-track">
                  <div className="dash-progress-fill dash-progress-fill--primary" style={{ width: `${nestingPct}%` }} />
                </div>
              </div>

              <div className="dash-metric-row">
                <div className="dash-metric-row-header">
                  <span className="dash-metric-row-label">Recursion Risk</span>
                  <span className="dash-metric-row-value warn">{recCount > 0 ? `${recCount} recursive` : 'None'}</span>
                </div>
                <div className="dash-progress-track">
                  <div className="dash-progress-fill dash-progress-fill--warn" style={{ width: `${complexityPct}%` }} />
                </div>
              </div>

              <div className="dash-metric-row">
                <div className="dash-metric-row-header">
                  <span className="dash-metric-row-label">Code Coverage Est.</span>
                  <span className="dash-metric-row-value">{coveragePct.toFixed(0)}%</span>
                </div>
                <div className="dash-progress-track">
                  <div className="dash-progress-fill dash-progress-fill--success" style={{ width: `${coveragePct}%` }} />
                </div>
              </div>

              {/* Issue distribution mini bar chart */}
              <div>
                <div
                  style={{ display: 'flex', alignItems: 'flex-end', gap: 5, height: 52 }}
                  aria-label="Issue severity distribution"
                >
                  {[
                    { label: 'CRIT', count: issueDist.critical, color: '#ff6f7e' },
                    { label: 'HIGH', count: issueDist.high,     color: '#ffc843' },
                    { label: 'MED',  count: issueDist.medium,   color: '#3bbffa' },
                    { label: 'LOW',  count: issueDist.low,      color: '#69f6b8' },
                  ].map(({ label, count, color }) => (
                    <div key={label} style={{ flex: 1, display: 'flex', flexDirection: 'column', alignItems: 'center', gap: 3, height: '100%', justifyContent: 'flex-end' }}>
                      <div
                        style={{
                          width: '100%',
                          height: `${Math.ceil((count / maxIssue) * 40) + 4}px`,
                          background: color,
                          opacity: count ? 0.75 : 0.15,
                          borderRadius: '3px 3px 0 0',
                        }}
                      />
                      <span style={{ fontSize: '0.52rem', fontFamily: 'var(--font-mono)', color: 'rgba(163,170,196,0.5)', letterSpacing: '0.04em' }}>
                        {label}
                      </span>
                    </div>
                  ))}
                </div>
                <div className="dash-barchart-sublabel">Activity Last Analysis</div>
              </div>
            </div>
          </div>

          {/* Compiler ENV / System Info */}
          <div className="dash-sysinfo-panel">
            <div className="dash-sysinfo-header">Compiler ENV</div>
            <div className="dash-sysinfo-list">
              {[
                { icon: '⊞', key: 'Total Symbols',    val: symTotal },
                { icon: 'ƒ', key: 'Function Calls',   val: totalCall },
                { icon: '↺', key: 'Recursion Count',  val: recCount },
                { icon: '⬡', key: 'Source Files',     val: '1 C File' },
                { icon: '⚙', key: 'Analyzer',         val: 'CodeMaster v1.0' },
                { icon: '◎', key: 'Stability Score',  val: stabilityScore !== null ? `${stabilityScore}%` : 'N/A' },
              ].map(({ icon, key, val }) => (
                <div key={key} className="dash-sysinfo-row">
                  <div className="dash-sysinfo-icon">{icon}</div>
                  <div className="dash-sysinfo-text">
                    <span className="dash-sysinfo-key">{key}</span>
                    <span className="dash-sysinfo-val">{val}</span>
                  </div>
                </div>
              ))}
            </div>
          </div>

        </div>
      </div>

      {/* ── Upload Strip ── */}
      <UploadStrip onDone={fetchAll} />

    </div>
  );
}
