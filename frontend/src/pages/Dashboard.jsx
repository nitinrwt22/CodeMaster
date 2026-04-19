import { useState, useCallback, useEffect } from 'react';
import { getSymbols, getIssues } from '../services/api';
import MetricCard from '../components/ui/MetricCard';
import FileUpload from '../components/ui/FileUpload';
import { Bug, Activity, ShieldAlert, Binary } from 'lucide-react';
import './Dashboard.css';

export default function Dashboard() {
  const [metrics, setMetrics] = useState({
    symbols: 0,
    issues: 0,
    critical: 0,
    functions: 0
  });

  const [loading, setLoading] = useState(true);

  const fetchData = useCallback(async () => {
    try {
      setLoading(true);
      const [symData, issueData] = await Promise.all([
        getSymbols(),
        getIssues()
      ]);
      
      const functions = symData.symbols ? symData.symbols.filter(s => s.category === 'function').length : 0;
      const critical = issueData.issues ? issueData.issues.filter(i => i.severity_label === 'critical').length : 0;

      setMetrics({
        symbols: symData.total_symbols || 0,
        issues: issueData.total_issues || 0,
        critical,
        functions
      });
    } catch (err) {
      console.error('Failed to load dashboard data', err);
    } finally {
      setLoading(false);
    }
  }, []);

  useEffect(() => {
    fetchData();
  }, [fetchData]);

  return (
    <div className="dashboard-container">
      <header className="page-header">
        <h1>Dashboard</h1>
        <p>CodeMaster Analysis Overview</p>
      </header>

      <FileUpload onAnalysisComplete={fetchData} />
      
      <div className="metrics-grid">
        <MetricCard
          title="Total Symbols"
          value={metrics.symbols}
          label="Variables & Structures"
          icon={Binary}
          glowColor="primary"
        />
        <MetricCard
          title="Functions"
          value={metrics.functions}
          label="Tracked routines"
          icon={Activity}
          glowColor="secondary"
        />
        <MetricCard
          title="Static Issues"
          value={metrics.issues}
          label="Total detected"
          icon={Bug}
          glowColor="medium"
        />
        <MetricCard
          title="Critical Issues"
          value={metrics.critical}
          label="Requires attention"
          icon={ShieldAlert}
          glowColor="critical"
        />
      </div>

      <div className="dashboard-content ethereal-card">
        <h3>System Status</h3>
        <p style={{ marginTop: '1rem', color: 'var(--color-on-surface-variant)' }}>
          {loading ? 'Refreshing data...' : 'All analysis hooks are running locally. Upload a real file to re-run the backend and see live results.'}
        </p>
      </div>
    </div>
  );
}
