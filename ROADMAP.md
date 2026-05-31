# Roadmap - CatNet Scanner

O **CatNet Scanner** está passando por uma evolução massiva. Após a estabilização do novo núcleo arquitetural em **Go + Wails + React**, substituindo a antiga stack em C, o software entrou em sua fase de maturação para se tornar o scanner de rede mais estiloso, robusto e eficiente do mercado corporativo e entusiasta.

Este documento traça o caminho desde a estabilização atual até a consolidação comercial competitiva (v1.0.0).

---

## 🎯 v0.4.0 — Estabilização & Fundação (Current Release)
**Objetivo**: Mudar a arquitetura base, estabilizar a interface gráfica e garantir velocidade brutal.

- [x] **Motor de Scan Concorrente**: Reescrever motor de ping/ICMP usando `Goroutines` nativas, atingindo paralelismo massivo.
- [x] **UI/UX Cyberpunk**: Abandonar interfaces TUI engessadas por um visual moderno, de alto contraste (*glassmorphism*, neon), construído em React e TypeScript.
- [x] **Arquitetura Desacoplada**: Implementar Wails para servir o binário compilado sem depender de servidores Node.js externos.
- [x] **Auto-detect IP Range**: Detecção inteligente e automática da sub-rede (`/24`) da interface ativa.
- [x] **Exportação Base**: Exportar varreduras nos formatos JSON, CSV e XML.

---

## 🛠️ v0.5.0 — Produto Utilizável (Próximas Sprints)
**Objetivo**: Expandir as features essenciais do dia-a-dia que os SysAdmins já esperam.

- [ ] **Perfis de Scan**: Permitir salvar "Ranges Favoritos" para acesso com 1 clique (ex: Rede Visitantes, Rede Servidores).
- [ ] **Vendor OUI Lookup**: Resolver fabricantes de placas de rede de forma offline com base nos primeiros octetos do MAC Address.
- [ ] **Ordenação de Tabela (Sort)**: Clicar no cabeçalho das colunas (IP, Status, Hostname) para ordenar os dados alfabeticamente ou numericamente.
- [ ] **Painel Lateral de Host**: Ao clicar em um IP específico na tabela principal, abrir um painel lateral/inferior exibindo mais detalhes (MAC completo, ping rate, fabricante).
- [ ] **Feedback Sonoro (Opcional)**: Pequenos sons discretos para scan iniciado e concluído (cyberpunk theme).

---

## 🚀 v0.6.0 — Diferenciação e Ferramentas Power User
**Objetivo**: Fornecer funcionalidades de produtividade rápida que destronem os scanners nativos do SO.

- [ ] **Openers Customizáveis**: Integração real para invocar chamadas de sistema no IP alvo com apenas um clique:
  - SSH (`ssh user@ip`)
  - RDP (`mstsc /v:ip`)
  - HTTP/HTTPS (abrir no navegador padrão)
- [ ] **Identificação de Portas Básica**: Opção para sondar rapidamente 5-10 portas comuns (80, 443, 22, 3389) em background sem a lentidão do Nmap.
- [ ] **Enriquecimento NetBIOS/SMB**: Puxar nomes NetBIOS e workgroups em redes locais Windows.
- [ ] **Filtros Dinâmicos**: Botões de toggle para "Apenas hosts vivos" ou "Esconder hosts sem nome".
- [ ] **Histórico Local e Comparação**: Identificar visualmente se novos hosts surgiram ou sumiram em comparação à última varredura na mesma rede.

---

## 👑 v1.0.0 — "O Matador do Angry IP" (Release Competitivo)
**Objetivo**: Empacotamento em nível de produção, extensibilidade corporativa e marketing focado.

- [ ] **Plugin API (Extensibilidade)**: Permitir injeção de pequenos scripts ou módulos Go para tarefas de detecção exóticas (ex: checar resposta SNMP, testar credencial default).
- [ ] **Documentação Exaustiva**: Manuais de usuário e guias de troubleshooting completos.
- [ ] **Releases Multiplataforma**: Instaladores automatizados via CI/CD (.MSI para Windows, .APP/DMG para macOS, AppImage para Linux).
- [ ] **Benchmark Oficial**: Página de documentação publicando testes de carga provando que o CatNet atinge performance superior (Goroutines vs Threads limitadas do concorrente Java).
- [ ] **Landing Page**: Site institucional para download do produto com demonstração da UI.
