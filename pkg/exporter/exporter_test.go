package exporter

import (
	"catnet_scanner_wails/pkg/scanner"
	"strings"
	"testing"
)

func TestExportCSV(t *testing.T) {
	devices := []scanner.DeviceInfo{
		{IP: "192.168.1.1", Hostname: "router", MAC: "AA-BB-CC-DD-EE-FF", IsAlive: true, OpenPorts: []int{80, 443}},
		{IP: "192.168.1.2", Hostname: "server,backup", MAC: "11-22-33-44-55-66", IsAlive: true, OpenPorts: []int{22}},
		{IP: "192.168.1.3", Hostname: "=cmd|' /C calc'!A0", MAC: "00-11-22-33-44-55", IsAlive: true, OpenPorts: []int{8080}},
		{IP: "192.168.1.4", Hostname: "+1+1", MAC: "00-11-22-33-44-56", IsAlive: true, OpenPorts: []int{8081}},
	}

	out, err := ExportCSV(devices)
	if err != nil {
		t.Fatalf("ExportCSV failed: %v", err)
	}

	str := string(out)
	if !strings.Contains(str, "192.168.1.1,router,AA-BB-CC-DD-EE-FF,Alive,80;443") {
		t.Errorf("Missing correct router line in CSV: %s", str)
	}
	
	// Server has a comma in hostname, it should be escaped properly by encoding/csv
	if !strings.Contains(str, "\"server,backup\"") {
		t.Errorf("Hostname with comma not properly escaped in CSV: %s", str)
	}

	// CSV Injection tests
	if !strings.Contains(str, "192.168.1.3,'=cmd|' /C calc'!A0") {
		t.Errorf("Hostname starting with '=' not properly sanitized against CSV injection: %s", str)
	}
	if !strings.Contains(str, "192.168.1.4,'+1+1") {
		t.Errorf("Hostname starting with '+' not properly sanitized against CSV injection: %s", str)
	}
}

func TestExportXML(t *testing.T) {
	devices := []scanner.DeviceInfo{
		{IP: "192.168.1.1", Hostname: "<script>alert(1)</script>", MAC: "AA", IsAlive: false, OpenPorts: nil},
	}

	out, err := ExportXML(devices)
	if err != nil {
		t.Fatalf("ExportXML failed: %v", err)
	}

	str := string(out)
	if strings.Contains(str, "<script>") {
		t.Errorf("XML Injection vulnerability present: %s", str)
	}
	if !strings.Contains(str, "&lt;script&gt;alert(1)&lt;/script&gt;") {
		t.Errorf("Expected escaped XML but got: %s", str)
	}
}
