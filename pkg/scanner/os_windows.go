//go:build windows

package scanner

import (
	"fmt"
	"net"
	"os/exec"
	"syscall"
	"unsafe"
)

func osPing(ip string, timeoutMs int) bool {
	cmd := exec.Command("ping", "-n", "1", "-w", fmt.Sprintf("%d", timeoutMs), ip)
	cmd.SysProcAttr = &syscall.SysProcAttr{HideWindow: true}
	return cmd.Run() == nil
}

func osGetMAC(ip string) string {
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
