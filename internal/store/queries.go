package store

import (
	"database/sql"
	"encoding/json"
	"fmt"
	"time"

	"github.com/catnet-io/engine/pkg/results"
)

func (s *sqliteStore) SaveReport(target string, report *results.ScanReport) (int64, error) {
	if report == nil {
		return 0, fmt.Errorf("report cannot be nil")
	}

	tx, err := s.db.Begin()
	if err != nil {
		return 0, fmt.Errorf("failed to begin transaction: %w", err)
	}
	defer func() { _ = tx.Rollback() }()

	res, err := tx.Exec(`
		INSERT INTO scans (start_time, end_time, target, total_hosts, alive_hosts) 
		VALUES (?, ?, ?, ?, ?)`,
		report.StartTime, report.EndTime, target, report.Total, report.Alive,
	)
	if err != nil {
		return 0, fmt.Errorf("failed to insert scan: %w", err)
	}

	scanID, err := res.LastInsertId()
	if err != nil {
		return 0, fmt.Errorf("failed to get scan id: %w", err)
	}

	stmt, err := tx.Prepare(`
		INSERT INTO devices (scan_id, ip, hostname, mac, open_ports, is_alive) 
		VALUES (?, ?, ?, ?, ?, ?)`)
	if err != nil {
		return 0, fmt.Errorf("failed to prepare device stmt: %w", err)
	}
	defer stmt.Close()

	for _, dev := range report.Devices {
		portsJSON, err := json.Marshal(dev.OpenPorts)
		if err != nil {
			return 0, fmt.Errorf("failed to marshal ports: %w", err)
		}
		
		_, err = stmt.Exec(scanID, dev.IP, dev.Hostname, dev.MAC, string(portsJSON), dev.IsAlive)
		if err != nil {
			return 0, fmt.Errorf("failed to insert device: %w", err)
		}
	}

	if err := tx.Commit(); err != nil {
		return 0, fmt.Errorf("failed to commit transaction: %w", err)
	}

	return scanID, nil
}

func (s *sqliteStore) GetScans() ([]ScanSummary, error) {
	rows, err := s.db.Query(`SELECT id, start_time, end_time, target, total_hosts, alive_hosts FROM scans ORDER BY id DESC`)
	if err != nil {
		return nil, fmt.Errorf("failed to query scans: %w", err)
	}
	defer rows.Close()

	var summaries []ScanSummary
	for rows.Next() {
		var sm ScanSummary
		var start, end time.Time
		if err := rows.Scan(&sm.ID, &start, &end, &sm.Target, &sm.TotalHosts, &sm.AliveHosts); err != nil {
			return nil, fmt.Errorf("failed to scan summary row: %w", err)
		}
		sm.StartTime = start.Format(time.RFC3339)
		sm.EndTime = end.Format(time.RFC3339)
		summaries = append(summaries, sm)
	}

	if err := rows.Err(); err != nil {
		return nil, fmt.Errorf("row iteration error: %w", err)
	}

	return summaries, nil
}

func (s *sqliteStore) GetReport(scanID int64) (*results.ScanReport, error) {
	var start, end time.Time
	var target string
	var total, alive int
	
	err := s.db.QueryRow(`SELECT start_time, end_time, target, total_hosts, alive_hosts FROM scans WHERE id = ?`, scanID).
		Scan(&start, &end, &target, &total, &alive)
	if err == sql.ErrNoRows {
		return nil, fmt.Errorf("scan not found")
	} else if err != nil {
		return nil, fmt.Errorf("failed to query scan: %w", err)
	}

	report := &results.ScanReport{
		SchemaVersion: "2.0.0",
		StartTime:     start,
		EndTime:       end,
		Total:         total,
		Alive:         alive,
		Devices:       []results.DeviceInfo{},
	}

	rows, err := s.db.Query(`SELECT ip, hostname, mac, open_ports, is_alive FROM devices WHERE scan_id = ?`, scanID)
	if err != nil {
		return nil, fmt.Errorf("failed to query devices: %w", err)
	}
	defer rows.Close()

	for rows.Next() {
		var dev results.DeviceInfo
		var portsJSON string
		if err := rows.Scan(&dev.IP, &dev.Hostname, &dev.MAC, &portsJSON, &dev.IsAlive); err != nil {
			return nil, fmt.Errorf("failed to scan device: %w", err)
		}
		
		if portsJSON != "" && portsJSON != "null" {
			if err := json.Unmarshal([]byte(portsJSON), &dev.OpenPorts); err != nil {
				return nil, fmt.Errorf("failed to unmarshal ports: %w", err)
			}
		}
		
		if dev.OpenPorts == nil {
			dev.OpenPorts = []int{}
		}

		report.Devices = append(report.Devices, dev)
	}

	if err := rows.Err(); err != nil {
		return nil, fmt.Errorf("device row iteration error: %w", err)
	}

	return report, nil
}

func (s *sqliteStore) DeleteScan(scanID int64) error {
	// Devices are deleted automatically due to ON DELETE CASCADE 
	// Make sure foreign_keys PRAGMA is enabled, but even if not, we can do it manually or just rely on the DB
	// Let's enable PRAGMA foreign_keys = ON; just in case on init, or manually delete devices here for safety.
	tx, err := s.db.Begin()
	if err != nil {
		return err
	}
	defer func() { _ = tx.Rollback() }()

	if _, err := tx.Exec(`DELETE FROM devices WHERE scan_id = ?`, scanID); err != nil {
		return err
	}
	if _, err := tx.Exec(`DELETE FROM scans WHERE id = ?`, scanID); err != nil {
		return err
	}
	
	return tx.Commit()
}
