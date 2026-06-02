package scanner

import (
	"testing"
)

func TestParseRange(t *testing.T) {
	tests := []struct {
		name      string
		input     string
		wantCount int
		wantErr   bool
	}{
		{"Single IP", "192.168.1.100", 1, false},
		{"Dash range small", "192.168.1.1-192.168.1.5", 5, false},
		{"Dash range shorthand", "192.168.1.1-5", 5, false},
		{"CIDR /30", "192.168.1.0/30", 2, false}, // 192.168.1.1 and 192.168.1.2
		{"Invalid input", "invalid-range", 0, true},
		{"Empty input", " ", 0, true},
		{"Reversed range", "192.168.1.10-192.168.1.1", 0, true},
		{"Range too large", "10.0.0.1-10.2.0.0", 0, true}, // > 65536 is error
		{"CIDR too large", "10.0.0.0/8", 0, true}, // > 65536 is error
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			got, err := ParseRange(tt.input)
			if (err != nil) != tt.wantErr {
				t.Errorf("ParseRange() error = %v, wantErr %v", err, tt.wantErr)
				return
			}
			if !tt.wantErr && len(got) != tt.wantCount {
				t.Errorf("ParseRange() got %v items, want %v", len(got), tt.wantCount)
			}
		})
	}
}
