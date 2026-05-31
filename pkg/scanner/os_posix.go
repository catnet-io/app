//go:build !windows

package scanner

import (
	"os/exec"
	"strings"
)

func osPing(ip string, timeoutMs int) bool {
	// macOS/Linux fallback
	cmd := exec.Command("ping", "-c", "1", "-W", "1", ip)
	return cmd.Run() == nil
}

func osGetMAC(ip string) string {
	// Try arp -n to get MAC
	cmd := exec.Command("arp", "-n", ip)
	out, err := cmd.Output()
	if err != nil {
		return ""
	}
	
	// Parses arp output looking for MAC
	lines := strings.Split(string(out), "\n")
	for _, line := range lines {
		if strings.Contains(line, ip) {
			parts := strings.Fields(line)
			for _, p := range parts {
				if strings.Contains(p, ":") && len(p) == 17 {
					return strings.ToUpper(strings.ReplaceAll(p, ":", "-"))
				}
			}
		}
	}
	return ""
}
