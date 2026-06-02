package scanner

import (
	"encoding/binary"
	"fmt"
	"net"
	"strings"
)

// ParseRange parses a CIDR (e.g. 192.168.1.0/24) or a dash range (192.168.1.1-192.168.1.254)
// and returns a slice of all IP strings in that range.
func ParseRange(input string) ([]string, error) {
	input = strings.TrimSpace(input)
	if input == "" {
		return nil, fmt.Errorf("empty input")
	}

	if strings.Contains(input, "/") {
		return parseCIDR(input)
	}
	if strings.Contains(input, "-") {
		return parseDashRange(input)
	}

	// Single IP fallback
	parsed := net.ParseIP(input)
	if parsed == nil {
		return nil, fmt.Errorf("invalid IP format")
	}
	return []string{parsed.String()}, nil
}

func parseCIDR(cidr string) ([]string, error) {
	ip, ipnet, err := net.ParseCIDR(cidr)
	if err != nil {
		return nil, err
	}

	ones, bits := ipnet.Mask.Size()
	// Prevent DoS: if the subnet is too large (more than /16 for IPv4 or equivalent), return an error
	// A /16 network has 65536 addresses, which is our max limit.
	if bits-ones > 16 {
		return nil, fmt.Errorf("CIDR range too large (max 65536)")
	}

	var ips []string
	for ip := ip.Mask(ipnet.Mask); ipnet.Contains(ip); inc(ip) {
		ips = append(ips, ip.String())
		// Backup safeguard
		if len(ips) > 65536 {
			return nil, fmt.Errorf("CIDR range too large (max 65536)")
		}
	}

	// Remove network address and broadcast address for standard IPv4 subnets
	if len(ips) > 2 {
		return ips[1 : len(ips)-1], nil
	}
	return ips, nil
}

func parseDashRange(dashStr string) ([]string, error) {
	parts := strings.Split(dashStr, "-")
	if len(parts) != 2 {
		return nil, fmt.Errorf("invalid range format, expected start-end")
	}

	startStr := strings.TrimSpace(parts[0])
	endStr := strings.TrimSpace(parts[1])

	// Handle short-hand ending like 192.168.1.1-254
	if !strings.Contains(endStr, ".") {
		lastDot := strings.LastIndex(startStr, ".")
		if lastDot != -1 {
			endStr = startStr[:lastDot+1] + endStr
		}
	}

	startIP := net.ParseIP(startStr).To4()
	endIP := net.ParseIP(endStr).To4()

	if startIP == nil || endIP == nil {
		return nil, fmt.Errorf("invalid IP in range")
	}

	start := binary.BigEndian.Uint32(startIP)
	end := binary.BigEndian.Uint32(endIP)

	if start > end {
		return nil, fmt.Errorf("start IP is greater than end IP")
	}

	// Limit to prevent memory exhaustion on crazy ranges
	if end-start > 65536 {
		return nil, fmt.Errorf("range too large (max 65536)")
	}

	var ips []string
	for i := start; i <= end; i++ {
		ip := make(net.IP, 4)
		binary.BigEndian.PutUint32(ip, i)
		ips = append(ips, ip.String())
	}

	return ips, nil
}

func inc(ip net.IP) {
	for j := len(ip) - 1; j >= 0; j-- {
		ip[j]++
		if ip[j] > 0 {
			break
		}
	}
}
