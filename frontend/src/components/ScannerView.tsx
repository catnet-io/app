import { useState, useEffect, useRef, KeyboardEvent } from 'react';
import { StartScan, StopScan, ParseRange, ExportResults, GetLocalIPRange } from '../../wailsjs/go/main/App';
import { EventsOn, EventsOff } from '../../wailsjs/runtime/runtime';
import { Play, Square, Terminal, Download, Search } from 'lucide-react';
import nyanImg from '../assets/nyan.png';
import { results, profile } from '../../wailsjs/go/models';

export function ScannerView() {
  const [ipRange, setIpRange] = useState('192.168.1.1-254');
  const [devices, setDevices] = useState<results.HostResult[]>([]);
  const [isScanning, setIsScanning] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<{time: string, msg: string}[]>([]);
  const [sortCol, setSortCol] = useState<keyof results.HostResult | ''>('');
  const [sortAsc, setSortAsc] = useState(true);
  const logsEndRef = useRef<HTMLDivElement>(null);

  const isValidIpRange = (value: string): boolean => {
    const cidrPattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\/\d{1,2}$/;
    const dashPattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}-(\d{1,3}|\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})$/;
    const singlePattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
    return cidrPattern.test(value) || dashPattern.test(value) || singlePattern.test(value);
  };

  const addLog = (msg: string) => {
    setLogs(prev => [...prev, { time: new Date().toLocaleTimeString(), msg }]);
  };

  useEffect(() => {
    logsEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [logs]);

  useEffect(() => {
    EventsOn("scan_started", () => {
      setIsScanning(true);
      setDevices([]);
      setProgress(0);
      addLog("Scan started");
    });
    EventsOn("scan_finished", () => {
      setIsScanning(false);
      setProgress(1);
      addLog("Scan finished");
    });
    EventsOn("scan_progress", (p: number) => {
      setProgress(p);
    });
    EventsOn("scan_result", (host: any) => {
      setDevices(prev => [...prev, new results.HostResult(host)]);
    });
    return () => {
      EventsOff("scan_started");
      EventsOff("scan_finished");
      EventsOff("scan_progress");
      EventsOff("scan_result");
    };
  }, []);

  const handleAutoDetect = async () => {
    try {
      const range = await GetLocalIPRange();
      setIpRange(range);
      addLog(`Auto-detected local subnet: ${range}`);
    } catch (e) {
      addLog(`Failed to auto-detect: ${e}`);
    }
  };

  useEffect(() => {
    handleAutoDetect();
  }, []);


  const handleScan = async () => {
    if (isScanning) return;
    try {
      addLog(`Preparing to scan range: ${ipRange}`);
      const ips = await ParseRange(ipRange); 
      if (!ips || ips.length === 0) {
        addLog("No IPs found in range");
        return;
      }
      addLog(`Found ${ips.length} IPs to scan.`);
      const config = profile.ScanProfile.createFrom({
        name: "default",
        discovery_mode: "icmp+tcp",
        concurrency: 64,
        timeout_ms: 1000,
        resolve_dns: true,
        resolve_mac: true,
        export_formats: ["json"],
        ports: [22, 80, 443, 139, 445, 3389]
      });
      StartScan(ips, config).catch(err => addLog(`Scan error: ${err}`));
    } catch (e) {
      addLog(`Error parsing range: ${e}`);
    }
  };

  const handleStop = () => {
    StopScan();
    addLog("Stop signal sent");
  };

  const handleSort = (col: keyof results.HostResult) => {
    if (sortCol === col) setSortAsc(!sortAsc);
    else { setSortCol(col); setSortAsc(true); }
  };

  const handleSortKeyDown = (e: KeyboardEvent<HTMLTableCellElement>, col: keyof results.HostResult) => {
    if (e.key === 'Enter' || e.key === ' ') {
      e.preventDefault();
      handleSort(col);
    }
  };

  const sortedDevices = [...devices].sort((a, b) => {
    if (!sortCol) return 0;
    let aVal: any = a[sortCol];
    let bVal: any = b[sortCol];
    if (Array.isArray(aVal)) aVal = aVal.length;
    if (Array.isArray(bVal)) bVal = bVal.length;
    if (aVal === undefined) aVal = "";
    if (bVal === undefined) bVal = "";
    if (aVal < bVal) return sortAsc ? -1 : 1;
    if (aVal > bVal) return sortAsc ? 1 : -1;
    return 0;
  });

  const handleExport = async () => {
    if (devices.length === 0) return;
    try {
      const path = await ExportResults(devices);
      if (path) addLog(`Exported results to: ${path}`);
    } catch (e) {
      addLog(`Failed to export: ${e}`);
    }
  };

  return (
    <div className="scanner-view">
      <div className="glass-panel header">
        <div className="header-title">
          <img src={nyanImg} alt="logo" style={{ height: '54px', width: '108px', objectFit: 'cover', objectPosition: 'center', marginRight: '8px', borderRadius: '6px', border: '1px solid rgba(102, 252, 241, 0.3)' }} />
          <div style={{ display: 'flex', flexDirection: 'column', lineHeight: '1.1', justifyContent: 'center' }}>
            <span>CATNET</span>
            <span>SCANNER</span>
          </div>
        </div>
        <div className="header-controls">
          <div className="input-group" style={{ position: 'relative' }}>
            <input 
              className="cyber-input" 
              value={ipRange}
              onChange={e => setIpRange(e.target.value)}
              onKeyDown={(e) => {
                if (e.key === 'Enter') {
                  e.preventDefault();
                  if (!isScanning && isValidIpRange(ipRange)) handleScan();
                }
              }}
              disabled={isScanning}
              placeholder="IP Range / CIDR"
              aria-label="IP Range or CIDR"
              aria-invalid={!isValidIpRange(ipRange) && ipRange !== '' ? 'true' : 'false'}
              style={{ borderColor: !isValidIpRange(ipRange) && ipRange !== '' ? 'var(--status-dead)' : undefined }}
            />
            <button className="icon-btn" onClick={handleAutoDetect} disabled={isScanning} title="Auto Detect Subnet">
              <Search size={16} />
            </button>
          </div>
          <button className="cyber-btn" onClick={handleScan} disabled={isScanning || !isValidIpRange(ipRange)}>
            <Play size={18} /> {isScanning ? 'Scanning...' : 'Start'}
          </button>
          <button className="cyber-btn danger" onClick={handleStop} disabled={!isScanning}>
            <Square size={18} /> Stop
          </button>
          <button className="cyber-btn" onClick={handleExport} disabled={isScanning || devices.length === 0} style={{ borderColor: 'var(--text-muted)', color: 'var(--text-muted)' }}>
            <Download size={18} /> Export
          </button>
        </div>
      </div>

      {isScanning && (
        <div className="progress-container" role="progressbar" aria-valuenow={Math.round(progress * 100)} aria-valuemin={0} aria-valuemax={100}>
          <div className="progress-bar" style={{ width: `${progress * 100}%` }}>
            <img src={nyanImg} alt="nyan" className="nyan-cat-img" />
          </div>
        </div>
      )}

      <div className="glass-panel table-container">
        <table className="cyber-table">
          <thead>
            <tr>
              <th>Status</th>
              <th onClick={() => handleSort('hostname')} onKeyDown={(e) => handleSortKeyDown(e, 'hostname')} tabIndex={0}>
                Hostname {sortCol === 'hostname' && (sortAsc ? '▲' : '▼')}
              </th>
              <th onClick={() => handleSort('ip')} onKeyDown={(e) => handleSortKeyDown(e, 'ip')} tabIndex={0}>
                IP {sortCol === 'ip' && (sortAsc ? '▲' : '▼')}
              </th>
              <th onClick={() => handleSort('open_ports')} onKeyDown={(e) => handleSortKeyDown(e, 'open_ports')} tabIndex={0}>
                Ports {sortCol === 'open_ports' && (sortAsc ? '▲' : '▼')}
              </th>
              <th onClick={() => handleSort('mac')} onKeyDown={(e) => handleSortKeyDown(e, 'mac')} tabIndex={0}>
                MAC {sortCol === 'mac' && (sortAsc ? '▲' : '▼')}
              </th>
            </tr>
          </thead>
          <tbody>
            {sortedDevices.map((dev, i) => (
              <tr key={i}>
                <td><span className={`status-dot ${dev.alive ? 'status-alive' : 'status-dead'}`} role="img" aria-label={dev.alive ? 'Device is online' : 'Device is offline'} title={dev.alive ? 'Online' : 'Offline'}></span></td>
                <td>{dev.hostname || '--'}</td>
                <td>{dev.ip}</td>
                <td>{dev.open_ports?.join(', ') || 'None'}</td>
                <td>{dev.mac || '--'}</td>
              </tr>
            ))}
            {devices.length === 0 && (
              <tr>
                <td colSpan={5} style={{ textAlign: 'center', padding: '30px', color: 'var(--text-muted)' }}>
                  {isScanning ? 'Scanning network...' : 'Ready to scan. Awaiting input.'}
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>

      <div className="glass-panel terminal-panel">
        <div className="terminal-header">
          <Terminal size={12} style={{ marginRight: '6px', display: 'inline' }} /> Debug Log
        </div>
        <div className="terminal-content">
          {logs.map((l, i) => (
            <div key={i} className="log-entry">
              <span className="log-time">[{l.time}]</span><span>{l.msg}</span>
            </div>
          ))}
          <div ref={logsEndRef} />
        </div>
      </div>
    </div>
  );
}
