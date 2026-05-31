# CatNet Scanner

<p align="center">
  <img src="frontend/src/assets/nyan.png" width="200" alt="CatNet Scanner Logo">
</p>

O **CatNet Scanner** é um scanner de rede incrivelmente rápido e estiloso, desenvolvido para quem busca a agilidade de um software de linha de comando com a beleza de um dashboard Cyberpunk/SOC.

Construído sobre o robusto ecosistema **Go** e empacotado através do **Wails** usando **React/TypeScript**, o CatNet fornece detecção massivamente paralela sem engasgos na interface. O foco? Destronar softwares clássicos como o Angry IP Scanner entregando mais performance, sem Java, num binário único.

---

## 🌟 Features (v0.4.0)

- ⚡ **Scan Brutalmente Paralelo**: Feito inteiramente com Goroutines, scaneia sub-redes inteiras numa fração de segundo.
- 🎨 **Cyberpunk UI**: Uma interface React moderna usando vidro translúcido (*Glassmorphism*), linhas néon, grids responsivos e animações sutis. E claro, com o Nyan Cat puxando a sua barra de progresso.
- 📡 **Auto-Detect Inteligente**: Carrega instantaneamente a sub-rede (`/24`) da sua interface de rede principal ao inicializar.
- 💾 **Exportação Prática**: Salve relatórios em formato `CSV`, `TXT` ou `XML` com um simples clique.
- 🛠️ **Arquitetura Desacoplada**: Motor de rede seguro em Go e interface rica e desacoplada em React, empacotados em um único arquivo `.exe` stand-alone.

## 🚀 Próximas Implementações

Consulte o nosso [`ROADMAP.md`](ROADMAP.md) para ver para onde estamos caminhando. Recursos como Perfis de Scan, Openers Customizáveis (SSH, RDP) e Resolução Vendor OUI já estão no forno.

## ⚙️ Compilação e Desenvolvimento

O CatNet Scanner utiliza o Wails. Você não precisa instalar dependências pesadas do React ou Node caso queira apenas compilar o backend.

### Pré-requisitos
- [Go 1.20+](https://go.dev/dl/)
- [Bun](https://bun.sh/) (Gerenciador de pacotes rápido para o frontend)
- [Wails CLI](https://wails.io/docs/gettingstarted/installation)

### Instalando o Wails
```bash
go install github.com/wailsapp/wails/v2/cmd/wails@latest
```

### Rodando em Ambiente de Desenvolvimento (Live Reload)
Para modificar a UI ou o Go com live-reload, use:
```bash
wails dev
```

### Gerando o Executável de Produção
Para compilar a versão final stand-alone, execute:
```bash
wails build -clean
```
O executável final estará disponível em `build/bin/catnet.exe`.

---

## 🤝 Contribuindo
Sugestões são bem-vindas. Sinta-se livre para abrir Issues de novas ferramentas que você gostaria de ver na aba "Quick Tools" ou reportar bugs de UI.

> Copyright © 2026 Mendsec. Criado por Fabio Mendes.
