#!/bin/bash
# =============================================================================
# instalar-menu.sh - coloca a Bíblia Almeida ARC-1911 no menu de aplicativos
# Criado em 28/11/2026.
# Como usar: clique com o botão direito neste arquivo e escolha
# "Executar como programa". Ou, no Terminal, dentro desta pasta: ./instalar-menu.sh
# Não pede senha (sem sudo). Pode rodar de novo quantas vezes quiser.
# Só mexe em ~/.local/share/biblia-arc1911 e no atalho
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
    { echo "== $(date '+%d/%m/%Y %H:%M:%S') instalar-menu.sh ($1)"; echo "$2"; echo; } >> "$LOG" 2>/dev/null || true
}

# --- 0b. Erro inesperado (28/11/2026) ----------------------------------------
# O quê: se algum comando falhar no meio (set -e), avisa antes de sair.
# Por quê: sem isso o script pararia calado quando aberto com dois cliques.
trap 'avisar erro "Erro: a instalação não terminou. Abra o Terminal nesta pasta e rode ./instalar-menu.sh para ver o motivo."' ERR

# --- 1. Achar a pasta do pacote (28/11/2026) ---------------------------------
# O quê: entra na pasta onde este arquivo está.
# Por quê: assim funciona com dois cliques, de qualquer lugar.
cd "$(dirname "$(readlink -f "$0")")"
ORIGEM="$(pwd)"

if [ -z "$HOME" ] || [ ! -d "$HOME" ]; then
    trap - ERR
    avisar erro "Erro: pasta pessoal (HOME) não encontrada."
    exit 1
fi
if [ ! -f "$ORIGEM/biblia" ] || [ ! -f "$ORIGEM/ui/index.html" ] || [ ! -d "$ORIGEM/textos" ]; then
    trap - ERR
    avisar erro "Erro: rode este arquivo de dentro da pasta BibliaARC1911 (a que veio no download)."
    exit 1
fi

# --- 2. Para onde vai (28/11/2026) -------------------------------------------
# O quê: a pasta de programas do próprio usuário.
# Por quê: não precisa de senha e não mexe no sistema.
DESTINO="$HOME/.local/share/biblia-arc1911"
MENU="$HOME/.local/share/applications"
ATALHO="$MENU/biblia-arc1911.desktop"

# --- 3. Copiar o aplicativo (28/11/2026) -------------------------------------
# O quê: copia o programa, a tela e os textos.
# Por quê: troca os arquivos antigos pelos novos, mas guarda a pasta "dados"
# (onde a pessoa parou, tamanho da letra e tema).
mkdir -p "$DESTINO"
if [ "$ORIGEM" != "$(readlink -f "$DESTINO")" ]; then
    rm -rf "$DESTINO/ui" "$DESTINO/textos"
    cp -f "$ORIGEM/biblia" "$DESTINO/"
    cp -r "$ORIGEM/ui" "$ORIGEM/textos" "$DESTINO/"
    for f in Abrir_Biblia.sh LEIA-ME.txt instalar-menu.sh desinstalar.sh; do
        if [ -f "$ORIGEM/$f" ]; then cp -f "$ORIGEM/$f" "$DESTINO/"; fi
    done
fi

# --- 4. Permissões (28/11/2026) ----------------------------------------------
# O quê: só o programa e os scripts podem ser executados; o resto é só leitura.
# Por quê: segurança e ordem (antes todos os arquivos vinham como executáveis).
find "$DESTINO" -type d -exec chmod 755 {} +
find "$DESTINO" -type f -not -path "$DESTINO/dados/*" -exec chmod 644 {} +
chmod 755 "$DESTINO/biblia"
for f in Abrir_Biblia.sh instalar-menu.sh desinstalar.sh; do
    if [ -f "$DESTINO/$f" ]; then chmod 755 "$DESTINO/$f"; fi
done

# --- 5. Criar o atalho no menu (28/11/2026) ----------------------------------
# O quê: escreve o arquivo .desktop com nome, ícone e comando.
# Por quê: é assim que o Linux mostra o aplicativo no menu.
# StartupWMClass: igual ao id do GtkApplication em src/app_linux.c
# (br.com.romulocsr.biblia1911), para a janela aberta usar o mesmo ícone na barra.
# Não chamamos update-desktop-database (revisão de 28/11/2026): o app não abre
# tipos de arquivo (sem MimeType), e o menu acha o atalho novo sozinho.
mkdir -p "$MENU"
cat > "$ATALHO" <<EOF
[Desktop Entry]
Type=Application
Version=1.5
Name=Bíblia Almeida ARC-1911
GenericName=Bíblia
Comment=Ler a Bíblia Almeida 1911 sem internet
Exec="$DESTINO/biblia"
Path=$DESTINO
Icon=$DESTINO/ui/icone.png
Terminal=false
Categories=Education;Literature;
Keywords=Bíblia;Biblia;Almeida;Bible;
StartupNotify=true
StartupWMClass=br.com.romulocsr.biblia1911
EOF
chmod 644 "$ATALHO"

# --- 6. Conferir a peça que falta (28/11/2026) -------------------------------
# O quê: vê se o sistema tem as bibliotecas que o programa usa.
# Por quê: se faltar, mostra o comando certo em vez de um erro sem explicação.
FALTA=""
if command -v ldd >/dev/null 2>&1 && ldd "$DESTINO/biblia" 2>/dev/null | grep -q "not found"; then
    FALTA="

Falta uma peça no sistema. Abra o Terminal e rode (uma vez só; pede a senha do computador):
  sudo apt install -y libwebkit2gtk-4.1-0"
fi

# --- 7. Mostrar o resultado (28/11/2026) -------------------------------------
# O quê: avisa que terminou, onde ficou e como remover.
trap - ERR
avisar info "Pronto! Procure \"Bíblia\" no menu de aplicativos.
Instalado em: $DESTINO
Para remover: rode $DESTINO/desinstalar.sh$FALTA"
if [ -t 0 ] && [ -z "$BIBLIA_SEM_PAUSA" ]; then
    read -r -p "Aperte ENTER para fechar..." _ || true
fi
