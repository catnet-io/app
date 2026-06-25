import { useState } from 'react';
import { Database, Play, Trash2, Download } from 'lucide-react';

interface MockScan {
  id: number;
  date: string;
  target: string;
  hostsAlive: number;
  duration: string;
}

const mockScans: MockScan[] = [
  { id: 4, date: "2026-06-25 09:30:00", target: "192.168.1.1-254", hostsAlive: 12, duration: "3s" },
  { id: 3, date: "2026-06-24 15:45:10", target: "10.0.0.1/24", hostsAlive: 4, duration: "1s" },
  { id: 2, date: "2026-06-23 11:20:00", target: "192.168.1.1-254", hostsAlive: 10, duration: "2.5s" },
  { id: 1, date: "2026-06-20 08:00:00", target: "192.168.1.1-254", hostsAlive: 15, duration: "4s" },
];

export function HistoryView({ onCompare }: { onCompare: (scanId: number) => void }) {
  const [scans, setScans] = useState(mockScans);

  const handleDelete = (id: number) => {
    setScans(scans.filter(s => s.id !== id));
  };

  return (
    <div className="history-view">
      <div className="glass-panel header" style={{ padding: '15px 20px', marginBottom: '20px' }}>
        <div className="header-title" style={{ fontSize: '1.2rem', display: 'flex', alignItems: 'center' }}>
          <Database size={20} style={{ marginRight: '10px' }} />
          Scan History (Mocked)
        </div>
      </div>

      <div className="glass-panel table-container">
        <table className="cyber-table">
          <thead>
            <tr>
              <th>Scan ID</th>
              <th>Date</th>
              <th>Target</th>
              <th>Hosts Alive</th>
              <th>Duration</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {scans.map((scan) => (
              <tr key={scan.id}>
                <td style={{ color: 'var(--text-muted)' }}>#{scan.id}</td>
                <td>{scan.date}</td>
                <td><span className="mono-badge">{scan.target}</span></td>
                <td><span className="status-dot status-alive"></span> {scan.hostsAlive}</td>
                <td>{scan.duration}</td>
                <td style={{ display: 'flex', gap: '8px' }}>
                  <button className="icon-btn" title="Compare against last scan" onClick={() => onCompare(scan.id)}>
                    <Play size={16} /> Diff
                  </button>
                  <button className="icon-btn" title="Export JSON">
                    <Download size={16} />
                  </button>
                  <button className="icon-btn" style={{ color: 'var(--status-dead)' }} title="Delete" onClick={() => handleDelete(scan.id)}>
                    <Trash2 size={16} />
                  </button>
                </td>
              </tr>
            ))}
            {scans.length === 0 && (
              <tr>
                <td colSpan={6} style={{ textAlign: 'center', padding: '30px', color: 'var(--text-muted)' }}>
                  No historical scans found.
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
}
