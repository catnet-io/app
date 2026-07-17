package handlers

import (
	"github.com/catnet-io/engine/pkg/scan"
)

// Ping wrapper for Quick Tools
func (a *AppHandlers) Ping(ip string) bool {
	return scan.Ping(ip, 1000)
}

// ReverseDNS wrapper
func (a *AppHandlers) ReverseDNS(ip string) string {
	return scan.ReverseDNS(ip)
}

// GetMAC wrapper
func (a *AppHandlers) GetMAC(ip string) string {
	return scan.GetMAC(ip)
}

// ScanPorts wrapper
func (a *AppHandlers) ScanPorts(ip string, ports []int) []int {
	return scan.ScanPorts(ip, ports, 500)
}
