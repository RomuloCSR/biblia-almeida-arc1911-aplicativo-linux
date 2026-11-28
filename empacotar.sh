#!/bin/bash
# empacotar.sh - gera build/BibliaARC1911-Linux.tar.gz, pronto para anexar na release
# O nome do arquivo precisa ser sempre BibliaARC1911-Linux.tar.gz (o link do README usa esse nome).
set -e
cd "$(dirname "$0")"
./compilar.sh

# --- 1. Pasta temporária no disco do Linux (28/11/2026) ----------------------
# O quê: monta o pacote numa pasta temporária, fora do disco do Windows.
# Por quê: no disco do Windows (/mnt/c) todo arquivo aparece como 777 e o chmod
# não funciona. Aqui as permissões ficam certas.
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
P="$TMP/BibliaARC1911"
mkdir -p "$P"
cp biblia "$P/"
cp -r ui textos "$P/"
cp instalar-menu.sh desinstalar.sh "$P/"

cat > "$P/Abrir_Biblia.sh" <<'EOF'
#!/bin/bash
# Abre a Bíblia Almeida ARC-1911
cd "$(dirname "$0")"
if ! ./biblia "$@"; then
    echo
    echo "Não abriu? Instale a peça que falta (uma vez só) e tente de novo:"
    echo "  sudo apt install -y libwebkit2gtk-4.1-0"
    read -r -p "Aperte ENTER para fechar..." _
fi
EOF

# --- 2. LEIA-ME do pacote (28/11/2026) ---------------------------------------
# O quê: instruções curtas para quem baixou.
# Por quê: o programa pronto precisa de glibc 2.38, ou seja, Ubuntu 24.04 ou mais novo.
cat > "$P/LEIA-ME.txt" <<'EOF'
Bíblia Almeida ARC-1911 - Aplicativo para Linux

PARA ABRIR
Dê dois cliques em "biblia" (se perguntar, escolha "Executar").
Ou, no Terminal, dentro desta pasta:  ./biblia
Não precisa de internet. Para levar no pendrive: copie esta pasta inteira.

PARA COLOCAR NO MENU DE APLICATIVOS (sem senha)
Clique com o botão direito em "instalar-menu.sh" e escolha "Executar como programa".
Ou, no Terminal, dentro desta pasta:  ./instalar-menu.sh
Depois procure "Bíblia" no menu.

PARA REMOVER DO MENU E DO COMPUTADOR
No Terminal:  ~/.local/share/biblia-arc1911/desinstalar.sh

SE NÃO ABRIR
Instale a peça que falta (uma vez só):
  sudo apt install -y libwebkit2gtk-4.1-0
Se disser que o arquivo não é executável, no Terminal, dentro desta pasta:
  chmod +x biblia Abrir_Biblia.sh instalar-menu.sh desinstalar.sh

PRECISA DE
Ubuntu 24.04 ou mais novo (glibc 2.38 ou mais nova), ou outro Linux equivalente.
Em versões mais antigas, veja no site como compilar:
https://github.com/RomuloCSR/biblia-almeida-arc1911-aplicativo-linux

Gratuito, sem anúncios, sem cadastro e sem coleta de dados.
EOF

# --- 3. Permissões certas (28/11/2026) ---------------------------------------
# O quê: pastas 0755, arquivos 0644, só o programa e os scripts 0755.
# Por quê: antes o pacote vinha com todos os arquivos 777 (textos executáveis).
find "$P" -type d -exec chmod 0755 {} +
find "$P" -type f -exec chmod 0644 {} +
chmod 0755 "$P/biblia" "$P/Abrir_Biblia.sh" "$P/instalar-menu.sh" "$P/desinstalar.sh"

# --- 4. Gerar o .tar.gz (28/11/2026) -----------------------------------------
# Dono "root" e ordem por nome: o pacote não carrega o nome do usuário que gerou.
mkdir -p build
rm -f build/BibliaARC1911-Linux.tar.gz
tar --sort=name --owner=0 --group=0 --numeric-owner -czf build/BibliaARC1911-Linux.tar.gz -C "$TMP" BibliaARC1911
ls -lh build/BibliaARC1911-Linux.tar.gz
