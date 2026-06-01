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
