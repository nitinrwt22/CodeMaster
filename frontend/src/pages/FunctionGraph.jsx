import { useState, useEffect } from 'react';
import { getGraph } from '../services/api';
import './GraphView.css';

export default function FunctionGraph() {
  const [data, setData] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getGraph();
        setData(result);
      } catch (err) {
        console.error('Failed to fetch graph data', err);
      } finally {
        setLoading(false);
      }
    }
    fetchData();
  }, []);

  return (
    <div className="graph-view">
      <header className="page-header">
        <h1>Function Call Graph</h1>
        <p>Execution flow mapping (Data preview mode - Visualization Library Pending)</p>
      </header>

      <div className="ethereal-card graph-container">
        {loading ? (
          <div>Loading graph data...</div>
        ) : (
          <pre className="data-preview">
            {JSON.stringify(data, null, 2)}
          </pre>
        )}
      </div>
    </div>
  );
}
