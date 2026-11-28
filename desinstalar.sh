#!/bin/bash
# =============================================================================
# desinstalar.sh - tira a Bíblia Almeida ARC-1911 do menu e do computador
# Criado em 28/11/2026.
# Como usar: clique com o botão direito e escolha "Executar como programa".
# Ou, no Terminal:  ~/.local/share/biblia-arc1911/desinstalar.sh
# Não pede senha (sem sudo). Não mexe na pasta que você baixou.
# =============================================================================
set -e

# --- 1. O que será apagado (28/11/2026) --------------------------------------
# O quê: só o que o instalar-menu.sh criou.
# Por quê: segurança. Nada fora destes dois lugares é tocado.
if [ -z "$HOME" ] || [ ! -d "$HOME" ]; then
    echo "Erro: pasta pessoal (HOME) não encontrada."
    exit 1
fi
DESTINO="$HOME/.local/share/biblia-arc1911"
ATALHO="$HOME/.local/share/applications/biblia-arc1911.desktop"

# --- 2. Tirar o atalho do menu (28/11/2026) ----------------------------------
# O quê: apaga o arquivo .desktop e pede para o menu reler a lista.
rm -f "$ATALHO"
if command -v update-desktop-database >/dev/null 2>&1; then
    update-desktop-database "$HOME/.local/share/applications" >/dev/null 2>&1 || true
fi

# --- 3. Apagar a cópia instalada (28/11/2026) --------------------------------
# O quê: apaga a pasta do aplicativo, inclusive "dados" (onde você parou).
# Por quê: deixar o computador como estava antes de instalar.
case "$DESTINO" in
    */.local/share/biblia-arc1911) rm -rf "$DESTINO" ;;
    *) echo "Erro: pasta inesperada, nada foi apagado."; exit 1 ;;
esac

echo "Pronto! A Bíblia foi removida do menu e do computador."
echo "A pasta que você baixou (BibliaARC1911) não foi apagada."
if [ -t 0 ] && [ -z "$BIBLIA_SEM_PAUSA" ]; then
    read -r -p "Aperte ENTER para fechar..." _ || true
fi
