# Análise Crítica — CatNet Scanner (mendsec/catnet_scanner)

---

## 1. Estado Atual do Projeto

| Métrica | Valor | Avaliação |
|---|---|---|
| Stars | 0 | Sem adoção externa |
| Forks | 0 | Sem contribuidores |
| Issues abertas | 0 | Sem uso real ou silêncio total |
| Issues fechadas | 0 | Nunca usou o sistema de issues |
| Pull Requests | 0 | Desenvolvimento 100% solo |
| Releases publicadas | **Nenhuma** | Sem versão distribuível |
| Contributors | 1 (mendsec) | Projeto de um desenvolvedor |
| CI/CD ativo | **Nenhum** | Aba Actions vazia |
| Branches | Apenas `main` | Sem estratégia de branching |
| Commits totais | 85 | Volume razoável para o estágio |
| Última atualização | Recente (2026) | Ativo no momento |
| Versão declarada | v0.4.0 (apenas no README) | Nunca publicada via GitHub Releases |
| Linguagens | Go 43.8%, TypeScript 28.4%, CSS 26.9% | Stack coerente com o produto |

**Diagnóstico de status:** O projeto está em desenvolvimento ativo por um único autor, mas em estágio de **protótipo pessoal** — sem nenhum dos sinais que caracterizam um projeto open source maduro: sem releases, sem CI, sem comunidade, sem testes automatizados em execução.

---

## 2. Qualidade do Código

### Arquitetura

A arquitetura Go/Wails/React é tecnicamente sólida e bem escolhida para o problema. A separação em `pkg/scanner/` com três arquivos (`net.go`, `scan.go`, `utils.go`) é limpa. O problema está nos **limites entre camadas**: `app.go` na raiz mistura responsabilidades de validação, formatação de saída (CSV/XML/TXT inline) e orquestração IPC sem nenhuma abstração.

**Problemas identificados no código:**

`app.go` — ExportResults tem 50+ linhas de formatação de string inline sem nenhum módulo separado:
```go
// Problema: lógica de formatação diretamente no handler IPC
if len(savePath) > 4 && savePath[len(savePath)-4:] == ".txt" {
    data = "CatNet Scanner Results\n---..."
```
Isso deveria estar em `pkg/exporter/` com interfaces por formato.

`pkg/scanner/scan.go` — `cancelScan` é uma variável global de pacote, não protegida por mutex, criando race condition real quando `StopScan` é chamado de uma goroutine enquanto `StartScan` ainda está inicializando:
```go
// Race condition: cancelScan pode ser nil quando StopScan é chamado
var cancelScan context.CancelFunc  // global não protegido

func StopScan() {
    if cancelScan != nil {  // sem lock — data race
        cancelScan()
    }
}
```

`pkg/scanner/net.go` — `GetMAC` usa `syscall.NewLazyDLL` que tem carregamento tardio sem tratamento de erro explícito. Se `iphlpapi.dll` não existir (Linux/macOS com Wine ou ambiente virtualizado), o panic ocorre na chamada `.Call()`, não em `NewLazyDLL`.

`frontend/src/App.tsx` — Ausência de `useCallback` em handlers de evento (`handleScan`, `handleStop`, `handleExport`) causa re-renders desnecessários. O estado de ordenação (`sortCol`, `sortAsc`) junto com `devices` (array grande) força re-sort O(n log n) a cada render.

### Complexidade e Duplicação

| Arquivo | Linhas | Problema |
|---|---|---|
| `app.go` | ~130 | Formatação CSV/XML/TXT inline, sem módulo |
| `frontend/src/App.tsx` | ~160 | Componente único sem separação de responsabilidade |
| `pkg/scanner/net.go` | ~70 | Adequado |
| `pkg/scanner/scan.go` | ~90 | Adequado, mas com race condition |

O `CHANGES.md` registra que `docs/RELEASE_NOTES_v0.2.0.md` foi removido, mas o arquivo ainda aparece no repositório real (`docs/RELEASE_NOTES_v0.2.0.md` está presente). Indica que as mudanças do workflow do Antigravity não foram totalmente commitadas ou há divergência entre o estado local e o remoto.

---

## 3. Documentação

| Item | Existe? | Qualidade |
|---|---|---|
| README com instalação | ✅ | Adequado mas apenas em PT-BR |
| Exemplos de uso com screenshots | ❌ | Referenciado mas sem imagem real |
| CONTRIBUTING.md | ❌ | Ausente |
| SECURITY.md / security policy | ❌ | Ausente |
| LICENSE | ❌ | **Ausente** — repositório público sem licença |
| Changelog estruturado | ⚠️ | CHANGES.md existe mas não segue padrão (Keep a Changelog) |
| Documentação de API/IPC | ❌ | Ausente |
| Descrição do repositório no GitHub | ❌ | "No description, website, or topics provided" |
| Topics/tags no GitHub | ❌ | Nenhum |

**O ponto mais crítico: ausência de licença.** Um repositório público sem `LICENSE` é tecnicamente "todos os direitos reservados" por padrão no GitHub, o que significa que ninguém pode legalmente usar, copiar, modificar ou distribuir o código sem permissão explícita, tornando inviável qualquer contribuição ou adoção.

O README está inteiramente em português, o que limita a descoberta internacional. Para um projeto que se posiciona como alternativa ao Angry IP Scanner (ferramenta com audiência global), isso é uma barreira significativa.

---

## 4. Segurança

### Vulnerabilidades nas Dependências

| Dependência | Versão | Situação |
|---|---|---|
| `github.com/wailsapp/wails/v2` | v2.12.0 | A verificar — Wails tem histórico de CVEs |
| `golang.org/x/crypto` | v0.33.0 | Recente, adequada |
| `golang.org/x/net` | v0.35.0 | Recente, adequada |
| `github.com/gorilla/websocket` | v1.5.3 | Recente, adequada |
| `vite` (frontend) | v3.2.11 | **Desatualizada** — v3 está em EOL, v6 é atual |
| `react` | v18.3.1 | Adequada |
| `typescript` | v4.9.5 | **Desatualizada** — v5.x é atual |
| `@vitejs/plugin-react` | v2.2.0 | **Desatualizada** — vinculada ao Vite 3 |

**Não existe `govulncheck`, `dependabot` ou qualquer scanner automático de vulnerabilidades configurado.**

### Práticas de Segurança no Código (estado atual pós-refatoração do Antigravity)

As correções do workflow já foram aplicadas (`validateIPv4`, cap de threads, sanitização de path). Porém, permanecem os seguintes problemas não cobertos:

**Race condition em `cancelScan`** (não coberta pelo workflow anterior):
```go
// Problema real que permanece: sem sync.Mutex protegendo cancelScan
var (
    isScanning atomic.Bool
    cancelScan context.CancelFunc  // não protegido
)
```
`cancelScan` é escrito em `StartScan` (goroutine de trabalho) e lido em `StopScan` (pode ser chamado do frontend a qualquer momento). `go race detector` detectaria isso.

**Exportação CSV sem escape de campos com vírgula** — o formato CSV gerado em `app.go` usa vírgula como separador mas não escapa campos que contêm vírgulas:
```go
data += fmt.Sprintf("%s,%s,%s,%s,%s\n", d.IP, d.Hostname, d.MAC, status, ports)
```
Um hostname como `server,backup` quebraria a estrutura do CSV.

**XML sem escape** — o formato XML também concatena strings diretamente sem `xml.EscapeText`, vulnerável a XML injection em hostnames maliciosos:
```go
data += fmt.Sprintf("\t\t<hostname>%s</hostname>", d.Hostname)
// hostname: "</hostname><malicious>" — injeta XML válido
```

**Ausência de `SECURITY.md`** — não há canal para reporte responsável de vulnerabilidades.

---

## 5. Testes e Qualidade

| Aspecto | Status | Detalhe |
|---|---|---|
| Testes unitários Go | ❌ | Nenhum arquivo `*_test.go` |
| Testes de integração | ❌ | Ausentes |
| CI/CD pipeline | ❌ | Aba Actions vazia, sem workflows |
| `go test ./...` configurado | ❌ | Não |
| `go vet` automatizado | ❌ | Não |
| `staticcheck` / `golangci-lint` | ❌ | Não configurados |
| Race detector (`go test -race`) | ❌ | Não |
| Frontend tests (Vitest/Jest) | ❌ | Não configurados |
| Coverage reporting | ❌ | Não |
| Quality gates | ❌ | Inexistentes |

Este é o gap mais crítico do projeto inteiro. **Zero cobertura de testes em qualquer camada.** O workflow do Antigravity gerou um `CHANGES.md` detalhado, mas nenhum teste automatizado foi criado para o código Go — apenas os testes C do legado existiam, e foram removidos do rastreamento.

---

## 6. Comunidade e Adoção

| Métrica | Valor |
|---|---|
| Stars | 0 |
| Forks | 0 |
| Watchers | 0 |
| Issues abertas | 0 |
| Issues resolvidas | 0 |
| PRs externos | 0 |
| Discussões | Não habilitadas |
| Sponsor | Não configurado |
| Topics no GitHub | Nenhum |
| Website/demo | Não existe |

O projeto é invisível para a comunidade por ausência de todos os sinais de descoberta: sem topics (`network-scanner`, `go`, `wails`, `windows`), sem descrição no repo, sem release com binário para download, sem screenshots reais. Alguém pesquisando "network scanner Go Windows" no GitHub não encontraria este projeto.

---

## 7. Competitividade

| Ferramenta | Linguagem | UI | Plataformas | Stars aprox. | Diferencial |
|---|---|---|---|---|---|
| **Angry IP Scanner** | Java | Swing | Win/Mac/Linux | 10k+ | Veterano, ampla adoção |
| **nmap** | C/Lua | CLI + Zenmap | Multiplataforma | 10k+ | Padrão da indústria |
| **Masscan** | C | CLI | Win/Linux | 22k+ | Velocidade extrema |
| **Advanced IP Scanner** | C# | WinForms | Windows | Proprietário | Muito popular em PT |
| **CatNet Scanner** | Go | React/Wails | Windows | 0 | Cyberpunk UI, binário único |

**Diferenciais reais do CatNet:**
- Binário único sem JRE (vantagem sobre Angry IP Scanner)
- UI moderna vs. interfaces antigas dos concorrentes
- Ping via executável nativo (contorna restrição de raw socket sem admin)
- Código aberto com stack moderna

**Lacunas críticas em relação à concorrência:**
- Sem scan UDP (todos os concorrentes têm)
- Sem detecção de SO (OS fingerprinting)
- Sem Vendor OUI lookup (mesmo o Angry IP Scanner tem)
- Sem scan de portas configurável pela UI
- Sem histórico/comparação de scans
- Sem exportação JSON
- Windows-only na prática (GetMAC usa `iphlpapi.dll`)

---

## 8. TOP 10 Pontos Críticos para Melhoria

| # | Problema | Impacto | Esforço | Urgência |
|---|---|---|---|---|
| 1 | **Ausência de LICENSE** | Alto | Baixo | Imediata |
| 2 | **Zero testes Go (`go test`)** | Alto | Médio | Imediata |
| 3 | **CI/CD inexistente** | Alto | Baixo | Imediata |
| 4 | **Race condition em `cancelScan`** | Alto | Baixo | Imediata |
| 5 | **XML injection na exportação** | Alto | Baixo | Imediata |
| 6 | **CSV sem escape de campos** | Médio | Baixo | Curto prazo |
| 7 | **Nenhuma Release publicada com binário** | Alto | Baixo | Curto prazo |
| 8 | **Dependências frontend desatualizadas (Vite 3, TS 4)** | Médio | Médio | Curto prazo |
| 9 | **Sem descrição/topics no GitHub** | Médio | Baixo | Imediata |
| 10 | **`app.go` sem módulo de exportação separado** | Médio | Médio | Longo prazo |

---

## 9. Roadmap Sugerido — 3 a 6 Meses

### Mês 1 — Fundação (sem custo de features)

**Semana 1:**
- Adicionar `LICENSE` (MIT recomendado para adoção máxima)
- Adicionar descrição e topics no GitHub: `network-scanner`, `go`, `wails`, `react`, `windows`, `cyberpunk`
- Corrigir race condition em `cancelScan` com `sync.Mutex`

```go
var scanMu sync.Mutex
var cancelScan context.CancelFunc

func StopScan() {
    scanMu.Lock()
    defer scanMu.Unlock()
    if cancelScan != nil {
        cancelScan()
    }
}
```

**Semana 2:**
- Criar `.github/workflows/ci.yml` com `go build`, `go vet`, `go test -race ./...`
- Corrigir XML injection usando `encoding/xml`
- Corrigir CSV usando `encoding/csv`

**Semana 3-4:**
- Escrever primeiros testes Go em `pkg/scanner/`:
  - `utils_test.go`: testes de `ParseRange`
  - `net_test.go`: testes de `validateIPv4`
  - `scan_test.go`: testes de `StartScan` com mock

### Mês 2 — Primeira Release Pública

- Publicar GitHub Release `v0.4.0` com binário `.exe` compilado via Actions
- Adicionar screenshot real da UI no README
- Adicionar `CONTRIBUTING.md` e `SECURITY.md`
- Atualizar frontend: Vite 3 → Vite 5, TypeScript 4 → TypeScript 5
- Mover formatação CSV/XML/TXT de `app.go` para `pkg/exporter/`

### Meses 3-4 — Features de Diferenciação

- **Vendor OUI Lookup** offline (arquivo `.csv` embutido no binário via `embed.FS`)
- **Scan de portas configurável** pela UI (input de portas customizadas)
- **Painel lateral de host** ao clicar em IP
- **Suporte macOS/Linux** removendo dependência de `iphlpapi.dll` para MAC (usar `arp -n` como fallback cross-platform)

### Meses 5-6 — Maturidade e Visibilidade

- **Dependabot** configurado para alertas automáticos de dependências
- **Histórico de scans** persistido localmente (SQLite via `modernc.org/sqlite`)
- **Comparação de scans** — "3 novos hosts detectados desde ontem"
- **Landing page** simples no GitHub Pages com demo em GIF
- Submeter ao **Hacker News** (Show HN) e **r/golang** após ter release, testes e README em inglês

---

### Ferramentas Recomendadas para Implementação Imediata

| Ferramenta | Propósito | Link |
|---|---|---|
| `govulncheck` | Scan de CVEs nas dependências Go | [pkg.go.dev/golang.org/x/vuln](https://pkg.go.dev/golang.org/x/vuln) |
| `golangci-lint` | Linter agregado (inclui `staticcheck`, `errcheck`) | [golangci-lint.run](https://golangci-lint.run) |
| Dependabot | Alertas automáticos de dependências | Configurar em `.github/dependabot.yml` |
| `go test -race` | Detecção de race conditions | Nativo no Go |
| `encoding/xml` e `encoding/csv` | Escape correto nos exportadores | Stdlib Go |
