# Bíblia Almeida ARC-1911 — Aplicativo para Linux

## 📥 [BAIXAR PARA LINUX (clique aqui)](https://github.com/RomuloCSR/biblia-almeida-arc1911-aplicativo-linux/releases/latest/download/BibliaARC1911-Linux.tar.gz)

**Gratuito • Sem anúncios • Funciona sem internet • Sempre a versão mais nova**

Ou aponte a câmera do celular para o código abaixo e envie o link para o seu computador:

<img src="docs/qr-baixar-linux.png" alt="QR code para baixar o aplicativo para Linux" width="200">

![Tela do aplicativo](docs/tela-principal.png)

**Compartilhe com quem precisa:** copie e envie este link:

```
https://github.com/RomuloCSR/biblia-almeida-arc1911-aplicativo-linux/releases/latest/download/BibliaARC1911-Linux.tar.gz
```

---

## Guia para iniciantes

Funciona no **Ubuntu 24.04 ou mais novo** (e em Linux parecidos, como Linux Mint 22 e Debian 13).
Não precisa de senha. Não precisa de internet depois de baixar.

### É seguro?

**Sim.**

* **Código aberto.** Todo o código e os textos dos 66 livros estão nesta página. Qualquer pessoa pode ler.
* **Não coleta dados.** Sem cadastro, sem conta, sem anúncios, sem rastreamento.
* **Não usa a internet sozinho.** Só quando **você** clica em **ENVIAR** ou em **Abrir GitHub**.
* **Não pede senha.** Ele não mexe no sistema. Fica só na sua pasta pessoal.

**Conferir se o arquivo baixado é o original (opcional):**

1. Abra o **Terminal** (aperte `Ctrl` + `Alt` + `T`).
2. Digite e aperte `Enter`:

```bash
sha256sum ~/Downloads/BibliaARC1911-Linux.tar.gz
```

3. Vai aparecer um código longo de letras e números.
4. Compare com o código **SHA-256** escrito na página da versão ([Releases](https://github.com/RomuloCSR/biblia-almeida-arc1911-aplicativo-linux/releases/latest)).
5. Se for igual, o arquivo é o original. Se for diferente, apague e baixe de novo por aqui.

> Se a sua pasta se chama *Transferências*, troque `Downloads` por `Transferências` no comando.

### Passo a passo

**1. Baixar.** Clique em **BAIXAR PARA LINUX**, no topo desta página.
O arquivo `BibliaARC1911-Linux.tar.gz` vai para a pasta *Downloads*.

**2. Extrair.** Abra a pasta *Downloads*.
Clique com o botão direito no arquivo e escolha **Extrair aqui**.
Vai aparecer a pasta `BibliaARC1911`.

**3. Abrir.** Entre na pasta `BibliaARC1911`.
Dê **dois cliques** em **`biblia`**. Se perguntar, escolha **Executar**.
A Bíblia abre numa janela, como na foto acima.

**4. Colocar no menu (opcional, recomendado).**
Na pasta `BibliaARC1911`, clique com o botão direito em **`instalar-menu.sh`**.
Escolha **Executar como programa**.
Pronto: procure **Bíblia** no menu de aplicativos. Ela tem este ícone:

<img src="ui/icone.png" alt="Ícone da Bíblia Almeida ARC-1911" width="64">

Depois disso, você pode apagar o arquivo baixado e a pasta `BibliaARC1911`.
O aplicativo fica guardado em `~/.local/share/biblia-arc1911`.

> Não apareceu **Executar como programa**? Use o Terminal:
> clique com o botão direito dentro da pasta `BibliaARC1911`, escolha **Abrir no Terminal** e digite:
>
> ```bash
> ./instalar-menu.sh
> ```

### Se der problema

**"O arquivo não é executável" ou abriu um editor de texto em vez da Bíblia:**
clique com o botão direito dentro da pasta `BibliaARC1911`, escolha **Abrir no Terminal** e digite:

```bash
chmod +x biblia Abrir_Biblia.sh instalar-menu.sh desinstalar.sh
./biblia
```

**Não abre nada, ou aparece um erro com `libwebkit2gtk`:** falta uma peça no sistema.
Abra o Terminal (`Ctrl` + `Alt` + `T`) e digite (uma vez só; ele pede a sua senha):

```bash
sudo apt install -y libwebkit2gtk-4.1-0
```

Depois abra a Bíblia de novo.

**Aparece um erro com `GLIBC`:** o seu Linux é mais antigo que o Ubuntu 24.04.
Use o jeito de compilar, mais abaixo. Funciona também em versões antigas.

### Remover

Abra o Terminal (`Ctrl` + `Alt` + `T`) e digite:

```bash
~/.local/share/biblia-arc1911/desinstalar.sh
```

Isso tira a Bíblia do menu e apaga a cópia instalada (inclusive onde você parou).
Não pede senha.
Se você não usou o `instalar-menu.sh`, basta apagar a pasta `BibliaARC1911`.

---

Aplicativo **gratuito, leve e 100% offline** da Bíblia **Almeida 1911 (Domínio Público)**, com atualização ortográfica e gramatical realizada em **11/2026** para o Português do Brasil.

Feito para quem quer ler a Palavra **sem distrações**: tela limpa, letra grande, botões simples. Se a pessoa sabe ler, ela sabe usar.

> **Nota editorial:** O conteúdo está em processo contínuo de atualização e pode eventualmente apresentar pequenas falhas ortográficas ou gramaticais. Colabore nos enviando seu feedback pela aba **Issues** deste repositório. Obrigado!

---

## Por que usar

* **Sem internet.** Os 66 livros e os 1189 capítulos já estão dentro da pasta.
* **Roda do pendrive.** O app guarda onde você parou numa pasta `dados` ao lado dele.
* **Motor em C.** Rápido e leve: cada capítulo abre na hora.
* **Sem anúncios, sem cadastro, sem coleta de dados.**
* **Texto fiel à Almeida 1911.** Só a grafia foi atualizada para o português de hoje, sem mudar o sentido.

---

## Compilar você mesmo (Ubuntu, Debian, Linux Mint e parecidos, inclusive versões antigas)

1. Abra o **Terminal** e instale o que o app precisa (uma vez só):

```bash
sudo apt install -y git build-essential pkg-config libwebkit2gtk-4.1-dev
```

2. Baixe o aplicativo e gere o programa:

```bash
git clone https://github.com/RomuloCSR/biblia-almeida-arc1911-aplicativo-linux.git
cd biblia-almeida-arc1911-aplicativo-linux
./compilar.sh
```

3. Abra a Bíblia:

```bash
./biblia
```

Das próximas vezes, basta entrar na pasta e rodar `./biblia`.
Para colocar no menu, rode `./instalar-menu.sh` nessa mesma pasta.

---

## Como usar

| Botão | O que faz |
|---|---|
| **AT / NT** | Troca entre Antigo e Novo Testamento |
| **Nome do livro** | Abre a grade com todos os livros |
| **CAP.** | Abre a grade com os capítulos |
| **‹ ANT.** e **PRÓX. ›** | Capítulo anterior e próximo |
| **A−** e **A+** | Diminui e aumenta a letra |
| **ENVIAR** | WhatsApp, Telegram, E-mail, X ou Copiar |
| **◐** | Tema claro, sépia ou escuro |
| **i** | Sobre o projeto |

**Dicas:**

* **Enviar só um versículo:** clique no versículo para marcar e depois em **ENVIAR**.
* Ao enviar, o texto **também é copiado**. Se precisar, cole com `Ctrl` + `V`.
* O app **lembra** onde você parou, o tamanho da letra e o tema.

### Atalhos

* **`←`** e **`→`** — Capítulo anterior e próximo.
* **`Ctrl`** + **`+`** / **`-`** (ou **`Ctrl`** + roda do mouse) — Aumenta ou diminui a letra.
* **`Ctrl`** + **`0`** — Volta a letra ao tamanho normal.
* **`Esc`** — Fecha a janela de livros ou capítulos.

---

## Para desenvolvedores

```
textos/*.md  ->  kernel/biblia_kernel.c (gerado de motor/motor.biblia.c)  ->  src/app_linux.c (janela GTK)  ->  ui/index.html (tela, WebKitGTK)
```

| Pasta / arquivo | O que é |
|---|---|
| `motor/motor.biblia.c` | Motor de limpeza e atualização ortográfica (as regras ficam aqui) |
| `kernel/` | Kernel em C gerado do motor (mesmo texto, mais rápido) |
| `src/app_linux.c` | Janela do Linux (GTK 3 + WebKitGTK 4.1) |
| `ui/index.html` | Visual do aplicativo (HTML, CSS e JavaScript) |
| `ui/icone.png` | Ícone da janela e do menu |
| `textos/` | Os 66 livros (1189 capítulos), texto base da Almeida 1911 |
| `compilar.sh` | Gera o programa `biblia` |
| `empacotar.sh` | Gera o `BibliaARC1911-Linux.tar.gz` para anexar na release |
| `instalar-menu.sh` / `desinstalar.sh` | Põem e tiram o app do menu, sem senha |
| `flatpak/` | Arquivos para publicar no Flathub (veja `flatpak/LEIAME.md`) |

---

* **Autor:** Romulo CSR ([github.com/RomuloCSR](https://github.com/RomuloCSR))
* **Apoio espontâneo (PIX):** `21980934118`
