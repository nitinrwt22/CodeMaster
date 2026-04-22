import { NavLink } from 'react-router-dom';
import { LayoutDashboard, ListTree, Activity, Bug, Binary } from 'lucide-react';
import logo from '../../assets/logo.png';
import './Sidebar.css';

const navItems = [
  { path: '/', label: 'Dashboard', icon: LayoutDashboard },
  { path: '/symbols', label: 'Symbol Explorer', icon: Binary },
  { path: '/graph', label: 'Functions Graph', icon: Activity },
  { path: '/ast', label: 'AST Viewer', icon: ListTree },
  { path: '/issues', label: 'Issues Report', icon: Bug },
];

export default function Sidebar() {
  return (
    <aside className="sidebar glass-panel">
      <div className="sidebar-brand">
        <img src={logo} alt="CodeMaster Logo" className="brand-logo-img" />
      </div>
      <nav className="sidebar-nav">
        {navItems.map((item) => (
          <NavLink
            key={item.path}
            to={item.path}
            className={({ isActive }) => `nav-item ${isActive ? 'active' : ''}`}
          >
            <item.icon size={20} className="nav-icon" />
            <span>{item.label}</span>
          </NavLink>
        ))}
      </nav>
      <div className="sidebar-footer">
        <div className="status-indicator">
          <span className="status-dot"></span>
          Analyzer Active
        </div>
      </div>
    </aside>
  );
}
