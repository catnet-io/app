# Implementation Plan: CatNet Scanner Hardening

## Overview

Este plano converte o design tecnico em tarefas atomicas e sequenciadas por dependencias reais do codigo. A ordem e obrigatoria: utils.c e raylib_shims.c devem ser concluidos antes de range_parser.c; range_parser.c deve existir antes dos testes unitarios; scan.c (remocao do g_logger) deve preceder parallel_scan.c (maquina de estados); export.c (JSON) deve preceder os botoes de exportacao na UI.

## Tasks

- [x] 1. Criar src/raylib_shims.c e src/raylib_shims.h
  - Criar src/raylib_shims.h com declaracoes de todos os shims: TraceLog, MemAlloc, MemRealloc, MemFree, LoadFileData, UnloadFileData, SaveFileData, LoadFileText, UnloadFileText, SaveFileText
  - Mover as implementacoes dos shims do final de src/utils.c para src/raylib_shims.c
  - Remover os shims de src/utils.c (manter apenas ip_to_uint, uint_to_ip, trim_newline, safe_strcpy)
  - Adicionar #include "raylib_shims.h" em src/main_raygui.c (ou onde necessario para resolver simbolos)
  - Verificar que o build com build.ps1 -Compiler Clang continua produzindo bin/catnet_scanner.exe sem erros de simbolo duplicado ou ausente
  - _Requirements: 2.1, 2.2, 2.3_

- [x] 2. Atualizar src/utils.c e src/utils.h com APIs modernas e novos helpers
  - [x] 2.1 Substituir inet_addr por InetPton em ip_to_uint e substituir inet_ntoa por InetNtop em uint_to_ip
    - Remover o workaround de comparacao com INADDR_NONE em ip_to_uint
    - Garantir que uint_to_ip usa InetNtop com buffer de tamanho INET_ADDRSTRLEN
    - _Requirements: 7.1, 7.3_
  - [x] 2.2 Adicionar filter_digits e parse_octet em src/utils.c e declarar em src/utils.h
    - Implementar void filter_digits(char* buf, int max_len) que remove caracteres nao numericos in-place, preservando no maximo max_len-1 digitos
    - Implementar int parse_octet(const char* s) que retorna -1 se invalido ou fora de [0,255]
    - _Requirements: 1.2, 1.4_
  - [ ]* 2.3 Escrever testes unitarios para ip_to_uint, uint_to_ip, filter_digits e parse_octet
    - Criar tests/test_utils.c com os casos da tabela do design: ip_to_uint("192.168.1.1"), round-trip, filter_digits("1a2b3", 4) -> "123", filter_digits("1234567", 4) -> "123", parse_octet("256") -> -1, parse_octet("") -> -1
    - _Requirements: 13.1, 13.5_

- [x] 3. Criar src/range_parser.c e src/range_parser.h (extrair parse_range de main_raygui.c)
  - Criar src/range_parser.h com a interface: bool parse_range(const char* in, char* out_start, size_t sz_start, char* out_end, size_t sz_end, char* errbuf, size_t errsz)
  - Criar src/range_parser.c implementando parse_range com todas as validacoes obrigatorias: prefixo CIDR fora de [0,32], range > 65536 enderecos, start > end, octeto fora de [0,255], formato sem - nem /
  - Usar filter_digits e parse_octet de utils.c para validacao de octetos
  - Usar ip_to_uint e uint_to_ip de utils.c para conversao
  - Remover a funcao parse_range de src/main_raygui.c e substituir pela chamada ao novo modulo (adicionar #include "range_parser.h")
  - Atualizar a chamada em main_raygui.c para passar os novos parametros errbuf/errsz
  - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5_

- [x] 4. Configurar framework de testes Unity e estrutura tests/
  - Criar o diretorio tests/ e os subdiretorios tests/unity/
  - Copiar unity.c, unity.h e unity_internals.h para tests/unity/ (baixar de https://github.com/ThrowTheSwitch/Unity ou incluir como submodulo)
  - Criar tests/test_main.c como ponto de entrada que registra e executa todos os suites
  - Criar tests/test_list.c com testes de device_list_push (N=1, N=100 para testar realocacao) e device_list_clear
  - Criar build_tests.ps1 na raiz do projeto com o comando de compilacao do design: clang-cl compilando test_main.c, test_utils.c, test_list.c, test_parse_range.c, test_export.c, unity.c, src/utils.c, src/list.c, src/range_parser.c, src/export.c sem raylib e sem net.c
  - _Requirements: 13.2, 13.3, 13.4_

- [x] 5. Escrever testes unitarios para parse_range e device_list
  - [x] 5.1 Criar tests/test_parse_range.c com todos os casos obrigatorios do design
    - Range simples "192.168.1.1-254" -> start 192.168.1.1, end 192.168.1.254, retorna true
    - Range completo "10.0.0.1-10.0.0.100" -> correto, retorna true
    - CIDR "192.168.1.0/24" -> start 192.168.1.0, end 192.168.1.255, retorna true
    - CIDR "/32" -> start == end, retorna true
    - CIDR "/0" -> retorna false, errbuf contem "exceeds maximum"
    - CIDR invalido "/33" -> retorna false, errbuf = "CIDR prefix must be between 0 and 32"
    - Formato invalido "not-an-ip" -> retorna false, errbuf contem "Invalid range format"
    - Start > end "192.168.1.100-10.0.0.1" -> retorna false, errbuf correto
    - Octeto invalido "192.168.1.300/24" -> retorna false, errbuf contem "octet out of range"
    - Range de exatamente 65536 enderecos -> retorna true
    - Range de 65537 enderecos -> retorna false
    - _Requirements: 13.5_
  - [ ]* 5.2 Escrever property test para Property 2: parse_range rejeita prefixos CIDR fora de [0,32]
    - Para qualquer N < 0 ou N > 32 com IP valido, parse_range deve retornar false
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 2
    - _Requirements: 5.1_
  - [ ]* 5.3 Escrever property test para Property 3: parse_range rejeita ranges com mais de 65536 enderecos
    - Para qualquer range onde end - start + 1 > 65536, parse_range deve retornar false
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 3
    - _Requirements: 5.2_
  - [ ]* 5.4 Escrever property test para Property 4: parse_range rejeita entradas com formato invalido
    - Para strings sem - nem /, ou com octetos fora de [0,255], parse_range deve retornar false
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 4
    - _Requirements: 5.3, 5.5_
  - [ ]* 5.5 Escrever property test para Property 5: parse_range rejeita ranges onde start > end
    - Para qualquer par (start, end) onde ip_to_uint(start) > ip_to_uint(end), parse_range deve retornar false
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 5
    - _Requirements: 5.4_
  - [ ]* 5.6 Escrever property test para Property 1: filter_digits preserva apenas digitos
    - Para qualquer string de entrada e max_len >= 1, o resultado contem apenas ['0'..'9'] e tem comprimento < max_len
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 1
    - _Requirements: 1.2, 1.4_

- [x] 6. Checkpoint - Executar build_tests.ps1 e verificar que bin/catnet_tests.exe retorna codigo 0
  - Garantir que todos os testes unitarios passam, perguntar ao usuario se houver duvidas.

- [x] 7. Atualizar src/scan.c e src/scan.h para remover o logger global
  - Remover static ScanLogFn g_logger e a funcao scan_set_logger de scan.c e scan.h
  - Atualizar a assinatura de ScanLogFn em scan.h para incluir contexto: typedef void (*ScanLogFn)(const char* msg, void* ctx)
  - Atualizar identify_device para receber ScanLogFn logger e void* logger_ctx como parametros adicionais
  - Atualizar scan_subnet para receber logger e logger_ctx como parametros adicionais
  - Atualizar scan_range para receber logger e logger_ctx como parametros adicionais
  - Substituir todas as chamadas a g_logger por logger(msg, logger_ctx) dentro de scan.c
  - Quando logger for NULL, nao invocar o callback (verificar antes de chamar)
  - Remover a chamada scan_set_logger(gui_logger) de src/main_raygui.c
  - _Requirements: 3.1, 3.2, 3.3_

- [x] 8. Refatorar src/parallel_scan.c e src/parallel_scan.h com maquina de estados
  - [x] 8.1 Adicionar ScanStateMachine enum e campos de estado em parallel_scan.h e parallel_scan.c
    - Definir enum ScanStateMachine { SCAN_IDLE=0, SCAN_RUNNING=1, SCAN_STOPPING=2 } em parallel_scan.h
    - Adicionar campo ScanStateMachine state e CRITICAL_SECTION state_lock na struct ScanState interna
    - Adicionar flags results_lock_initialized e state_lock_initialized na struct ScanState
    - Expor parallel_scan_get_state(void) retornando ScanStateMachine em parallel_scan.h
    - _Requirements: 4.1, 4.6_
  - [x] 8.2 Corrigir parallel_scan_start para usar maquina de estados e teardown seguro
    - Substituir a verificacao "cancel == 0 && num_threads > 0" por InterlockedCompareExchange no campo state (SCAN_IDLE -> SCAN_RUNNING)
    - Retornar 0 imediatamente se o estado nao for SCAN_IDLE
    - Substituir memset(&g_state, 0, sizeof(g_state)) por teardown explicito: DeleteCriticalSection se inicializado, device_list_clear, inicializacao campo a campo
    - Atualizar assinatura para receber void* logger_ctx alem de ScanLogFn logger
    - Passar logger e logger_ctx para os workers via ScanState
    - _Requirements: 4.2, 4.3, 4.4_
  - [x] 8.3 Corrigir worker_proc para passar logger para identify_device
    - Substituir identify_device(&di, &st->cfg) por identify_device(&di, &st->cfg, st->logger, st->logger_ctx)
    - _Requirements: 3.4_
  - [x] 8.4 Corrigir parallel_scan_stop para usar transicoes de estado corretas
    - Transicionar para SCAN_STOPPING antes de aguardar workers
    - Transicionar para SCAN_IDLE apos WaitForMultipleObjects completar
    - Tornar parallel_scan_stop idempotente: no-op se estado ja for SCAN_IDLE
    - _Requirements: 4.5_
  - [x] 8.5 Implementar parallel_scan_progress
    - Adicionar declaracao de parallel_scan_progress(float* out_fraction) em parallel_scan.h
    - Implementar em parallel_scan.c: calcular processed = next_ip - start_ip, total = end_ip - start_ip + 1, retornar (float)processed / (float)total clampado em [0.0, 1.0]
    - _Requirements: 10.1_
  - [ ]* 8.6 Escrever property test para Property 6: parallel_scan_progress retorna sempre valor em [0.0, 1.0]
    - Para qualquer combinacao valida de start_ip, end_ip e next_ip com start <= end, o resultado deve satisfazer 0.0f <= f <= 1.0f
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 6
    - _Requirements: 10.1_
  - [x] 8.7 Atualizar parallel_scan.h com as novas assinaturas publicas
    - Atualizar parallel_scan_start para incluir void* logger_ctx
    - Adicionar parallel_scan_get_state e parallel_scan_progress
    - _Requirements: 4.1, 4.6, 10.1_

- [x] 9. Atualizar src/net.c para remover LoadLibraryA e usar APIs modernas
  - [x] 9.1 Corrigir net_ping_ipv4 para usar linkagem direta com IcmpCreateFile/IcmpSendEcho/IcmpCloseHandle
    - Remover LoadLibraryA("Icmp.dll"), GetProcAddress e FreeLibrary
    - Chamar IcmpCreateFile() diretamente; se retornar INVALID_HANDLE_VALUE, logar WSAGetLastError() e retornar 0
    - Chamar IcmpSendEcho diretamente; se retornar 0, chamar IcmpCloseHandle antes de retornar 0
    - _Requirements: 8.1, 8.2, 8.3, 8.4_
  - [x] 9.2 Substituir inet_addr por InetPton em net_ping_ipv4, net_reverse_dns, net_get_mac e connect_with_timeout
    - Usar InetPton(AF_INET, ip, &addr) em vez de inet_addr(ip)
    - Remover comparacoes com INADDR_NONE
    - _Requirements: 7.1_
  - [x] 9.3 Substituir gethostbyaddr por getnameinfo em net_reverse_dns
    - Usar getnameinfo com NI_NAMEREQD ou NI_NOFQDN conforme apropriado
    - Em caso de falha, preencher hostname com string vazia e retornar 0
    - _Requirements: 7.2_
  - [x] 9.4 Adicionar logging de WSAGetLastError em falhas de rede
    - Em net_scan_ports, net_ping_ipv4 e net_reverse_dns, logar o codigo WSA quando um logger estiver disponivel
    - Nota: net.c nao recebe logger diretamente; o logging de erros WSA pode ser feito via stderr ou via o logger passado por scan.c
    - _Requirements: 7.4_

- [x] 10. Checkpoint - Verificar que build.ps1 -Compiler Clang compila sem warnings de API deprecada
  - Remover /D _WINSOCK_DEPRECATED_NO_WARNINGS de build.ps1 e confirmar que o build continua limpo
  - Garantir que bin/catnet_scanner.exe e produzido sem erros, perguntar ao usuario se houver duvidas.
  - _Requirements: 7.5_

- [x] 11. Implementar exportacao JSON em src/export.c e src/export.h
  - Adicionar declaracao de export_results_to_json(const char* path, const DeviceList* list) em src/export.h
  - Implementar json_write_string(FILE* f, const char* s) como funcao estatica interna que escapa ", \, \n, \r, \t e caracteres de controle (< 0x20) como \uXXXX
  - Implementar export_results_to_json gerando {"devices":[...]} com campos ip, hostname, mac e ports (array de inteiros)
  - Campos hostname e mac vazios devem emitir string vazia ""
  - Retornar 0 se fopen falhar
  - _Requirements: 12.1, 12.2, 12.3, 12.4_

- [-] 12. Escrever testes unitarios e de propriedade para export.c
  - [-] 12.1 Criar tests/test_export.c com casos unitarios
    - export_results_to_json com lista vazia -> arquivo contem {"devices":[]}
    - export_results_to_json com 1 dispositivo -> IP, hostname, mac e ports corretos
    - export_results_to_json com hostname contendo " -> campo escapado como \"
    - export_results_to_json com hostname contendo \n -> campo escapado como \n
    - export_results_to_file com path invalido -> retorna 0
    - export_results_to_json com path invalido -> retorna 0
    - _Requirements: 13.1_
  - [ ]* 12.2 Escrever property test para Property 7: export_results_to_json produz JSON estruturalmente correto
    - Para qualquer DeviceList com campos de string arbitrarios (incluindo ", \, \n), o arquivo gerado deve comecar com {"devices":[ e terminar com ]}, conter exatamente list->count objetos e ter caracteres especiais escapados
    - Minimo 100 iteracoes; anotar com comentario: Feature: catnet-scanner-hardening, Property 7
    - _Requirements: 12.1, 12.2, 12.3, 12.4_

- [ ] 13. Checkpoint - Executar build_tests.ps1 e verificar que todos os testes passam
  - Garantir que bin/catnet_tests.exe retorna codigo 0 com todos os suites (utils, list, parse_range, export), perguntar ao usuario se houver duvidas.

- [ ] 14. Refatorar src/main_raygui.c - Parte 1: estado, helpers e verificacao de admin
  - [ ] 14.1 Definir struct UIState e consolidar variaveis de estado
    - Definir typedef struct UIState com todos os campos do design: ip_range_text[64], ip_range_edit, ip_q[4][4], ip_q_edit[4], quick_ip_text[64], auto_fill_subnet, scan_on_startup, startup_scan_done, quick_tools_expanded, quick_tools_active_mode, sort_column, sort_ascending, splitter_ratio, dragging_splitter, selected_index, scroll, dbg_scroll, is_admin
    - Remover as 17 variaveis estaticas globais (quickToolsExpanded, ipQ1..4, splitterRatio, etc.) e a variavel lanOnly (dead code)
    - Inicializar UIState no inicio de main() com os valores padrao do design
    - _Requirements: 1.3_
  - [ ] 14.2 Implementar check_is_admin e GUI Logger thread-safe
    - Implementar static bool check_is_admin(void) usando AllocateAndInitializeSid + CheckTokenMembership
    - Implementar gui_log_init, gui_log_destroy, gui_log_push(const char* msg, void* ctx), gui_log_snapshot e gui_log_clear com CRITICAL_SECTION
    - gui_log_push deve atualizar g_statusText dentro da secao critica
    - Substituir a funcao gui_logger existente (sem CRITICAL_SECTION) pela nova gui_log_push
    - _Requirements: 6.1, 6.2, 6.3, 6.4, 6.5, 9.1, 9.3_

- [ ] 15. Refatorar src/main_raygui.c - Parte 2: extrair funcoes de rendering
  - [ ] 15.1 Extrair draw_toolbar
    - Criar static void draw_toolbar(UIState* ui, DeviceList* results, ScanConfig* cfg)
    - Incluir botoes Scan, Stop (com estados SCAN_IDLE/RUNNING/STOPPING), Export CSV, Export JSON, Clear Log, Quick Tools, Settings, Help
    - Incluir barra de progresso (GuiProgressBar) visivel quando estado for SCAN_RUNNING ou SCAN_STOPPING
    - Incluir indicador textual de estado ("Idle", "Scanning", "Stopping")
    - Exibir aviso de admin em amarelo se !ui->is_admin
    - _Requirements: 1.1, 4.7, 9.2, 9.4, 10.2, 10.3, 10.4, 10.5_
  - [ ] 15.2 Extrair draw_config_panel
    - Criar static void draw_config_panel(UIState* ui, Rectangle area)
    - Mover o bloco "Scan Configuration" (IP Range/CIDR TextBox, Auto-fill toggle, Scan on startup toggle) para esta funcao
    - _Requirements: 1.1, 1.3_
  - [ ] 15.3 Extrair draw_quick_tools
    - Criar static void draw_quick_tools(UIState* ui, Rectangle area, ScanConfig* cfg)
    - Substituir os 4 blocos duplicados de filtragem por loop usando filter_digits(ui->ip_q[i], 4)
    - Substituir os 4 blocos duplicados de validacao por loop usando parse_octet(ui->ip_q[i])
    - Manter logica de auto-avanco entre octetos (Tab, '.', backspace)
    - _Requirements: 1.1, 1.2, 1.3_
  - [ ] 15.4 Extrair draw_results_table, draw_debug_log e draw_status_bar
    - Criar static void draw_results_table(UIState* ui, Rectangle area, DeviceList* results)
    - Criar static void draw_debug_log(UIState* ui, Rectangle area)
    - Criar static void draw_status_bar(UIState* ui, Rectangle area, DeviceList* results)
    - draw_status_bar deve exibir "Done. Devices found: N" apos conclusao do scan
    - _Requirements: 1.1, 1.3, 10.6_

- [ ] 16. Refatorar src/main_raygui.c - Parte 3: extrair funcoes de acao e simplificar main
  - [ ] 16.1 Extrair action_start_scan
    - Criar static void action_start_scan(UIState* ui, ScanConfig* cfg, DeviceList* results)
    - Mover logica do botao Scan: auto-fill de subnet, parse_range com errbuf, ip_to_uint, parallel_scan_start com logger_ctx
    - Exibir mensagem de erro descritiva no log se parse_range retornar false
    - _Requirements: 1.3, 5.1, 5.2, 5.3, 5.4, 5.5_
  - [ ] 16.2 Extrair action_stop_scan, action_export_csv e action_export_json
    - Criar static void action_stop_scan(void) chamando parallel_scan_stop
    - Criar static void action_export_csv(DeviceList* results) com GetSaveFileNameA, timestamp no nome padrao, chamada a export_results_to_file e feedback no log
    - Criar static void action_export_json(DeviceList* results) com GetSaveFileNameA, timestamp no nome padrao, chamada a export_results_to_json e feedback no log
    - _Requirements: 11.1, 11.2, 11.3, 11.4, 11.5, 12.5_
  - [ ] 16.3 Simplificar main para ~40 linhas
    - Inicializar UIState, DeviceList, ScanConfig, gui_log_init
    - Loop principal: parallel_scan_snapshot, parallel_scan_get_state, chamadas as funcoes draw_*
    - Encerramento: parallel_scan_stop, device_list_clear, gui_log_destroy, CloseWindow
    - _Requirements: 1.1, 1.3_

- [ ] 17. Checkpoint - Verificar que build.ps1 -Compiler Clang compila e o executavel abre sem crash
  - Executar build.ps1 -Compiler Clang e confirmar que bin/catnet_scanner.exe e produzido sem erros de compilacao ou link, perguntar ao usuario se houver duvidas.

- [ ] 18. Criar .github/workflows/build.yml para CI
  - Criar o arquivo .github/workflows/build.yml com o conteudo exato do design: trigger em push e pull_request para qualquer branch, runner windows-latest, checkout com submodules: recursive, step Build executando build.ps1 -Compiler Clang, step Run unit tests executando build_tests.ps1 e bin/catnet_tests.exe, step Upload artifact para bin/catnet_scanner.exe
  - _Requirements: 14.1, 14.2, 14.3, 14.4, 14.5, 14.6_

- [ ] 19. Criar .clang-format e aplicar formatacao nos arquivos modificados
  - Criar .clang-format na raiz com: BasedOnStyle: Microsoft, IndentWidth: 4, ColumnLimit: 120, UseTab: Never, BreakBeforeBraces: Allman, AllowShortFunctionsOnASingleLine: None, AllowShortIfStatementsOnASingleLine: Never, SortIncludes: false
  - Executar clang-format -i em todos os arquivos .c e .h modificados neste conjunto de melhorias
  - Verificar com clang-format --dry-run --Werror que nao ha diferencas de formatacao
  - _Requirements: 15.1, 15.2, 15.3_

- [ ] 20. Atualizar README.md e documentacao
  - Adicionar secao de requisitos de privilegio de Administrador com instrucao de como executar como admin
  - Documentar build.ps1 com parametros -Compiler Clang e -Compiler MSVC
  - Documentar exportacao CSV e JSON com descricao do formato dos arquivos
  - Documentar limitacoes conhecidas: MAC via ARP apenas na mesma sub-rede, ICMP requer admin, range maximo de 65536 enderecos
  - Adicionar secao ## Roadmap com melhorias futuras planejadas
  - Corrigir referencias a funcionalidades nao implementadas (ex.: "LAN only" que e dead code removido)
  - _Requirements: 16.1, 16.2, 16.3, 16.4, 16.5, 16.6_

- [ ] 21. Checkpoint final - Build completo, testes e verificacao de CI
  - Executar build.ps1 -Compiler Clang e confirmar bin/catnet_scanner.exe produzido sem erros
  - Executar build_tests.ps1 e confirmar bin/catnet_tests.exe retorna codigo 0
  - Executar clang-format --dry-run --Werror nos arquivos modificados e confirmar sem diferencas
  - Garantir que todos os testes passam, perguntar ao usuario se houver duvidas.

## Notes

- Tarefas marcadas com `*` sao opcionais e podem ser puladas para um MVP mais rapido
- A ordem das tarefas e obrigatoria: utils.c (task 2) -> range_parser.c (task 3) -> testes (tasks 4-5) -> scan.c (task 7) -> parallel_scan.c (task 8) -> net.c (task 9) -> export.c (task 11) -> UI (tasks 14-16)
- raylib_shims.c (task 1) deve ser concluido antes de qualquer modificacao em utils.c para evitar simbolos duplicados
- Cada tarefa referencia requisitos especificos para rastreabilidade
- Checkpoints garantem validacao incremental a cada conjunto de mudancas relacionadas
- Property tests validam propriedades universais de corretude; unit tests validam exemplos especificos e casos de borda
