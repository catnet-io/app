package exporter

import (
	"bytes"
	"catnet_scanner_wails/pkg/scanner"
	"encoding/csv"
	"encoding/json"
	"encoding/xml"
	"fmt"
	"strconv"
	"strings"
)

func ExportJSON(devices []scanner.DeviceInfo) ([]byte, error) {
	out, err := json.MarshalIndent(devices, "", "  ")
	if err != nil {
		return nil, fmt.Errorf("failed to encode JSON: %w", err)
	}
	return out, nil
}

func ExportXML(devices []scanner.DeviceInfo) ([]byte, error) {
	type XMLDevice struct {
		IP       string `xml:"ip"`
		Hostname string `xml:"hostname"`
		MAC      string `xml:"mac"`
		Status   string `xml:"status"`
	}
	type XMLResults struct {
		XMLName xml.Name    `xml:"results"`
		Devices []XMLDevice `xml:"device"`
	}

	res := XMLResults{}
	for _, d := range devices {
		status := "Dead"
		if d.IsAlive {
			status = "Alive"
		}
		res.Devices = append(res.Devices, XMLDevice{
			IP:       d.IP,
			Hostname: d.Hostname,
			MAC:      d.MAC,
			Status:   status,
		})
	}

	out, err := xml.MarshalIndent(res, "", "\t")
	if err != nil {
		return nil, fmt.Errorf("failed to encode XML: %w", err)
	}
	data := append([]byte("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"), out...)
	return data, nil
}

func sanitizeCSVField(field string) string {
	if len(field) > 0 {
		firstChar := field[0]
		if firstChar == '=' || firstChar == '+' || firstChar == '-' || firstChar == '@' || firstChar == '\t' || firstChar == '\r' {
			return "'" + field
		}
	}
	return field
}

func ExportCSV(devices []scanner.DeviceInfo) ([]byte, error) {
	var buf bytes.Buffer
	writer := csv.NewWriter(&buf)
	err := writer.Write([]string{"IP", "Hostname", "MAC", "Status", "Open Ports"})
	if err != nil {
		return nil, err
	}

	for _, d := range devices {
		status := "Dead"
		if d.IsAlive {
			status = "Alive"
		}

		var strPorts []string
		for _, p := range d.OpenPorts {
			strPorts = append(strPorts, strconv.Itoa(p))
		}
		ports := strings.Join(strPorts, ";")

		hostname := sanitizeCSVField(d.Hostname)

		err = writer.Write([]string{d.IP, hostname, d.MAC, status, ports})
		if err != nil {
			return nil, err
		}
	}
	writer.Flush()
	return buf.Bytes(), nil
}
