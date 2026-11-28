# Publicar no Flathub (próximos passos do dono do projeto)

Esta pasta tem tudo o que o Flathub pede. **Nada foi enviado ainda.**

| Arquivo | O que é |
|---|---|
| `br.app.biblialivre.BibliaARC1911.yml` | Receita de compilação (GNOME 51, WebKitGTK 4.1, compila com `compilar.sh`) |
| `br.app.biblialivre.BibliaARC1911.metainfo.xml` | Descrição da loja (pt-BR e inglês, classificação OARS, foto) |
| `br.app.biblialivre.BibliaARC1911.desktop` | Atalho do menu |
| `br.app.biblialivre.BibliaARC1911.svg` | Ícone |

## Antes de enviar (obrigatório)

1. **Licença.** O repositório não tem arquivo `LICENSE`. Escolha a licença do código,
   crie o arquivo `LICENSE` na raiz e coloque o mesmo identificador SPDX em
   `<project_license>` no `metainfo.xml`. O Flathub recusa app sem licença.
2. **Nome do app (app-id).** `br.app.biblialivre.BibliaARC1911` exige que você seja dono
   do domínio `biblialivre.app.br`; o Flathub confere. Se não for, troque o id em todos os
   arquivos desta pasta para `io.github.RomuloCSR.BibliaARC1911` (prova de posse pelo GitHub).
3. **Id da janela.** Em `src/app_linux.c`, `gtk_application_new("br.com.romulocsr.biblia1911", ...)`
   deve passar a usar o mesmo app-id, para a janela mostrar o ícone certo na barra.
   Depois disso, troque também `StartupWMClass` no `.desktop`.
4. **Release v1.3.** Crie a tag `v1.3` e preencha `commit:` no `.yml` com o SHA completo dela.
5. **Foto.** A foto do `metainfo.xml` aponta para `docs/tela-principal.png` na `main`.
   Ela só fica no ar depois que o branch for juntado na `main`.

## Testar no seu Linux

```bash
sudo apt install -y flatpak flatpak-builder appstream desktop-file-utils
flatpak remote-add --if-not-exists --user flathub https://dl.flathub.org/repo/flathub.flatpakrepo
cd flatpak
# Para testar com o código local, troque a fonte "git" no .yml por "type: dir, path: .."
flatpak-builder --user --install --install-deps-from=flathub --force-clean build-dir br.app.biblialivre.BibliaARC1911.yml
flatpak run br.app.biblialivre.BibliaARC1911
appstreamcli validate br.app.biblialivre.BibliaARC1911.metainfo.xml
desktop-file-validate br.app.biblialivre.BibliaARC1911.desktop
```

Confira também com o verificador do Flathub:

```bash
flatpak install --user -y flathub org.flatpak.Builder
flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest br.app.biblialivre.BibliaARC1911.yml
```

## Enviar

1. Leia as regras: https://docs.flathub.org/docs/for-app-authors/submission
2. No GitHub, faça um *fork* de https://github.com/flathub/flathub (marque para copiar
   todos os ramos, não só a `master`).
3. No seu fork, crie um ramo a partir de `new-pr` e coloque nele só o arquivo `.yml`
   (os outros arquivos já vêm do seu repositório pela fonte `git`).
4. Abra um *pull request* para o ramo `new-pr` de `flathub/flathub`.
   Título: `Add br.app.biblialivre.BibliaARC1911`.
5. Os revisores do Flathub comentam no PR. Responda e ajuste até aprovarem.
6. Aprovado, o Flathub cria o repositório `flathub/br.app.biblialivre.BibliaARC1911`
   e convida você. As próximas versões são atualizadas por lá.
