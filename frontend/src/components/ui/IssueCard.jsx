import { AlertTriangle, AlertCircle, Info, Bug } from 'lucide-react';
import './IssueCard.css';

const severityConfig = {
  critical: { icon: AlertTriangle, badgeColor: 'badge-critical' },
  high: { icon: AlertCircle, badgeColor: 'badge-high' },
  medium: { icon: Bug, badgeColor: 'badge-medium' },
  low: { icon: Info, badgeColor: 'badge-low' },
};

export default function IssueCard({ issue }) {
  const { severity_label, type, function: funcName, line, message } = issue;
  const config = severityConfig[severity_label] || severityConfig.medium;
  const Icon = config.icon;

  return (
    <div className="ethereal-card issue-card">
      <div className="issue-header">
        <div className="issue-title">
          <Icon size={18} className={`icon-${severity_label}`} />
          <span className={`badge ${config.badgeColor}`}>{severity_label}</span>
          <span className="issue-type">{type.replace(/_/g, ' ')}</span>
        </div>
        <div className="issue-location">
          <span className="loc-func">{funcName}</span>
          <span className="loc-line">Line {line}</span>
        </div>
      </div>
      <div className="issue-body">
        <p>{message}</p>
      </div>
    </div>
  );
}
