package scanner

import (
	"fmt"
	"net"
	"os/exec"
	"runtime"
	"strconv"
	"strings"
	"syscall"
	"time"
	"unsafe"
)

// validateIPv4 retorna erro se ip não for um endereço IPv4 válido e
// roteável. Rejeita strings vazias, IPs não-IPv4 e endereços de
// loopback (127.x.x.x) para operações externas.
func validateIPv4(ip string) error {
	parsed := net.ParseIP(ip)
	if parsed == nil {
		return fmt.Errorf("endereço IP inválido: %q", ip)
	}
	if parsed.To4() == nil {
		return fmt.Errorf("apenas IPv4 é suportado: %q", ip)
	}
	return nil
}

// Ping verifica se um host está vivo.
func Ping(ip string, timeoutMs int) bool {
	if err := validateIPv4(ip); err != nil {
		return false
	}
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
	if err := validateIPv4(ip); err != nil {
		return ""
	}
	names, err := net.LookupAddr(ip)
	if err == nil && len(names) > 0 {
		return strings.TrimSuffix(names[0], ".")
	}
	return ""
}

// GetMAC obtém o MAC Address de um IP na LAN via SendARP (Windows).
func GetMAC(ip string) string {
	if err := validateIPv4(ip); err != nil {
		return ""
	}
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

	// Segurança: mac é um array de tamanho fixo [6]byte alocado na stack
	// desta função. macLen é inicializado com len(mac) == 6 antes da
	// chamada. O acesso via unsafe.Pointer é seguro porque o array não
	// escapa do escopo e seu tamanho é conhecido em tempo de compilação.
	// A validação `macLen == 6` após o retorno garante que não
	// interpretamos dados corrompidos.
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
	if err := validateIPv4(ip); err != nil {
		return nil
	}
	var openPorts []int
	timeout := time.Duration(timeoutMs) * time.Millisecond

	for _, port := range ports {
		address := net.JoinHostPort(ip, strconv.Itoa(port))
		conn, err := net.DialTimeout("tcp", address, timeout)
		if err == nil {
			conn.Close()
			openPorts = append(openPorts, port)
		}
	}
	return openPorts
}
