# Fase 2: Refatoração do Desktop e Criação da CLI

Nesta fase vamos conectar as pontas: arrancar a lógica duplicada do Desktop e fazer os novos clientes utilizarem o `catnet-core` como a única fonte de verdade.

## 1. Refatoração do `catnet-scanner` (Desktop)

O repositório GUI desktop deve perder sua responsabilidade de domínio e atuar puramente como uma ponte (Wails Bindings) entre a interface React e o motor remoto.

### Alterações Propostas:
1. **[DELETE] `pkg/scanner/` e `pkg/exporter/`**
   - Vamos excluir estas pastas inteiras do projeto atual, pois elas já residem no `catnet-core`.
2. **[MODIFY] `go.mod`**
   - Adicionar a dependência oficial: `require github.com/mendsec/catnet-core v0.0.0`.
   - Adicionar diretiva local para facilitar desenvolvimento ágil sem precisar commitar: `replace github.com/mendsec/catnet-core => ../catnet-core`.
3. **[MODIFY] `app.go` (Wails bindings)**
   - Trocar todas as referências de `scanner.*` para o pacote do core (ex: `results.HostResult`, `scan.Engine`).
   - Refatorar a função `StartScan` para assinar o canal de eventos Go do `scan.Engine` em uma goroutine, e então emitir o evento traduzido para o `wails.EventsEmit`. 

## 2. Inicialização do `catnet` (CLI)

> **Nota sobre repositório local:** Você perguntou se precisaríamos clonar remotamente. A resposta é **não**! Nós já criamos a pasta `C:\Antigravity\catnet` localmente junto com os outros durante o bootstrap e ela já está amarrada ao GitHub. Trabalharemos diretamente nela.

### Alterações Propostas:
1. **[MODIFY] `catnet/go.mod`**
   - Adicionar dependência para o core: `github.com/mendsec/catnet-core` (com replace local).
   - Instalar biblioteca de CLI `github.com/spf13/cobra` para gerenciar os subcomandos.
2. **[NEW] `cmd/catnet/root.go`**
   - Comando raiz base (`catnet`).
3. **[NEW] `cmd/catnet/version.go`**
   - Retorna a versão da engine.
4. **[NEW] `cmd/catnet/scan.go`**
   - Subcomando que instancia o `scan.Engine`, passa os targets, consome o channel de eventos no terminal, e chama o `export.ExportJSON` imprimindo o resultado final na tela.

## Verification Plan

### Automated/Local Tests
- Rodar `go build` no `catnet-scanner` para provar que a refatoração de dependências da UI está válida.
- Rodar `go run ./cmd/catnet scan 8.8.8.8` no diretório da CLI e verificar se o Output em texto json é gerado com sucesso pelo novo Core.
