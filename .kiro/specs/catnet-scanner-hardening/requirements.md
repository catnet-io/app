# Requirements Document

## Introduction

CatNet Scanner Hardening é um conjunto abrangente de melhorias estruturais, de segurança, concorrência, UX e build/CI para o CatNet Scanner — utilitário Windows com GUI para diagnóstico de rede, escrito em C, usando raylib/raygui, WinSock2, ICMP API e IP Helper API.

O escopo cobre: refatoração da UI em funções coesas, separação de módulos (shims raylib), eliminação de estado global inseguro no logger, máquina de estados explícita para o scan paralelo, validação rigorosa de ranges/CIDR, logger da GUI thread-safe, migração de APIs de rede deprecadas, correção do uso de ICMP, verificação de privilégios de administrador, melhorias de UX (progresso real, estados do botão Stop), exportação CSV e JSON integradas à UI, testes unitários, CI com GitHub Actions, padronização de estilo com clang-format e atualização do README/documentação.

## Glossary

- **Scanner**: O executável `catnet_scanner.exe` e seu conjunto de módulos C.
- **UI**: A interface gráfica construída com raylib/raygui em `src/main_raygui.c`.
- **Parallel_Scan**: O módulo `src/parallel_scan.c` responsável pelo scan multi-thread.
- **Net**: O módulo `src/net.c` responsável pelas operações de rede (ping ICMP, DNS reverso, ARP, TCP).
- **Scan**: O módulo `src/scan.c` responsável pela lógica de identificação de dispositivos.
- **Utils**: O módulo `src/utils.c` com utilitários genéricos de string/IP.
- **Raylib_Shims**: O novo módulo `src/raylib_shims.c` / `src/raylib_shims.h` para shims de compatibilidade do raylib.
- **Export**: O módulo `src/export.c` responsável pela exportação de resultados.
- **GUI_Logger**: O subsistema de log da interface gráfica, protegido por CRITICAL_SECTION.
- **ScanState**: A estrutura de estado do scan paralelo em `parallel_scan.c`.
- **ScanStateMachine**: A máquina de estados explícita com valores SCAN_IDLE, SCAN_RUNNING, SCAN_STOPPING.
- **DeviceList**: A estrutura de lista de dispositivos descobertos (`DeviceList` em `app.h`).
- **DeviceInfo**: A estrutura de informações de um dispositivo (`DeviceInfo` em `app.h`).
- **ScanLogFn**: O tipo de ponteiro de função para callback de log (`typedef void (*ScanLogFn)(const char* msg)`).
- **CIDR**: Notação de roteamento inter-domínio sem classe (ex.: `192.168.1.0/24`).
- **ARP**: Address Resolution Protocol, usado para obter endereços MAC na mesma sub-rede.
- **ICMP**: Internet Control Message Protocol, usado para ping.
- **CI**: Integração Contínua via GitHub Actions.
- **clang-format**: Ferramenta de formatação automática de código C/C++.

---

## Requirements

### Requirement 1: Refatoração Estrutural da UI

**User Story:** Como desenvolvedor, quero que a UI seja dividida em funções menores e coesas, para que o código seja mais fácil de manter, revisar e testar.

#### Acceptance Criteria

1. THE UI SHALL extrair os blocos de renderização de `src/main_raygui.c` para as funções `draw_toolbar`, `draw_config_panel`, `draw_quick_tools`, `draw_results_table`, `draw_debug_log` e `draw_status_bar`, cada uma com responsabilidade única.
2. THE UI SHALL criar um helper reutilizável `filter_digits(char* buf, int max_len)` que remove caracteres não numéricos de um buffer, eliminando a duplicação da lógica de filtragem de octetos presente nos quatro campos de IP.
3. THE UI SHALL separar o estado da UI (variáveis estáticas de controle), o parsing/validação de entradas, a renderização e as ações de botões em seções ou arquivos distintos, de modo que cada responsabilidade seja identificável sem ambiguidade.
4. WHEN `filter_digits` é chamada com um buffer contendo caracteres não numéricos, THE UI SHALL remover todos os caracteres não numéricos e preservar apenas os dígitos, sem exceder `max_len - 1` caracteres úteis.

---

### Requirement 2: Coesão de Módulos — Raylib Shims

**User Story:** Como desenvolvedor, quero que os shims/hacks do raylib sejam isolados em um módulo dedicado, para que `utils.c` contenha apenas utilitários genéricos e a separação de responsabilidades seja clara.

#### Acceptance Criteria

1. THE Scanner SHALL criar os arquivos `src/raylib_shims.c` e `src/raylib_shims.h` contendo todas as implementações de shims de compatibilidade do raylib (`TraceLog`, `MemAlloc`, `MemRealloc`, `MemFree`, `LoadFileData`, `UnloadFileData`, `SaveFileData`, `LoadFileText`, `UnloadFileText`, `SaveFileText`).
2. THE Utils SHALL conter apenas funções de utilitários de string/IP/helpers genéricos (`ip_to_uint`, `uint_to_ip`, `trim_newline`, `safe_strcpy`) após a migração dos shims.
3. WHEN o projeto é compilado com os novos módulos, THE Scanner SHALL produzir o executável `bin/catnet_scanner.exe` sem erros de símbolo duplicado ou ausente.

---

### Requirement 3: Remoção do Logger Global Inseguro

**User Story:** Como desenvolvedor, quero eliminar o estado global `g_logger` em `scan.c`, para que o módulo de scan seja seguro para uso em contextos paralelos e testável de forma isolada.

#### Acceptance Criteria

1. THE Scan SHALL remover a variável estática `static ScanLogFn g_logger` e a função `scan_set_logger` de `scan.c` e `scan.h`.
2. THE Scan SHALL passar o logger e o contexto explicitamente como parâmetros nas funções `identify_device(DeviceInfo* info, const ScanConfig* cfg, ScanLogFn logger, void* logger_ctx)`, `scan_subnet` e `scan_range`.
3. WHEN `identify_device` é chamada com `logger` igual a `NULL`, THE Scan SHALL executar normalmente sem tentar invocar o callback de log.
4. THE Parallel_Scan SHALL passar o logger armazenado em `ScanState` para `identify_device` em cada chamada do worker, garantindo compatibilidade com scan paralelo.

---

### Requirement 4: Máquina de Estados do Scan Paralelo

**User Story:** Como desenvolvedor, quero uma máquina de estados explícita no scan paralelo, para que as transições de ciclo de vida sejam seguras, previsíveis e reflitam corretamente o estado na UI.

#### Acceptance Criteria

1. THE Parallel_Scan SHALL definir o enum `ScanStateMachine` com os valores `SCAN_IDLE`, `SCAN_RUNNING` e `SCAN_STOPPING`.
2. THE Parallel_Scan SHALL proteger todas as transições de estado com sincronização adequada (ex.: `InterlockedCompareExchange` ou `CRITICAL_SECTION`) para evitar condições de corrida.
3. WHEN `parallel_scan_start` é chamada enquanto o estado é `SCAN_RUNNING` ou `SCAN_STOPPING`, THE Parallel_Scan SHALL retornar 0 (falha) sem iniciar um novo scan.
4. THE Parallel_Scan SHALL nunca zerar a `ScanState` com `memset` enquanto existirem handles de thread ativos.
5. WHEN `parallel_scan_stop` é chamada, THE Parallel_Scan SHALL transicionar para `SCAN_STOPPING`, aguardar o término dos workers e então transicionar para `SCAN_IDLE`.
6. THE Scanner SHALL expor a função `parallel_scan_get_state(void)` que retorna o valor atual do `ScanStateMachine`.
7. THE UI SHALL refletir os estados `SCAN_IDLE`, `SCAN_RUNNING` e `SCAN_STOPPING` com rótulos visuais distintos na toolbar e no status bar.

---

### Requirement 5: Validação e Limite de Ranges de Scan

**User Story:** Como usuário, quero que entradas inválidas de range/CIDR sejam rejeitadas com mensagens descritivas, para que eu não inicie scans acidentais em ranges incorretos ou excessivamente grandes.

#### Acceptance Criteria

1. THE Scanner SHALL validar prefixos CIDR no intervalo `[0, 32]`; IF o prefixo estiver fora desse intervalo, THEN THE Scanner SHALL rejeitar a entrada e exibir a mensagem `"CIDR prefix must be between 0 and 32"`.
2. THE Scanner SHALL calcular o número de endereços no range antes de iniciar o scan; IF o número de endereços exceder 65536, THEN THE Scanner SHALL rejeitar a entrada e exibir a mensagem `"Range exceeds maximum of 65536 addresses"`.
3. WHEN `parse_range` recebe uma string com formato inválido (sem `-` ou `/`, ou com octetos fora de `[0, 255]`), THE Scanner SHALL retornar falso e preencher um buffer de erro com uma mensagem descritiva.
4. WHEN o IP de início é numericamente maior que o IP de fim em um range explícito, THE Scanner SHALL rejeitar a entrada e exibir a mensagem `"Start IP must be less than or equal to end IP"`.
5. THE Scanner SHALL validar que cada octeto de um endereço IP está no intervalo `[0, 255]` antes de aceitar a entrada.

---

### Requirement 6: Logger da GUI Thread-Safe

**User Story:** Como desenvolvedor, quero que o buffer de logs da GUI seja protegido por sincronização, para que chamadas concorrentes de múltiplos workers não causem corrupção de dados ou condições de corrida.

#### Acceptance Criteria

1. THE GUI_Logger SHALL proteger o buffer de log circular (`g_logLines`, `g_logCount`) com uma `CRITICAL_SECTION` inicializada antes do primeiro uso e destruída no encerramento.
2. THE GUI_Logger SHALL expor as funções `gui_log_push(const char* msg)`, `gui_log_snapshot(char out[][160], int* count, int max_lines)` e `gui_log_clear(void)` como interface pública thread-safe.
3. WHEN `gui_log_push` é chamada simultaneamente por múltiplos threads, THE GUI_Logger SHALL garantir que nenhuma mensagem seja corrompida ou perdida por condição de corrida.
4. WHEN `gui_log_snapshot` é chamada, THE GUI_Logger SHALL copiar o estado atual do buffer para `out` de forma atômica em relação a chamadas concorrentes de `gui_log_push`.
5. THE GUI_Logger SHALL atualizar `g_statusText` dentro da seção crítica ao receber uma nova mensagem via `gui_log_push`.

---

### Requirement 7: Migração de APIs de Rede Deprecadas

**User Story:** Como desenvolvedor, quero substituir as APIs de rede deprecadas por equivalentes modernos, para que o código seja compatível com versões futuras do Windows SDK e livre de avisos de deprecação.

#### Acceptance Criteria

1. THE Net SHALL substituir todas as chamadas a `inet_addr` por `InetPton` (ou `inet_pton`) para conversão de endereços IPv4 em `net.c` e `utils.c`.
2. THE Net SHALL substituir todas as chamadas a `gethostbyaddr` por `getnameinfo` para resolução DNS reversa em `net.c`.
3. THE Net SHALL substituir todas as chamadas a `inet_ntoa` por `InetNtop` (ou `inet_ntop`) para conversão de endereços IPv4 em `utils.c`.
4. WHEN uma chamada de API de rede falha, THE Net SHALL registrar o código de erro do WSA (`WSAGetLastError()`) na mensagem de log, em vez de retornar silenciosamente.
5. THE Scanner SHALL compilar sem os defines `_WINSOCK_DEPRECATED_NO_WARNINGS` após a migração das APIs.

---

### Requirement 8: Correção do Uso de ICMP

**User Story:** Como desenvolvedor, quero remover o carregamento dinâmico desnecessário de `Icmp.dll`, para que o código seja mais simples, sem risco de vazamento de handle de módulo e com linkagem direta e previsível.

#### Acceptance Criteria

1. THE Net SHALL remover as chamadas a `LoadLibraryA("Icmp.dll")`, `GetProcAddress` e `FreeLibrary` de `net_ping_ipv4`.
2. THE Net SHALL chamar `IcmpCreateFile`, `IcmpSendEcho` e `IcmpCloseHandle` diretamente, via linkagem estática com `Iphlpapi.lib` (já presente no build).
3. WHEN `IcmpCreateFile` retorna `INVALID_HANDLE_VALUE`, THE Net SHALL registrar o erro e retornar 0 sem vazar recursos.
4. WHEN `IcmpSendEcho` retorna 0 (falha), THE Net SHALL fechar o handle ICMP com `IcmpCloseHandle` antes de retornar.

---

### Requirement 9: Verificação de Privilégios de Administrador

**User Story:** Como usuário, quero ser avisado quando o Scanner não está sendo executado como Administrador, para que eu entenda por que ping ICMP e ARP podem falhar.

#### Acceptance Criteria

1. WHEN o Scanner é iniciado, THE Scanner SHALL verificar se o processo está sendo executado com privilégios de Administrador usando `IsUserAnAdmin()` ou equivalente via `CheckTokenMembership`.
2. IF o Scanner não está sendo executado como Administrador, THEN THE UI SHALL exibir um aviso visível na toolbar ou no status bar com o texto `"Warning: not running as Administrator — ICMP/ARP may fail"`.
3. THE Scanner SHALL realizar a verificação de privilégios uma única vez no startup e armazenar o resultado em uma variável booleana imutável durante a execução.
4. WHEN o Scanner está sendo executado como Administrador, THE UI SHALL não exibir o aviso de privilégios.

---

### Requirement 10: Melhorias de UX — Progresso, Stop e Status

**User Story:** Como usuário, quero ver o progresso real do scan, estados visuais corretos no botão Stop e o status atual na toolbar, para que eu tenha feedback claro sobre o que está acontecendo.

#### Acceptance Criteria

1. THE Parallel_Scan SHALL expor a função `parallel_scan_progress(float* out_fraction)` que retorna a fração de endereços processados no intervalo `[0.0, 1.0]`.
2. THE UI SHALL exibir uma barra de progresso real durante o scan, atualizada a cada frame com o valor retornado por `parallel_scan_progress`.
3. THE UI SHALL exibir o botão "Stop" desabilitado (estado visual cinza) quando o estado for `SCAN_IDLE` e habilitado quando o estado for `SCAN_RUNNING`.
4. WHEN o estado é `SCAN_STOPPING`, THE UI SHALL exibir o botão "Stop" com o rótulo `"Stopping..."` e desabilitado para cliques adicionais.
5. THE UI SHALL exibir na toolbar um indicador de estado textual com os valores `"Idle"`, `"Scanning"`, `"Stopping"` ou `"Error"` correspondentes ao estado atual do `ScanStateMachine`.
6. WHEN o scan é concluído, THE UI SHALL atualizar o status bar com a contagem de dispositivos encontrados no formato `"Done. Devices found: N"`.

---

### Requirement 11: Exportação CSV Integrada à UI

**User Story:** Como usuário, quero exportar os resultados do scan para um arquivo CSV diretamente pela interface, para que eu possa analisar os dados em outras ferramentas.

#### Acceptance Criteria

1. THE UI SHALL exibir um botão `"Export CSV"` na toolbar, visível quando `results.count > 0`.
2. WHEN o botão `"Export CSV"` é clicado, THE UI SHALL abrir um diálogo de seleção de arquivo usando `GetSaveFileNameA` com filtro `"CSV Files (*.csv)\0*.csv\0"`.
3. WHEN o usuário confirma o caminho no diálogo, THE Export SHALL chamar `export_results_to_file(path, &results)` e exibir `"Export successful: <path>"` no status bar em caso de sucesso.
4. IF `export_results_to_file` retorna 0 (falha), THEN THE UI SHALL exibir `"Export failed: could not write file"` no status bar.
5. THE UI SHALL pré-preencher o nome do arquivo no diálogo com um timestamp no formato `catnet_YYYYMMDD_HHMMSS.csv`.

---

### Requirement 12: Exportação JSON

**User Story:** Como usuário, quero exportar os resultados do scan para um arquivo JSON, para que eu possa integrá-los com ferramentas de automação e scripts.

#### Acceptance Criteria

1. THE Export SHALL implementar a função `export_results_to_json(const char* path, const DeviceList* list)` que retorna 1 em sucesso e 0 em falha.
2. THE Export SHALL gerar um JSON com a estrutura `{"devices": [{"ip": "...", "hostname": "...", "mac": "...", "ports": [...]}]}`, onde `ports` é um array de inteiros com as portas abertas.
3. WHEN `hostname` ou `mac` estão vazios, THE Export SHALL emitir uma string vazia `""` para o campo correspondente no JSON.
4. THE Export SHALL escapar corretamente caracteres especiais JSON (`"`, `\`, e caracteres de controle) nos campos de string.
5. THE UI SHALL exibir um botão `"Export JSON"` na toolbar, visível quando `results.count > 0`, com comportamento análogo ao botão `"Export CSV"` (diálogo de arquivo, feedback de sucesso/erro, nome padrão com timestamp).

---

### Requirement 13: Testes Unitários

**User Story:** Como desenvolvedor, quero testes unitários para as funções críticas do Scanner, para que regressões sejam detectadas automaticamente no CI.

#### Acceptance Criteria

1. THE Scanner SHALL incluir um conjunto de testes unitários para as funções `parse_range`, `ip_to_uint`, `uint_to_ip`, `device_list_push` e `filter_digits`.
2. THE Scanner SHALL usar um framework de testes leve (Unity ou harness próprio em C puro) sem dependências externas além do compilador C.
3. WHEN todos os testes passam, THE Scanner SHALL retornar código de saída 0 do executável de testes.
4. IF qualquer teste falha, THEN THE Scanner SHALL imprimir o nome do teste, o valor esperado e o valor obtido, e retornar código de saída não-zero.
5. THE Scanner SHALL incluir testes para os seguintes casos de `parse_range`:
   a. Range simples `"192.168.1.1-254"` → start `192.168.1.1`, end `192.168.1.254`.
   b. Range completo `"10.0.0.1-10.0.0.100"` → start `10.0.0.1`, end `10.0.0.100`.
   c. CIDR `"192.168.1.0/24"` → start `192.168.1.0`, end `192.168.1.255`.
   d. CIDR inválido `"192.168.1.0/33"` → retorna falso.
   e. Formato inválido `"not-an-ip"` → retorna falso.
6. THE Scanner SHALL incluir testes de round-trip para `ip_to_uint` e `uint_to_ip`: para todo IP válido `s`, `uint_to_ip(ip_to_uint(s))` SHALL produzir string equivalente a `s`.

---

### Requirement 14: CI com GitHub Actions

**User Story:** Como desenvolvedor, quero um workflow de CI que compile e teste o projeto automaticamente em cada push e PR, para que problemas de build sejam detectados antes do merge.

#### Acceptance Criteria

1. THE Scanner SHALL incluir o arquivo `.github/workflows/build.yml` com um workflow que é acionado em `push` e `pull_request` para qualquer branch.
2. THE CI SHALL usar o runner `windows-latest` para compilação nativa com clang-cl/MSVC.
3. THE CI SHALL fazer checkout do repositório com `submodules: recursive` para incluir raylib e raygui.
4. THE CI SHALL executar `build.ps1 -Compiler Clang` e falhar o workflow se o código de saída for não-zero.
5. THE CI SHALL compilar e executar o executável de testes unitários e falhar o workflow se o código de saída for não-zero.
6. WHEN o build e os testes passam, THE CI SHALL produzir o artefato `bin/catnet_scanner.exe` disponível para download no workflow.

---

### Requirement 15: Padronização de Estilo com clang-format

**User Story:** Como desenvolvedor, quero um arquivo `.clang-format` e formatação consistente nos arquivos modificados, para que o estilo de código seja uniforme e revisões de PR não sejam poluídas por diferenças de formatação.

#### Acceptance Criteria

1. THE Scanner SHALL incluir um arquivo `.clang-format` na raiz do projeto com estilo baseado em `Microsoft` ou `LLVM`, com indentação de 4 espaços e largura de coluna de 120.
2. THE Scanner SHALL aplicar a formatação do `.clang-format` a todos os arquivos `.c` e `.h` modificados neste conjunto de melhorias.
3. WHEN `clang-format --dry-run --Werror` é executado nos arquivos modificados, THE Scanner SHALL não reportar diferenças de formatação.

---

### Requirement 16: README e Documentação

**User Story:** Como usuário e contribuidor, quero um README atualizado e preciso, para que eu possa compilar, executar e entender as limitações e funcionalidades do Scanner sem ambiguidade.

#### Acceptance Criteria

1. THE Scanner SHALL atualizar o `README.md` para documentar os requisitos de privilégio de Administrador para ICMP e ARP, com instrução explícita de como executar como Administrador.
2. THE Scanner SHALL documentar no `README.md` o processo de compilação com `build.ps1`, incluindo os parâmetros `-Compiler Clang` e `-Compiler MSVC`.
3. THE Scanner SHALL documentar no `README.md` as funcionalidades de exportação CSV e JSON, incluindo o formato dos arquivos gerados.
4. THE Scanner SHALL documentar no `README.md` as limitações conhecidas: MAC via ARP funciona apenas na mesma sub-rede; ICMP requer privilégios de Administrador; range máximo de 65536 endereços.
5. THE Scanner SHALL incluir uma seção `## Roadmap` no `README.md` listando melhorias futuras planejadas.
6. THE Scanner SHALL corrigir quaisquer inconsistências entre o README e o comportamento real do Scanner (ex.: referências a funcionalidades não implementadas ou comandos incorretos).
