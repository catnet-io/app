import { useState, useEffect } from 'react';
import { Database, Play, Trash2, Download } from 'lucide-react';
import { GetScans, DeleteScan } from '../../wailsjs/go/main/App';
import { store } from '../../wailsjs/go/models';

export function HistoryView({ onCompare }: { onCompare: (scanId: number) => void }) {
  const [scans, setScans] = useState<store.ScanSummary[]>([]);
  const [loading, setLoading] = useState(true);

  const fetchScans = async () => {
    try {
      const data = await GetScans();
      setScans(data || []);
    } catch (e) {
      console.error("Failed to load scans", e);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchScans();
  }, []);

  const handleDelete = async (id: number) => {
    if (!window.confirm(`Are you sure you want to delete scan #${id}? This action cannot be undone.`)) {
      return;
    }
    try {
      await DeleteScan(id);
      fetchScans();
    } catch (e) {
      console.error("Failed to delete", e);
    }
  };

  return (
    <div className="history-view">
      <div className="glass-panel header" style={{ padding: '15px 20px', marginBottom: '20px' }}>
        <div className="header-title" style={{ fontSize: '1.2rem', display: 'flex', alignItems: 'center' }}>
          <Database size={20} style={{ marginRight: '10px' }} />
          Scan History
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
              <th>Total Hosts</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan={6} style={{ textAlign: 'center', padding: '30px' }}>Loading...</td></tr>
            ) : scans.map((scan) => (
              <tr key={scan.id}>
                <td style={{ color: 'var(--text-muted)' }}>#{scan.id}</td>
                <td>{new Date(scan.start_time).toLocaleString()}</td>
                <td><span className="mono-badge">{scan.target}</span></td>
                <td><span className="status-dot status-alive"></span> {scan.alive_hosts}</td>
                <td>{scan.total_hosts}</td>
                <td style={{ display: 'flex', gap: '8px' }}>
                  <button className="icon-btn" aria-label={`Compare scan #${scan.id}`} title="Compare against last scan" onClick={() => onCompare(scan.id)}>
                    <Play size={16} /> Diff
                  </button>
                  <button className="icon-btn" aria-label={`Export scan #${scan.id}`} title="Export JSON">
                    <Download size={16} />
                  </button>
                  <button className="icon-btn" aria-label={`Delete scan #${scan.id}`} style={{ color: 'var(--status-dead)' }} title="Delete" onClick={() => handleDelete(scan.id)}>
                    <Trash2 size={16} />
                  </button>
                </td>
              </tr>
            ))}
            {!loading && scans.length === 0 && (
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
