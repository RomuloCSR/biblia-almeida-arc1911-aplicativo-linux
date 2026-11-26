#!/bin/bash
# empacotar.sh - gera build/BibliaARC1911-Linux.tar.gz, pronto para anexar na release
# O nome do arquivo precisa ser sempre BibliaARC1911-Linux.tar.gz (o link do README usa esse nome).
set -e
cd "$(dirname "$0")"
./compilar.sh

P=build/pacote/BibliaARC1911
rm -rf build/pacote
mkdir -p "$P"
cp biblia "$P/"
cp -r ui textos "$P/"

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

cat > "$P/LEIA-ME.txt" <<'EOF'
Bíblia Almeida ARC-1911 - Aplicativo para Linux

Para abrir: dê dois cliques em "biblia" (se perguntar, escolha "Executar").
Ou, no terminal, dentro desta pasta:  ./biblia
Não precisa de internet. Para levar no pendrive: copie esta pasta inteira.

Se não abrir, instale a peça que falta (uma vez só):
  sudo apt install -y libwebkit2gtk-4.1-0

Programa pronto para Ubuntu 26.04 ou mais novo.
Em versões mais antigas, veja no site como compilar:
https://github.com/RomuloCSR/biblia-almeida-arc1911-aplicativo-linux
EOF

chmod +x "$P/biblia" "$P/Abrir_Biblia.sh"
rm -f build/BibliaARC1911-Linux.tar.gz
tar -czf build/BibliaARC1911-Linux.tar.gz -C build/pacote BibliaARC1911
ls -lh build/BibliaARC1911-Linux.tar.gz
