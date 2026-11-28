#!/bin/bash
# compilar.sh - gera o programa "biblia" nesta pasta
# Precisa (uma vez):  sudo apt install -y build-essential pkg-config libwebkit2gtk-4.1-dev
set -e
cd "$(dirname "$0")"
mkdir -p build

# 1. Kernel da Bíblia (28/11/2026). Gerado a partir de motor/motor.biblia.c; mesmo texto, mais rápido.
#    Para mudar uma regra: edite motor/motor.biblia.c e gere o kernel de novo (veja kernel/LEIAME.md).
gcc -O2 -Wall -Wextra -c kernel/biblia_kernel.c -o build/kernel.o

# 2. Janela do aplicativo
gcc -O2 -Wall -Wextra -c src/app_linux.c -o build/app.o $(pkg-config --cflags gtk+-3.0 webkit2gtk-4.1)

# 3. Juntar tudo
gcc -o biblia build/app.o build/kernel.o $(pkg-config --libs gtk+-3.0 webkit2gtk-4.1)

echo "Pronto: $(pwd)/biblia"
