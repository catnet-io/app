import { useState } from 'react';
import './index.css';
import { ScannerView } from './components/ScannerView';
import { HistoryView } from './components/HistoryView';
import { DiffView } from './components/DiffView';
import { Scan, Database, GitCompare } from 'lucide-react';

function App() {
  const [activeTab, setActiveTab] = useState<'SCANNER' | 'HISTORY' | 'DIFF'>('SCANNER');
  const [selectedScanId, setSelectedScanId] = useState<number | null>(null);

  const handleCompare = (scanId: number) => {
    setSelectedScanId(scanId);
    setActiveTab('DIFF');
  };

  return (
    <div className="app-layout">
      {/* Sidebar / Tabs */}
      <div className="glass-panel sidebar">
        <div className="sidebar-logo">
          <span>CATNET</span>
        </div>
        <div className="nav-items">
          <button 
            className={`nav-btn ${activeTab === 'SCANNER' ? 'active' : ''}`}
            onClick={() => setActiveTab('SCANNER')}
          >
            <Scan size={20} /> Scanner
          </button>
          <button 
            className={`nav-btn ${activeTab === 'HISTORY' ? 'active' : ''}`}
            onClick={() => setActiveTab('HISTORY')}
          >
            <Database size={20} /> History
          </button>
          <button 
            className={`nav-btn ${activeTab === 'DIFF' ? 'active' : ''}`}
            onClick={() => setActiveTab('DIFF')}
            disabled={activeTab !== 'DIFF' && selectedScanId === null}
          >
            <GitCompare size={20} /> Compare
          </button>
        </div>
      </div>

      {/* Main Content Area */}
      <div className="main-content">
        {activeTab === 'SCANNER' && <ScannerView />}
        {activeTab === 'HISTORY' && <HistoryView onCompare={handleCompare} />}
        {activeTab === 'DIFF' && (
          <DiffView 
            scanId={selectedScanId} 
            onBack={() => setActiveTab('HISTORY')} 
          />
        )}
      </div>
    </div>
  );
}

export default App;
