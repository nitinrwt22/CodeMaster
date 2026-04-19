import { useState, useEffect, useMemo } from 'react';
import { getSymbols } from '../services/api';
import Table from '../components/ui/Table';
import './SymbolExplorer.css';

export default function SymbolExplorer() {
  const [data, setData] = useState([]);
  const [loading, setLoading] = useState(true);
  const [search, setSearch] = useState('');

  useEffect(() => {
    async function fetchData() {
      try {
        const result = await getSymbols();
        setData(result.symbols || []);
      } catch (err) {
        console.error('Failed to fetch symbols', err);
      } finally {
        setLoading(false);
      }
    }
    fetchData();
  }, []);

  const columns = [
    { header: 'Name', accessor: 'name' },
    { header: 'Type', accessor: 'type' },
    { 
      header: 'Category', 
      accessor: 'category',
      render: (val) => <span className="cat-badge">{val}</span>
    },
    { header: 'Line', accessor: 'line' },
  ];

  const filteredData = useMemo(() => {
    if (!search) return data;
    const lower = search.toLowerCase();
    return data.filter(s => 
      s.name.toLowerCase().includes(lower) || 
      s.category.toLowerCase().includes(lower)
    );
  }, [data, search]);

  return (
    <div className="symbol-explorer">
      <header className="page-header">
        <h1>Symbol Explorer</h1>
        <p>Identifiers, functions, and variables localized in code.</p>
      </header>
      
      <div className="toolbar">
        <input 
          type="text" 
          className="search-input" 
          placeholder="Search symbols..." 
          value={search}
          onChange={(e) => setSearch(e.target.value)}
        />
      </div>

      <div className="symbol-table-wrapper">
        {loading ? (
          <div>Loading table...</div>
        ) : (
          <Table columns={columns} data={filteredData} />
        )}
      </div>
    </div>
  );
}
