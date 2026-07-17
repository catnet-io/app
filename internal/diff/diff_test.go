package diff

import (
	"testing"

	"github.com/catnet-io/engine/pkg/results"
)

func TestCompare(t *testing.T) {
	oldReport := &results.ScanReport{
		Devices: []results.DeviceInfo{
			{IP: "192.168.1.1", Hostname: "router", IsAlive: true, OpenPorts: []int{80, 443}},
			{IP: "192.168.1.5", Hostname: "old-pc", IsAlive: true, OpenPorts: []int{22}},
			{IP: "192.168.1.10", Hostname: "server", IsAlive: true, OpenPorts: []int{8080}},
		},
	}

	newReport := &results.ScanReport{
		Devices: []results.DeviceInfo{
			{IP: "192.168.1.1", Hostname: "router", IsAlive: true, OpenPorts: []int{80, 443}}, // Unchanged
			{IP: "192.168.1.10", Hostname: "server", IsAlive: true, OpenPorts: []int{80}},    // Changed: closed 8080, opened 80
			{IP: "192.168.1.50", Hostname: "new-phone", IsAlive: true, OpenPorts: []int{}},   // New
		},
	}

	diffs := Compare(oldReport, newReport)

	if len(diffs) != 4 {
		t.Fatalf("expected 4 diffs, got %d", len(diffs))
	}

	var hasNew, hasLost, hasChanged, hasUnchanged bool
	for _, d := range diffs {
		switch d.Status {
		case StatusNew:
			if d.IP == "192.168.1.50" {
				hasNew = true
			}
		case StatusLost:
			if d.IP == "192.168.1.5" {
				hasLost = true
			}
		case StatusChanged:
			if d.IP == "192.168.1.10" {
				hasChanged = true
			}
		case StatusUnchanged:
			if d.IP == "192.168.1.1" {
				hasUnchanged = true
			}
		}
	}

	if !hasNew || !hasLost || !hasChanged || !hasUnchanged {
		t.Errorf("missing expected statuses in diff results")
	}
}
