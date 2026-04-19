import { useState, useEffect } from 'react';
import { getIssues } from '../services/api';
import IssueCard from '../components/ui/IssueCard';
import './IssuesReport.css';

export default function IssuesReport() {
  const [issues, setIssues] = useState([]);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getIssues();
        // Sort by severity (critical first)
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

  return (
    <div className="issues-report">
      <header className="page-header">
        <h1>Issues Report</h1>
        <p>Ranked analysis issues requiring developer attention.</p>
      </header>

      <div className="issues-list">
        {loading ? (
          <div>Loading issues...</div>
        ) : issues.length === 0 ? (
          <div className="no-issues">No issues found!</div>
        ) : (
          issues.map((issue, idx) => <IssueCard key={idx} issue={issue} />)
        )}
      </div>
    </div>
  );
}
