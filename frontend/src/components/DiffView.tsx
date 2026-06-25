import { useState, useEffect } from 'react';
import { ArrowLeft, GitCompare } from 'lucide-react';
import { CompareScans, GetScans } from '../../wailsjs/go/main/App';
import { diff, store } from '../../wailsjs/go/models';

export function DiffView({ scanId, onBack }: { scanId: number | null, onBack: () => void }) {
  const [diffData, setDiffData] = useState<diff.HostDiff[]>([]);
  const [loading, setLoading] = useState(false);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (scanId !== null) {
      const loadDiff = async () => {
        setLoading(true);
        setError(null);
        try {
          const scans = await GetScans();
          let latestId = -1;
          for (let s of scans || []) {
             if (s.id !== scanId && s.id > latestId) {
               latestId = s.id;
             }
          }
          if (latestId === -1 && scans && scans.length > 0) {
             latestId = scans[0].id;
          }
          
          if (latestId !== -1) {
             const result = await CompareScans(scanId, latestId);
             setDiffData(result || []);
          } else {
             setError("No other scan to compare with");
          }
        } catch (e: any) {
          setError(e.toString());
        } finally {
          setLoading(false);
        }
      };
      loadDiff();
    }
  }, [scanId]);

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
          Diff: Scan #{scanId}
        </div>
      </div>

      {error && (
        <div style={{ color: 'var(--status-dead)', padding: '20px' }}>{error}</div>
      )}

      {!error && (
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
            {loading ? (
               <tr><td colSpan={4} style={{textAlign: 'center', padding: '20px'}}>Loading...</td></tr>
            ) : diffData.map((diffItem, i) => (
              <tr key={i} className={`diff-row diff-${diffItem.status?.toLowerCase() || 'unknown'}`}>
                <td>
                  <span className={`diff-badge diff-badge-${diffItem.status?.toLowerCase() || 'unknown'}`}>
                    {diffItem.status}
                  </span>
                </td>
                <td><span className="mono-badge">{diffItem.ip}</span></td>
                <td>{diffItem.hostname}</td>
                <td>{diffItem.details}</td>
              </tr>
            ))}
            {!loading && diffData.length === 0 && (
               <tr><td colSpan={4} style={{textAlign: 'center', padding: '20px', color: 'var(--text-muted)'}}>No changes detected.</td></tr>
            )}
          </tbody>
        </table>
      </div>
      )}
    </div>
  );
}
