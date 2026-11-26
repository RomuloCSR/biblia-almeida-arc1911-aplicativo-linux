#!/bin/bash
# compilar.sh - gera o programa "biblia" nesta pasta
# Precisa (uma vez):  sudo apt install -y build-essential pkg-config libwebkit2gtk-4.1-dev
set -e
cd "$(dirname "$0")"
mkdir -p build

# 1. Motor (o mesmo do terminal). O main() do terminal é renomeado e não é usado.
gcc -O2 -Wall -Wextra -Wno-unused-result -Dmain=motor_main_terminal -c motor/motor.biblia.c -o build/motor.o

# 2. Janela do aplicativo
gcc -O2 -Wall -Wextra -c src/app_linux.c -o build/app.o $(pkg-config --cflags gtk+-3.0 webkit2gtk-4.1)

# 3. Juntar tudo
gcc -o biblia build/app.o build/motor.o $(pkg-config --libs gtk+-3.0 webkit2gtk-4.1)

echo "Pronto: $(pwd)/biblia"
