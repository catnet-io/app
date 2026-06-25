import { ArrowLeft, GitCompare } from 'lucide-react';

interface MockDiff {
  ip: string;
  hostname: string;
  status: 'NEW' | 'LOST' | 'CHANGED' | 'UNCHANGED';
  details: string;
}

const mockDiffData: MockDiff[] = [
  { ip: "192.168.1.1", hostname: "router.local", status: "UNCHANGED", details: "No changes" },
  { ip: "192.168.1.5", hostname: "unknown", status: "LOST", details: "Host went offline" },
  { ip: "192.168.1.10", hostname: "nas.local", status: "CHANGED", details: "New port opened: 8080" },
  { ip: "192.168.1.50", hostname: "android-phone", status: "NEW", details: "Host came online" },
];

export function DiffView({ scanId, onBack }: { scanId: number | null, onBack: () => void }) {
  if (scanId === null) {
    return (
      <div className="diff-view glass-panel" style={{ padding: '30px', textAlign: 'center', color: 'var(--text-muted)' }}>
        Select a scan from History to compare against the last scan.
      </div>
    );
  }

  return (
    <div className="diff-view">
      <div className="glass-panel header" style={{ padding: '15px 20px', marginBottom: '20px', display: 'flex', gap: '15px', alignItems: 'center' }}>
        <button className="icon-btn" onClick={onBack} title="Back to History">
          <ArrowLeft size={20} />
        </button>
        <div className="header-title" style={{ fontSize: '1.2rem', display: 'flex', alignItems: 'center' }}>
          <GitCompare size={20} style={{ marginRight: '10px' }} />
          Diff: Scan #{scanId} vs Last Scan
        </div>
      </div>

      <div className="glass-panel table-container">
        <table className="cyber-table">
          <thead>
            <tr>
              <th>Status</th>
              <th>IP</th>
              <th>Hostname</th>
              <th>Changes</th>
            </tr>
          </thead>
          <tbody>
            {mockDiffData.map((diff, i) => (
              <tr key={i} className={`diff-row diff-${diff.status.toLowerCase()}`}>
                <td>
                  <span className={`diff-badge diff-badge-${diff.status.toLowerCase()}`}>
                    {diff.status}
                  </span>
                </td>
                <td><span className="mono-badge">{diff.ip}</span></td>
                <td>{diff.hostname}</td>
                <td>{diff.details}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
}
