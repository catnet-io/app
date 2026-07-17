import { useState, useEffect, useRef, KeyboardEvent } from 'react';
import { StartScan, StopScan, ParseRange, ExportResults, GetLocalIPRange, Ping, ReverseDNS, ScanPorts } from '../../wailsjs/go/main/App';
import { EventsOn, EventsOff } from '../../wailsjs/runtime/runtime';
import { Play, Square, Terminal, Download, Search } from 'lucide-react';
import nyanImg from '../assets/nyan.png';
import { results, profile } from '../../wailsjs/go/models';

export function ScannerView() {
  const [ipRange, setIpRange] = useState('192.168.1.1-254');
  const [devices, setDevices] = useState<results.DeviceInfo[]>([]);
  const [isScanning, setIsScanning] = useState(false);
  const [progress, setProgress] = useState(0);
  const [logs, setLogs] = useState<{time: string, msg: string}[]>([]);
  const [sortCol, setSortCol] = useState<keyof results.DeviceInfo | ''>('');
  const [sortAsc, setSortAsc] = useState(true);
  const logsEndRef = useRef<HTMLDivElement>(null);

  // Detail panel state
  const [selectedDevice, setSelectedDevice] = useState<results.DeviceInfo | null>(null);
  const [pingStatus, setPingStatus] = useState<string>('');
  const [reverseDnsStatus, setReverseDnsStatus] = useState<string>('');
  const [portScanStatus, setPortScanStatus] = useState<string>('');

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
      setSelectedDevice(null);
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
      setDevices(prev => [...prev, new results.DeviceInfo(host)]);
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

  const handleSort = (col: keyof results.DeviceInfo) => {
    if (sortCol === col) setSortAsc(!sortAsc);
    else { setSortCol(col); setSortAsc(true); }
  };

  const handleSortKeyDown = (e: KeyboardEvent<HTMLTableCellElement>, col: keyof results.DeviceInfo) => {
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
      // Cast list to correct format for binding wrapper
      const path = await ExportResults(devices as any);
      if (path) addLog(`Exported results to: ${path}`);
    } catch (e) {
      addLog(`Failed to export: ${e}`);
    }
  };

  const handleRowClick = (dev: results.DeviceInfo) => {
    setSelectedDevice(dev);
    setPingStatus('');
    setReverseDnsStatus('');
    setPortScanStatus('');
  };

  const handlePing = async (ip: string) => {
    setPingStatus('Pinging...');
    try {
      const ok = await Ping(ip);
      setPingStatus(ok ? 'Online (RTT < 1000ms)' : 'Offline/No Response');
    } catch (e) {
      setPingStatus(`Error: ${e}`);
    }
  };

  const handleReverseDNS = async (ip: string) => {
    setReverseDnsStatus('Querying...');
    try {
      const hostname = await ReverseDNS(ip);
      setReverseDnsStatus(hostname || 'No record found');
      if (hostname) {
        setSelectedDevice(prev => prev ? { ...prev, hostname } : null);
        setDevices(prev => prev.map(d => d.ip === ip ? { ...d, hostname } : d));
      }
    } catch (e) {
      setReverseDnsStatus(`Error: ${e}`);
    }
  };

  const handleScanPorts = async (ip: string) => {
    setPortScanStatus('Scanning...');
    try {
      const ports = [21, 22, 23, 25, 53, 80, 110, 135, 139, 443, 445, 1433, 3306, 3389, 8080];
      const openPorts = await ScanPorts(ip, ports);
      setPortScanStatus(`Done. Found: ${openPorts.join(', ') || 'None'}`);
      setSelectedDevice(prev => prev ? { ...prev, openPorts } : null);
      setDevices(prev => prev.map(d => d.ip === ip ? { ...d, openPorts } : d));
    } catch (e) {
      setPortScanStatus(`Error: ${e}`);
    }
  };

  return (
    <div className="scanner-view" style={{ position: 'relative' }}>
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
              <th onClick={() => handleSort('openPorts')} onKeyDown={(e) => handleSortKeyDown(e, 'openPorts')} tabIndex={0}>
                Ports {sortCol === 'openPorts' && (sortAsc ? '▲' : '▼')}
              </th>
              <th onClick={() => handleSort('mac')} onKeyDown={(e) => handleSortKeyDown(e, 'mac')} tabIndex={0}>
                MAC {sortCol === 'mac' && (sortAsc ? '▲' : '▼')}
              </th>
              <th onClick={() => handleSort('vendor')} onKeyDown={(e) => handleSortKeyDown(e, 'vendor')} tabIndex={0}>
                Vendor {sortCol === 'vendor' && (sortAsc ? '▲' : '▼')}
              </th>
            </tr>
          </thead>
          <tbody>
            {sortedDevices.map((dev, i) => (
              <tr 
                key={i} 
                onClick={() => handleRowClick(dev)} 
                style={{ cursor: 'pointer', background: selectedDevice?.ip === dev.ip ? 'rgba(102, 252, 241, 0.15)' : undefined }}
              >
                <td><span className={`status-dot ${dev.isAlive ? 'status-alive' : 'status-dead'}`} role="img" aria-label={dev.isAlive ? 'Device is online' : 'Device is offline'} title={dev.isAlive ? 'Online' : 'Offline'}></span></td>
                <td>{dev.hostname || '--'}</td>
                <td>{dev.ip}</td>
                <td>{dev.openPorts?.join(', ') || 'None'}</td>
                <td>{dev.mac || '--'}</td>
                <td>{dev.vendor || '--'}</td>
              </tr>
            ))}
            {devices.length === 0 && (
              <tr>
                <td colSpan={6} style={{ textAlign: 'center', padding: '30px', color: 'var(--text-muted)' }}>
                  {isScanning ? 'Scanning network...' : 'Ready to scan. Awaiting input.'}
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>

      {/* Host Details Side Drawer */}
      <div className={`glass-panel host-details-drawer ${selectedDevice ? 'open' : ''}`}>
        {selectedDevice && (
          <>
            <div className="drawer-header">
              <span className="drawer-title">Host Details</span>
              <button className="drawer-close" onClick={() => setSelectedDevice(null)} title="Close Panel">
                ✕
              </button>
            </div>
            <div className="drawer-body">
              <div className="detail-section">
                <div className="detail-row">
                  <span className="detail-label">IP Address</span>
                  <span className="detail-value">{selectedDevice.ip}</span>
                </div>
                <div className="detail-row">
                  <span className="detail-label">Hostname</span>
                  <span className="detail-value">{selectedDevice.hostname || '--'}</span>
                </div>
                <div className="detail-row">
                  <span className="detail-label">MAC Address</span>
                  <span className="detail-value">{selectedDevice.mac || '--'}</span>
                </div>
                <div className="detail-row">
                  <span className="detail-label">Vendor</span>
                  <span className="detail-value">{selectedDevice.vendor || '--'}</span>
                </div>
                <div className="detail-row">
                  <span className="detail-label">OS Heuristic</span>
                  <span className="detail-value">{selectedDevice.os ? `${selectedDevice.os} (${selectedDevice.osFamily || 'unknown'})` : '--'}</span>
                </div>
                <div className="detail-row">
                  <span className="detail-label">Device Type</span>
                  <span className="detail-value">{selectedDevice.deviceType || '--'}</span>
                </div>
                <div className="detail-row">
                  <span className="detail-label">Open Ports</span>
                  <span className="detail-value">{selectedDevice.openPorts?.join(', ') || 'None'}</span>
                </div>
              </div>

              <div className="detail-section">
                <span className="drawer-title" style={{ fontSize: '13px', marginBottom: '8px' }}>Quick Actions</span>
                <div className="quick-tools-grid">
                  <button className="cyber-btn tool-btn" onClick={() => handlePing(selectedDevice.ip)}>
                    Ping
                  </button>
                  <button className="cyber-btn tool-btn" onClick={() => handleReverseDNS(selectedDevice.ip)}>
                    Reverse DNS
                  </button>
                  <button className="cyber-btn tool-btn" onClick={() => handleScanPorts(selectedDevice.ip)} style={{ gridColumn: 'span 2' }}>
                    Scan Common Ports
                  </button>
                </div>
              </div>

              <div className="detail-section" style={{ marginTop: 'auto' }}>
                {pingStatus && (
                  <div className="detail-row">
                    <span className="detail-label">Ping Status</span>
                    <span className="detail-value">{pingStatus}</span>
                  </div>
                )}
                {reverseDnsStatus && (
                  <div className="detail-row">
                    <span className="detail-label">DNS Record</span>
                    <span className="detail-value">{reverseDnsStatus}</span>
                  </div>
                )}
                {portScanStatus && (
                  <div className="detail-row">
                    <span className="detail-label">Port Scan</span>
                    <span className="detail-value">{portScanStatus}</span>
                  </div>
                )}
              </div>
            </div>
          </>
        )}
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
