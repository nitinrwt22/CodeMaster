import { BrowserRouter, Routes, Route } from 'react-router-dom';
import Layout from './components/layout/Layout';
import Dashboard from './pages/Dashboard';
import SymbolExplorer from './pages/SymbolExplorer';
import FunctionGraph from './pages/FunctionGraph';
import ASTViewer from './pages/ASTViewer';
import IssuesReport from './pages/IssuesReport';

export default function App() {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/" element={<Layout />}>
          <Route index element={<Dashboard />} />
          <Route path="symbols" element={<SymbolExplorer />} />
          <Route path="graph" element={<FunctionGraph />} />
          <Route path="ast" element={<ASTViewer />} />
          <Route path="issues" element={<IssuesReport />} />
        </Route>
      </Routes>
    </BrowserRouter>
  );
}
