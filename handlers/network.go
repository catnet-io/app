package handlers

import (
	"fmt"
	"net"

	"github.com/catnet-io/engine/pkg/targets"
)

// ParseRange expands an IP range string (e.g. 192.168.1.1-254) into a list of IPs.
func (a *AppHandlers) ParseRange(input string) ([]string, error) {
	return targets.ParseRange(input)
}

// GetLocalIPRange attempts to find the primary network interface and returns its CIDR or range.
func (a *AppHandlers) GetLocalIPRange() string {
	// Use UDP dialing to find the preferred outbound IP address
	conn, err := net.Dial("udp", "8.8.8.8:80")
	if err == nil {
		defer conn.Close()
		localAddr := conn.LocalAddr().(*net.UDPAddr)

		addrs, _ := net.InterfaceAddrs()
		for _, addr := range addrs {
			if ipnet, ok := addr.(*net.IPNet); ok && ipnet.IP.To4() != nil {
				if ipnet.IP.Equal(localAddr.IP) {
					ip := ipnet.IP.To4()
					mask := ipnet.Mask
					network := net.IP{ip[0] & mask[0], ip[1] & mask[1], ip[2] & mask[2], ip[3] & mask[3]}

					// If it's a standard /24 subnet, format it nicely as 192.168.X.1-254
					ones, _ := mask.Size()
					if ones == 24 {
						return fmt.Sprintf("%d.%d.%d.1-254", network[0], network[1], network[2])
					}
					return fmt.Sprintf("%s/%d", network.String(), ones)
				}
			}
		}
	}

	// Fallback to loop over interfaces
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		return "192.168.1.1-254"
	}

	for _, addr := range addrs {
		if ipnet, ok := addr.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
			ip := ipnet.IP.To4()
			if ip != nil {
				if ip[0] == 169 && ip[1] == 254 {
					continue
				}

				if ip[0] == 192 || ip[0] == 10 || ip[0] == 172 {
					mask := ipnet.Mask
					ones, _ := mask.Size()
					if ones == 24 {
						return fmt.Sprintf("%d.%d.%d.1-254", ip[0], ip[1], ip[2])
					}
					network := net.IP{ip[0] & mask[0], ip[1] & mask[1], ip[2] & mask[2], ip[3] & mask[3]}
					return fmt.Sprintf("%s/%d", network.String(), ones)
				}
			}
		}
	}

	return "192.168.1.1-254"
}
