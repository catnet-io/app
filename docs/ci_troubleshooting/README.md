# Troubleshooting Logs (PR #22, #23, #24)

Este diretório contém a documentação das soluções de CI/CD realizadas para estabilizar a pipeline do projeto `catnet_scanner` durante atualizações via Dependabot.

## Metodologia de Investigação

Para contornar limitações do comando `gh` em ambientes CI sem autenticação, utilizamos a API pública do GitHub REST para buscar e analisar logs de CI. 
Utilizamos requisições `Invoke-RestMethod` via PowerShell e extraímos dados com um script Python local (`parse_github_api.py`), o qual identifica falhas e anotações dos jobs.

Exemplo de uso:
```powershell
Invoke-RestMethod -Uri "https://api.github.com/repos/mendsec/catnet_scanner/commits/<branch>/check-runs" | ConvertTo-Json -Depth 5 > check_runs.json
python parse_github_api.py check_runs check_runs.json
```

## Casos de Estudo

### 1. PR #22 - Falha no Govulncheck

**Sintoma:** O _check_ `govulncheck` falhava consistentemente ao analisar o repositório, apesar de rodar corretamente no ambiente local.

**Causa Raiz:** 
1. **Falta do Frontend Mock:** O `govulncheck` executa um comando de compilação interno no Go para construir a AST e a árvore de chamadas. O projeto, sendo baseado em Wails, requer via `embed.FS` que o diretório `frontend/dist` exista. Como a Action rodava isolada do build Frontend do CI principal, ela não encontrava o diretório e abortava.
2. **Dependências Linux (CGO):** Originalmente, tentamos migrar a verificação para `windows-latest` para contornar problemas de CGO. No entanto, descobrimos que a action `golang/govulncheck-action` tentava realizar o checkout interno do repositório (`repo-checkout: true`), o que sobrescrevia qualquer mock do diretório do frontend.

**Solução Aplicada:**
- Alterado o runner para `ubuntu-latest`.
- Instalado manualmente as dependências de compilação do CGO e GTK para Wails (`libgtk-3-dev libwebkit2gtk-4.1-dev`).
- Criado o diretório `frontend/dist` com um mock antes da análise.
- Desabilitado o clone automático (`repo-checkout: false`) na action `golang/govulncheck-action`, forçando o uso do _workspace_ mockado.

---

### 2. PR #23 - Falha no Snyk (Dependabot Secrets)

**Sintoma:** Os jobs `snyk-go` e `snyk-frontend` falharam subitamente (Exit Code 2).

**Causa Raiz:** 
O PR #23 foi aberto pelo Dependabot (que atualizou o `setup-bun` para v2). Por padrão, PRs criados pelo Dependabot ou forks externos não possuem acesso aos "Repository Secrets" por questões de segurança. Logo, a variável `${{ secrets.SNYK_TOKEN }}` ficava vazia, impedindo o Snyk de autenticar e rodar o scanner.

**Solução Aplicada:**
- Feito um commit vazio (`git commit --allow-empty`) a partir de uma conta mantenedora do projeto. Isso fez com que a pipeline rodasse novamente sob o contexto de um membro do repositório, habilitando o uso do Secret do Snyk.
- (Nota: Uma solução mais permanente seria adicionar o Token do Snyk na seção de "Dependabot Secrets" do GitHub).

---

### 3. PR #24 - Falha do Go Test (-race) com setup-go@v6

**Sintoma:** O CI `test` falhou na execução `go test -race -v ./...` após o bump do `actions/setup-go@v5` para `v6`.

**Causa Raiz:** 
- O detector de concorrência (`-race`) do Go exige o compilador C (CGO) ativado. Na nova versão do `setup-go` ou nas novas imagens do runner do Windows, `CGO_ENABLED` não estava mais vindo habilitado por padrão de uma forma que satisfizesse o comando `-race`.
- Além disso, o PR estava desatualizado com a `main`, não possuindo a correção de *Data Race* que consertaria as falhas caso o teste conseguisse rodar.

**Solução Aplicada:**
- Feito o merge da `main` para dentro da branch do PR #24 para puxar as correções de código recentes.
- Adicionado explicitamente `CGO_ENABLED: 1` no step `Test` dentro de `ci.yml` para assegurar a disponibilidade do CGO ao rodar os testes.

---

### 4. PR #24 - Falha no `go mod verify` (Conflito de Versão do Go)

**Sintoma:** O CI passou a falhar logo na etapa `go mod verify` com o erro: `go: go.mod requires go >= 1.25.10 (running go 1.23.12; GOTOOLCHAIN=local)`.

**Causa Raiz:** 
Após atualizar a versão do Go no `go.mod` para `1.25.10` na branch `main` e realizar o merge dessa branch no PR #24, a versão exigida pelo projeto deixou de ser compatível com a versão configurada nas Github Actions (`1.23`). O step `setup-go@v6` define por padrão a variável de ambiente `GOTOOLCHAIN=local`, que impede o Go de baixar automaticamente a toolchain necessária para a versão exigida. Sendo assim, o ambiente continuou forçando a versão `1.23`, que se recusou a construir módulos que exigem a versão `1.25.10`.

**Solução Aplicada:**
- Alterado o parâmetro `go-version` (e `go-version-input`) nos arquivos `.github/workflows/ci.yml`, `.github/workflows/govulncheck.yml` e `.github/workflows/release.yml` de `'1.23'` para `'1.25.x'`. Isso garante que o runner do CI instale e utilize a versão do Go compatível com o arquivo `go.mod`.

---

### 5. PR #25 e PR #26 - Falha no Snyk (Dependabot Secrets - Recorrência)

**Sintoma:** Semelhante ao PR #23, os jobs `snyk-go` e `snyk-frontend` falharam com `401 Unauthorized` (Erro de Autenticação SNYK-0005) nestes PRs.

**Causa Raiz:** 
Como os PRs #25 e #26 também foram criados pelo Dependabot (para atualizar as actions `actions/upload-artifact` e `actions/download-artifact`), eles recaem na mesma limitação de segurança do GitHub Actions: não ter acesso aos "Repository Secrets". Consequentemente, o token do Snyk fica inacessível.

**Solução Aplicada:**
- Feito um commit vazio (`git commit --allow-empty`) a partir de uma conta mantenedora do projeto diretamente nas branches dos PRs.
- Isso reiniciou o CI rodando sob o contexto de um membro mantenedor, permitindo que a Action consumisse o token corretamente.
- *(Reforça a necessidade de adicionar o token aos "Dependabot Secrets" para evitar retrabalho futuro).*

---

### 6. PR #27 - Falha nos Workflows após Migração para Multi-Repo

**Sintoma:** Os workflows `release.yml`, `govulncheck.yml` e `snyk.yml` começaram a falhar com erros de "module not found" (falha ao resolver a diretiva `replace` de `../catnet-core`) e/ou conflito de versão do Go (`go.mod requires go >= 1.26.3`).

**Causa Raiz:** 
Durante a mudança arquitetural para "The CatNet Ecosystem", o repositório adotou um padrão multi-repo (workspace) que inseriu a diretiva `replace github.com/mendsec/catnet-core => ../catnet-core` no `go.mod`. Na branch `develop`, o checkout duplo foi implementado em `ci.yml`, mas não foi propagado para os demais workflows. Consequentemente, esses actions executavam no diretório raiz sem clonar a dependência principal lado-a-lado. Além disso, as definições do `go-version` nestes workflows divergiram do novo baseline da aplicação.

**Solução Aplicada:**
- **Checkouts Multi-repo:** Alteramos `release.yml`, `govulncheck.yml` e `snyk.yml` para realizar o checkout de ambos os repositórios (`catnet-scanner` e `catnet-core`) usando a flag `path:`, espelhando a configuração correta do `ci.yml`.
- **Redirecionamento de Diretório (Working Directory):** Adicionamos as propriedades de `working-directory: catnet-scanner` (ou o prefixo `catnet-scanner/` em campos específicos de Custom Actions como `work-dir` e `args: --file=...`) aos passos de compilação, Snyk, Govulncheck, e extração de release para que operem sob o diretório local onde o projeto foi clonado.
- **Normalização do Go:** Atualizamos `go-version` e `go-version-input` de todos os actions para a nova baseline (`1.26.x`), resolvendo qualquer downgrade assíncrono ocorrido nos merges entre `main` e `develop`.

---

### 7. PR #28 - Falhas Ocultas em CGO e Dependências Nativas no Ubuntu 24.04

**Sintoma:** Após estabilizar os checkouts, o backend test continuou falhando na Action de CI, reclamando ora que `gcc` não estava disponível para a compilação paralela com a flag `-race`, ora com o pacote de WebKit resultando em `Exit code 100` (`Unable to locate package libwebkit2gtk-4.0-dev`).

**Causa Raiz:** 
1. **Runner `ubuntu-latest` (Noble Numbat):** O GitHub migrou os runners de `ubuntu-latest` para Ubuntu 24.04, o qual substituiu totalmente o pacote `libwebkit2gtk-4.0-dev` pela versão mais moderna `libwebkit2gtk-4.1-dev`. O comando estático com `-4.0-dev` quebrou imediatamente.
2. **Dependência do Frontend Mock em Testes:** A flag `CGO_ENABLED: 1` obrigou o runner a cruzar dependências de C. Como usamos _Wails_, isso gerou dependência real no GTK. Além disso, o teste `go test` verificava a integridade de `//go:embed all:frontend/dist`. Como os testes rodam em paralelo ao `bun build`, o diretório frontend mock não existia nativamente no escopo do runner backend.

**Solução Aplicada:**
- Alteramos permanentemente nos workflows (`ci.yml` e `release.yml`) a dependência instalada para `libwebkit2gtk-4.1-dev`.
- Injetamos o script de **Mock do Frontend** antes do step de teste (`mkdir -p frontend/dist; echo "mock" > frontend/dist/index.html`) para enganar a validação do `go:embed`.
