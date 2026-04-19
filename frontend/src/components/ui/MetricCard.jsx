import './MetricCard.css';

export default function MetricCard({ title, value, label, icon: Icon, glowColor }) {
  return (
    <div className={`ethereal-card metric-card ${glowColor ? 'glow-' + glowColor : ''}`}>
      <div className="metric-header">
        <h3 className="metric-title">{title}</h3>
        {Icon && <Icon className="metric-icon" size={20} />}
      </div>
      <div className="metric-value">{value}</div>
      {label && <div className="metric-label">{label}</div>}
    </div>
  );
}
