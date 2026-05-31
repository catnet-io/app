# CHANGELOG - Security Refactor & Limpeza Estrutural

## Adicionados
- `.archive-notice.md`: Criado para atestar o arquivamento do código C/Raylib na branch `legacy/c-raylib`.
- `CHANGES.md`: Este arquivo, sumarizando e atestando a integridade das modificações.

## Removidos
- `MANUAL.md`: Conteúdo integralmente movido e consolidado em `ARCHITECTURE.md` para evitar dualidade de informação.
- `docs/PR_DRAFT_v0.2.0.md`: Arquivo removido permanentemente (draft de PR não pertence ao repositório).
- `frontend/package.json.md5`: Artefato de build removido permanentemente.

## Modificados
- `.gitignore`: Novas exclusões bloqueando a subida de pastas `legacy_c/`, `tests/`, arquivos temporários e de notas (`.jules/`, `.kiro/`).
- `ARCHITECTURE.md`: Atualizado para conter o diagrama de Wails, módulos Go e bindings, unificando a documentação.
- `wails.json`: E-mail pessoal modificado para o genérico `contact@catnet-scanner.dev`.
- `pkg/scanner/net.go`: 
  - Adicionado `validateIPv4` sanitizando imediatamente a entrada para as funções `Ping`, `ReverseDNS`, `GetMAC` e `ScanPorts`.
  - Inserção de atestado formal da validação de acesso em ponteiros na memória (Cgo/syscall) com comentário explicativo em `sendARP.Call`.
- `pkg/scanner/scan.go`: Embutido cap máximo inegociável de threads (`maxAllowedThreads = 256`) protegendo o core independentemente das requisições via frontend/IPC.
- `app.go`: 
  - O endpoint de exportação (`ExportResults`) passa agora pela sanitização nativa via `filepath.Clean(savePath)`.
  - Tratativa de segurança na validação estrutural do payload de `ScanConfig`.
- `frontend/src/App.tsx`: Adição da restrição UX inline validando dinamicamente IPs ou CIDRs, impedindo o disparo equivocado via UI.
