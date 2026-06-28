#!/usr/bin/env bash
# Ativa ou desativa replace directive para desenvolvimento local
# Uso: ./scripts/dev-replace.sh [on|off]
set -euo pipefail

ENGINE_PATH="${ENGINE_PATH:-../engine}"
MODULE="github.com/catnet-io/engine"

case "${1:-on}" in
  on)
    go mod edit -replace "${MODULE}=${ENGINE_PATH}"
    echo "✓ Replace ativo: ${MODULE} → ${ENGINE_PATH}"
    echo "  ATENÇÃO: remover antes de git push"
    ;;
  off)
    go mod edit -dropreplace "${MODULE}"
    go mod tidy
    echo "✓ Replace removido — usando versão publicada de ${MODULE}"
    ;;
  *)
    echo "Uso: $0 [on|off]"
    exit 1
    ;;
esac