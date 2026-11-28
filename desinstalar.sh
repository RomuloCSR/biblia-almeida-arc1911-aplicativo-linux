#!/bin/bash
# =============================================================================
# desinstalar.sh - tira a Bíblia Almeida ARC-1911 do menu e do computador
# Criado em 28/11/2026.
# Como usar: clique com o botão direito e escolha "Executar como programa".
# Ou, no Terminal:  ~/.local/share/biblia-arc1911/desinstalar.sh
# Não pede senha (sem sudo). Não mexe na pasta que você baixou.
# Só apaga ~/.local/share/biblia-arc1911 e o atalho
# ~/.local/share/applications/biblia-arc1911.desktop. Sem janela nem aviso
# disponível, escreve o resultado em ~/biblia-arc1911-instalacao.log.
# =============================================================================
set -e

# --- 0. Mostrar o resultado para a pessoa (28/11/2026) -----------------------
# O quê: escreve a mensagem no Terminal e, se não houver Terminal, mostra numa
# janela (zenity), numa notificação (notify-send) ou, em último caso, grava em
# ~/biblia-arc1911-instalacao.log.
# Por quê: com "Executar como programa" não abre Terminal; só com echo a pessoa
# não via se deu certo ou errado.
TITULO="Bíblia Almeida ARC-1911"
LOG="${HOME:-/tmp}/biblia-arc1911-instalacao.log"
avisar() {
    # $1 = info ou erro; $2 = mensagem
    if [ "$1" = erro ]; then echo "$2" >&2; else echo "$2"; fi
    if [ -t 0 ] || [ -t 1 ]; then return 0; fi
    if [ -n "$DISPLAY$WAYLAND_DISPLAY" ] && command -v zenity >/dev/null 2>&1; then
        if [ "$1" = erro ]; then
            zenity --error --title="$TITULO" --no-wrap --text="$2" >/dev/null 2>&1 && return 0
        else
            zenity --info --title="$TITULO" --no-wrap --text="$2" >/dev/null 2>&1 && return 0
        fi
    fi
    if [ -n "$DISPLAY$WAYLAND_DISPLAY" ] && command -v notify-send >/dev/null 2>&1; then
        notify-send --app-name="$TITULO" "$TITULO" "$2" >/dev/null 2>&1 && return 0
    fi
    { echo "== $(date '+%d/%m/%Y %H:%M:%S') desinstalar.sh ($1)"; echo "$2"; echo; } >> "$LOG" 2>/dev/null || true
}

# --- 0b. Erro inesperado (28/11/2026) ----------------------------------------
# O quê: se algum comando falhar no meio (set -e), avisa antes de sair.
# Por quê: sem isso o script pararia calado quando aberto com dois cliques.
trap 'avisar erro "Erro: a remoção não terminou. No Terminal, rode ~/.local/share/biblia-arc1911/desinstalar.sh para ver o motivo."' ERR

# --- 1. O que será apagado (28/11/2026) --------------------------------------
# O quê: só o que o instalar-menu.sh criou.
# Por quê: segurança. Nada fora destes dois lugares é apagado.
if [ -z "$HOME" ] || [ ! -d "$HOME" ]; then
    trap - ERR
    avisar erro "Erro: pasta pessoal (HOME) não encontrada."
    exit 1
fi
DESTINO="$HOME/.local/share/biblia-arc1911"
ATALHO="$HOME/.local/share/applications/biblia-arc1911.desktop"

# --- 2. Tirar o atalho do menu (28/11/2026) ----------------------------------
# O quê: apaga o arquivo .desktop.
# Não chamamos update-desktop-database (revisão de 28/11/2026): o app não abre
# tipos de arquivo (sem MimeType), e o menu nota sozinho que o atalho sumiu.
rm -f "$ATALHO"

# --- 3. Apagar a cópia instalada (28/11/2026) --------------------------------
# O quê: apaga a pasta do aplicativo, inclusive "dados" (onde você parou).
# Por quê: deixar o computador como estava antes de instalar.
case "$DESTINO" in
    */.local/share/biblia-arc1911) rm -rf "$DESTINO" ;;
    *) trap - ERR; avisar erro "Erro: pasta inesperada, nada foi apagado."; exit 1 ;;
esac

# --- 4. Mostrar o resultado (28/11/2026) -------------------------------------
trap - ERR
avisar info "Pronto! A Bíblia foi removida do menu e do computador.
A pasta que você baixou (BibliaARC1911) não foi apagada."
if [ -t 0 ] && [ -z "$BIBLIA_SEM_PAUSA" ]; then
    read -r -p "Aperte ENTER para fechar..." _ || true
fi
