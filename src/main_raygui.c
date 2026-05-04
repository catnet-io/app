// Main window built with Raygui and C
// Build via the project script: `build.ps1 -Compiler Clang -UI Raygui` (default).
// The script compiles raylib modules from `third_party/raylib/src` and integrates raygui,
// activating Windows SDK headers/libs when available. No prebuilt raylib binaries required.

#include "raylib.h"
#include <stdbool.h>

// Define raygui implementation header in a C file to avoid build issues
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "app.h"
#include "scan.h"
#include "utils.h"
#include "net.h"
#include "parallel_scan.h"
#include "range_parser.h"
#include <string.h>

static Color g_bgColor = {24,24,24,255};
static bool quickToolsExpanded = true;
static bool quickToolsActiveMode = false;
static bool lanOnly = true;
static char quickIpText[64] = "192.168.1.1";
static bool quickIpEdit = false;
// Quick Tools: decomposed IP octets and edit flags
static char ipQ1[4] = "192";
static char ipQ2[4] = "168";
static char ipQ3[4] = "1";
static char ipQ4[4] = "1";
static bool ipQ1Edit = false;
static bool ipQ2Edit = false;
static bool ipQ3Edit = false;
static bool ipQ4Edit = false;
// Added UI state for new layout features
static bool autoFillSubnet = true; // auto-fill range from primary subnet
static bool scanOnStartup = false;
static bool startupScanDone = false;
static int sortColumn = -1; // 0=Status,1=Hostname,2=IP,3=Open Ports,4=MAC Address
static bool sortAscending = true;
static float splitterRatio = 0.65f; // vertical space for results in content area
static bool draggingSplitter = false;
static void apply_theme(bool dark)
{
    Color bg = dark ? (Color){24,24,24,255} : RAYWHITE;
    Color text = dark ? (Color){235,235,235,255} : (Color){20,20,20,255};
    Color line = dark ? (Color){80,80,80,255} : (Color){180,180,180,255};
    Color base = dark ? (Color){60,60,60,255} : (Color){220,220,220,255};
    g_bgColor = bg;
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(bg));
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, ColorToInt(text));
    GuiSetStyle(DEFAULT, TEXT_COLOR_FOCUSED, ColorToInt(text));
    GuiSetStyle(DEFAULT, TEXT_COLOR_PRESSED, ColorToInt(text));
    GuiSetStyle(DEFAULT, LINE_COLOR, ColorToInt(line));
    GuiSetStyle(DEFAULT, BASE_COLOR_NORMAL, ColorToInt(base));
    GuiSetStyle(DEFAULT, BASE_COLOR_FOCUSED, ColorToInt(base));
    GuiSetStyle(DEFAULT, BASE_COLOR_PRESSED, ColorToInt(base));
}

static char g_statusText[128] = "Ready";
static char g_logLines[256][160];
static int g_logCount = 0;
static void gui_logger(const char* msg, void* ctx)
{
    (void)ctx;
    if (!msg) { g_statusText[0] = '\0'; return; }
    strncpy(g_statusText, msg, sizeof(g_statusText) - 1);
    g_statusText[sizeof(g_statusText) - 1] = '\0';
    strncpy(g_logLines[g_logCount % 256], msg, sizeof(g_logLines[0]) - 1);
    g_logLines[g_logCount % 256][sizeof(g_logLines[0]) - 1] = '\0';
    g_logCount++;
}

// Sorting helpers for Scan Results
static int device_compare(const void* a, const void* b)
{
    const DeviceInfo* da = (const DeviceInfo*)a;
    const DeviceInfo* db = (const DeviceInfo*)b;
    int c = 0;
    switch (sortColumn) {
        case 0: c = (da->is_alive - db->is_alive); break;
        case 1: c = strcmp(da->hostname, db->hostname); break;
        case 2: {
            unsigned long ua=0, ub=0; ip_to_uint(da->ip, &ua); ip_to_uint(db->ip, &ub);
            if (ua < ub) c = -1; else if (ua > ub) c = 1; else c = 0; break;
        }
        case 3: c = (da->open_ports_count - db->open_ports_count); break;
        case 4: c = strcmp(da->mac, db->mac); break;
        default: c = 0; break;
    }
    if (!sortAscending) c = -c;
    if (c < 0) return -1; if (c > 0) return 1; return 0;
}

static void sort_results(DeviceList* list)
{
    if (sortColumn < 0 || list->count == 0) return;
    qsort(list->items, list->count, sizeof(DeviceInfo), device_compare);
}

int main(void)
{
    // --- Window Configuration ---
    const int initialWidth = 1100;
    const int initialHeight = 700;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(initialWidth, initialHeight, "catnet_scanner (Raygui)");
    SetTargetFPS(60);
    // NEW: Layout constants for a consistent appearance
    const int padding = 10;         // Internal spacing for panels
    const int itemSpacing = 5;      // Spacing between buttons and other items
    const int toolbarH = 34;        // Height of the top toolbar
    const int statusH = 25;         // Height of the status bar
    const int sidebarW = 0;         // No sidebar in this layout
const int splitBarH = padding;  // Vertical gap equals global padding
    int rowHeight = 25;             // Height of each row in the results list (scaled later)

    // --- Application state ---
    DeviceList results; device_list_init(&results);
    ScanConfig cfg; scan_config_init(&cfg);
    bool isScanning = false;
    apply_theme(true);
    // Increase global font size for readability
    GuiSetStyle(DEFAULT, TEXT_SIZE, 18);
    GuiSetStyle(DEFAULT, TEXT_SPACING, 2);
    // Increase row height for readability (zebra stripes look better at 30)
    rowHeight = 30;
    int selectedIndex = -1;
    Vector2 scroll = (Vector2){0,0};
    Vector2 dbgScroll = (Vector2){0,0};
    scan_set_logger(gui_logger);
    // Shared IP range buffer used by toolbar TextBox and scan action
    static char ipRangeText[64] = "192.168.1.1-254";
    static bool ipRangeEdit = false;

    // --- Main loop ---
    while (!WindowShouldClose())
    {
        // --- Interaction ---

        // --- UI drawing (per frame) ---
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();
        BeginDrawing();
        ClearBackground(g_bgColor);

        // --- 1. Top Toolbar: Primary actions left, icons right ---
        float currentX = (float)padding;
        if (GuiButton((Rectangle){ currentX, padding, 90, 26 }, "Scan")) {
            if (autoFillSubnet) {
                SubnetV4 sn;
                if (net_get_primary_subnet(&sn)) {
                    char netBuf[64] = {0};
                    uint_to_ip(sn.network, netBuf, sizeof(netBuf));
                    unsigned long m = sn.mask; int prefix = 0; while (m) { prefix += (int)(m & 1u); m >>= 1; }
                    snprintf(ipRangeText, sizeof(ipRangeText), "%s/%d", netBuf, prefix);
                    gui_logger("Auto-fill: range set to primary subnet", NULL);
                } else {
                    gui_logger("Auto-fill: failed to get primary subnet", NULL);
                }
            }
            if (!isScanning) {
                isScanning = true;
                g_statusText[0] = '\0'; strncat(g_statusText, "Scanning...", sizeof(g_statusText)-1);
                device_list_clear(&results);
                char startBuf[64] = {0}, endBuf[64] = {0};
                char errbuf[128] = {0};
                if (parse_range(ipRangeText, startBuf, sizeof(startBuf), endBuf, sizeof(endBuf), errbuf, sizeof(errbuf))) {
                    unsigned long s = 0, e = 0;
                    if (ip_to_uint(startBuf, &s) && ip_to_uint(endBuf, &e) && e >= s) {
                        parallel_scan_start(s, e, &cfg, gui_logger, NULL);
                    } else {
                        SubnetV4 sn; if (net_get_primary_subnet(&sn)) { parallel_scan_start(sn.start_ip, sn.end_ip, &cfg, gui_logger, NULL); }
                        else { isScanning = false; gui_logger("Invalid IP range", NULL); }
                    }
                } else {
                    gui_logger(errbuf[0] ? errbuf : "Invalid IP range", NULL);
                    SubnetV4 sn; if (net_get_primary_subnet(&sn)) { parallel_scan_start(sn.start_ip, sn.end_ip, &cfg, gui_logger, NULL); } else { isScanning = false; }
                }
            }
        }
        currentX += 90 + itemSpacing;
        if (GuiButton((Rectangle){ currentX, padding, 90, 26 }, "Stop")) { if (isScanning) { parallel_scan_stop(); gui_logger("Scan cancelled by user", NULL); isScanning = false; } }
        currentX += 90 + itemSpacing;
        if (GuiButton((Rectangle){ currentX, padding, 110, 26 }, "Clear Log")) { g_logCount = 0; }
        currentX += 110 + itemSpacing;
        Vector2 tQuick = MeasureTextEx(GetFontDefault(), "Quick Tools", (float)GuiGetStyle(DEFAULT, TEXT_SIZE), (float)GuiGetStyle(DEFAULT, TEXT_SPACING));
        float quickW = tQuick.x + 24; // extra padding to avoid truncation
        if (GuiButton((Rectangle){ currentX, padding, quickW, 26 }, "Quick Tools")) {
            quickToolsExpanded = !quickToolsExpanded;
            quickToolsActiveMode = quickToolsExpanded;
            gui_logger(quickToolsExpanded ? "Quick Tools: shown" : "Quick Tools: hidden", NULL);
        }
        currentX += quickW + itemSpacing;
        // Right-aligned global controls: Settings and Help icon buttons
        float rightX = (float)(screenWidth - padding*3); // move a bit left from the edge
        float btnW = 34, btnH = 28;
        rightX -= btnW;
        if (GuiButton((Rectangle){ rightX, padding, btnW, btnH }, NULL)) { gui_logger("Help clicked", NULL); }
        GuiDrawIcon(ICON_HELP, (int)(rightX + btnW/2 - 8), (int)(padding + btnH/2 - 8), 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        rightX -= itemSpacing;
        rightX -= btnW;
        if (GuiButton((Rectangle){ rightX, padding, btnW, btnH }, NULL)) { gui_logger("Settings clicked", NULL); }
        GuiDrawIcon(ICON_GEAR, (int)(rightX + btnW/2 - 8), (int)(padding + btnH/2 - 8), 1, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        
        // ---- 2. Scan Configuration Panel ----
        Rectangle configArea = (Rectangle){ (float)(padding), (float)(toolbarH + padding), (float)(screenWidth - padding*2), 78.0f };
        GuiGroupBox(configArea, "Scan Configuration");
        float cx = configArea.x + padding;
        float cy = configArea.y + padding + 6;
        const float fontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        const float fontSpacing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);
        Font df = GetFontDefault();
        Vector2 tRange = MeasureTextEx(df, "IP Range/CIDR:", fontSize, fontSpacing);
        GuiLabel((Rectangle){ cx, cy+2, tRange.x + 6, 24 }, "IP Range/CIDR:");
        float cfgTextW = configArea.width - padding*3 - (tRange.x + 6) - 360; // leave room for checkboxes
        if (cfgTextW < 320) cfgTextW = 320;
        if (GuiTextBox((Rectangle){ cx + (tRange.x + 6), cy, cfgTextW, 28 }, ipRangeText, sizeof(ipRangeText), ipRangeEdit)) { ipRangeEdit = !ipRangeEdit; }
        float cbx = cx + (tRange.x + 6) + cfgTextW + itemSpacing;
        const char* autoTxt = "Auto-fill from primary subnet";
        Vector2 tAuto = MeasureTextEx(df, autoTxt, fontSize, fontSpacing);
        Vector2 mouse = GetMousePosition();
        Vector2 autoDot = (Vector2){ cbx + 10, cy + 11 };
        DrawCircleV(autoDot, 6.0f, autoFillSubnet ? LIME : RED);
        if (GuiLabelButton((Rectangle){ cbx + 24, cy, tAuto.x, 22 }, autoTxt)) autoFillSubnet = !autoFillSubnet;
        if (CheckCollisionPointRec(mouse, (Rectangle){ cbx, cy, 22, 22 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) autoFillSubnet = !autoFillSubnet;
        cy += 22 + itemSpacing;
        const char* startTxt = "Scan on startup";
        Vector2 tStart = MeasureTextEx(df, startTxt, fontSize, fontSpacing);
        Vector2 startDot = (Vector2){ cx + 10, cy + 11 };
        DrawCircleV(startDot, 6.0f, scanOnStartup ? LIME : RED);
        if (GuiLabelButton((Rectangle){ cx + 24, cy, tStart.x, 22 }, startTxt)) scanOnStartup = !scanOnStartup;
        if (CheckCollisionPointRec(mouse, (Rectangle){ cx, cy, 22, 22 }) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) scanOnStartup = !scanOnStartup;

        // ---- 3. Quick Tools Panel ----
        float quickH = quickToolsExpanded ? 70.0f : 0.0f;
        Rectangle quickArea = (Rectangle){ (float)(padding), (float)(configArea.y + configArea.height + padding), (float)(screenWidth - padding*2), quickH };
        if (quickToolsExpanded) {
        GuiGroupBox(quickArea, "Quick Tools");
        float qx = quickArea.x + padding;
        float qy = quickArea.y + padding + 8;
        const float octetW = 60.0f; const float dotW = 10.0f; const float fieldH = 26.0f;
        Vector2 tTarget = MeasureTextEx(df, "Target IP:", fontSize, fontSpacing);
        GuiLabel((Rectangle){ qx, qy+2, tTarget.x + 6, fieldH }, "Target IP:");
        qx += tTarget.x + 6;
        Rectangle r1 = (Rectangle){ qx, qy, octetW, fieldH };
        bool prev1 = ipQ1Edit;
        bool togg1 = GuiTextBox(r1, ipQ1, sizeof(ipQ1), ipQ1Edit);
        if (togg1) { ipQ1Edit = !ipQ1Edit; }
        { int w = 0; for (int r = 0; ipQ1[r] != '\0'; ++r) { if (ipQ1[r] >= '0' && ipQ1[r] <= '9') { ipQ1[w++] = ipQ1[r]; if (w >= 3) break; } } ipQ1[w] = '\0'; }
        qx += octetW + 2; GuiLabel((Rectangle){ qx, qy+2, dotW, fieldH }, "."); qx += dotW + itemSpacing;
        Rectangle r2 = (Rectangle){ qx, qy, octetW, fieldH };
        bool prev2 = ipQ2Edit;
        bool togg2 = GuiTextBox(r2, ipQ2, sizeof(ipQ2), ipQ2Edit);
        if (togg2) { ipQ2Edit = !ipQ2Edit; }
        { int w = 0; for (int r = 0; ipQ2[r] != '\0'; ++r) { if (ipQ2[r] >= '0' && ipQ2[r] <= '9') { ipQ2[w++] = ipQ2[r]; if (w >= 3) break; } } ipQ2[w] = '\0'; }
        qx += octetW + 2; GuiLabel((Rectangle){ qx, qy+2, dotW, fieldH }, "."); qx += dotW + itemSpacing;
        Rectangle r3 = (Rectangle){ qx, qy, octetW, fieldH };
        bool prev3 = ipQ3Edit;
        bool togg3 = GuiTextBox(r3, ipQ3, sizeof(ipQ3), ipQ3Edit);
        if (togg3) { ipQ3Edit = !ipQ3Edit; }
        { int w = 0; for (int r = 0; ipQ3[r] != '\0'; ++r) { if (ipQ3[r] >= '0' && ipQ3[r] <= '9') { ipQ3[w++] = ipQ3[r]; if (w >= 3) break; } } ipQ3[w] = '\0'; }
        qx += octetW + 2; GuiLabel((Rectangle){ qx, qy+2, dotW, fieldH }, "."); qx += dotW + itemSpacing;
        Rectangle r4 = (Rectangle){ qx, qy, octetW, fieldH };
        bool prev4 = ipQ4Edit;
        bool togg4 = GuiTextBox(r4, ipQ4, sizeof(ipQ4), ipQ4Edit);
        if (togg4) { ipQ4Edit = !ipQ4Edit; }
        { int w = 0; for (int r = 0; ipQ4[r] != '\0'; ++r) { if (ipQ4[r] >= '0' && ipQ4[r] <= '9') { ipQ4[w++] = ipQ4[r]; if (w >= 3) break; } } ipQ4[w] = '\0'; }
        int v1=-1,v2=-1,v3=-1,v4=-1;
        if (ipQ1[0]) { v1 = 0; for (int i=0; ipQ1[i] && i<3; ++i) { if (ipQ1[i]<'0'||ipQ1[i]>'9'){v1=-1;break;} v1 = v1*10 + (ipQ1[i]-'0'); } }
        if (ipQ2[0]) { v2 = 0; for (int i=0; ipQ2[i] && i<3; ++i) { if (ipQ2[i]<'0'||ipQ2[i]>'9'){v2=-1;break;} v2 = v2*10 + (ipQ2[i]-'0'); } }
        if (ipQ3[0]) { v3 = 0; for (int i=0; ipQ3[i] && i<3; ++i) { if (ipQ3[i]<'0'||ipQ3[i]>'9'){v3=-1;break;} v3 = v3*10 + (ipQ3[i]-'0'); } }
        if (ipQ4[0]) { v4 = 0; for (int i=0; ipQ4[i] && i<3; ++i) { if (ipQ4[i]<'0'||ipQ4[i]>'9'){v4=-1;break;} v4 = v4*10 + (ipQ4[i]-'0'); } }
        if (v1<0 || v1>255) DrawRectangleLinesEx(r1, 2, RED);
        if (v2<0 || v2>255) DrawRectangleLinesEx(r2, 2, RED);
        if (v3<0 || v3>255) DrawRectangleLinesEx(r3, 2, RED);
        if (v4<0 || v4>255) DrawRectangleLinesEx(r4, 2, RED);
        // Auto-avançar (smart): ao digitar 3 dígitos ou pressionar '.'/Tab; permite voltar
        bool shiftHeld = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        bool goNextKey = IsKeyPressed(KEY_PERIOD) || (!shiftHeld && IsKeyPressed(KEY_TAB));
        bool goPrevKey = (shiftHeld && IsKeyPressed(KEY_TAB)) || IsKeyPressed(KEY_LEFT);
        if (ipQ1Edit) {
            if ((int)strlen(ipQ1) >= 3 || goNextKey) { ipQ1Edit = false; ipQ2Edit = true; }
        } else if (ipQ2Edit) {
            if (((int)strlen(ipQ2) >= 3) || goNextKey) { ipQ2Edit = false; ipQ3Edit = true; }
            else if (goPrevKey || (IsKeyPressed(KEY_BACKSPACE) && (int)strlen(ipQ2) == 0)) { ipQ2Edit = false; ipQ1Edit = true; }
        } else if (ipQ3Edit) {
            if (((int)strlen(ipQ3) >= 3) || goNextKey) { ipQ3Edit = false; ipQ4Edit = true; }
            else if (goPrevKey || (IsKeyPressed(KEY_BACKSPACE) && (int)strlen(ipQ3) == 0)) { ipQ3Edit = false; ipQ2Edit = true; }
        } else if (ipQ4Edit) {
            if (goPrevKey || (IsKeyPressed(KEY_BACKSPACE) && (int)strlen(ipQ4) == 0)) { ipQ4Edit = false; ipQ3Edit = true; }
        }
        snprintf(quickIpText, sizeof(quickIpText), "%s.%s.%s.%s", ipQ1, ipQ2, ipQ3, ipQ4);
        qx += octetW + itemSpacing;
        Vector2 tPing = MeasureTextEx(df, "Ping", fontSize, fontSpacing);
        float pingW = tPing.x + 20;
        if (GuiButton((Rectangle){ qx, qy, pingW, 26 }, "Ping")) {
            if (quickToolsActiveMode) { g_logCount = 0; }
            int ok = net_ping_ipv4(quickIpText);
            char msg[128]; snprintf(msg, sizeof(msg), ok ? "Ping %s: success" : "Ping %s: failed", quickIpText);
            gui_logger(msg, NULL);
        }
        qx += pingW + itemSpacing;
        Vector2 tDns = MeasureTextEx(df, "DNS Query", fontSize, fontSpacing);
        float dnsW = tDns.x + 24;
        if (GuiButton((Rectangle){ qx, qy, dnsW, 26 }, "DNS Query")) {
            if (quickToolsActiveMode) { g_logCount = 0; }
            char host[128] = {0};
            int ok = net_reverse_dns(quickIpText, host, sizeof(host));
            char msg[196]; snprintf(msg, sizeof(msg), ok && host[0] ? "DNS %s -> %s" : "DNS %s: not found", quickIpText, host);
            gui_logger(msg, NULL);
        }
        qx += dnsW + itemSpacing;
        Vector2 tPort = MeasureTextEx(df, "Port Scan", fontSize, fontSpacing);
        float portW = tPort.x + 24;
        if (GuiButton((Rectangle){ qx, qy, portW, 26 }, "Port Scan")) {
            if (quickToolsActiveMode) { g_logCount = 0; }
            int open[64]; int openCount = 0;
            int ok = net_scan_ports(quickIpText, cfg.default_ports, cfg.default_ports_count, cfg.port_timeout_ms, open, &openCount);
            char msg[256] = {0};
            if (ok && openCount > 0) {
                char buf[160] = {0};
                for (int i = 0; i < openCount; ++i) {
                    char t[12]; snprintf(t, sizeof(t), "%d%s", open[i], (i<openCount-1?",":""));
                    strncat(buf, t, sizeof(buf) - strlen(buf) - 1);
                }
                snprintf(msg, sizeof(msg), "Open ports %s: %s", quickIpText, buf);
            } else {
                snprintf(msg, sizeof(msg), "Open ports %s: none", quickIpText);
            }
            gui_logger(msg, NULL);
        }
        } // end Quick Tools expanded

        // Navigation sidebar removed (not needed for current functionality)

        // --- 3. Main panel (ListView with columns) ---
        // Refresh results snapshot each frame
        if (isScanning) {
            parallel_scan_snapshot(&results);
            if (!parallel_scan_is_running()) {
                isScanning = false;
                // Contar apenas dispositivos encontrados (alive)
                size_t found = 0; for (size_t i = 0; i < results.count; ++i) { if (results.items[i].is_alive) ++found; }
                snprintf(g_statusText, sizeof(g_statusText), "Done. Devices: %zu", found);
            }
        }
        // ---- 4. Main content area with vertical splitter ----
        float contentTopY = quickArea.y + quickArea.height + padding;
        float contentHeight = (float)screenHeight - statusH - padding*3 - contentTopY;
        if (contentHeight < 160) contentHeight = 160;
        // Compute areas based on splitter ratio
        float resultsH = contentHeight * splitterRatio - (splitBarH*0.5f);
        const float minPanelH = 120.0f;
        if (resultsH < minPanelH) resultsH = minPanelH;
        float debugH = contentHeight - splitBarH - resultsH;
        if (debugH < minPanelH) {
            debugH = minPanelH; resultsH = contentHeight - splitBarH - debugH;
        }
        Rectangle mainArea = (Rectangle){ (float)padding, contentTopY, (float)(screenWidth - padding*2), resultsH };
        GuiGroupBox(mainArea, "Scan Results");

        // NEW: Dynamic Column System
        // Define widths and names here. The layout will adjust automatically.
        const char* headers[] = { "Status", "Hostname", "IP", "Open Ports", "MAC Address" };
        float innerW = mainArea.width - padding*2;
        float w0 = 70.0f;                 // Status icon column
        float w1 = innerW * 0.32f;        // Hostname (flex)
        float w2 = innerW * 0.20f;        // IP
        float w3 = innerW * 0.22f;        // Open Ports
        float w4 = innerW - (w0 + w1 + w2 + w3); // Remaining for MAC Address
        // Enforce minimums to avoid truncation
        const float minHost = 140.0f;
        const float minIP   = 160.0f;
        const float minPorts= 230.0f;
        const float minMAC  = 240.0f;
        if (w2 < minIP)   { float d = (minIP - w2);   w2 = minIP;   w1 = (w1 > minHost + d ? w1 - d : minHost); }
        if (w3 < minPorts){ float d = (minPorts - w3); w3 = minPorts; w1 = (w1 > minHost + d ? w1 - d : minHost); }
        if (w4 < minMAC)  { float d = (minMAC - w4);  w4 = minMAC;  w1 = (w1 > minHost + d ? w1 - d : minHost); }
        // If total still exceeds innerW, trim hostname down to min
        float totalW = w0 + w1 + w2 + w3 + w4;
        if (totalW > innerW) {
            float over = totalW - innerW;
            w1 = (w1 > minHost + over) ? (w1 - over) : minHost;
        }
        float columnWidths[] = { w0, w1, w2, w3, w4 };
        float columnOffsets[5];
        columnOffsets[0] = mainArea.x + padding;
        for (int i = 1; i < 5; i++) { columnOffsets[i] = columnOffsets[i-1] + columnWidths[i-1]; }

        // Header row background for visual distinction
        DrawRectangle(mainArea.x + 1, mainArea.y + padding, mainArea.width - 2, 24, (Color){50,50,50,255});
        // Draw the header as clickable buttons (sortable)
        for (int i = 0; i < 5; i++) {
            char title[48];
            if (i == sortColumn) snprintf(title, sizeof(title), "%s %s", headers[i], (sortAscending ? "\xE2\x96\xB2" : "\xE2\x96\xBC"));
            else snprintf(title, sizeof(title), "%s", headers[i]);
            Rectangle hrec = (Rectangle){ columnOffsets[i], mainArea.y + padding, columnWidths[i], 24 };
            if (GuiButton(hrec, title)) {
                if (sortColumn != i) { sortColumn = i; sortAscending = true; }
                else { sortAscending = !sortAscending; }
                sort_results(&results);
            }
        }
        GuiLine((Rectangle){ mainArea.x, mainArea.y + padding + 28, mainArea.width, 1 }, NULL);

        // Scroll area for results
        // Scrollable area for results
        Rectangle panelRec = { mainArea.x, mainArea.y + padding + 32, mainArea.width, mainArea.height - padding - 32 };
        Rectangle view = { 0 };
        // Mostrar apenas dispositivos com ping OK (is_alive)
        size_t visibleCount = 0; for (size_t i = 0; i < results.count; ++i) { if (results.items[i].is_alive) ++visibleCount; }
        GuiScrollPanel(panelRec, NULL, (Rectangle){ 0, 0, panelRec.width - 20, (float)visibleCount * rowHeight }, &scroll, &view);
        
        BeginScissorMode((int)view.x, (int)view.y, (int)view.width, (int)view.height);

        int displayIndex = 0;
        for (int i = 0; i < (int)results.count; i++)
        {
            const DeviceInfo* di = &results.items[i];
            if (!di->is_alive) continue; // filtrar apenas encontrados
            float yPos = panelRec.y + (float)(displayIndex * rowHeight) + scroll.y;

            // "Zebra Stripes" para melhor leitura
            if (displayIndex % 2 != 0) {
                DrawRectangle(panelRec.x, yPos, panelRec.width, (float)rowHeight, (Color){40, 40, 40, 255});
            }
            if (selectedIndex == displayIndex) {
                DrawRectangle(panelRec.x, yPos, panelRec.width, (float)rowHeight, (Color){0, 120, 215, 100});
            }

            if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){ panelRec.x, yPos, panelRec.width, (float)rowHeight })) {
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) selectedIndex = displayIndex;
            }

            // Desenhar dados por coluna
            DrawCircle(columnOffsets[0] + 15, yPos + rowHeight/2, 5, LIME);
            GuiLabel((Rectangle){ columnOffsets[1], yPos, columnWidths[1], (float)rowHeight }, di->hostname[0] ? di->hostname : "(unnamed)");
            GuiLabel((Rectangle){ columnOffsets[2], yPos, columnWidths[2], (float)rowHeight }, di->ip);
            char portsBuf[128] = {0};
            for (int p = 0; p < di->open_ports_count; ++p) {
                char tmp[16]; snprintf(tmp, sizeof(tmp), "%d%s", di->open_ports[p], (p<di->open_ports_count-1?",":""));
                strncat(portsBuf, tmp, sizeof(portsBuf)-strlen(portsBuf)-1);
            }
            GuiLabel((Rectangle){ columnOffsets[3], yPos, columnWidths[3], (float)rowHeight }, portsBuf);
            GuiLabel((Rectangle){ columnOffsets[4], yPos, columnWidths[4], (float)rowHeight }, di->mac);
            displayIndex++;
        }

        EndScissorMode();

        // --- 5. Vertical Splitter bar ---
        Rectangle splitBar = (Rectangle){ mainArea.x, mainArea.y + mainArea.height + 0, mainArea.width, (float)splitBarH };
        DrawRectangleRec(splitBar, (Color){70,70,70,255});
        // Hover/drag behavior
        if (CheckCollisionPointRec(GetMousePosition(), splitBar)) SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
        if (CheckCollisionPointRec(GetMousePosition(), splitBar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) draggingSplitter = true;
        if (draggingSplitter) {
            SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
            float minY = contentTopY + minPanelH;
            float maxY = contentTopY + contentHeight - minPanelH - splitBarH;
            float y = GetMousePosition().y;
            if (y < minY) y = minY; if (y > maxY) y = maxY;
            splitterRatio = (y - contentTopY) / contentHeight;
            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) draggingSplitter = false;
        }

        // --- 6. Debug Terminal ---
        Rectangle dbgBox = (Rectangle){ mainArea.x, splitBar.y + splitBar.height, mainArea.width, debugH };
        // Desenha apenas a borda do GroupBox e escreve o título abaixo da borda
        GuiGroupBox(dbgBox, "");
        DrawTextEx(df, "Debug Log", (Vector2){ dbgBox.x + padding, dbgBox.y + padding + 2 }, fontSize, fontSpacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        // Área interna começa abaixo do título para evitar sobreposição
        Rectangle dbgPanelRec = { dbgBox.x + padding, dbgBox.y + padding + (fontSize + 8), dbgBox.width - padding*2, dbgBox.height - padding*2 - (fontSize + 8) };
        Rectangle dbgView = { 0 };
        int lineH = GuiGetStyle(DEFAULT, TEXT_SIZE) + 10;
        float contentH = (float)((g_logCount < 256 ? g_logCount : 256) * lineH);
        GuiScrollPanel(dbgPanelRec, NULL, (Rectangle){ 0, 0, dbgPanelRec.width - 20, contentH }, &dbgScroll, &dbgView);
        BeginScissorMode((int)dbgView.x, (int)dbgView.y, (int)dbgView.width, (int)dbgView.height);
        int linesToShow = (g_logCount < 256 ? g_logCount : 256);
        for (int i = 0; i < linesToShow; ++i) {
            float y = dbgPanelRec.y + (float)(i * lineH) + dbgScroll.y;
            float dbgSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE) + 2.0f;
            Color dbgColor = (Color){ 235, 235, 235, 255 };
            DrawTextEx(GetFontDefault(), g_logLines[i], (Vector2){ dbgPanelRec.x + 5, y + 2 }, dbgSize, (float)GuiGetStyle(DEFAULT, TEXT_SPACING), dbgColor);
        }
        EndScissorMode();

        // --- 7. Split-layout Status bar ---
        Rectangle statusRect = (Rectangle){ 0, screenHeight - statusH, screenWidth, statusH };
        GuiStatusBar(statusRect, ""); // Draw background only
        // Left: primary status message
        const float sFontSize = (float)GuiGetStyle(DEFAULT, TEXT_SIZE);
        const float sFontSpacing = (float)GuiGetStyle(DEFAULT, TEXT_SPACING);
        Font sFont = GetFontDefault();
        float sy = statusRect.y + (statusH - sFontSize)/2.0f;
        DrawTextEx(sFont, g_statusText, (Vector2){ statusRect.x + padding, sy }, sFontSize, sFontSpacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));
        // Right: device count and scan state
        size_t foundStatus = 0; for (size_t i = 0; i < results.count; ++i) { if (results.items[i].is_alive) ++foundStatus; }
        char rightText[96]; snprintf(rightText, sizeof(rightText), "Devices: %zu%s", foundStatus, (isScanning?" (scanning)":""));
        Vector2 tRight = MeasureTextEx(sFont, rightText, sFontSize, sFontSpacing);
        float rx = statusRect.x + statusRect.width - (padding*3) - tRight.x;
        DrawTextEx(sFont, rightText, (Vector2){ rx, sy }, sFontSize, sFontSpacing, GetColor(GuiGetStyle(DEFAULT, TEXT_COLOR_NORMAL)));

        EndDrawing();
    }

    // --- Shutdown ---
    device_list_clear(&results);
    CloseWindow();
    return 0;
}