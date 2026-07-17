package store

import (
	"testing"
	"time"

	"github.com/catnet-io/engine/pkg/profile"
	"github.com/catnet-io/engine/pkg/results"
)

func TestStore_SaveAndGet(t *testing.T) {
	db, err := NewSQLiteStore(":memory:")
	if err != nil {
		t.Fatalf("failed to open memory db: %v", err)
	}
	defer db.Close()

	report := &results.ScanReport{
		SchemaVersion: "2.0.0",
		StartTime:     time.Now().Add(-time.Minute),
		EndTime:       time.Now(),
		Total:         2,
		Alive:         2,
		Devices: []results.DeviceInfo{
			{IP: "192.168.1.1", IsAlive: true, Hostname: "router", MAC: "AA:BB:CC", OpenPorts: []int{80, 443}},
			{IP: "192.168.1.10", IsAlive: true, Hostname: "nas", MAC: "DD:EE:FF", OpenPorts: []int{22}},
		},
	}

	id, err := db.SaveReport("192.168.1.0/24", report)
	if err != nil {
		t.Fatalf("failed to save report: %v", err)
	}

	if id != 1 {
		t.Errorf("expected id 1, got %d", id)
	}

	summaries, err := db.GetScans()
	if err != nil {
		t.Fatalf("failed to get scans: %v", err)
	}
	if len(summaries) != 1 {
		t.Fatalf("expected 1 summary, got %d", len(summaries))
	}

	fetched, err := db.GetReport(id)
	if err != nil {
		t.Fatalf("failed to get report: %v", err)
	}

	if len(fetched.Devices) != 2 {
		t.Fatalf("expected 2 devices, got %d", len(fetched.Devices))
	}
	
	if fetched.Devices[0].IP != "192.168.1.1" {
		t.Errorf("expected IP 192.168.1.1, got %s", fetched.Devices[0].IP)
	}
	if len(fetched.Devices[0].OpenPorts) != 2 {
		t.Errorf("expected 2 open ports, got %v", fetched.Devices[0].OpenPorts)
	}

	err = db.DeleteScan(id)
	if err != nil {
		t.Fatalf("failed to delete scan: %v", err)
	}

	summaries, _ = db.GetScans()
	if len(summaries) != 0 {
		t.Fatalf("expected 0 summaries after deletion, got %d", len(summaries))
	}
}

func TestStore_Profiles(t *testing.T) {
	db, err := NewSQLiteStore(":memory:")
	if err != nil {
		t.Fatalf("failed to open memory db: %v", err)
	}
	defer db.Close()

	prof := profile.ScanProfile{
		Concurrency:  32,
		TimeoutMs:    500,
		DefaultPorts: []int{80, 443},
	}

	id, err := db.SaveProfile("Test Profile", prof)
	if err != nil {
		t.Fatalf("failed to save profile: %v", err)
	}

	if id != 1 {
		t.Errorf("expected profile id 1, got %d", id)
	}

	profiles, err := db.GetProfiles()
	if err != nil {
		t.Fatalf("failed to get profiles: %v", err)
	}
	if len(profiles) != 1 {
		t.Fatalf("expected 1 profile, got %d", len(profiles))
	}
	if profiles[0].Name != "Test Profile" {
		t.Errorf("expected name 'Test Profile', got %s", profiles[0].Name)
	}
	if profiles[0].Profile.Concurrency != 32 {
		t.Errorf("expected concurrency 32, got %d", profiles[0].Profile.Concurrency)
	}

	err = db.DeleteProfile(id)
	if err != nil {
		t.Fatalf("failed to delete profile: %v", err)
	}

	profiles, err = db.GetProfiles()
	if err != nil {
		t.Fatalf("failed to get profiles: %v", err)
	}
	if len(profiles) != 0 {
		t.Errorf("expected 0 profiles after deletion, got %d", len(profiles))
	}
}
