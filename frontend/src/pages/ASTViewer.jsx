import { useState, useEffect } from 'react';
import { getAST } from '../services/api';
import './GraphView.css';

export default function ASTViewer() {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getAST();
        setData(result);
      } catch (err) {
        console.error('Failed to fetch AST data', err);
      } finally {
        setLoading(false);
      }
    }
    fetchData();
  }, []);

  return (
    <div className="ast-view">
      <header className="page-header">
        <h1>AST Viewer</h1>
        <p>Abstract Syntax Tree Explorer (Data preview mode - Visualization Library Pending)</p>
      </header>

      <div className="ethereal-card graph-container">
        {loading ? (
          <div>Loading AST...</div>
        ) : (
          <pre className="data-preview">
            {JSON.stringify(data, null, 2)}
          </pre>
        )}
      </div>
    </div>
  );
}
