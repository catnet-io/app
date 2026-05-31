---
description: ``` Audita e corrige falhas de segurança no CatNet Scanner (Go/Wails): valida IPs, limita threads, sanitiza paths de exportação, arquiva legado C/Raylib, remove arquivos fora de escopo e consolida documentação duplicada. ```
---

# CatNet Scanner — Refatoração de Segurança e Organização Estrutural

## Contexto do Projeto

Este repositório contém o **CatNet Scanner**, um scanner de rede Windows
construído em Go + Wails + React (produto ativo) com um legado C/Raylib
em `legacy_c/` (obsoleto). O projeto acumula falhas de segurança,
arquivos no lugar errado e uma dualidade arquitetural não resolvida.
Sua missão é executar todas as correções descritas abaixo de forma
incremental, verificando o build a cada etapa antes de avançar.

---

## Regras Globais para Esta Sessão

- Nunca modificar arquivos dentro de `legacy_c/` — eles serão movidos,
  não editados.
- Nunca modificar arquivos dentro de `tests/` que referenciam `../src/`
  — eles serão movidos junto com o legado.
- O produto Go/Wails (`pkg/`, `app.go`, `main.go`, `frontend/`) é o
  alvo de todas as correções de segurança.
- Após cada grupo de tarefas, executar `go build ./...` e confirmar
  saída sem erros antes de prosseguir.
- Cada arquivo criado ou modificado deve ter no máximo uma
  responsabilidade.

---

## Fase 1 — Limpeza Estrutural e Arquivamento do Legado

### Tarefa 1.1 — Criar branch de arquivamento
Crie o arquivo `.archive-notice.md` na raiz com o seguinte conteúdo:

# Aviso de Arquivamento

O diretório `legacy_c/` contém a implementação original em C/Raylib
do CatNet Scanner, substituída pela stack Go/Wails/React em novembro de
2025. Este código não é compilado pelo sistema de build atual.

Para acesso ao histórico completo do legado, consulte a branch
`legacy/c-raylib` neste repositório.

### Tarefa 1.2 — Atualizar `.gitignore`
Adicione as seguintes entradas ao `.gitignore` existente, sem remover
as entradas atuais:

# Artefatos de build e hash de verificação
*.md5

# Arquivos temporários gerados pelos testes C
tmp_test_*.json
tmp_test_*.csv
tmp_test_*.txt
tmp_test_*.bin

# Notas de IA (não devem entrar no controle de versão)
.jules/
.kiro/

# Diretório de legado (mantido localmente, não rastreado)
legacy_c/
tests/

### Tarefa 1.3 — Remover arquivos fora de escopo do controle de versão
Exclua os seguintes arquivos do repositório (git rm --cached ou
remoção direta, conforme aplicável):

- `docs/PR_DRAFT_v0.2.0.md`
- `frontend/package.json.md5`

Estes arquivos não devem existir em um repositório de produto:
rascunhos de PR pertencem a Pull Requests reais; hashes de build
pertencem a pipelines de CI, não ao código-fonte.

### Tarefa 1.4 — Consolidar documentação duplicada
`MANUAL.md` e `ARCHITECTURE.md` descrevem a mesma arquitetura Go/Wails.
Faça o seguinte:

1. Leia ambos os arquivos na íntegra.
2. Crie um novo `ARCHITECTURE.md` consolidado que contenha:
   - Diagrama de arquitetura Go/Wails/React (seção 1)
   - Descrição dos módulos Go em `pkg/scanner/` (seção 2)
   - Bindings IPC via Wails (seção 3)
   - Nota explícita sobre o legado C arquivado (seção 4)
3. Delete `MANUAL.md`.

### Tarefa 1.5 — Remover e-mail pessoal de `wails.json`
Em `wails.json`, substitua o campo `"email"` por um endereço genérico
de projeto:

"email": "contact@catnet-scanner.dev"

---

## Fase 2 — Correções de Segurança no Backend Go

### Tarefa 2.1 — Validar IPs antes de qualquer operação de rede
**Arquivo:** `pkg/scanner/net.go`

Crie uma função interna de validação no início do arquivo:

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

Aplique `validateIPv4` no início de cada função pública:
`Ping`, `ReverseDNS`, `GetMAC`, `ScanPorts`.

Cada função deve retornar imediatamente com o zero-value seguro quando
a validação falhar (ex: `Ping` retorna `false`, `ReverseDNS` retorna
`""`, `GetMAC` retorna `""`, `ScanPorts` retorna `nil`).

### Tarefa 2.2 — Impor limite máximo de threads no backend
**Arquivo:** `pkg/scanner/scan.go`

Na função `StartScan`, após a leitura de `cfg.MaxThreads`, adicione:

const maxAllowedThreads = 256
if threads > maxAllowedThreads {
    threads = maxAllowedThreads
}

Este limite deve estar no backend e ser independente do valor enviado
pelo frontend.

### Tarefa 2.3 — Sanitizar o caminho de exportação
**Arquivo:** `app.go`

Na função `ExportResults`, após obter `filepath` do `SaveFileDialog`,
adicione validação antes de `os.WriteFile`:

import "path/filepath"

// Sanitizar e validar o caminho retornado pelo diálogo
cleanPath := filepath.Clean(filepath)
if cleanPath != filepath {
    return "", fmt.Errorf("caminho de arquivo inválido")
}
// Garantir que o arquivo não está em diretório do sistema
// (validação básica — o diálogo nativo já restringe, mas
// este check adiciona defesa em profundidade)
dir := filepath.Dir(cleanPath)
if dir == "" || dir == "." {
    return "", fmt.Errorf("diretório de destino inválido")
}

### Tarefa 2.4 — Documentar o uso de `unsafe.Pointer` em `net.go`
**Arquivo:** `pkg/scanner/net.go`

Adicione um comentário de segurança acima do bloco `sendARP.Call`
explicando as invariantes:

// Segurança: mac é um array de tamanho fixo [6]byte alocado na stack
// desta função. macLen é inicializado com len(mac) == 6 antes da
// chamada. O acesso via unsafe.Pointer é seguro porque o array não
// escapa do escopo e seu tamanho é conhecido em tempo de compilação.
// A validação `macLen == 6` após o retorno garante que não
// interpretamos dados corrompidos.

### Tarefa 2.5 — Validar `MaxThreads` e `PortTimeoutMs` na entrada
**Arquivo:** `app.go`, função `StartScan`

Antes de chamar `scanner.StartScan`, adicione validação dos campos
do `ScanConfig` recebido do frontend:

// Sanitizar configuração recebida do frontend
if cfg.MaxThreads <= 0 || cfg.MaxThreads > 256 {
    cfg.MaxThreads = 16
}
if cfg.PortTimeoutMs <= 0 || cfg.PortTimeoutMs > 10000 {
    cfg.PortTimeoutMs = 500
}
if cfg.PingTimeoutMs <= 0 || cfg.PingTimeoutMs > 10000 {
    cfg.PingTimeoutMs = 1000
}

---

## Fase 3 — Melhorias de UX e Feedback de Erro no Frontend

### Tarefa 3.1 — Validação inline de IP Range no React
**Arquivo:** `frontend/src/App.tsx`

Adicione uma função de validação client-side para o campo `ipRange`
que mostra feedback visual antes de disparar o scan:

// Valida se a string é um range IP ou CIDR válido antes de enviar
// ao backend. Não substitui a validação do backend.
const isValidIpRange = (value: string): boolean => {
  const cidrPattern = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}\/\d{1,2}$/;
  const dashPattern =
    /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}-(\d{1,3}|\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})$/;
  return cidrPattern.test(value) || dashPattern.test(value);
};

No botão "Start", desabilite-o e exiba uma mensagem de erro inline
(abaixo do input) quando `!isValidIpRange(ipRange)`.

---

## Fase 4 — Verificação Final

### Tarefa 4.1 — Build e lint
Execute em sequência:

go build ./...
go vet ./...

Corrija todos os erros e warnings reportados antes de concluir.

### Tarefa 4.2 — Verificar ausência de arquivos sensíveis
Confirme que os seguintes padrões não existem no repositório após as
mudanças:

- Nenhum arquivo `.md5` rastreado pelo git
- Nenhum arquivo `PR_DRAFT_*.md` em `docs/`
- Nenhum e-mail pessoal em `wails.json`
- `.gitignore` contém entradas para `legacy_c/`, `.jules/` e `.kiro/`

### Tarefa 4.3 — Resumo das mudanças
Ao concluir todas as tarefas, gere um arquivo `CHANGES.md` na raiz
listando cada arquivo modificado, criado ou removido, com uma linha
de descrição por item. Este arquivo será usado como base para o
Pull Request.

---

## Critérios de Conclusão

- `go build ./...` passa sem erros
- `go vet ./...` passa sem warnings
- Todos os IPs são validados com `net.ParseIP` antes de operações de rede
- `MaxThreads` tem cap de 256 no backend, independente do frontend
- Caminho de exportação é sanitizado com `filepath.Clean`
- `legacy_c/`, `.jules/`, `.kiro/` estão no `.gitignore`
- `docs/PR_DRAFT_v0.2.0.md` e `frontend/package.json.md5` removidos
- `MANUAL.md` removido, conteúdo consolidado em `ARCHITECTURE.md`
- `CHANGES.md` gerado com resumo completo
