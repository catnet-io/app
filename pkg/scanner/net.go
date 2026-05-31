package scanner

import (
	"fmt"
	"net"
	"os/exec"
	"runtime"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

// Ping verifica se um host está vivo.
func Ping(ip string, timeoutMs int) bool {
	if runtime.GOOS == "windows" {
		cmd := exec.Command("ping", "-n", "1", "-w", fmt.Sprintf("%d", timeoutMs), ip)
		cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
		err := cmd.Run()
		return err == nil
	} else {
		cmd := exec.Command("ping", "-c", "1", "-W", "1", ip)
		err := cmd.Run()
		return err == nil
	}
}

// ReverseDNS resolve o nome do host a partir do IP.
func ReverseDNS(ip string) string {
	names, err := net.LookupAddr(ip)
	if err == nil && len(names) > 0 {
		return strings.TrimSuffix(names[0], ".")
	}
	return ""
}

// GetMAC obtém o MAC Address de um IP na LAN via SendARP (Windows).
func GetMAC(ip string) string {
	if runtime.GOOS != "windows" {
		return ""
	}

	iphlpapi := syscall.NewLazyDLL("iphlpapi.dll")
	sendARP := iphlpapi.NewProc("SendARP")

	destIP := net.ParseIP(ip).To4()
	if destIP == nil {
		return ""
	}

	var destIPUint32 uint32
	destIPUint32 = uint32(destIP[0]) | uint32(destIP[1])<<8 | uint32(destIP[2])<<16 | uint32(destIP[3])<<24

	var mac [6]byte
	macLen := uint32(len(mac))

	ret, _, _ := sendARP.Call(
		uintptr(destIPUint32),
		0,
		uintptr(unsafe.Pointer(&mac[0])),
		uintptr(unsafe.Pointer(&macLen)),
	)

	if ret == 0 && macLen == 6 {
		return fmt.Sprintf("%02X-%02X-%02X-%02X-%02X-%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5])
	}
	return ""
}

// ScanPorts verifica quais das portas especificadas estão abertas.
func ScanPorts(ip string, ports []int, timeoutMs int) []int {
	var openPorts []int
	timeout := time.Duration(timeoutMs) * time.Millisecond

	for _, port := range ports {
		address := fmt.Sprintf("%s:%d", ip, port)
		conn, err := net.DialTimeout("tcp", address, timeout)
		if err == nil {
			conn.Close()
			openPorts = append(openPorts, port)
		}
	}
	return openPorts
}
