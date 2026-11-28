/* =============================================================================
- PROJETO: Bíblia Digital Almeida 1911 em PT-BR (Domínio Público)
-          Atualização Ortográfica e Gramatical realizada em 11/2026
- OBJETIVO: Motor de navegação para Windows 11, limpeza Markdown e
-           atualização ortográfica/gramatical completa para PT-BR.
- AUTOR: Romulo CSR (GitHub: RomuloCSR)
- AMBIENTE: Windows 11 (PowerShell / Windows Terminal) / IDE Geany
- COMPILAR: gcc -O2 -Wall -Wextra motor.biblia.c -o motor.biblia.exe
- =============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#ifdef _WIN32
    #include <windows.h>
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif

#define MAX_LINHA 8192
#define LARGURA_PAGINA 74

/* Códigos ANSI suaves para conforto de leitura */
#define COR_RESET   "\033[0m"
#define COR_NEGRITO "\033[1m"
#define COR_ITALICO "\033[3m"
#define COR_VERSO   "\033[1;36m"
#define COR_TITULO  "\033[1;33m"
#define COR_BORDA   "\033[36m"
#define COR_DIM     "\033[2m"

/* -----------------------------------------------------------------------------
- 1. MAPEAMENTO DE DADOS (PASTAS, NOMES E CONTAGEM DE CAPÍTULOS)
- ----------------------------------------------------------------------------- */

const char *pastas_at[39] = {
    "01-GEN", "02-EXO", "03-LEV", "04-NUM", "05-DEU", "06-JOS", "07-JDG", "08-RUT",
    "09-1SA", "10-2SA", "11-1KI", "12-2KI", "13-1CH", "14-2CH", "15-EZR",
    "16-NEH", "17-EST", "18-JOB", "19-PSA", "20-PRO", "21-ECC", "22-SNG",
    "23-ISA", "24-JER", "25-LAM", "26-EZK", "27-DAN", "28-HOS", "29-JOL",
    "30-AMO", "31-OBA", "32-JON", "33-MIC", "34-NAM", "35-HAB", "36-ZEP",
    "37-HAG", "38-ZEC", "39-MAL"
};

const char *pastas_nt[27] = {
    "41-MAT", "42-MRK", "43-LUK", "44-JHN", "45-ACT", "46-ROM", "47-1CO", "48-2CO",
    "49-GAL", "50-EPH", "51-PHP", "52-COL", "53-1TH", "54-2TH",
    "55-1TI", "56-2TI", "57-TIT", "58-PHM", "59-HEB", "60-JAS", "61-1PE", "62-2PE",
    "63-1JN", "64-2JN", "65-3JN", "66-JUD", "67-REV"
};

const char *nomes_at[39] = {
    "Gênesis", "Êxodo", "Levítico", "Números", "Deuteronômio", "Josué", "Juízes", "Rute",
    "1 Samuel", "2 Samuel", "1 Reis", "2 Reis", "1 Crônicas", "2 Crônicas", "Esdras",
    "Neemias", "Ester", "Jó", "Salmos", "Provérbios", "Eclesiastes", "Cânticos",
    "Isaías", "Jeremias", "Lamentações", "Ezequiel", "Daniel", "Oseias", "Joel",
    "Amós", "Obadias", "Jonas", "Miqueias", "Naum", "Habacuque", "Sofonias",
    "Ageu", "Zacarias", "Malaquias"
};

const char *nomes_nt[27] = {
    "Mateus", "Marcos", "Lucas", "João", "Atos", "Romanos", "1 Coríntios", "2 Coríntios",
    "Gálatas", "Efésios", "Filipenses", "Colossenses", "1 Tessalonicenses", "2 Tessalonicenses",
    "1 Timóteo", "2 Timóteo", "Tito", "Filemom", "Hebreus", "Tiago", "1 Pedro", "2 Pedro",
    "1 João", "2 João", "3 João", "Judas", "Apocalipse"
};

const int capitulos_at[39] = {
    50, 40, 27, 36, 34, 24, 21, 4,
    31, 24, 22, 25, 29, 36, 10,
    13, 10, 42, 150, 31, 12, 8,
    66, 52, 5, 48, 12, 14, 3,
    9, 1, 4, 7, 3, 3, 3,
    2, 14, 4
};

const int capitulos_nt[27] = {
    28, 16, 24, 21, 28, 16, 16, 13,
    6, 6, 4, 4, 5, 3,
    6, 4, 3, 1, 13, 5, 5, 3,
    5, 1, 1, 1, 22
};

/* -----------------------------------------------------------------------------
- 2. UTILITÁRIOS DE INTERFACE, CENTRALIZAÇÃO E ENTRADA
- ----------------------------------------------------------------------------- */

void configurar_terminal(void) {
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    SetConsoleTitleA("Bíblia Digital Almeida 1911 PT-BR");

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif
}

void limpar_tela(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void limpar_buffer_entrada(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Lê uma linha de comando do usuário (1 único ENTER) e retorna o 1º caractere digitado */
int ler_comando_tecla(void) {
    char entrada[64];
    if (fgets(entrada, sizeof(entrada), stdin) == NULL) {
        return '\n';
    }
    for (int i = 0; entrada[i] != '\0'; i++) {
        if (entrada[i] != ' ' && entrada[i] != '\t' && entrada[i] != '\r') {
            return (unsigned char)entrada[i];
        }
    }
    return '\n';
}

void obter_dimensoes_terminal(int *largura, int *altura) {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *largura = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *altura = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        if (*largura > 0 && *altura > 0) return;
    }
    *largura = 80;
    *altura = 24;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0 && w.ws_row > 0) {
        *largura = w.ws_col;
        *altura = w.ws_row;
    } else {
        *largura = 80;
        *altura = 24;
    }
#endif
}

int calcular_margem_esquerda(int largura_conteudo) {
    int cols = 80, rows = 24;
    obter_dimensoes_terminal(&cols, &rows);
    if (cols <= largura_conteudo) return 2;
    return (cols - largura_conteudo) / 2;
}

void imprimir_espacos(int qtd) {
    for (int i = 0; i < qtd; i++) {
        putchar(' ');
    }
}

int comprimento_visivel_utf8(const char *texto) {
    int visivel = 0;
    for (int i = 0; texto[i] != '\0'; i++) {
        if ((texto[i] & 0xC0) != 0x80) {
            visivel++;
        }
    }
    return visivel;
}

void imprimir_linha_centralizada(const char *texto, int largura_pagina) {
    int visivel = comprimento_visivel_utf8(texto);
    int margem = calcular_margem_esquerda(largura_pagina);
    int recuo_interno = (largura_pagina > visivel) ? (largura_pagina - visivel) / 2 : 0;
    imprimir_espacos(margem + recuo_interno);
    printf("%s\n", texto);
}

void imprimir_separador(int largura_pagina, char simbolo) {
    int margem = calcular_margem_esquerda(largura_pagina);
    imprimir_espacos(margem);
    printf("%s", COR_BORDA);
    for (int i = 0; i < largura_pagina; i++) {
        putchar(simbolo);
    }
    printf("%s\n", COR_RESET);
}

void pausar_tela(void) {
    int margem = calcular_margem_esquerda(LARGURA_PAGINA);
    printf("\n");
    imprimir_espacos(margem);
    printf("%s[Pressione ENTER para voltar ao menu...]%s", COR_DIM, COR_RESET);
    fflush(stdout);
    ler_comando_tecla();
}

void exibir_data_hora(void) {
    time_t t = time(NULL);
    struct tm *agora = localtime(&t);
    if (agora == NULL) return;
    struct tm tm = *agora;

    const char *dias_semana[] = {
        "Domingo", "Segunda-feira", "Terça-feira", "Quarta-feira",
        "Quinta-feira", "Sexta-feira", "Sábado"
    };
    const char *meses[] = {
        "Jan", "Fev", "Mar", "Abr", "Mai", "Jun",
        "Jul", "Ago", "Set", "Out", "Nov", "Dez"
    };

    char cabecalho[128];
    snprintf(cabecalho, sizeof(cabecalho), "%s, %02d de %s de %d  |  %02d:%02d",
             dias_semana[tm.tm_wday],
             tm.tm_mday,
             meses[tm.tm_mon],
             tm.tm_year + 1900,
             tm.tm_hour,
             tm.tm_min);

    printf("%s", COR_DIM);
    imprimir_linha_centralizada(cabecalho, LARGURA_PAGINA);
    printf("%s", COR_RESET);
}

void imprimir_coluna(int numero, const char *texto) {
    int tamanho_visivel = comprimento_visivel_utf8(texto);

    printf(" %2d - %s", numero, texto);

    int espacos = 20 - tamanho_visivel;
    for (int j = 0; j < espacos; j++) {
        printf(" ");
    }
}

/* -----------------------------------------------------------------------------
- 3. MOTOR DE LIMPEZA E ATUALIZAÇÃO ORTOGRÁFICA / GRAMATICAL (PT-BR)
- ----------------------------------------------------------------------------- */

/* REVISÃO DO KERNEL (27/11/2026) — feita com Claude Opus 5.5 e testada na Bíblia inteira.
   Segurança: as funções substituir_* e atualizar_sufixos_e_enclises não cortam nem estouram
   mais o texto: se uma troca não couber em MAX_LINHA, o texto fica como estava. Regra vazia é
   ignorada. O gerúndio troca no lugar, sem buffer (antes podia estourar com palavra enorme).
   Texto: mesóclise -hei/-hão depois de pronome, fôssemos/fôsseis, acento de hiato em -i-lo-á,
   variantes de maiúscula, ordem de regras e o gerúndio que trocava "ficou a mulher".
   Teste: AddressSanitizer + UBSan sem erros; 120 versículos corrigidos, nenhum outro mudou.
   Relatórios: biblia-build/kernel_rev/relatorio_seguranca.md e relatorio_texto.md.
   Texto: buffer de pelo menos MAX_LINHA bytes; o texto pode crescer até MAX_LINHA - 2. */

void substituir_texto(char *str, const char *antigo, const char *novo) {
    char buffer[MAX_LINHA];
    char *pos;
    size_t tam_antigo = strlen(antigo);
    size_t tam_novo = strlen(novo);
    size_t usado = 0;
    if (tam_antigo == 0) return;

    buffer[0] = '\0';
    char *atual = str;

    while ((pos = strstr(atual, antigo)) != NULL) {
        size_t trecho = (size_t)(pos - atual);
        if (usado + trecho + tam_novo >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */

        memcpy(buffer + usado, atual, trecho);
        usado += trecho;

        memcpy(buffer + usado, novo, tam_novo);
        usado += tam_novo;

        atual = pos + tam_antigo;
    }

    size_t resto = strlen(atual);
    if (usado + resto >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
    memcpy(buffer + usado, atual, resto);
    usado += resto;
    buffer[usado] = '\0';
    memcpy(str, buffer, usado + 1);
}

void substituir_inicio_palavra(char *str, const char *antigo, const char *novo) {
    char buffer[MAX_LINHA];
    char *pos;
    size_t tam_antigo = strlen(antigo);
    size_t tam_novo = strlen(novo);
    size_t usado = 0;
    if (tam_antigo == 0) return;

    /* A regra acrescenta acento? (o "novo" tem mais bytes não-ASCII que o "antigo") */
    int nao_ascii_antigo = 0, nao_ascii_novo = 0;
    for (size_t k = 0; k < tam_antigo; k++) if ((unsigned char)antigo[k] >= 0x80) nao_ascii_antigo++;
    for (size_t k = 0; k < tam_novo; k++) if ((unsigned char)novo[k] >= 0x80) nao_ascii_novo++;
    int acrescenta_acento = nao_ascii_novo > nao_ascii_antigo;
    /* Regras que só tiram o "h" antigo (sahi -> saí, trahi -> traí) valem para todo o verbo */
    if (strchr(antigo, 'h') != NULL && strchr(novo, 'h') == NULL) acrescenta_acento = 0;
    /* Regra que já termina em espaço/pontuação ("Poz ", "Ha "): não há resto de palavra para conferir */
    if (tam_antigo > 0 && (unsigned char)antigo[tam_antigo - 1] < 0x80 && !isalpha((unsigned char)antigo[tam_antigo - 1])) acrescenta_acento = 0;

    buffer[0] = '\0';
    char *atual = str;

    while ((pos = strstr(atual, antigo)) != NULL) {
        int eh_inicio = 0;
        if (pos == str) {
            eh_inicio = 1;
        } else {
            unsigned char prev = (unsigned char)*(pos - 1);
            if (prev < 0x80 && !isalpha(prev)) {
                eh_inicio = 1;
            }
        }

        /* Proteção: se a regra ACRESCENTA acento (ex.: gloria -> glória),
           só vale para a palavra e suas formas nominais (-s, -o, -a, -os, -as).
           Evita "gloriar" -> "glóriar", "continuou" -> "contínuou", "homens" -> "homems". */
        if (eh_inicio && acrescenta_acento) {
            const char *resto_p = pos + tam_antigo;
            int tam_resto = 0;
            while (resto_p[tam_resto] != '\0' &&
                   !((unsigned char)resto_p[tam_resto] < 0x80 && !isalpha((unsigned char)resto_p[tam_resto]))) {
                tam_resto++;
            }
            int resto_ok = (tam_resto == 0) ||
                (tam_resto == 1 && (resto_p[0] == 's' || resto_p[0] == 'o' || resto_p[0] == 'a')) ||
                (tam_resto == 2 && resto_p[1] == 's' && (resto_p[0] == 'o' || resto_p[0] == 'a' || resto_p[0] == 'e'));
            if (!resto_ok) eh_inicio = 0;
        }

        if (!eh_inicio) {
            size_t avanco = (size_t)(pos - atual) + 1;
            if (usado + avanco >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
            memcpy(buffer + usado, atual, avanco);
            usado += avanco;
            atual = pos + 1;
            continue;
        }

        size_t trecho = (size_t)(pos - atual);
        if (usado + trecho + tam_novo >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */

        memcpy(buffer + usado, atual, trecho);
        usado += trecho;

        memcpy(buffer + usado, novo, tam_novo);
        usado += tam_novo;

        atual = pos + tam_antigo;
    }

    size_t resto = strlen(atual);
    if (usado + resto >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
    memcpy(buffer + usado, atual, resto);
    usado += resto;
    buffer[usado] = '\0';
    memcpy(str, buffer, usado + 1);
}

void substituir_palavra_inteira(char *str, const char *antigo, const char *novo) {
    char buffer[MAX_LINHA];
    char *pos;
    size_t tam_antigo = strlen(antigo);
    size_t tam_novo = strlen(novo);
    size_t usado = 0;
    if (tam_antigo == 0) return;

    buffer[0] = '\0';
    char *atual = str;

    while ((pos = strstr(atual, antigo)) != NULL) {
        int eh_inicio = 0;
        int eh_fim = 0;

        if (pos == str) {
            eh_inicio = 1;
        } else {
            unsigned char prev = (unsigned char)*(pos - 1);
            if (prev < 0x80 && !isalpha(prev)) {
                eh_inicio = 1;
            }
        }

        unsigned char next = (unsigned char)*(pos + tam_antigo);
        if (next == '\0' || (next < 0x80 && !isalpha(next))) {
            eh_fim = 1;
        }

        if (!eh_inicio || !eh_fim) {
            size_t avanco = (size_t)(pos - atual) + 1;
            if (usado + avanco >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
            memcpy(buffer + usado, atual, avanco);
            usado += avanco;
            atual = pos + 1;
            continue;
        }

        size_t trecho = (size_t)(pos - atual);
        if (usado + trecho + tam_novo >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */

        memcpy(buffer + usado, atual, trecho);
        usado += trecho;

        memcpy(buffer + usado, novo, tam_novo);
        usado += tam_novo;

        atual = pos + tam_antigo;
    }

    size_t resto = strlen(atual);
    if (usado + resto >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
    memcpy(buffer + usado, atual, resto);
    usado += resto;
    buffer[usado] = '\0';
    memcpy(str, buffer, usado + 1);
}

void converter_infinitivo_lusitano_para_gerundio(char *texto) {
    static const char *auxiliares[] = {
        "estava a ", "Estava a ",
        "estavam a ", "Estavam a ",
        "estando a ", "Estando a ",
        "esteve a ", "Esteve a ",
        "estiveram a ", "Estiveram a ",
        "está a ", "Está a ",
        "estão a ", "Estão a ",
        "andava a ", "Andava a ",
        "andavam a ", "Andavam a ",
        "andando a ", "Andando a ",
        "continuava a ", "Continuava a ",
        "continuavam a ", "Continuavam a ",
        "ficou a ", "Ficou a ",
        "ficaram a ", "Ficaram a "
    };

    int total_aux = (int)(sizeof(auxiliares) / sizeof(auxiliares[0]));

    for (int a = 0; a < total_aux; a++) {
        const char *aux = auxiliares[a];
        size_t tam_aux = strlen(aux);
        char *pos = texto;

        while ((pos = strstr(pos, aux)) != NULL) {
            if (pos != texto) {
                unsigned char prev = (unsigned char)*(pos - 1);
                if (!(prev < 0x80 && !isalpha(prev))) {
                    pos++;
                    continue;
                }
            }

            char *verbo = pos + tam_aux;
            int v_len = 0;
            while (verbo[v_len] != '\0' && (unsigned char)verbo[v_len] >= 'A') {
                if ((unsigned char)verbo[v_len] < 0x80 && !isalpha((unsigned char)verbo[v_len])) {
                    break;
                }
                v_len++;
            }

            static const char *nao_verbos[] = {"mulher", "senhor", "Senhor", "amor", "temor", "favor",
                                               "louvor", "clamor", "pastor", "lugar", "altar", "pomar", "manjar", "suor", NULL};
            int eh_nome = 0;
            for (int s = 0; nao_verbos[s] != NULL; s++)
                if ((int)strlen(nao_verbos[s]) == v_len && strncmp(verbo, nao_verbos[s], (size_t)v_len) == 0) eh_nome = 1;

            if (v_len >= 4 && !eh_nome) {
                char vogal = verbo[v_len - 2];
                char erre = verbo[v_len - 1];
                if (erre == 'r' && (vogal == 'a' || vogal == 'e' || vogal == 'i' || vogal == 'o')) {
                    /* "estava a falar" -> "estava falando": mesmo numero de bytes
                       (tira "a " e troca "r" por "ndo"). Troca so esta ocorrencia, no lugar. */
                    memmove(pos + tam_aux - 2, verbo, (size_t)(v_len - 1));
                    memcpy(pos + tam_aux - 2 + (v_len - 1), "ndo", 3);
                    pos = texto;
                    continue;
                }
            }
            pos += tam_aux;
        }
    }
}

int eh_delimitador(unsigned char c) {
    return (c == '\0' || c == ' ' || c == ',' || c == '.' || c == ';' ||
            c == ':' || c == '!' || c == '?' || c == ')' || c == '(' ||
            c == '"' || c == '\n' || c == '\r');
}

int eh_vogal_simples(char c) {
    c = (char)tolower((unsigned char)c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u');
}

void normalizar_composto_hifen(char *palavra) {
    char *hifen = strchr(palavra, '-');
    if (hifen == NULL) return;

    int pos_hifen = (int)(hifen - palavra);
    char radical[128];
    char sufixo[256];

    if (pos_hifen <= 0 || pos_hifen >= 120) return;
    memcpy(radical, palavra, (size_t)pos_hifen);
    radical[pos_hifen] = '\0';
    strcpy(sufixo, hifen);

    if (pos_hifen >= 3 && radical[pos_hifen - 2] == 'a' && radical[pos_hifen - 1] == 'e') {
        radical[pos_hifen - 1] = 'i';
    }
    else if (strcmp(radical, "pôz") == 0 || strcmp(radical, "poz") == 0) {
        strcpy(radical, "pôs");
    } else if (strcmp(radical, "Pôz") == 0 || strcmp(radical, "Poz") == 0) {
        strcpy(radical, "Pôs");
    } else if (strcmp(radical, "quiz") == 0) {
        strcpy(radical, "quis");
    } else if (strcmp(radical, "poude") == 0) {
        strcpy(radical, "pôde");
    } else if (strcmp(radical, "sahi") == 0) {
        strcpy(radical, "saí");
    } else if (strcmp(radical, "cahi") == 0) {
        strcpy(radical, "caí");
    } else if (strcmp(radical, "trahi") == 0) {
        strcpy(radical, "traí");
    } else if (strcmp(radical, "attrahi") == 0) {
        strcpy(radical, "atraí");
    } else if (strcmp(radical, "distrahi") == 0) {
        strcpy(radical, "distraí");
    }

    sprintf(palavra, "%s%s", radical, sufixo);
}

void atualizar_sufixos_e_enclises(char *str) {
    char buffer[MAX_LINHA];
    int i = 0, pos = 0;

    while (str[i] != '\0') {
        if (eh_delimitador((unsigned char)str[i])) {
            if (pos >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
            buffer[pos++] = str[i++];
            continue;
        }

        int inicio = i;
        while (!eh_delimitador((unsigned char)str[i])) {
            i++;
        }
        int len = i - inicio;
        char palavra[256];
        if (len >= 250) {
            if (pos + len >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
            memcpy(buffer + pos, str + inicio, (size_t)len);
            pos += len;
            continue;
        }
        memcpy(palavra, str + inicio, (size_t)len);
        palavra[len] = '\0';

        normalizar_composto_hifen(palavra);
        len = (int)strlen(palavra);

        struct RegraSufixo {
            const char *antigo;
            const char *novo;
            int min_raiz;
        };

        static const struct RegraSufixo regras[] = {
            {"al-o-hei", "á-lo-ei", 1}, {"al-a-hei", "á-la-ei", 1},
            {"al-os-hei", "á-los-ei", 1}, {"al-as-hei", "á-las-ei", 1},
            {"el-o-hei", "ê-lo-ei", 1}, {"el-a-hei", "ê-la-ei", 1},
            {"el-os-hei", "ê-los-ei", 1}, {"el-as-hei", "ê-las-ei", 1},
            {"ôl-o-hei", "ô-lo-ei", 1}, {"ol-o-hei", "ô-lo-ei", 1},
            {"il-o-hei", "i-lo-ei", 1}, {"il-a-hei", "i-la-ei", 1},
            {"al-o-hão", "á-lo-ão", 1}, {"al-a-hão", "á-la-ão", 1},
            {"al-os-hão", "á-los-ão", 1}, {"al-as-hão", "á-las-ão", 1},
            {"al-o-ha", "á-lo-á", 1}, {"al-a-ha", "á-la-á", 1},
            {"el-o-hão", "ê-lo-ão", 1}, {"el-a-hão", "ê-la-ão", 1},
            {"el-os-hão", "ê-los-ão", 1}, {"el-as-hão", "ê-las-ão", 1},
            {"el-o-ha", "ê-lo-á", 1}, {"el-a-ha", "ê-la-á", 1},
            {"il-o-hão", "i-lo-ão", 1}, {"il-a-hão", "i-la-ão", 1},
            {"il-o-ha", "i-lo-á", 1}, {"il-a-ha", "i-la-á", 1},
            {"-se-ha", "-se-á", 2}, {"-se-hão", "-se-ão", 2},
            {"-lhe-ha", "-lhe-á", 2}, {"-lhe-hão", "-lhe-ão", 2},
            {"-lhes-ha", "-lhes-á", 2}, {"-lhes-hão", "-lhes-ão", 2},

            {"nol-o", "no-lo", 0}, {"nol-a", "no-la", 0}, {"nol-os", "no-los", 0}, {"nol-as", "no-las", 0},
            {"vol-o", "vo-lo", 0}, {"vol-a", "vo-la", 0}, {"vol-os", "vo-los", 0}, {"vol-as", "vo-las", 0},
            {"al-o", "á-lo", 1}, {"al-a", "á-la", 1}, {"al-os", "á-los", 1}, {"al-as", "á-las", 1},
            {"el-o", "ê-lo", 1}, {"el-a", "ê-la", 1}, {"el-os", "ê-los", 1}, {"el-as", "ê-las", 1},
            {"ôl-o", "ô-lo", 1}, {"ôl-a", "ô-la", 1}, {"ôl-os", "ô-los", 1}, {"ôl-as", "ô-las", 1},
            {"ol-o", "ô-lo", 1}, {"ol-a", "ô-la", 1}, {"ol-os", "ô-los", 1}, {"ol-as", "ô-las", 1},

            {"ámos", "amos", 2},
            {"áras", "aras", 2},
            {"ára", "ara", 2},
            {"êras", "eras", 2},
            {"êra", "era", 2},
            {"aes", "ais", 1},
            {"ae", "ai", 1},

            {"avamos", "ávamos", 2},
            {"iamos", "íamos", 2},
            {"ieis", "íeis", 2},
            {"assemos", "ássemos", 2},
            {"essemos", "êssemos", 2},
            {"issemos", "íssemos", 2},
            {"asseis", "ásseis", 2},
            {"esseis", "êsseis", 2},
            {"isseis", "ísseis", 2},

            {"aveis", "áveis", 2},
            {"iveis", "íveis", 2},
            {"avel", "ável", 2},
            {"ivel", "ível", 2},
            {"issimos", "íssimos", 2},
            {"issimas", "íssimas", 2},
            {"issimo", "íssimo", 2},
            {"issima", "íssima", 2},
            {"encias", "ências", 2},
            {"encia", "ência", 2},
            {"ancias", "âncias", 0},
            {"ancia", "ância", 0}
        };

        int aplicado = 0;
        int total_regras = (int)(sizeof(regras) / sizeof(regras[0]));

        for (int r = 0; r < total_regras; r++) {
            int tam_ant = (int)strlen(regras[r].antigo);
            if (len >= tam_ant + regras[r].min_raiz &&
                strcmp(palavra + len - tam_ant, regras[r].antigo) == 0) {
                /* "il-o-hei", "il-o-ha"...: depois de vogal leva acento (restituil-o-ha -> restituí-lo-á),
                   menos no "gu"/"qu" (seguil-o-hão -> segui-lo-ão) */
                const char *novo_suf = regras[r].novo;
                char novo_hiato[32];
                int ini = len - tam_ant;
                if (regras[r].antigo[0] == 'i' && regras[r].antigo[1] == 'l' && ini >= 1 &&
                    eh_vogal_simples(palavra[ini - 1]) &&
                    !(ini >= 2 && palavra[ini - 1] == 'u' && (palavra[ini - 2] == 'g' || palavra[ini - 2] == 'q'))) {
                    snprintf(novo_hiato, sizeof(novo_hiato), "í%s", regras[r].novo + 1);
                    novo_suf = novo_hiato;
                }
                palavra[ini] = '\0';
                strcat(palavra, novo_suf);
                aplicado = 1;
                break;
            }
        }

        if (!aplicado && len >= 5) {
            const char *suf_il[] = {"il-o", "il-a", "il-os", "il-as"};
            const char *rep_hiato[] = {"í-lo", "í-la", "í-los", "í-las"};
            const char *rep_cons[] = {"i-lo", "i-la", "i-los", "i-las"};

            for (int k = 0; k < 4; k++) {
                int tam_s = (int)strlen(suf_il[k]);
                if (len > tam_s && strcmp(palavra + len - tam_s, suf_il[k]) == 0) {
                    char antes = palavra[len - tam_s - 1];
                    palavra[len - tam_s] = '\0';
                    char antes2 = len - tam_s - 2 >= 0 ? palavra[len - tam_s - 2] : ' ';
                    int hiato = eh_vogal_simples(antes) && !(antes == 'u' && (antes2 == 'g' || antes2 == 'q'));
                    strcat(palavra, hiato ? rep_hiato[k] : rep_cons[k]);
                    break;
                }
            }
        }

        int novo_len = (int)strlen(palavra);
        if (pos + novo_len >= MAX_LINHA - 1) return; /* nao cabe: texto fica como estava */
        memcpy(buffer + pos, palavra, (size_t)novo_len);
        pos += novo_len;
    }

    buffer[pos] = '\0';
    strcpy(str, buffer);
}

void atualizar_ortografia(char *texto) {
    // 1. Padroniza apóstrofo curvo UTF-8 (’) para apóstrofo simples (')
    substituir_texto(texto, "\xE2\x80\x99", "'");

    // 2. Contrações antigas com apóstrofo (formas aglutinadas)
    substituir_inicio_palavra(texto, "d'elle", "dele");      substituir_inicio_palavra(texto, "D'elle", "Dele");
    substituir_inicio_palavra(texto, "d'ella", "dela");      substituir_inicio_palavra(texto, "D'ella", "Dela");
    substituir_inicio_palavra(texto, "d'elles", "deles");    substituir_inicio_palavra(texto, "D'elles", "Deles");
    substituir_inicio_palavra(texto, "d'ellas", "delas");    substituir_inicio_palavra(texto, "D'ellas", "Delas");
    substituir_inicio_palavra(texto, "n'elle", "nele");      substituir_inicio_palavra(texto, "N'elle", "Nele");
    substituir_inicio_palavra(texto, "n'ella", "nela");      substituir_inicio_palavra(texto, "N'ella", "Nela");
    substituir_inicio_palavra(texto, "n'elles", "neles");    substituir_inicio_palavra(texto, "N'elles", "Neles");
    substituir_inicio_palavra(texto, "n'ellas", "nelas");    substituir_inicio_palavra(texto, "N'ellas", "Nelas");
    substituir_inicio_palavra(texto, "d'este", "deste");     substituir_inicio_palavra(texto, "D'este", "Deste");
    substituir_inicio_palavra(texto, "d'esta", "desta");     substituir_inicio_palavra(texto, "D'esta", "Desta");
    substituir_inicio_palavra(texto, "d'estes", "destes");   substituir_inicio_palavra(texto, "d'estas", "destas");
    substituir_inicio_palavra(texto, "n'este", "neste");     substituir_inicio_palavra(texto, "n'esta", "nesta");
    substituir_inicio_palavra(texto, "n'estes", "nestes");   substituir_inicio_palavra(texto, "n'estas", "nestas");
    substituir_inicio_palavra(texto, "d'isto", "disto");     substituir_inicio_palavra(texto, "n'isto", "nisto");
    substituir_inicio_palavra(texto, "d'isso", "disso");     substituir_inicio_palavra(texto, "n'isso", "nisso");
    substituir_inicio_palavra(texto, "d'aquillo", "daquilo");substituir_inicio_palavra(texto, "n'aquillo", "naquilo");
    substituir_inicio_palavra(texto, "d'um", "dum");         substituir_inicio_palavra(texto, "d'uma", "duma");
    substituir_inicio_palavra(texto, "d'uns", "duns");       substituir_inicio_palavra(texto, "d'umas", "dumas");
    substituir_inicio_palavra(texto, "n'um", "num");         substituir_inicio_palavra(texto, "n'uma", "numa");
    substituir_inicio_palavra(texto, "n'uns", "nuns");       substituir_inicio_palavra(texto, "n'umas", "numas");
    substituir_inicio_palavra(texto, "d'aquelle", "daquele");substituir_inicio_palavra(texto, "d'aquella", "daquela");
    substituir_inicio_palavra(texto, "n'aquelle", "naquele");substituir_inicio_palavra(texto, "n'aquella", "naquela");
    substituir_inicio_palavra(texto, "aquell'outro", "aqueloutro");
    substituir_inicio_palavra(texto, "d'aqui", "daqui");     substituir_inicio_palavra(texto, "d'alli", "dali");
    substituir_inicio_palavra(texto, "d'ali", "dali");
    substituir_inicio_palavra(texto, "d'onde", "donde");     substituir_inicio_palavra(texto, "D'onde", "Donde");
    substituir_inicio_palavra(texto, "lh'o", "lho");         substituir_inicio_palavra(texto, "lh'a", "lha");
    substituir_inicio_palavra(texto, "lh'os", "lhos");       substituir_inicio_palavra(texto, "lh'as", "lhas");

    /* Contrações com maiúscula e outras que faltavam (antes: "N'aquelle" -> "Em aquele") */
    substituir_inicio_palavra(texto, "N'aquelles", "Naqueles"); substituir_inicio_palavra(texto, "N'aquellas", "Naquelas");
    substituir_inicio_palavra(texto, "N'aquelle", "Naquele");   substituir_inicio_palavra(texto, "N'aquella", "Naquela");
    substituir_inicio_palavra(texto, "N'aquillo", "Naquilo");   substituir_inicio_palavra(texto, "D'aquillo", "Daquilo");
    substituir_inicio_palavra(texto, "D'aquelle", "Daquele");   substituir_inicio_palavra(texto, "D'aquella", "Daquela");
    substituir_inicio_palavra(texto, "D'aquelles", "Daqueles"); substituir_inicio_palavra(texto, "D'aquellas", "Daquelas");
    substituir_inicio_palavra(texto, "d'aquelles", "daqueles"); substituir_inicio_palavra(texto, "d'aquellas", "daquelas");
    substituir_inicio_palavra(texto, "n'aquelles", "naqueles"); substituir_inicio_palavra(texto, "n'aquellas", "naquelas");
    substituir_inicio_palavra(texto, "N'isto", "Nisto");        substituir_inicio_palavra(texto, "N'isso", "Nisso");
    substituir_inicio_palavra(texto, "D'isto", "Disto");        substituir_inicio_palavra(texto, "D'isso", "Disso");
    substituir_inicio_palavra(texto, "N'este", "Neste");        substituir_inicio_palavra(texto, "N'esta", "Nesta");
    substituir_inicio_palavra(texto, "N'estes", "Nestes");      substituir_inicio_palavra(texto, "N'estas", "Nestas");
    substituir_inicio_palavra(texto, "N'esse", "Nesse");        substituir_inicio_palavra(texto, "N'essa", "Nessa");
    substituir_inicio_palavra(texto, "n'esse", "nesse");        substituir_inicio_palavra(texto, "n'essa", "nessa");
    substituir_inicio_palavra(texto, "n'esses", "nesses");      substituir_inicio_palavra(texto, "n'essas", "nessas");
    substituir_inicio_palavra(texto, "d'esse", "desse");        substituir_inicio_palavra(texto, "d'essa", "dessa");
    substituir_inicio_palavra(texto, "N'um", "Num");            substituir_inicio_palavra(texto, "N'uma", "Numa");
    substituir_inicio_palavra(texto, "n'algum", "nalgum");      substituir_inicio_palavra(texto, "n'alguma", "nalguma");
    substituir_inicio_palavra(texto, "n'alguns", "nalguns");    substituir_inicio_palavra(texto, "n'algumas", "nalgumas");
    substituir_inicio_palavra(texto, "n'outro", "noutro");      substituir_inicio_palavra(texto, "n'outra", "noutra");
    substituir_inicio_palavra(texto, "n'outros", "noutros");    substituir_inicio_palavra(texto, "n'outras", "noutras");
    /* Pronome depois de hífen: "ataram-n'o" -> "ataram-no" (antes virava "ataram-em o") */
    substituir_texto(texto, "-n'os", "-nos");                   substituir_texto(texto, "-n'as", "-nas");
    substituir_texto(texto, "-n'o", "-no");                     substituir_texto(texto, "-n'a", "-na");
    /* me/te + o/a: "m'o" -> "mo", "t'a" -> "ta" */
    substituir_inicio_palavra(texto, "m'os", "mos");            substituir_inicio_palavra(texto, "m'as", "mas");
    substituir_inicio_palavra(texto, "m'o", "mo");              substituir_inicio_palavra(texto, "m'a", "ma");
    substituir_inicio_palavra(texto, "t'os", "tos");            substituir_inicio_palavra(texto, "t'as", "tas");
    substituir_inicio_palavra(texto, "t'o", "to");              substituir_inicio_palavra(texto, "t'a", "ta");

    substituir_inicio_palavra(texto, "d'", "de ");
    substituir_inicio_palavra(texto, "D'", "De ");
    substituir_inicio_palavra(texto, "n'", "em ");
    substituir_inicio_palavra(texto, "N'", "Em ");

    // 3. Crase antiga com acento agudo e locuções
    substituir_texto(texto, " á ", " à ");
    substituir_texto(texto, " ás ", " às ");
    substituir_inicio_palavra(texto, "Á ", "À ");
    substituir_inicio_palavra(texto, "Ás ", "Às ");
    substituir_inicio_palavra(texto, "áquel", "àquel");
    substituir_inicio_palavra(texto, "Áquel", "Àquel");
    substituir_inicio_palavra(texto, "ámanhã", "amanhã");
    substituir_inicio_palavra(texto, "Ámanhã", "Amanhã");
    substituir_inicio_palavra(texto, "áparte", "à parte");
    substituir_inicio_palavra(texto, "áquem", "aquém");
    substituir_inicio_palavra(texto, "ácerca", "acerca");
    substituir_inicio_palavra(texto, "Ácerca", "Acerca");
    substituir_texto(texto, "afim de", "a fim de");

    // 4. Regras críticas ANTES da simplificação de consoantes dobradas
    substituir_inicio_palavra(texto, "sabbado", "sábado");
    substituir_inicio_palavra(texto, "Sabbado", "Sábado");
    substituir_inicio_palavra(texto, "SABBADO", "SÁBADO");
    substituir_inicio_palavra(texto, "attrahi", "atraí");
    substituir_inicio_palavra(texto, "Attrahi", "Atraí");
    substituir_inicio_palavra(texto, "attrahe", "atrai");
    substituir_inicio_palavra(texto, "ressurg", "ressurg");
    substituir_inicio_palavra(texto, "resvall", "resval");

    // 5. Dígrafos antigos (Ph, Th, Y) e consoantes dobradas (maiúsculas e minúsculas)
    substituir_texto(texto, "PH", "F");
    substituir_texto(texto, "Ph", "F");
    substituir_texto(texto, "ph", "f");
    substituir_texto(texto, "TH", "T");
    substituir_texto(texto, "Th", "T");
    substituir_texto(texto, "th", "t");
    substituir_texto(texto, "Y", "I");
    substituir_texto(texto, "y", "i");

    substituir_texto(texto, "ll", "l"); substituir_texto(texto, "LL", "L");
    substituir_texto(texto, "mm", "m"); substituir_texto(texto, "MM", "M");
    substituir_texto(texto, "nn", "n"); substituir_texto(texto, "NN", "N");
    substituir_texto(texto, "ff", "f"); substituir_texto(texto, "FF", "F");
    substituir_texto(texto, "pp", "p"); substituir_texto(texto, "PP", "P");
    substituir_texto(texto, "tt", "t"); substituir_texto(texto, "TT", "T");
    substituir_texto(texto, "cc", "c"); substituir_texto(texto, "CC", "C");
    substituir_texto(texto, "dd", "d"); substituir_texto(texto, "DD", "D");
    substituir_texto(texto, "bb", "b"); substituir_texto(texto, "BB", "B");
    substituir_texto(texto, "gg", "g"); substituir_texto(texto, "GG", "G");

    // 6. Grupos consonantais mudos (cç, ct, pt, mn, gm, ch -> c/qu)
    substituir_texto(texto, "cç", "ç");
    substituir_inicio_palavra(texto, "CHRIST", "CRIST");
    substituir_inicio_palavra(texto, "Christ", "Crist");
    substituir_inicio_palavra(texto, "christ", "crist");
    substituir_inicio_palavra(texto, "chronic", "crônic");
    substituir_inicio_palavra(texto, "archanjo", "arcanjo");
    substituir_inicio_palavra(texto, "architecto", "arquiteto");
    substituir_inicio_palavra(texto, "sepulchr", "sepulcr");
    substituir_inicio_palavra(texto, "patriarch", "patriarc");
    substituir_inicio_palavra(texto, "tetrarch", "tetrarc");
    substituir_inicio_palavra(texto, "eunuch", "eunuc");
    substituir_inicio_palavra(texto, "mechanic", "mecânic");
    substituir_inicio_palavra(texto, "paschoa", "Páscoa");
    substituir_inicio_palavra(texto, "Paschoa", "Páscoa");
    substituir_inicio_palavra(texto, "chalcedonia", "calcedônia");
    substituir_inicio_palavra(texto, "Chaldea", "Caldeia");
    substituir_inicio_palavra(texto, "chaldeo", "caldeu");

    substituir_texto(texto, "fruct", "frut");
    substituir_texto(texto, "Fruct", "Frut");
    substituir_inicio_palavra(texto, "sanct", "sant");
    substituir_inicio_palavra(texto, "Sanct", "Sant");
    substituir_inicio_palavra(texto, "SANCT", "SANT");
    substituir_inicio_palavra(texto, "afflict", "aflit");
    substituir_inicio_palavra(texto, "victima", "vítima");
    substituir_inicio_palavra(texto, "victoria", "vitória");
    substituir_inicio_palavra(texto, "victorios", "vitorios");
    substituir_inicio_palavra(texto, "luct", "lut");
    substituir_inicio_palavra(texto, "Luct", "Lut");
    substituir_inicio_palavra(texto, "nocturn", "noturn");
    substituir_inicio_palavra(texto, "delict", "delit");
    substituir_inicio_palavra(texto, "edict", "édit");
    substituir_inicio_palavra(texto, "distinct", "distint");
    substituir_inicio_palavra(texto, "extinct", "extint");
    substituir_inicio_palavra(texto, "contract", "contrat");
    substituir_inicio_palavra(texto, "detractor", "detrator");
    substituir_inicio_palavra(texto, "predilect", "predilet");
    substituir_inicio_palavra(texto, "rectidão", "retidão");
    substituir_inicio_palavra(texto, "rectamente", "retamente");
    substituir_inicio_palavra(texto, "rectaguarda", "retaguarda");
    substituir_inicio_palavra(texto, "object", "objet");
    substituir_inicio_palavra(texto, "project", "projet");
    substituir_inicio_palavra(texto, "Project", "Projet");
    substituir_inicio_palavra(texto, "product", "produt");
    substituir_inicio_palavra(texto, "exact", "exat");
    substituir_inicio_palavra(texto, "acto", "ato");
    substituir_inicio_palavra(texto, "actos", "atos");
    substituir_texto(texto, "escript", "escrit");
    substituir_texto(texto, "Escript", "Escrit");
    substituir_inicio_palavra(texto, "captiv", "cativ");
    substituir_inicio_palavra(texto, "prompt", "pront");
    substituir_inicio_palavra(texto, "assump", "assun");
    substituir_inicio_palavra(texto, "presump", "presun");
    substituir_inicio_palavra(texto, "redemp", "reden");
    substituir_inicio_palavra(texto, "Redemp", "Reden");
    substituir_inicio_palavra(texto, "exempt", "isent");
    substituir_inicio_palavra(texto, "bapt", "bat");
    substituir_inicio_palavra(texto, "Bapt", "Bat");
    substituir_inicio_palavra(texto, "sceptr", "cetr");
    substituir_inicio_palavra(texto, "inscrip", "inscri");
    substituir_inicio_palavra(texto, "descrip", "descri");
    substituir_inicio_palavra(texto, "prescrip", "prescri");
    substituir_inicio_palavra(texto, "augment", "aument");
    substituir_inicio_palavra(texto, "Augment", "Aument");
    substituir_inicio_palavra(texto, "condemn", "conden");
    substituir_inicio_palavra(texto, "Condemn", "Conden");
    substituir_inicio_palavra(texto, "solemn", "solen");
    substituir_inicio_palavra(texto, "Solemn", "Solen");
    substituir_inicio_palavra(texto, "damn", "dan");
    substituir_inicio_palavra(texto, "somno", "sono");
    substituir_inicio_palavra(texto, "hymno", "hino");
    substituir_inicio_palavra(texto, "alumno", "aluno");
    substituir_inicio_palavra(texto, "calumni", "calúni");    /* calúnia(s) */
    substituir_inicio_palavra(texto, "calumni", "caluni");    /* caluniar, caluniador... */
    substituir_inicio_palavra(texto, "columna", "coluna");
    substituir_inicio_palavra(texto, "Omnipotente", "Onipotente");
    substituir_inicio_palavra(texto, "omnipotente", "onipotente");
    substituir_inicio_palavra(texto, "signa", "sina");
    substituir_inicio_palavra(texto, "signal", "sinal");
    substituir_inicio_palavra(texto, "assign", "assin");
    substituir_inicio_palavra(texto, "Assign", "Assin");
    substituir_inicio_palavra(texto, "psalm", "salm");
    substituir_inicio_palavra(texto, "Psalm", "Salm");
    substituir_inicio_palavra(texto, "sciencia", "ciência");

    // 7. Aplica regras morfológicas de sufixos, ênclises, mesóclises e compostos com hífen
    /* "eil-o" -> "ei-lo" (sem acento); precisa vir antes das ênclises */
    substituir_palavra_inteira(texto, "eil-o", "ei-lo");     substituir_palavra_inteira(texto, "Eil-o", "Ei-lo");
    substituir_palavra_inteira(texto, "eil-a", "ei-la");     substituir_palavra_inteira(texto, "Eil-a", "Ei-la");
    substituir_palavra_inteira(texto, "eil-os", "ei-los");   substituir_palavra_inteira(texto, "Eil-os", "Ei-los");
    substituir_palavra_inteira(texto, "eil-as", "ei-las");   substituir_palavra_inteira(texto, "Eil-as", "Ei-las");

    atualizar_sufixos_e_enclises(texto);

    // 8. Flexões irregulares e vocabulário histórico de 1911
    substituir_texto(texto, " he ", " é ");
    substituir_inicio_palavra(texto, "He ", "É ");
    substituir_texto(texto, " ha ", " há ");    substituir_texto(texto, " ha,", " há,");    substituir_inicio_palavra(texto, "Ha ", "Há ");
    substituir_inicio_palavra(texto, "hum ", "um ");
    substituir_inicio_palavra(texto, "huma ", "uma ");
    substituir_inicio_palavra(texto, "huns ", "uns ");
    substituir_inicio_palavra(texto, "humas ", "umas ");
    substituir_inicio_palavra(texto, "hontem", "ontem");
    substituir_inicio_palavra(texto, "Hontem", "Ontem");
    substituir_inicio_palavra(texto, "hombro", "ombro");
    substituir_inicio_palavra(texto, "hombreira", "ombreira");
    substituir_inicio_palavra(texto, "humidade", "umidade");
    substituir_inicio_palavra(texto, "humedecem", "umedecem");
    substituir_inicio_palavra(texto, "vehemen", "veemen");
    substituir_inicio_palavra(texto, "cohorte", "coorte");

    substituir_inicio_palavra(texto, "creou", "criou");       substituir_inicio_palavra(texto, "Creou", "Criou");
    substituir_inicio_palavra(texto, "crear", "criar");       substituir_inicio_palavra(texto, "creara", "criara");
    substituir_inicio_palavra(texto, "creará", "criará");     substituir_inicio_palavra(texto, "crearam", "criaram");
    substituir_inicio_palavra(texto, "crearão", "criarão");   substituir_inicio_palavra(texto, "creaste", "criaste");
    substituir_inicio_palavra(texto, "creado", "criado");     substituir_inicio_palavra(texto, "creada", "criada");
    substituir_inicio_palavra(texto, "Creador", "Criador");   substituir_inicio_palavra(texto, "creação", "criação");
    substituir_inicio_palavra(texto, "creatura", "criatura"); substituir_inicio_palavra(texto, "creança", "criança");
    substituir_inicio_palavra(texto, "creancinha", "criancinha");

    substituir_inicio_palavra(texto, "veiu", "veio");         substituir_inicio_palavra(texto, "Veiu", "Veio");
    substituir_inicio_palavra(texto, "quiz", "quis");
    substituir_inicio_palavra(texto, "poude", "pôde");
    substituir_inicio_palavra(texto, "podera", "pudera");     substituir_inicio_palavra(texto, "poderam", "puderam");
    substituir_inicio_palavra(texto, "podesse", "pudesse");   substituir_inicio_palavra(texto, "podessem", "pudessem");
    substituir_inicio_palavra(texto, "podeste", "pudeste");   substituir_inicio_palavra(texto, "podestes", "pudestes");
    substituir_inicio_palavra(texto, "puz", "pus");           substituir_inicio_palavra(texto, "Puz", "Pus");
    substituir_inicio_palavra(texto, "pozer", "puser");       substituir_inicio_palavra(texto, "pozemos", "pusemos");
    substituir_inicio_palavra(texto, "pozesse", "pusesse");   substituir_inicio_palavra(texto, "pozeste", "puseste");
    substituir_inicio_palavra(texto, "Pozeste", "Puseste");   substituir_inicio_palavra(texto, "pozestes", "pusestes");
    substituir_texto(texto, " poz ", " pôs ");                substituir_texto(texto, " poz,", " pôs,");
    substituir_texto(texto, " poz.", " pôs.");                substituir_texto(texto, " poz;", " pôs;");
    substituir_inicio_palavra(texto, "Poz ", "Pôs ");         substituir_inicio_palavra(texto, "pôz", "pôs");
    substituir_inicio_palavra(texto, "compoz", "compôs");     substituir_inicio_palavra(texto, "depoz", "depôs");
    substituir_inicio_palavra(texto, "Depoz", "Depôs");       substituir_inicio_palavra(texto, "dispoz", "dispôs");
    substituir_inicio_palavra(texto, "expoz", "expôs");       substituir_inicio_palavra(texto, "impoz", "impôs");
    substituir_inicio_palavra(texto, "interpoz", "interpôs"); substituir_inicio_palavra(texto, "oppoz", "opôs");
    substituir_inicio_palavra(texto, "propoz", "propôs");     substituir_inicio_palavra(texto, "Propoz", "Propôs");
    substituir_inicio_palavra(texto, "propuz", "propus");     substituir_inicio_palavra(texto, "Propuz", "Propus");

    substituir_inicio_palavra(texto, "sahi", "saí");          substituir_inicio_palavra(texto, "Sahi", "Saí");
    substituir_inicio_palavra(texto, "sahia", "saía");        substituir_inicio_palavra(texto, "sahis", "saís");
    substituir_inicio_palavra(texto, "cahi", "caí");          substituir_inicio_palavra(texto, "Cahi", "Caí");
    substituir_inicio_palavra(texto, "cahia", "caía");
    substituir_inicio_palavra(texto, "trahi", "traí");        substituir_inicio_palavra(texto, "trahir", "trair");
    substituir_inicio_palavra(texto, "distrahi", "distraí");
    substituir_inicio_palavra(texto, "trahe", "trai");

    substituir_inicio_palavra(texto, "bemaventurad", "bem-aventurad");
    substituir_inicio_palavra(texto, "Bemaventurad", "Bem-aventurad");
    substituir_inicio_palavra(texto, "bemaventuranç", "bem-aventuranç");
    substituir_inicio_palavra(texto, "Bemaventuranç", "Bem-aventuranç");
    substituir_inicio_palavra(texto, "Bem-Aventurad", "Bem-aventurad");
    substituir_inicio_palavra(texto, "Bem-Aventuranç", "Bem-aventuranç");

    /* Só a palavra "aza(s)"; antes estragava nomes: Azarias -> Asarias */
    substituir_palavra_inteira(texto, "aza", "asa");          substituir_palavra_inteira(texto, "azas", "asas");
    substituir_palavra_inteira(texto, "Azas", "Asas");
    substituir_inicio_palavra(texto, "vasia", "vazia");       substituir_inicio_palavra(texto, "vasio", "vazio");
    substituir_inicio_palavra(texto, "abraza", "abrasa");     substituir_inicio_palavra(texto, "abraze", "abrase");
    substituir_inicio_palavra(texto, "abrazou", "abrasou");   substituir_inicio_palavra(texto, "arraza", "arrasa");
    substituir_inicio_palavra(texto, "atraz", "atrás");       substituir_inicio_palavra(texto, "detraz", "detrás");
    substituir_inicio_palavra(texto, "apoz", "após");         substituir_inicio_palavra(texto, "Apoz", "Após");
    substituir_inicio_palavra(texto, "defeza", "defesa");     substituir_inicio_palavra(texto, "despeza", "despesa");
    substituir_inicio_palavra(texto, "princeza", "princesa"); substituir_inicio_palavra(texto, "Princeza", "Princesa");
    substituir_inicio_palavra(texto, "turqueza", "turquesa");
    substituir_inicio_palavra(texto, "dezeseis", "dezesseis");substituir_inicio_palavra(texto, "dezesete", "dezessete");
    substituir_inicio_palavra(texto, "enteza", "entesa");     substituir_inicio_palavra(texto, "pezo", "peso");
    substituir_inicio_palavra(texto, "Pezo", "Peso");         substituir_inicio_palavra(texto, "peza", "pesa");
    substituir_inicio_palavra(texto, "apezar", "apesar");     substituir_inicio_palavra(texto, "piza", "pisa");
    substituir_inicio_palavra(texto, "pizei", "pisei");       substituir_inicio_palavra(texto, "pizem", "pisem");
    substituir_inicio_palavra(texto, "pizou", "pisou");       substituir_inicio_palavra(texto, "pesquiza", "pesquisa");
    substituir_inicio_palavra(texto, "visinh", "vizinh");
    substituir_inicio_palavra(texto, "organisad", "organizad");
    substituir_inicio_palavra(texto, "atemoris", "atemoriz");
    substituir_inicio_palavra(texto, "aterroris", "aterroriz");
    substituir_inicio_palavra(texto, "cauteris", "cauteriz");
    substituir_inicio_palavra(texto, "encoleris", "encoleriz");
    substituir_inicio_palavra(texto, "penalis", "penaliz");
    substituir_inicio_palavra(texto, "realis", "realiz");
    substituir_inicio_palavra(texto, "paralys", "paralis");
    substituir_inicio_palavra(texto, "paralyt", "paralít");
    substituir_inicio_palavra(texto, "alcans", "alcanç");     substituir_inicio_palavra(texto, "Alcans", "Alcanç");
    substituir_inicio_palavra(texto, "cança", "cansa");       substituir_inicio_palavra(texto, "Cança", "Cansa");
    substituir_inicio_palavra(texto, "cance", "canse");       substituir_inicio_palavra(texto, "cançou", "cansou");
    substituir_inicio_palavra(texto, "descanç", "descans");   substituir_inicio_palavra(texto, "Descanç", "Descans");
    substituir_inicio_palavra(texto, "descanc", "descans");
    substituir_inicio_palavra(texto, "arremeç", "arremess");
    substituir_inicio_palavra(texto, "soceg", "sosseg");
    substituir_inicio_palavra(texto, "resuscit", "ressuscit");substituir_inicio_palavra(texto, "Resuscit", "Ressuscit");
    substituir_inicio_palavra(texto, "resurreição", "ressurreição");
    substituir_inicio_palavra(texto, "sanguesuga", "sanguessuga");
    substituir_inicio_palavra(texto, "outrosim", "outrossim");substituir_inicio_palavra(texto, "Outrosim", "Outrossim");

    substituir_inicio_palavra(texto, "adeante", "adiante");   substituir_inicio_palavra(texto, "deante", "diante");
    substituir_inicio_palavra(texto, "edade", "idade");       substituir_inicio_palavra(texto, "edosos", "idosos");
    substituir_inicio_palavra(texto, "egreja", "igreja");
    substituir_inicio_palavra(texto, "egual", "igual");       substituir_inicio_palavra(texto, "Egual", "Igual");
    substituir_inicio_palavra(texto, "eguais", "iguais");     substituir_inicio_palavra(texto, "desegua", "desigua");
    substituir_inicio_palavra(texto, "emquanto", "enquanto"); substituir_inicio_palavra(texto, "Emquanto", "Enquanto");
    substituir_inicio_palavra(texto, "comtanto", "contanto");
    substituir_inicio_palavra(texto, "comtudo", "contudo");   substituir_inicio_palavra(texto, "Comtudo", "Contudo");
    substituir_inicio_palavra(texto, "commigo", "comigo");    substituir_inicio_palavra(texto, "comnosco", "conosco");
    substituir_inicio_palavra(texto, "comsigo", "consigo");   substituir_inicio_palavra(texto, "comtigo", "contigo");
    substituir_inicio_palavra(texto, "Comtigo", "Contigo");   substituir_inicio_palavra(texto, "comvosco", "convosco");
    substituir_inicio_palavra(texto, "bemdiz", "bendiz");     substituir_inicio_palavra(texto, "Bemdiz", "Bendiz");
    substituir_inicio_palavra(texto, "bemdit", "bendit");     substituir_inicio_palavra(texto, "Bemdit", "Bendit");
    substituir_inicio_palavra(texto, "bemdiss", "bendiss");   substituir_inicio_palavra(texto, "bemdig", "bendig");
    substituir_inicio_palavra(texto, "bemdir", "bendir");
    substituir_inicio_palavra(texto, "circum", "circun");     substituir_inicio_palavra(texto, "Circum", "Circun");
    substituir_inicio_palavra(texto, "cincoenta", "cinquenta");substituir_inicio_palavra(texto, "Cincoenta", "Cinquenta");
    substituir_inicio_palavra(texto, "logar", "lugar");       substituir_inicio_palavra(texto, "herva", "erva");
    substituir_inicio_palavra(texto, "Herva", "Erva");        substituir_inicio_palavra(texto, "similh", "semelh");
    substituir_inicio_palavra(texto, "Similh", "Semelh");     substituir_inicio_palavra(texto, "quasi", "quase");
    substituir_inicio_palavra(texto, "Quasi", "Quase");       substituir_inicio_palavra(texto, "peior", "pior");
    substituir_inicio_palavra(texto, "engul", "engol");       substituir_inicio_palavra(texto, "Engul", "Engol");
    substituir_inicio_palavra(texto, "cubriram", "cobriram"); substituir_inicio_palavra(texto, "cubriu", "cobriu");
    substituir_inicio_palavra(texto, "inimisade", "inimizade");
    substituir_inicio_palavra(texto, "magestade", "majestade");substituir_inicio_palavra(texto, "Magestade", "Majestade");
    substituir_inicio_palavra(texto, "mangedoura", "manjedoura");
    substituir_inicio_palavra(texto, "engeita", "enjeita");   substituir_inicio_palavra(texto, "hereje", "herege");
    substituir_inicio_palavra(texto, "lisonge", "lisonje");   substituir_inicio_palavra(texto, "bozin", "buzin");
    /* Só "tribu(s)"; antes estragava tributo, tribuno, tribunal */
    substituir_palavra_inteira(texto, "tribu", "tribo");      substituir_palavra_inteira(texto, "tribus", "tribos");
    substituir_inicio_palavra(texto, "thesoiro", "tesouro");
    substituir_inicio_palavra(texto, "taboado", "tabuado");   substituir_inicio_palavra(texto, "taboinha", "tabuinha");
    substituir_inicio_palavra(texto, "taboa", "tábua");       substituir_inicio_palavra(texto, "taboas", "tábuas");
    substituir_inicio_palavra(texto, "sobaco", "sovaco");     substituir_inicio_palavra(texto, "gloton", "gluton");
    substituir_inicio_palavra(texto, "dextra", "destra");     substituir_inicio_palavra(texto, "Dextra", "Destra");
    substituir_inicio_palavra(texto, "euro-aquilão", "euroaquilão");
    substituir_inicio_palavra(texto, "comprehend", "compreend");
    substituir_inicio_palavra(texto, "reprehens", "repreens");
    substituir_inicio_palavra(texto, "reprehend", "repreend");substituir_inicio_palavra(texto, "Reprehend", "Repreend");
    substituir_inicio_palavra(texto, "surprehend", "surpreend");substituir_inicio_palavra(texto, "Surprehend", "Surpreend");
    substituir_inicio_palavra(texto, "emprehend", "empreend");
    substituir_inicio_palavra(texto, "incomprehens", "incompreens");
    substituir_inicio_palavra(texto, "ideia", "idea");        substituir_inicio_palavra(texto, "passeia", "passea");
    substituir_inicio_palavra(texto, "receia", "recea");      substituir_inicio_palavra(texto, "rodeia", "rodea");
    substituir_inicio_palavra(texto, "recreia", "recrea");    substituir_inicio_palavra(texto, "refreia", "refrea");
    substituir_inicio_palavra(texto, "ceiar", "cear");

    // 9. Acentuação moderna (remoção de acentos antigos e inclusão dos atuais)
    substituir_inicio_palavra(texto, "crêem", "creem");       substituir_inicio_palavra(texto, "dêem", "deem");
    substituir_inicio_palavra(texto, "Dêem", "Deem");         substituir_inicio_palavra(texto, "lêem", "leem");
    substituir_inicio_palavra(texto, "vêem", "veem");         substituir_inicio_palavra(texto, "Vêem", "Veem");
    substituir_inicio_palavra(texto, "vôo", "voo");           substituir_inicio_palavra(texto, "vôa", "voa");
    substituir_inicio_palavra(texto, "vôem", "voem");         substituir_inicio_palavra(texto, "abençô", "abenço");
    substituir_inicio_palavra(texto, "Abençô", "Abenço");     substituir_inicio_palavra(texto, "abençõe", "abençoe");
    substituir_inicio_palavra(texto, "apregô", "aprego");     substituir_inicio_palavra(texto, "entôa", "entoa");
    substituir_inicio_palavra(texto, "perdô", "perdo");       substituir_inicio_palavra(texto, "Perdô", "Perdo");
    substituir_inicio_palavra(texto, "magõe", "magoe");       substituir_inicio_palavra(texto, "povôem", "povoem");
    substituir_inicio_palavra(texto, "assembléa", "assembleia");
    substituir_inicio_palavra(texto, "balêa", "baleia");      substituir_inicio_palavra(texto, "pavêa", "paveia");
    substituir_inicio_palavra(texto, "têa", "teia");          substituir_inicio_palavra(texto, "côr", "cor");
    substituir_inicio_palavra(texto, "cór", "cor");           substituir_inicio_palavra(texto, "dôr", "dor");
    substituir_inicio_palavra(texto, "Dôr", "Dor");           substituir_inicio_palavra(texto, "flôr", "flor");
    substituir_inicio_palavra(texto, "fôr", "for");           substituir_inicio_palavra(texto, "fóra", "fora");
    substituir_inicio_palavra(texto, "Fóra", "Fora");         substituir_inicio_palavra(texto, "fôra", "fora");
    substituir_inicio_palavra(texto, "Fôra", "Fora");         substituir_inicio_palavra(texto, "afóra", "afora");
    substituir_inicio_palavra(texto, "Afóra", "Afora");       substituir_inicio_palavra(texto, "fórma", "forma");
    substituir_inicio_palavra(texto, "fórmo", "formo");       substituir_inicio_palavra(texto, "sómente", "somente");
    substituir_inicio_palavra(texto, "Sómente", "Somente");   substituir_inicio_palavra(texto, "sósinho", "sozinho");
    substituir_inicio_palavra(texto, "páo", "pau");           substituir_inicio_palavra(texto, "páu", "pau");
    substituir_inicio_palavra(texto, "máu", "mau");           substituir_inicio_palavra(texto, "váo", "vau");
    substituir_inicio_palavra(texto, "náo", "nau");           substituir_inicio_palavra(texto, "náu", "nau");
    substituir_inicio_palavra(texto, "degráu", "degrau");     substituir_inicio_palavra(texto, "póde", "pode");
    substituir_inicio_palavra(texto, "pódes", "podes");       substituir_inicio_palavra(texto, "préga", "prega");
    substituir_inicio_palavra(texto, "prég", "preg");         substituir_inicio_palavra(texto, "Prég", "Preg");
    substituir_inicio_palavra(texto, "crêr", "crer");         substituir_inicio_palavra(texto, "lêr", "ler");
    substituir_inicio_palavra(texto, "vêr", "ver");           substituir_inicio_palavra(texto, "crêde", "crede");
    substituir_inicio_palavra(texto, "Crêde", "Crede");       substituir_inicio_palavra(texto, "crêdes", "credes");
    substituir_inicio_palavra(texto, "Crêdes", "Credes");     substituir_inicio_palavra(texto, "crêmos", "cremos");
    substituir_inicio_palavra(texto, "crêste", "creste");     substituir_inicio_palavra(texto, "lêde", "lede");
    substituir_inicio_palavra(texto, "lêstes", "lestes");     substituir_inicio_palavra(texto, "vêde", "vede");
    substituir_inicio_palavra(texto, "Vêde", "Vede");         substituir_inicio_palavra(texto, "vêdes", "vedes");
    substituir_inicio_palavra(texto, "Vêdes", "Vedes");       substituir_inicio_palavra(texto, "dér", "der");
    substituir_inicio_palavra(texto, "dés", "des");           substituir_inicio_palavra(texto, "désse", "desse");
    substituir_inicio_palavra(texto, "déste", "deste");       substituir_inicio_palavra(texto, "Déste", "Deste");
    substituir_inicio_palavra(texto, "démos", "demos");       substituir_inicio_palavra(texto, "podér", "poder");
    substituir_inicio_palavra(texto, "podémos", "podemos");   substituir_inicio_palavra(texto, "têem", "têm");
    substituir_inicio_palavra(texto, "teem", "têm");          substituir_inicio_palavra(texto, "Teem", "Têm");
    substituir_inicio_palavra(texto, "detêem", "detêm");      substituir_inicio_palavra(texto, "deteem", "detêm");
    substituir_inicio_palavra(texto, "mantêem", "mantêm");    substituir_inicio_palavra(texto, "manteem", "mantêm");
    substituir_inicio_palavra(texto, "convêem", "convêm");    substituir_inicio_palavra(texto, "conveem", "convêm");
    substituir_inicio_palavra(texto, "sobrevêem", "sobrevêm");substituir_inicio_palavra(texto, "sobreveem", "sobrevêm");
    substituir_inicio_palavra(texto, "sêde", "sede");         substituir_inicio_palavra(texto, "Sêde", "Sede");
    substituir_inicio_palavra(texto, "convem", "convém");     substituir_inicio_palavra(texto, "Convem", "Convém");
    substituir_inicio_palavra(texto, "detem", "detém");       substituir_inicio_palavra(texto, "mantem", "mantém");
    substituir_inicio_palavra(texto, "sobrevem", "sobrevém");
    substituir_inicio_palavra(texto, "véla", "vela");         substituir_inicio_palavra(texto, "vélas", "velas");
    substituir_inicio_palavra(texto, "pátriarca", "patriarca");
    substituir_inicio_palavra(texto, "fábricador", "fabricador");
    substituir_texto(texto, "aos pães e às mães", "aos pais e às mães");
    /* Só a palavra "ahi" (= aí); antes estragava nomes: Ahimaas -> Aímaas */
    substituir_palavra_inteira(texto, "ahi", "aí");           substituir_palavra_inteira(texto, "Ahi", "Aí");
    substituir_inicio_palavra(texto, "alli", "ali");          substituir_inicio_palavra(texto, "nú", "nu");
    substituir_inicio_palavra(texto, "núa", "nua");           substituir_inicio_palavra(texto, "nús", "nus");
    /* Só a palavra inteira; antes "homens" virava "homems" (1158 vezes) */
    substituir_palavra_inteira(texto, "joven", "jovem");      substituir_palavra_inteira(texto, "homen", "homem");
    substituir_inicio_palavra(texto, "algums", "alguns");     substituir_palavra_inteira(texto, "mez", "mês");
    substituir_inicio_palavra(texto, "mezes", "meses");       substituir_inicio_palavra(texto, "tres", "três");
    substituir_inicio_palavra(texto, "Tres", "Três");         substituir_inicio_palavra(texto, "paiz", "país");
    substituir_inicio_palavra(texto, "paizes", "países");     substituir_inicio_palavra(texto, "Satanaz", "Satanás");
    substituir_inicio_palavra(texto, "anneis", "anéis");      substituir_inicio_palavra(texto, "anzoes", "anzóis");
    substituir_inicio_palavra(texto, "lençoes", "lençóis");   substituir_inicio_palavra(texto, "crueis", "cruéis");
    substituir_inicio_palavra(texto, "infieis", "infiéis");   substituir_inicio_palavra(texto, "fieis", "fiéis");
    substituir_inicio_palavra(texto, "chapeus", "chapéus");   substituir_inicio_palavra(texto, "arpéos", "arpéus");
    substituir_inicio_palavra(texto, "parabens", "parabéns"); substituir_inicio_palavra(texto, "refens", "reféns");
    substituir_inicio_palavra(texto, "armazens", "armazéns");

    // Vocabulário com acentuação moderna
    substituir_inicio_palavra(texto, "principio", "princípio");   substituir_inicio_palavra(texto, "Principio", "Princípio");
    substituir_inicio_palavra(texto, "Espirito", "Espírito");     substituir_inicio_palavra(texto, "espirito", "espírito");
    substituir_inicio_palavra(texto, "agua", "água");             substituir_inicio_palavra(texto, "Agua", "Água");
    substituir_inicio_palavra(texto, "arvore", "árvore");         substituir_inicio_palavra(texto, "Arvore", "Árvore");
    substituir_inicio_palavra(texto, "especie", "espécie");       substituir_inicio_palavra(texto, "reptil", "réptil");
    substituir_inicio_palavra(texto, "reptis", "répteis");        substituir_inicio_palavra(texto, "femea", "fêmea");
    substituir_inicio_palavra(texto, "gemeos", "gêmeos");         substituir_inicio_palavra(texto, "frutifera", "frutífera");
    substituir_inicio_palavra(texto, "oleo", "óleo");             substituir_inicio_palavra(texto, "calix", "cálice");
    substituir_inicio_palavra(texto, "gravida", "grávida");       substituir_inicio_palavra(texto, "gravido", "grávido");
    substituir_inicio_palavra(texto, "unigenito", "unigênito");   substituir_inicio_palavra(texto, "Unigenito", "Unigênito");
    substituir_inicio_palavra(texto, "folego", "fôlego");         substituir_inicio_palavra(texto, "Folego", "Fôlego");
    substituir_inicio_palavra(texto, "bdelio", "bdélio");         substituir_inicio_palavra(texto, "carcere", "cárcere");
    substituir_inicio_palavra(texto, "jubilo", "júbilo");         substituir_inicio_palavra(texto, "Jubilo", "Júbilo");
    substituir_inicio_palavra(texto, "licito", "lícito");         substituir_inicio_palavra(texto, "ilicito", "ilícito");
    substituir_inicio_palavra(texto, "unanime", "unânime");       substituir_inicio_palavra(texto, "Unanime", "Unânime");
    substituir_inicio_palavra(texto, "incomod", "incômod");       substituir_inicio_palavra(texto, "ancora", "âncora");
    substituir_inicio_palavra(texto, "imovel", "imóvel");         substituir_inicio_palavra(texto, "imoveis", "imóveis");
    substituir_inicio_palavra(texto, "mutua", "mútua");           substituir_inicio_palavra(texto, "mutuo", "mútuo");
    substituir_inicio_palavra(texto, "quadrupede", "quadrúpede"); substituir_inicio_palavra(texto, "imundicia", "imundícia");
    substituir_inicio_palavra(texto, "malicia", "malícia");       substituir_inicio_palavra(texto, "Malicia", "Malícia");
    substituir_inicio_palavra(texto, "seculo", "século");         substituir_inicio_palavra(texto, "Seculo", "Século");
    substituir_inicio_palavra(texto, "incredul", "incrédul");     substituir_inicio_palavra(texto, "Incredul", "Incrédul");
    substituir_inicio_palavra(texto, "proselit", "prosélit");     substituir_inicio_palavra(texto, "arabes", "árabes");
    substituir_inicio_palavra(texto, "arido", "árido");           substituir_inicio_palavra(texto, "arida", "árida");
    substituir_inicio_palavra(texto, "Amen", "Amém");             substituir_inicio_palavra(texto, "amen", "amém");
    substituir_inicio_palavra(texto, "alguem", "alguém");         substituir_inicio_palavra(texto, "Alguem", "Alguém");
    substituir_inicio_palavra(texto, "ninguem", "ninguém");       substituir_inicio_palavra(texto, "Ninguem", "Ninguém");
    substituir_inicio_palavra(texto, "tambem", "também");         substituir_inicio_palavra(texto, "Tambem", "Também");
    substituir_inicio_palavra(texto, "decima", "décima");         substituir_inicio_palavra(texto, "decimo", "décimo");
    substituir_inicio_palavra(texto, "discipulo", "discípulo");   substituir_inicio_palavra(texto, "Discipulo", "Discípulo");
    substituir_inicio_palavra(texto, "discipula", "discípula");
    substituir_inicio_palavra(texto, "judeo", "judeu");           substituir_inicio_palavra(texto, "Judeo", "Judeu");
    substituir_inicio_palavra(texto, "galileo", "galileu");       substituir_inicio_palavra(texto, "Galileo", "Galileu");
    substituir_inicio_palavra(texto, "fariseo", "fariseu");       substituir_inicio_palavra(texto, "Fariseo", "Fariseu");
    substituir_inicio_palavra(texto, "sadduceo", "saduceu");
    substituir_inicio_palavra(texto, "gloria", "glória");         substituir_inicio_palavra(texto, "Gloria", "Glória");
    substituir_inicio_palavra(texto, "misericordia", "misericórdia");substituir_inicio_palavra(texto, "Misericordia", "Misericórdia");
    substituir_inicio_palavra(texto, "juizo", "juízo");           substituir_inicio_palavra(texto, "paraiso", "paraíso");
    substituir_inicio_palavra(texto, "Paraiso", "Paraíso");       substituir_inicio_palavra(texto, "saude", "saúde");
    substituir_inicio_palavra(texto, "saudes", "saúdes");         substituir_inicio_palavra(texto, "saudo", "saúdo");
    substituir_inicio_palavra(texto, "saudam", "saúdam");         substituir_inicio_palavra(texto, "Saudam", "Saúdam");
    substituir_inicio_palavra(texto, "viuva", "viúva");           substituir_inicio_palavra(texto, "Viuva", "Viúva");
    substituir_inicio_palavra(texto, "ruina", "ruína");           substituir_inicio_palavra(texto, "raizes", "raízes");
    substituir_inicio_palavra(texto, "faisca", "faísca");         substituir_inicio_palavra(texto, "miuda", "miúda");
    substituir_inicio_palavra(texto, "miudo", "miúdo");           substituir_inicio_palavra(texto, "moido", "moído");
    substituir_inicio_palavra(texto, "roida", "roída");           substituir_inicio_palavra(texto, "saida", "saída");
    substituir_inicio_palavra(texto, "saido", "saído");           substituir_inicio_palavra(texto, "caida", "caída");
    substituir_inicio_palavra(texto, "caido", "caído");           substituir_inicio_palavra(texto, "destruida", "destruída");
    substituir_inicio_palavra(texto, "destruido", "destruído");   substituir_inicio_palavra(texto, "possuida", "possuída");
    substituir_inicio_palavra(texto, "possuido", "possuído");     substituir_inicio_palavra(texto, "restituida", "restituída");
    substituir_inicio_palavra(texto, "restituido", "restituído"); substituir_inicio_palavra(texto, "instruida", "instruída");
    substituir_inicio_palavra(texto, "instruido", "instruído");   substituir_inicio_palavra(texto, "construido", "construído");
    substituir_inicio_palavra(texto, "instituido", "instituído"); substituir_inicio_palavra(texto, "constituido", "constituído");
    substituir_inicio_palavra(texto, "concluido", "concluído");   substituir_inicio_palavra(texto, "excluido", "excluído");
    substituir_inicio_palavra(texto, "diminuido", "diminuído");   substituir_inicio_palavra(texto, "attribue", "atribui");
    substituir_inicio_palavra(texto, "constitue", "constitui");   substituir_inicio_palavra(texto, "diminue", "diminui");
    substituir_inicio_palavra(texto, "exclue", "exclui");         substituir_inicio_palavra(texto, "instrue", "instrui");
    substituir_inicio_palavra(texto, "possue", "possui");         substituir_inicio_palavra(texto, "restitue", "restitui");
    substituir_inicio_palavra(texto, "retribue", "retribui");     substituir_inicio_palavra(texto, "destroe", "destrói");
    substituir_inicio_palavra(texto, "Destroe", "Destrói");       substituir_inicio_palavra(texto, "obstroe", "obstrói");
    substituir_inicio_palavra(texto, "orfão", "órfão");           substituir_inicio_palavra(texto, "orphão", "órfão");
    substituir_inicio_palavra(texto, "Orphãos", "Órfãos");       substituir_inicio_palavra(texto, "orgão", "órgão");
    substituir_inicio_palavra(texto, "apostolo", "apóstolo");     substituir_inicio_palavra(texto, "diacono", "diácono");
    substituir_inicio_palavra(texto, "exercito", "exército");     substituir_inicio_palavra(texto, "Exercito", "Exército");
    substituir_inicio_palavra(texto, "sacrificio", "sacrifício"); substituir_inicio_palavra(texto, "Sacrificio", "Sacrifício");
    substituir_inicio_palavra(texto, "santuario", "santuário");   substituir_inicio_palavra(texto, "Santuario", "Santuário");
    substituir_inicio_palavra(texto, "tabernaculo", "tabernáculo");
    substituir_inicio_palavra(texto, "sinhedrio", "sinédrio");
    substituir_inicio_palavra(texto, "adulterio", "adultério");   substituir_inicio_palavra(texto, "Adulterio", "Adultério");
    substituir_inicio_palavra(texto, "homicidio", "homicídio");   substituir_inicio_palavra(texto, "fornicario", "fornicário");
    substituir_inicio_palavra(texto, "idolo", "ídolo");           substituir_inicio_palavra(texto, "idolatras", "idólatras");
    substituir_inicio_palavra(texto, "demonio", "demônio");       substituir_inicio_palavra(texto, "misterio", "mistério");
    substituir_inicio_palavra(texto, "Misterio", "Mistério");     substituir_inicio_palavra(texto, "ministerio", "ministério");
    substituir_inicio_palavra(texto, "epistola", "epístola");     substituir_inicio_palavra(texto, "cantico", "cântico");
    substituir_inicio_palavra(texto, "Cantico", "Cântico");       substituir_inicio_palavra(texto, "proverbio", "provérbio");
    substituir_inicio_palavra(texto, "Proverbio", "Provérbio");   substituir_inicio_palavra(texto, "sabio", "sábio");
    substituir_inicio_palavra(texto, "nescio", "néscio");         substituir_inicio_palavra(texto, "Nescio", "Néscio");
    substituir_inicio_palavra(texto, "impio", "ímpio");           substituir_inicio_palavra(texto, "iniquo", "iníquo");
    substituir_inicio_palavra(texto, "principe", "príncipe");     substituir_inicio_palavra(texto, "Principe", "Príncipe");
    substituir_inicio_palavra(texto, "palacio", "palácio");       substituir_inicio_palavra(texto, "edificio", "edifício");
    substituir_inicio_palavra(texto, "atrio", "átrio");           substituir_inicio_palavra(texto, "vestibulo", "vestíbulo");
    substituir_inicio_palavra(texto, "pinaculo", "pináculo");     substituir_inicio_palavra(texto, "portico", "pórtico");
    substituir_inicio_palavra(texto, "marmore", "mármore");       substituir_inicio_palavra(texto, "fabrica", "fábrica");
    substituir_inicio_palavra(texto, "lampada", "lâmpada");       substituir_inicio_palavra(texto, "perola", "pérola");
    substituir_inicio_palavra(texto, "covados", "côvados");       substituir_inicio_palavra(texto, "tunica", "túnica");
    substituir_inicio_palavra(texto, "sandalo", "sândalo");       substituir_inicio_palavra(texto, "ambar", "âmbar");
    substituir_inicio_palavra(texto, "antimonio", "antimônio");   substituir_inicio_palavra(texto, "onix", "ônix");
    substituir_inicio_palavra(texto, "Onix", "Ônix");             substituir_inicio_palavra(texto, "topazio", "topázio");
    substituir_inicio_palavra(texto, "sardio", "sárdio");         substituir_inicio_palavra(texto, "sardonia", "sardônia");
    substituir_inicio_palavra(texto, "sardonica", "sardônica");   substituir_inicio_palavra(texto, "crisolito", "crisólito");
    substituir_inicio_palavra(texto, "crisopraso", "crisópraso"); substituir_inicio_palavra(texto, "Omega", "Ômega");
    substituir_inicio_palavra(texto, "aguia", "águia");           substituir_inicio_palavra(texto, "passaro", "pássaro");
    substituir_inicio_palavra(texto, "vibora", "víbora");         substituir_inicio_palavra(texto, "aspide", "áspide");
    substituir_inicio_palavra(texto, "bufalo", "búfalo");         substituir_inicio_palavra(texto, "antilope", "antílope");
    substituir_inicio_palavra(texto, "dromedario", "dromedário"); substituir_inicio_palavra(texto, "unicornio", "unicórnio");
    substituir_inicio_palavra(texto, "lirio", "lírio");           substituir_inicio_palavra(texto, "aloes", "aloés");
    substituir_inicio_palavra(texto, "alamo", "álamo");           substituir_inicio_palavra(texto, "amendoas", "amêndoas");
    substituir_inicio_palavra(texto, "sicomoro", "sicômoro");     substituir_inicio_palavra(texto, "terebintho", "terebinto");
    substituir_inicio_palavra(texto, "hipocrita", "hipócrita");   substituir_inicio_palavra(texto, "Hipocrita", "Hipócrita");
    substituir_inicio_palavra(texto, "blasfemia", "blasfêmia");   substituir_inicio_palavra(texto, "perfidia", "perfídia");
    substituir_inicio_palavra(texto, "perfido", "pérfido");       substituir_inicio_palavra(texto, "barbaro", "bárbaro");
    substituir_inicio_palavra(texto, "bebado", "bêbado");         substituir_inicio_palavra(texto, "bebedo", "bêbedo");
    substituir_inicio_palavra(texto, "ebrios", "ébrios");         substituir_inicio_palavra(texto, "lunatico", "lunático");
    substituir_inicio_palavra(texto, "hidropico", "hidrópico");   substituir_inicio_palavra(texto, "pustula", "pústula");
    substituir_inicio_palavra(texto, "cadaver", "cadáver");       substituir_inicio_palavra(texto, "feretro", "féretro");
    substituir_inicio_palavra(texto, "relogio", "relógio");       substituir_inicio_palavra(texto, "salario", "salário");
    substituir_inicio_palavra(texto, "denario", "denário");       substituir_inicio_palavra(texto, "drachma", "dracma");
    substituir_inicio_palavra(texto, "didrachma", "didracma");    substituir_inicio_palavra(texto, "obolos", "óbolos");
    substituir_inicio_palavra(texto, "státer", "estáter");        substituir_inicio_palavra(texto, "adversario", "adversário");
    substituir_inicio_palavra(texto, "mercenario", "mercenário"); substituir_inicio_palavra(texto, "operario", "operário");
    substituir_inicio_palavra(texto, "boticario", "boticário");   substituir_inicio_palavra(texto, "secretario", "secretário");
    substituir_inicio_palavra(texto, "usurario", "usurário");     substituir_inicio_palavra(texto, "voluntario", "voluntário");
    substituir_inicio_palavra(texto, "tributario", "tributário"); substituir_inicio_palavra(texto, "hereditari", "hereditári");
    substituir_inicio_palavra(texto, "necessari", "necessári");   substituir_inicio_palavra(texto, "Necessari", "Necessári");
    substituir_inicio_palavra(texto, "ordinaria", "ordinária");   substituir_inicio_palavra(texto, "extraordinaria", "extraordinária");
    substituir_inicio_palavra(texto, "transitori", "transitóri"); substituir_inicio_palavra(texto, "momentanea", "momentânea");
    substituir_inicio_palavra(texto, "corporea", "corpórea");     substituir_inicio_palavra(texto, "idoneo", "idôneo");
    substituir_inicio_palavra(texto, "aspero", "áspero");         substituir_inicio_palavra(texto, "candido", "cândido");
    substituir_inicio_palavra(texto, "timido", "tímido");         substituir_inicio_palavra(texto, "sordido", "sórdido");
    substituir_inicio_palavra(texto, "sobrio", "sóbrio");         substituir_inicio_palavra(texto, "proprio", "próprio");
    substituir_inicio_palavra(texto, "propria", "própria");       substituir_inicio_palavra(texto, "proximo", "próximo");
    substituir_inicio_palavra(texto, "proxima", "próxima");       substituir_inicio_palavra(texto, "insipido", "insípido");
    substituir_inicio_palavra(texto, "ultimo", "último");         substituir_inicio_palavra(texto, "minimo", "mínimo");
    substituir_inicio_palavra(texto, "minimas", "mínimas");       substituir_inicio_palavra(texto, "intimo", "íntimo");
    substituir_inicio_palavra(texto, "continuo", "contínuo");     substituir_inicio_palavra(texto, "perpetuo", "perpétuo");
    substituir_inicio_palavra(texto, "contrario", "contrário");   substituir_inicio_palavra(texto, "vario", "vário");
    substituir_inicio_palavra(texto, "solitari", "solitári");     substituir_inicio_palavra(texto, "maritima", "marítima");
    substituir_inicio_palavra(texto, "domestico", "doméstico");   substituir_inicio_palavra(texto, "rustico", "rústico");
    substituir_inicio_palavra(texto, "magico", "mágico");         substituir_inicio_palavra(texto, "magnifico", "magnífico");
    substituir_inicio_palavra(texto, "pessimo", "péssimo");       substituir_inicio_palavra(texto, "atonito", "atônito");
    substituir_inicio_palavra(texto, "longanimo", "longânimo");   substituir_inicio_palavra(texto, "solicito", "solícito");
    substituir_inicio_palavra(texto, "temerario", "temerário");   substituir_inicio_palavra(texto, "propinquo", "propínquo");
    substituir_inicio_palavra(texto, "setima", "sétima");         substituir_inicio_palavra(texto, "setimo", "sétimo");
    substituir_inicio_palavra(texto, "undecim", "undécim");       substituir_inicio_palavra(texto, "duodecim", "duodécim");
    substituir_inicio_palavra(texto, "vigesim", "vigésim");       substituir_inicio_palavra(texto, "trigesim", "trigésim");
    substituir_inicio_palavra(texto, "quadragesim", "quadragésim");substituir_inicio_palavra(texto, "quinquagesim", "quinquagésim");
    substituir_inicio_palavra(texto, "lingua", "língua");         substituir_inicio_palavra(texto, "mingua", "míngua");
    substituir_inicio_palavra(texto, "argúe", "argui");           substituir_inicio_palavra(texto, "redargue", "redargui");
    substituir_inicio_palavra(texto, "beneficio", "benefício");   substituir_inicio_palavra(texto, "maleficio", "malefício");
    substituir_inicio_palavra(texto, "exercicio", "exercício");   substituir_inicio_palavra(texto, "officio", "ofício");
    substituir_inicio_palavra(texto, "desperdicio", "desperdício");substituir_inicio_palavra(texto, "precipicio", "precipício");
    substituir_inicio_palavra(texto, "prejuizo", "prejuízo");     substituir_inicio_palavra(texto, "prepucio", "prepúcio");
    substituir_inicio_palavra(texto, "negocio", "negócio");       substituir_inicio_palavra(texto, "sacerdocio", "sacerdócio");
    substituir_inicio_palavra(texto, "sacrilegio", "sacrilégio"); substituir_inicio_palavra(texto, "imperio", "império");
    substituir_inicio_palavra(texto, "dominio", "domínio");       substituir_inicio_palavra(texto, "testiculo", "testículo");
    substituir_inicio_palavra(texto, "musculo", "músculo");       substituir_inicio_palavra(texto, "musico", "músico");
    substituir_inicio_palavra(texto, "cimbalo", "címbalo");       substituir_inicio_palavra(texto, "salterio", "saltério");
    substituir_inicio_palavra(texto, "pulpito", "púlpito");       substituir_inicio_palavra(texto, "auditorio", "auditório");
    substituir_inicio_palavra(texto, "concilio", "concílio");     substituir_inicio_palavra(texto, "pretorio", "pretório");
    substituir_inicio_palavra(texto, "territorio", "território"); substituir_inicio_palavra(texto, "provincia", "província");
    substituir_inicio_palavra(texto, "planicie", "planície");     substituir_inicio_palavra(texto, "superficie", "superfície");
    substituir_inicio_palavra(texto, "vespera", "véspera");       substituir_inicio_palavra(texto, "vigilia", "vigília");
    substituir_inicio_palavra(texto, "memoria", "memória");       substituir_inicio_palavra(texto, "familia", "família");
    substituir_inicio_palavra(texto, "patria", "pátria");         substituir_inicio_palavra(texto, "materia", "matéria");
    substituir_inicio_palavra(texto, "miseria", "miséria");       substituir_inicio_palavra(texto, "lagrima", "lágrima");
    substituir_inicio_palavra(texto, "reliquia", "relíquia");     substituir_inicio_palavra(texto, "remedio", "remédio");
    substituir_inicio_palavra(texto, "refrigerio", "refrigério"); substituir_inicio_palavra(texto, "oprobrio", "opróbrio");
    substituir_inicio_palavra(texto, "vituperio", "vitupério");   substituir_inicio_palavra(texto, "escarnio", "escárnio");
    substituir_inicio_palavra(texto, "escarneo", "escárnio");     substituir_inicio_palavra(texto, "tedio", "tédio");
    substituir_inicio_palavra(texto, "odio", "ódio");             substituir_inicio_palavra(texto, "incendio", "incêndio");
    substituir_inicio_palavra(texto, "naufragio", "naufrágio");   substituir_inicio_palavra(texto, "prodigio", "prodígio");
    substituir_inicio_palavra(texto, "designio", "desígnio");     substituir_inicio_palavra(texto, "equilibrio", "equilíbrio");
    substituir_inicio_palavra(texto, "comercio", "comércio");     substituir_inicio_palavra(texto, "comodo", "cômodo");
    substituir_inicio_palavra(texto, "colera", "cólera");         substituir_inicio_palavra(texto, "colerico", "colérico");
    substituir_inicio_palavra(texto, "angulo", "ângulo");         substituir_inicio_palavra(texto, "animo", "ânimo");
    substituir_inicio_palavra(texto, "camara", "câmara");         substituir_inicio_palavra(texto, "cantaro", "cântaro");
    substituir_inicio_palavra(texto, "genero", "gênero");         substituir_inicio_palavra(texto, "craneo", "crânio");
    substituir_inicio_palavra(texto, "crepusculo", "crepúsculo"); substituir_inicio_palavra(texto, "espetaculo", "espetáculo");
    substituir_inicio_palavra(texto, "estadio", "estádio");       substituir_inicio_palavra(texto, "esteril", "estéril");
    substituir_inicio_palavra(texto, "facil", "fácil");           substituir_inicio_palavra(texto, "fertil", "fértil");
    substituir_inicio_palavra(texto, "fragil", "frágil");         substituir_inicio_palavra(texto, "habil", "hábil");
    substituir_inicio_palavra(texto, "util", "útil");             substituir_inicio_palavra(texto, "Util", "Útil");
    substituir_inicio_palavra(texto, "debil", "débil");           substituir_inicio_palavra(texto, "nivel", "nível");
    substituir_inicio_palavra(texto, "vacuo", "vácuo");           substituir_inicio_palavra(texto, "residuo", "resíduo");
    substituir_inicio_palavra(texto, "vomito", "vômito");         substituir_inicio_palavra(texto, "nadegas", "nádegas");
    substituir_inicio_palavra(texto, "palpebras", "pálpebras");   substituir_inicio_palavra(texto, "pateo", "pátio");
    substituir_inicio_palavra(texto, "alviçaras", "alvíssaras");  substituir_inicio_palavra(texto, "arruido", "arruído");
    substituir_inicio_palavra(texto, "arroido", "arruído");       substituir_inicio_palavra(texto, "vaevens", "vaivéns");
    substituir_inicio_palavra(texto, "vergonteas", "vergônteas"); substituir_inicio_palavra(texto, "violencia", "violência");
    substituir_inicio_palavra(texto, "Violencia", "Violência");   substituir_inicio_palavra(texto, "modestia", "modéstia");
    substituir_inicio_palavra(texto, "molestia", "moléstia");     substituir_inicio_palavra(texto, "estulticia", "estultícia");
    substituir_inicio_palavra(texto, "infamia", "infâmia");       substituir_inicio_palavra(texto, "concordia", "concórdia");
    substituir_inicio_palavra(texto, "primicia", "primícia");     substituir_inicio_palavra(texto, "primogenit", "primogênit");
    substituir_inicio_palavra(texto, "proposito", "propósito");   substituir_inicio_palavra(texto, "beneplacito", "beneplácito");
    substituir_inicio_palavra(texto, "emprestimo", "empréstimo"); substituir_inicio_palavra(texto, "artifice", "artífice");
    substituir_inicio_palavra(texto, "astrologo", "astrólogo");   substituir_inicio_palavra(texto, "dormitorio", "dormitório");
    substituir_inicio_palavra(texto, "incensario", "incensário"); substituir_inicio_palavra(texto, "luminaria", "luminária");
    substituir_inicio_palavra(texto, "propiciatorio", "propiciatório");
    substituir_inicio_palavra(texto, "decrepito", "decrépito");   substituir_inicio_palavra(texto, "efigie", "efígie");
    substituir_inicio_palavra(texto, "emissario", "emissário");   substituir_inicio_palavra(texto, "lapidario", "lapidário");
    substituir_inicio_palavra(texto, "natalicio", "natalício");   substituir_inicio_palavra(texto, "neofito", "neófito");
    substituir_inicio_palavra(texto, "reprobo", "réprobo");       substituir_inicio_palavra(texto, "satiro", "sátiro");
    substituir_inicio_palavra(texto, "serodia", "serôdia");       substituir_inicio_palavra(texto, "similes", "símiles");
    substituir_inicio_palavra(texto, "porfiro", "pórfiro");       substituir_inicio_palavra(texto, "setestrelo", "Sete-Estrelo");
    substituir_inicio_palavra(texto, "Setestrelo", "Sete-Estrelo");
    substituir_inicio_palavra(texto, "Todo-poderoso", "Todo-Poderoso");
    substituir_inicio_palavra(texto, "refugio", "refúgio");
    substituir_inicio_palavra(texto, "angustia", "angústia");
    substituir_inicio_palavra(texto, "angustias", "angústias");
    substituir_texto(texto, "-ha ", "-á ");                       substituir_texto(texto, "-ha.", "-á.");
    substituir_texto(texto, "-ha,", "-á,");

    // 10. Padronização de nomes próprios bíblicos e geográficos (pt-BR)
    substituir_inicio_palavra(texto, "Abrahão", "Abraão");
    substituir_inicio_palavra(texto, "Sarah", "Sara");
    substituir_inicio_palavra(texto, "David", "Davi");
    substituir_inicio_palavra(texto, "Jacob", "Jacó");
    substituir_inicio_palavra(texto, "Isaac", "Isaque");
    substituir_inicio_palavra(texto, "Rachab", "Raabe");
    substituir_inicio_palavra(texto, "Rahab", "Raabe");
    substituir_inicio_palavra(texto, "Rut", "Rute");
    substituir_inicio_palavra(texto, "Obed", "Obede");
    substituir_inicio_palavra(texto, "Josafat", "Josafá");
    substituir_inicio_palavra(texto, "Achaz", "Acaz");
    substituir_inicio_palavra(texto, "Amon", "Amom");
    substituir_inicio_palavra(texto, "Jechonias", "Jeconias");
    substituir_inicio_palavra(texto, "Eliakim", "Eliaquim");
    substituir_inicio_palavra(texto, "Sadoc", "Sadoque");
    substituir_inicio_palavra(texto, "Achim", "Aquim");
    substituir_inicio_palavra(texto, "Eliud", "Eliúde");
    substituir_inicio_palavra(texto, "Matan", "Matã");
    substituir_inicio_palavra(texto, "Emmanuel", "Emanuel");
    substituir_inicio_palavra(texto, "EMMANUEL", "EMANUEL");
    substituir_inicio_palavra(texto, "Gideon", "Gideão");
    substituir_inicio_palavra(texto, "Jefte", "Jefté");
    substituir_inicio_palavra(texto, "Zacharias", "Zacarias");
    substituir_inicio_palavra(texto, "Poncio", "Pôncio");
    substituir_inicio_palavra(texto, "Iturea", "Itureia");
    substituir_inicio_palavra(texto, "Abilinia", "Abilene");
    substituir_inicio_palavra(texto, "Eden", "Éden");
    substituir_inicio_palavra(texto, "Aristarcho", "Aristarco");
    substituir_inicio_palavra(texto, "macedonio", "macedônio");
    substituir_inicio_palavra(texto, "Fenix", "Fênix");
    substituir_inicio_palavra(texto, "Adriatico", "Adriático");
    substituir_inicio_palavra(texto, "Moises", "Moisés");
    substituir_inicio_palavra(texto, "Jehovah", "Jeová");         substituir_inicio_palavra(texto, "JEHOVAH", "JEOVÁ");
    substituir_inicio_palavra(texto, "Egipto", "Egito");
    substituir_inicio_palavra(texto, "egipci", "egípci");         substituir_inicio_palavra(texto, "Egipci", "Egípci");
    substituir_inicio_palavra(texto, "Jerusalem", "Jerusalém");
    substituir_inicio_palavra(texto, "Betlehem", "Belém");        substituir_inicio_palavra(texto, "Bethlehem", "Belém");
    substituir_inicio_palavra(texto, "Nazaret", "Nazaré");
    substituir_inicio_palavra(texto, "Galilea", "Galileia");      substituir_inicio_palavra(texto, "Galiléa", "Galileia");
    substituir_inicio_palavra(texto, "Judea", "Judeia");          substituir_inicio_palavra(texto, "Judéa", "Judeia");
    substituir_inicio_palavra(texto, "Laodicea", "Laodiceia");    substituir_inicio_palavra(texto, "Laodicéa", "Laodiceia");
    substituir_inicio_palavra(texto, "Cesarea", "Cesareia");      substituir_inicio_palavra(texto, "Berea", "Bereia");
    substituir_inicio_palavra(texto, "Betania", "Betânia");
    substituir_inicio_palavra(texto, "Getsemane", "Getsêmani");   substituir_inicio_palavra(texto, "Golgota", "Gólgota");
    substituir_inicio_palavra(texto, "Timoteo", "Timóteo");
    substituir_inicio_palavra(texto, "Bartolomeo", "Bartolomeu");
    substituir_inicio_palavra(texto, "Teofilo", "Teófilo");
    substituir_inicio_palavra(texto, "Enoch", "Enoque");
    substituir_inicio_palavra(texto, "Henoch", "Enoque");
    substituir_inicio_palavra(texto, "Lamech", "Lameque");
    substituir_inicio_palavra(texto, "Melquisedec", "Melquisedeque");
    substituir_inicio_palavra(texto, "Abimelech", "Abimeleque");
    substituir_inicio_palavra(texto, "Elimelech", "Elimeleque");
    substituir_inicio_palavra(texto, "Baruch", "Baruque");
    substituir_inicio_palavra(texto, "Eliseo", "Eliseu");
    substituir_inicio_palavra(texto, "Zaqueo", "Zaqueu");
    substituir_inicio_palavra(texto, "Mardoqueo", "Mardoqueu");
    substituir_inicio_palavra(texto, "Isaias", "Isaías");
    substituir_inicio_palavra(texto, "Oseas", "Oseias");
    substituir_inicio_palavra(texto, "Miqueas", "Miqueias");
    substituir_inicio_palavra(texto, "Ageo", "Ageu");
    substituir_inicio_palavra(texto, "corintios", "coríntios");
    substituir_inicio_palavra(texto, "Tessalonica", "Tessalônica");
    substituir_inicio_palavra(texto, "Iconio", "Icônio");
    substituir_inicio_palavra(texto, "Tiquico", "Tíquico");
    substituir_inicio_palavra(texto, "Onesiforo", "Onesíforo");
    substituir_inicio_palavra(texto, "Diotrefes", "Diótrefes");
    substituir_inicio_palavra(texto, "Demetrio", "Demétrio");
    substituir_inicio_palavra(texto, "Cornelio", "Cornélio");
    substituir_inicio_palavra(texto, "Sergio", "Sérgio");
    substituir_inicio_palavra(texto, "Claudio", "Cláudio");
    substituir_inicio_palavra(texto, "Julio", "Júlio");
    substituir_inicio_palavra(texto, "Felix", "Félix");
    substituir_inicio_palavra(texto, "Tiberio", "Tibério");
    substituir_inicio_palavra(texto, "Cesar", "César");
    substituir_inicio_palavra(texto, "Fenicia", "Fenícia");
    substituir_inicio_palavra(texto, "Frigia", "Frígia");
    substituir_inicio_palavra(texto, "Panfilia", "Panfília");
    substituir_inicio_palavra(texto, "Licia", "Lícia");
    substituir_inicio_palavra(texto, "Licaonia", "Licaônia");
    substituir_inicio_palavra(texto, "Misia", "Mísia");
    substituir_inicio_palavra(texto, "Bitinia", "Bitínia");
    substituir_inicio_palavra(texto, "Capadocia", "Capadócia");
    substituir_inicio_palavra(texto, "Cilicia", "Cilícia");
    substituir_inicio_palavra(texto, "Galacia", "Galácia");
    substituir_inicio_palavra(texto, "Macedonia", "Macedônia");
    substituir_inicio_palavra(texto, "Ilirico", "Ilírico");
    substituir_inicio_palavra(texto, "Italia", "Itália");
    substituir_inicio_palavra(texto, "ciprio", "cíprio");
    substituir_inicio_palavra(texto, "Libia", "Líbia");
    substituir_inicio_palavra(texto, "Etiopia", "Etiópia");       substituir_inicio_palavra(texto, "etiope", "etíope");
    substituir_inicio_palavra(texto, "Mesopotamia", "Mesopotâmia");
    substituir_inicio_palavra(texto, "Assiria", "Assíria");       substituir_inicio_palavra(texto, "assirios", "assírios");
    substituir_inicio_palavra(texto, "Babilonia", "Babilônia");   substituir_inicio_palavra(texto, "babilonios", "babilônios");
    substituir_inicio_palavra(texto, "Persia", "Pérsia");
    substituir_inicio_palavra(texto, "Grecia", "Grécia");
    substituir_inicio_palavra(texto, "Siquem", "Siquém");
    substituir_inicio_palavra(texto, "sidonios", "sidônios");     substituir_inicio_palavra(texto, "tirios", "tírios");
    substituir_inicio_palavra(texto, "Libano", "Líbano");
    substituir_inicio_palavra(texto, "Siao", "Sião");
    substituir_inicio_palavra(texto, "Moriah", "Moriá");
    substituir_inicio_palavra(texto, "hebreo", "hebreu");         substituir_inicio_palavra(texto, "Hebreo", "Hebreu");
    substituir_inicio_palavra(texto, "caldeo", "caldeu");         substituir_inicio_palavra(texto, "Caldeo", "Caldeu");
    substituir_inicio_palavra(texto, "cananeo", "cananeu");       substituir_inicio_palavra(texto, "Cananeo", "Cananeu");
    substituir_inicio_palavra(texto, "idumeo", "idumeu");         substituir_inicio_palavra(texto, "Idumeo", "Idumeu");
    substituir_inicio_palavra(texto, "amoreo", "amorreu");        substituir_inicio_palavra(texto, "Amoreo", "Amorreu");
    substituir_inicio_palavra(texto, "heteo", "heteu");           substituir_inicio_palavra(texto, "heveo", "heveu");
    substituir_inicio_palavra(texto, "Heveo", "Heveu");           substituir_inicio_palavra(texto, "jebuseo", "jebuseu");
    substituir_inicio_palavra(texto, "Jebuseo", "Jebuseu");       substituir_inicio_palavra(texto, "pereseo", "perizeu");
    substituir_inicio_palavra(texto, "fereseo", "perizeu");       substituir_inicio_palavra(texto, "Fereseus", "Perizeus");
    substituir_inicio_palavra(texto, "filisteo", "filisteu");     substituir_inicio_palavra(texto, "sabeo", "sabeu");
    substituir_inicio_palavra(texto, "sidoneo", "sidônio");

    // 11. Adaptação lexical e sintática para o leitor do Brasil (PT-BR)
    substituir_palavra_inteira(texto, "rapariga", "moça");
    substituir_palavra_inteira(texto, "Rapariga", "Moça");
    substituir_palavra_inteira(texto, "raparigas", "moças");
    substituir_palavra_inteira(texto, "Raparigas", "Moças");
    substituir_palavra_inteira(texto, "rapariguinha", "mocinha");
    substituir_palavra_inteira(texto, "Rapariguinha", "Mocinha");

    substituir_palavra_inteira(texto, "mancebo", "jovem");
    substituir_palavra_inteira(texto, "Mancebo", "Jovem");
    substituir_palavra_inteira(texto, "mancebos", "jovens");
    substituir_palavra_inteira(texto, "Mancebos", "Jovens");

    substituir_palavra_inteira(texto, "cachopa", "menina");
    substituir_palavra_inteira(texto, "Cachopa", "Menina");
    substituir_palavra_inteira(texto, "cachopas", "meninas");
    substituir_palavra_inteira(texto, "Cachopas", "Meninas");
    substituir_palavra_inteira(texto, "cachopo", "menino");
    substituir_palavra_inteira(texto, "Cachopo", "Menino");
    substituir_palavra_inteira(texto, "cachopos", "meninos");
    substituir_palavra_inteira(texto, "Cachopos", "Meninos");

    substituir_palavra_inteira(texto, "varões", "homens");
    substituir_palavra_inteira(texto, "Varões", "Homens");

    substituir_palavra_inteira(texto, "ósculo", "beijo");
    substituir_palavra_inteira(texto, "Ósculo", "Beijo");
    substituir_palavra_inteira(texto, "ósculos", "beijos");
    substituir_palavra_inteira(texto, "Ósculos", "Beijos");

    substituir_palavra_inteira(texto, "lume", "luz");
    substituir_palavra_inteira(texto, "Lume", "Luz");

    substituir_palavra_inteira(texto, "púcaro", "jarro");
    substituir_palavra_inteira(texto, "Púcaro", "Jarro");
    substituir_palavra_inteira(texto, "púcaros", "jarros");
    substituir_palavra_inteira(texto, "púcara", "jarra");
    substituir_palavra_inteira(texto, "Púcara", "Jarra");
    substituir_palavra_inteira(texto, "púcaras", "jarras");

    substituir_palavra_inteira(texto, "azorrague", "chicote");
    substituir_palavra_inteira(texto, "Azorrague", "Chicote");
    substituir_palavra_inteira(texto, "azorragues", "chicotes");
    substituir_palavra_inteira(texto, "alfaias", "utensílios");
    substituir_palavra_inteira(texto, "algibeira", "bolso");
    substituir_palavra_inteira(texto, "algibeiras", "bolsos");
    substituir_palavra_inteira(texto, "postigo", "janela");
    substituir_palavra_inteira(texto, "regato", "riacho");
    substituir_palavra_inteira(texto, "regatos", "riachos");
    substituir_palavra_inteira(texto, "verdugo", "carrasco");
    substituir_palavra_inteira(texto, "verdugos", "carrascos");

    substituir_palavra_inteira(texto, "Físico", "Médico");
    substituir_palavra_inteira(texto, "outrossim", "também");
    substituir_palavra_inteira(texto, "Outrossim", "Também");
    substituir_palavra_inteira(texto, "sobremaneira", "muito");
    substituir_palavra_inteira(texto, "Sobremaneira", "Muito");
    substituir_palavra_inteira(texto, "amiúde", "frequentemente");
    substituir_palavra_inteira(texto, "Amiúde", "Frequentemente");
    substituir_palavra_inteira(texto, "porventura", "acaso");
    substituir_palavra_inteira(texto, "Porventura", "Acaso");
    substituir_palavra_inteira(texto, "porquanto", "porque");
    substituir_palavra_inteira(texto, "Porquanto", "Porque");
    substituir_palavra_inteira(texto, "conquanto", "embora");
    substituir_palavra_inteira(texto, "Conquanto", "Embora");
    substituir_palavra_inteira(texto, "postoque", "embora");
    substituir_palavra_inteira(texto, "Postoque", "Embora");
    substituir_texto(texto, "de sorte que", "de modo que");
    substituir_texto(texto, "De sorte que", "De modo que");

// 12. Revisão complementar pós-varredura e leitura (Lote 1 - Nov/2026)
 // 12.1 Ditongos antigos (oi -> ou) e consoantes mudas (ct, pt)
 substituir_inicio_palavra(texto, "oiro", "ouro");
 substituir_inicio_palavra(texto, "Oiro", "Ouro");
 substituir_inicio_palavra(texto, "coiro", "couro");
 substituir_inicio_palavra(texto, "toiro", "touro");
 substituir_inicio_palavra(texto, "calaboiço", "calabouço");
 substituir_palavra_inteira(texto, "oiça", "ouça");
 substituir_palavra_inteira(texto, "oiçam", "ouçam");
 substituir_inicio_palavra(texto, "hemorrhoid", "hemorroid");

 substituir_inicio_palavra(texto, "recto", "reto");
 substituir_inicio_palavra(texto, "Recto", "Reto");
 substituir_inicio_palavra(texto, "recta", "reta");
 substituir_inicio_palavra(texto, "Recta", "Reta");
 substituir_inicio_palavra(texto, "esculptur", "escultur");
 substituir_inicio_palavra(texto, "auctor", "autor");
 /* "aspecto" e "expectação" ficam como estão: no PT-BR o "c" se mantém */
 substituir_inicio_palavra(texto, "excepto", "exceto");
 substituir_inicio_palavra(texto, "tecto", "teto");
 substituir_inicio_palavra(texto, "inspector", "inspetor");
 substituir_inicio_palavra(texto, "dictado", "ditado");
 substituir_inicio_palavra(texto, "espectaculo", "espetáculo");
 substituir_inicio_palavra(texto, "facto", "fato");
 substituir_inicio_palavra(texto, "aprompt", "apront");
 substituir_inicio_palavra(texto, "districto", "distrito");
 substituir_inicio_palavra(texto, "jactancia", "jactância");
 substituir_inicio_palavra(texto, "interdicto", "interdito");
 substituir_inicio_palavra(texto, "aqueducto", "aqueduto");
 substituir_inicio_palavra(texto, "estructur", "estrutur");
 substituir_inicio_palavra(texto, "indirect", "indiret");
 substituir_inicio_palavra(texto, "conductor", "condutor");

 // 12.2 Acentuação gráfica e vocabulário revisado
 substituir_inicio_palavra(texto, "talamo", "tálamo");
 substituir_inicio_palavra(texto, "heroe", "herói");
 substituir_inicio_palavra(texto, "simplices", "símplices");
 substituir_inicio_palavra(texto, "diluvio", "dilúvio");
 substituir_inicio_palavra(texto, "silencio", "silêncio");
 substituir_inicio_palavra(texto, "auxilio", "auxílio");
 substituir_inicio_palavra(texto, "notorio", "notório");
 substituir_inicio_palavra(texto, "psalterio", "saltério");
 substituir_inicio_palavra(texto, "artificio", "artifício");
 substituir_inicio_palavra(texto, "labio", "lábio");
 substituir_inicio_palavra(texto, "divorcio", "divórcio");
 substituir_inicio_palavra(texto, "premio", "prêmio");
 substituir_inicio_palavra(texto, "sanguinario", "sanguinário");
 substituir_inicio_palavra(texto, "repudio", "repúdio");
 substituir_inicio_palavra(texto, "cilicio", "cilício");
 substituir_inicio_palavra(texto, "arabio", "arábio");
 substituir_inicio_palavra(texto, "vicio", "vício");
 substituir_inicio_palavra(texto, "sitio", "sítio");
 substituir_inicio_palavra(texto, "habitavel", "habitável");
 substituir_inicio_palavra(texto, "historia", "história");
 substituir_inicio_palavra(texto, "ante-hontem", "anteontem");
 substituir_inicio_palavra(texto, "Hespanha", "Espanha");
 substituir_palavra_inteira(texto, "alumia", "ilumina");
 substituir_palavra_inteira(texto, "expurga-me", "limpa-me");
 substituir_palavra_inteira(texto, "assenhoreiem", "tomem posse");

 // 12.3 Nomes próprios e gentílicos bíblicos padronizados
 substituir_inicio_palavra(texto, "Zebedeo", "Zebedeu");
 substituir_inicio_palavra(texto, "amorrheo", "amorreu");
 substituir_inicio_palavra(texto, "nazireo", "nazireu");
 substituir_inicio_palavra(texto, "arameo", "arameu");
 substituir_inicio_palavra(texto, "keneo", "queneu");
 substituir_inicio_palavra(texto, "Keneo", "Queneu");
 substituir_inicio_palavra(texto, "kenezeo", "quenezeu");
 substituir_inicio_palavra(texto, "horeo", "horeu");    substituir_inicio_palavra(texto, "Horeo", "Horeu");    substituir_inicio_palavra(texto, "Heteo", "Heteu");
 substituir_inicio_palavra(texto, "Hebron", "Hebrom");
 substituir_inicio_palavra(texto, "Haman", "Hamã");
 substituir_inicio_palavra(texto, "Hesbon", "Hesbom");
 substituir_inicio_palavra(texto, "Hilkias", "Hilquias");
 substituir_inicio_palavra(texto, "Haran", "Harã");
 substituir_inicio_palavra(texto, "Horeb", "Horebe");
 substituir_inicio_palavra(texto, "Heman", "Hemã");
 substituir_inicio_palavra(texto, "Hezron", "Hezrom");
 substituir_inicio_palavra(texto, "Hanun", "Hanum");
 substituir_inicio_palavra(texto, "Habacuc", "Habacuque");

    /* 9. Acertos finais da revisão de 11/2026 (só grafia, sem trocar palavras) */
    substituir_texto(texto, "irreprehens", "irrepreens");
    substituir_palavra_inteira(texto, "Siria", "Síria");         substituir_palavra_inteira(texto, "siros", "sírios");
    substituir_palavra_inteira(texto, "Efeso", "Éfeso");         substituir_palavra_inteira(texto, "Tarsis", "Társis");
    substituir_palavra_inteira(texto, "aflicto", "aflito");      substituir_palavra_inteira(texto, "aflicta", "aflita");
    substituir_palavra_inteira(texto, "aflictos", "aflitos");    substituir_palavra_inteira(texto, "aflictas", "aflitas");
    substituir_palavra_inteira(texto, "mirrha", "mirra");
    substituir_palavra_inteira(texto, "oficio", "ofício");       substituir_palavra_inteira(texto, "oficios", "ofícios");
    substituir_palavra_inteira(texto, "paralitico", "paralítico"); substituir_palavra_inteira(texto, "paraliticos", "paralíticos");
    substituir_palavra_inteira(texto, "saduceos", "saduceus");
    substituir_palavra_inteira(texto, "aneis", "anéis");
    substituir_palavra_inteira(texto, "dificil", "difícil");     substituir_palavra_inteira(texto, "eficacia", "eficácia");
    substituir_palavra_inteira(texto, "geteo", "geteu");
    substituir_palavra_inteira(texto, "traír", "trair");         substituir_palavra_inteira(texto, "atraír", "atrair");
    substituir_palavra_inteira(texto, "saír", "sair");           substituir_palavra_inteira(texto, "caír", "cair");
    substituir_palavra_inteira(texto, "distraír", "distrair");
    /* saiu, caiu, traiu, saindo, atrairei: sem acento */
    substituir_texto(texto, "aíu", "aiu");                       substituir_texto(texto, "aíndo", "aindo");
    substituir_texto(texto, "aírei", "airei");                   substituir_texto(texto, "aíremos", "airemos");
    substituir_texto(texto, "aíria", "airia");
    /* Leitura de 25/11/2026 (Gn 6, Mt 2) */
    substituir_palavra_inteira(texto, "covado", "côvado");
    substituir_palavra_inteira(texto, "dadiva", "dádiva");       substituir_palavra_inteira(texto, "dadivas", "dádivas");
    substituir_texto(texto, "eis-que", "eis que");               substituir_texto(texto, "Eis-que", "Eis que");
    substituir_palavra_inteira(texto, "Judah", "Judá");
    /* Mesóclise antiga: far-lhe-has -> far-lhe-ás, dar-lhe-hemos -> dar-lhe-emos.
       Palavra inteira depois do hífen, para não tocar nomes como Maher-shalal-hash-baz. */
    {
        /* "-has" só depois de pronome (far-lhe-has -> far-lhe-ás); o "has" sozinho é "hás de" */
        static const char *pron_has[] = {"me", "te", "se", "nos", "vos", "lhe", "lhes", "o", "a", "os", "as",
                                         "mo", "ma", "mos", "mas", "to", "ta", "lho", "lha", "lhos", "lhas"};
        int total_pron = (int)(sizeof(pron_has) / sizeof(pron_has[0]));
        for (int p = 0; p < total_pron; p++) {
            char ant[16], nov[16];
            snprintf(ant, sizeof(ant), "-%s-has", pron_has[p]);
            snprintf(nov, sizeof(nov), "-%s-ás", pron_has[p]);
            substituir_texto(texto, ant, nov);
        }
        substituir_palavra_inteira(texto, "has", "hás");   /* "has de" -> "hás de" */
    }
    substituir_palavra_inteira(texto, "hemos", "emos");
    substituir_palavra_inteira(texto, "heis", "eis");            substituir_palavra_inteira(texto, "hia", "ia");
    substituir_palavra_inteira(texto, "hias", "ias");            substituir_palavra_inteira(texto, "hiam", "iam");
    {
        /* Pronome no meio: leval-o-ás -> levá-lo-ás, restituil-os-ás -> restituí-los-ás */
        static const char *pron[] = {"o", "a", "os", "as"};
        static const char *fim[] = {"ás", "eis", "emos", "ia", "ias", "iam"};
        static const char *rad_ant[] = {"uil-", "al-", "el-", "ôl-", "ol-", "il-"};
        static const char *rad_nov[] = {"uí-l", "á-l", "ê-l", "ô-l", "ô-l", "i-l"};
        for (int r = 0; r < 6; r++) {
            for (int p = 0; p < 4; p++) {
                for (int f = 0; f < 6; f++) {
                    char ant[32], nov[32];
                    snprintf(ant, sizeof(ant), "%s%s-%s", rad_ant[r], pron[p], fim[f]);
                    snprintf(nov, sizeof(nov), "%s%s-%s", rad_nov[r], pron[p], fim[f]);
                    substituir_texto(texto, ant, nov);
                }
            }
        }
    }

    /* Derivados de pôr: impozeram -> impuseram, propozera -> propusera */
    substituir_texto(texto, "poze", "puse");
    /* caluniar: o acento é só no substantivo "calúnia" */
    substituir_texto(texto, "calúniad", "caluniad");             substituir_texto(texto, "calúniam", "caluniam");
    substituir_texto(texto, "calúniando", "caluniando");         substituir_texto(texto, "calúnies", "calunies");
    substituir_palavra_inteira(texto, "quiseramos", "quiséramos");    substituir_palavra_inteira(texto, "fossemos", "fôssemos");    substituir_palavra_inteira(texto, "fosseis", "fôsseis");
    substituir_palavra_inteira(texto, "infíeis", "infiéis");     substituir_palavra_inteira(texto, "fíeis", "fiéis");
    /* -iar no presente não leva acento: confiamos, gloriamos, desvieis */
    substituir_palavra_inteira(texto, "confíamos", "confiamos"); substituir_palavra_inteira(texto, "gloríamos", "gloriamos");
    substituir_palavra_inteira(texto, "desvíeis", "desvieis");
    /* Pretérito forte leva acento agudo: tivésseis, tivéssemos */
    substituir_texto(texto, "tivêss", "tivéss");                 substituir_texto(texto, "Tivêss", "Tivéss");
    substituir_texto(texto, "estivêss", "estivéss");             substituir_texto(texto, "houvêss", "houvéss");
    substituir_texto(texto, "soubêss", "soubéss");               substituir_texto(texto, "quisêss", "quiséss");
    substituir_texto(texto, "fizêss", "fizéss");                 substituir_texto(texto, "pusêss", "puséss");
    substituir_texto(texto, "dissêss", "disséss");               substituir_texto(texto, "trouxêss", "trouxéss");
    substituir_texto(texto, "pudêss", "pudéss");                 substituir_texto(texto, "coubêss", "coubéss");


    /* 10. ZONA INVISÍVEL (varredura de 26/11/2026)
       Palavras que nenhuma regra acima trocava e que não existem no dicionário
       oficial pt-BR (VERO/LibreOffice). Cada par troca só a palavra inteira.
       Fonte: biblia-build/revisao/zona_todas.tsv + leitura no app. */
    {
        static const char *zona_invisivel[][2] = {
    /* Leitura no app (26/11/2026) */
    {"frecha", "flecha"}, {"Frecha", "Flecha"}, {"frechas", "flechas"}, {"Frechas", "Flechas"},
    {"frecheiro", "flecheiro"}, {"frecheiros", "flecheiros"}, {"Frecheiros", "Flecheiros"},
    {"frecharam", "flecharam"},
    {"dum", "de um"}, {"Dum", "De um"}, {"duma", "de uma"}, {"Duma", "De uma"},
    {"duns", "de uns"}, {"dumas", "de umas"},
    {"Pamfilia", "Panfília"}, {"Amfipolis", "Anfípolis"},
    {"numero", "número"}, {"Numero", "Número"}, {"numeros", "números"},
    /* Leitura no app: Gênesis 40-42 */
    {"Canaan", "Canaã"}, {"Manasseh", "Manassés"}, {"Porem", "Porém"},
    /* Zona invisível 2: palavras certas sozinhas, mas sem acento no sentido do versículo */
    {"numero", "número"}, {"Numero", "Número"},
    {"pacifico", "pacífico"}, {"Pacifico", "Pacífico"},
    {"estatua", "estátua"}, {"Estatua", "Estátua"},
    {"oraculo", "oráculo"}, {"Oraculo", "Oráculo"},
    {"figado", "fígado"}, {"Figado", "Fígado"},
    {"astucia", "astúcia"}, {"Astucia", "Astúcia"},
    {"impeto", "ímpeto"}, {"Impeto", "Ímpeto"},
    {"espadua", "espádua"}, {"Espadua", "Espádua"},
    {"ereis", "éreis"}, {"Ereis", "Éreis"},
    {"noticia", "notícia"}, {"Noticia", "Notícia"},
    {"divida", "dívida"}, {"Divida", "Dívida"},
    {"credito", "crédito"}, {"Credito", "Crédito"},
    {"copia", "cópia"}, {"Copia", "Cópia"},
    {"medico", "médico"}, {"Medico", "Médico"},
    {"copula", "cópula"}, {"Copula", "Cópula"},
    {"dizimo", "dízimo"}, {"Dizimo", "Dízimo"},
    {"deposito", "depósito"}, {"Deposito", "Depósito"},
    {"macula", "mácula"}, {"Macula", "Mácula"},
    {"osculo", "ósculo"}, {"Osculo", "Ósculo"},
    {"prospero", "próspero"}, {"Prospero", "Próspero"},
    {"balsamo", "bálsamo"}, {"Balsamo", "Bálsamo"},
    {"milicia", "milícia"}, {"Milicia", "Milícia"},
    {"ignominia", "ignomínia"}, {"Ignominia", "Ignomínia"},
    {"titulo", "título"}, {"Titulo", "Título"},
    {"estrepito", "estrépito"}, {"Estrepito", "Estrépito"},
    {"estomago", "estômago"}, {"Estomago", "Estômago"},
    {"abobada", "abóbada"}, {"Abobada", "Abóbada"},
    {"vinculo", "vínculo"}, {"Vinculo", "Vínculo"},
    {"magoa", "mágoa"}, {"Magoa", "Mágoa"},
    {"viveres", "víveres"}, {"Viveres", "Víveres"},
    {"adultero", "adúltero"}, {"Adultero", "Adúltero"},
    {"ciume", "ciúme"}, {"Ciume", "Ciúme"},
    {"alfandega", "alfândega"}, {"Alfandega", "Alfândega"},
    {"citara", "cítara"}, {"Citara", "Cítara"},
    {"alaude", "alaúde"}, {"Alaude", "Alaúde"},
    {"arratel", "arrátel"}, {"Arratel", "Arrátel"},
    {"solido", "sólido"}, {"Solido", "Sólido"},
    {"alivio", "alívio"}, {"Alivio", "Alívio"},
    {"hospede", "hóspede"}, {"Hospede", "Hóspede"},
    {"calamo", "cálamo"}, {"Calamo", "Cálamo"},
    {"regulo", "régulo"}, {"Regulo", "Régulo"},
    {"lamina", "lâmina"}, {"Lamina", "Lâmina"},
    {"aria", "ária"}, {"Aria", "Ária"},
    {"vangloria", "vanglória"}, {"Vangloria", "Vanglória"},
    {"propicio", "propício"}, {"Propicio", "Propício"},
    {"ulcera", "úlcera"}, {"Ulcera", "Úlcera"},
    {"matrimonio", "matrimônio"}, {"Matrimonio", "Matrimônio"},
    {"ludibrio", "ludíbrio"}, {"Ludibrio", "Ludíbrio"},
    {"cambio", "câmbio"}, {"Cambio", "Câmbio"},
    {"tremulo", "trêmulo"}, {"Tremulo", "Trêmulo"},
    {"valido", "válido"}, {"Valido", "Válido"},
    {"insignia", "insígnia"}, {"Insignia", "Insígnia"},
    {"legitimo", "legítimo"}, {"Legitimo", "Legítimo"},
    {"enxundia", "enxúndia"}, {"Enxundia", "Enxúndia"},
    {"concavo", "côncavo"}, {"Concavo", "Côncavo"},
    {"fabula", "fábula"}, {"Fabula", "Fábula"},
    {"litigio", "litígio"}, {"Litigio", "Litígio"},
    {"bedelio", "bedélio"}, {"Bedelio", "Bedélio"},
    {"trafico", "tráfico"}, {"Trafico", "Tráfico"},
    {"vilipendio", "vilipêndio"}, {"Vilipendio", "Vilipêndio"},
    {"absintio", "absíntio"}, {"Absintio", "Absíntio"},
    {"mascara", "máscara"}, {"Mascara", "Máscara"},
    {"arbitro", "árbitro"}, {"Arbitro", "Árbitro"},
    {"nevoa", "névoa"}, {"Nevoa", "Névoa"},
    {"indicio", "indício"}, {"Indicio", "Indício"},
    {"sairam", "saíram"},
    {"Sairam", "Saíram"},
    {"cairam", "caíram"},
    {"Cairam", "Caíram"},
    {"cherubins", "querubins"},
    {"Cherubins", "Querubins"},
    {"juizes", "juízes"},
    {"Juizes", "Juízes"},
    {"parabola", "parábola"},
    {"Parabola", "Parábola"},
    {"devéras", "deveras"},
    {"Devéras", "Deveras"},
    {"carmezim", "carmesim"},
    {"Carmezim", "Carmesim"},
    {"dizimos", "dízimos"},
    {"Dizimos", "Dízimos"},
    {"cherubim", "querubim"},
    {"Cherubim", "Querubim"},
    {"pacificos", "pacíficos"},
    {"Pacificos", "Pacíficos"},
    {"destruiram", "destruíram"},
    {"Destruiram", "Destruíram"},
    {"incircumcisos", "incircuncisos"},
    {"Incircumcisos", "Incircuncisos"},
    {"jámais", "jamais"},
    {"Jámais", "Jamais"},
    {"brazas", "brasas"},
    {"Brazas", "Brasas"},
    {"parabolas", "parábolas"},
    {"Parabolas", "Parábolas"},
    {"côxa", "coxa"},
    {"Côxa", "Coxa"},
    {"incircumcisão", "incircuncisão"},
    {"Incircumcisão", "Incircuncisão"},
    {"unico", "único"},
    {"Unico", "Único"},
    {"relampagos", "relâmpagos"},
    {"Relampagos", "Relâmpagos"},
    {"escandalo", "escândalo"},
    {"Escandalo", "Escândalo"},
    {"sobreveiu", "sobreveio"},
    {"Sobreveiu", "Sobreveio"},
    {"aromatico", "aromático"},
    {"Aromatico", "Aromático"},
    {"côxo", "coxo"},
    {"Côxo", "Coxo"},
    {"incircumciso", "incircunciso"},
    {"Incircumciso", "Incircunciso"},
    {"destrui", "destruí"},
    {"Destrui", "Destruí"},
    {"saistes", "saístes"},
    {"Saistes", "Saístes"},
    {"possuiram", "possuíram"},
    {"Possuiram", "Possuíram"},
    {"proseguiu", "prosseguiu"},
    {"Proseguiu", "Prosseguiu"},
    {"montezes", "monteses"},
    {"Montezes", "Monteses"},
    {"cenaculo", "cenáculo"},
    {"Cenaculo", "Cenáculo"},
    {"destruirem", "destruírem"},
    {"Destruirem", "Destruírem"},
    {"tinheis", "tínheis"},
    {"Tinheis", "Tínheis"},
    {"saires", "saíres"},
    {"Saires", "Saíres"},
    {"leãosinho", "leãozinho"},
    {"Leãosinho", "Leãozinho"},
    {"saiste", "saíste"},
    {"Saiste", "Saíste"},
    {"adulteros", "adúlteros"},
    {"Adulteros", "Adúlteros"},
    {"deshonra", "desonra"},
    {"Deshonra", "Desonra"},
    {"cadeiasinhas", "cadeiazinhas"},
    {"Cadeiasinhas", "Cadeiazinhas"},
    {"remoe", "remói"},
    {"Remoe", "Remói"},
    {"mólho", "molho"},
    {"Mólho", "Molho"},
    {"prohibição", "proibição"},
    {"Prohibição", "Proibição"},
    {"côxos", "coxos"},
    {"Côxos", "Coxos"},
    {"rodea", "rodeia"},
    {"Rodea", "Rodeia"},
    {"montez", "montês"},
    {"Montez", "Montês"},
    {"alforge", "alforje"},
    {"Alforge", "Alforje"},
    {"sairem", "saírem"},
    {"Sairem", "Saírem"},
    {"ância", "ânsia"},
    {"Ância", "Ânsia"},
    {"relampago", "relâmpago"},
    {"Relampago", "Relâmpago"},
    {"creei", "criei"},
    {"Creei", "Criei"},
    {"unica", "única"},
    {"Unica", "Única"},
    {"sairam-lhe", "saíram-lhe"},
    {"Sairam-lhe", "Saíram-lhe"},
    {"saira", "saíra"},
    {"Saira", "Saíra"},
    {"possuires", "possuíres"},
    {"Possuires", "Possuíres"},
    {"medicos", "médicos"},
    {"Medicos", "Médicos"},
    {"reteem", "retêm"},
    {"Reteem", "Retêm"},
    {"possuia", "possuía"},
    {"Possuia", "Possuía"},
    {"exhortação", "exortação"},
    {"Exhortação", "Exortação"},
    {"adultéra", "adúltera"},
    {"Adultéra", "Adúltera"},
    {"féras", "feras"},
    {"Féras", "Feras"},
    {"exhorta", "exorta"},
    {"Exhorta", "Exorta"},
    {"notoria", "notória"},
    {"Notoria", "Notória"},
    {"escandalos", "escândalos"},
    {"Escandalos", "Escândalos"},
    {"sauda", "saúda"},
    {"Sauda", "Saúda"},
    {"ishmaelitas", "ismaelitas"},
    {"Ishmaelitas", "Ismaelitas"},
    {"adopção", "adoção"},
    {"Adopção", "Adoção"},
    {"leõesinhos", "leõezinhos"},
    {"Leõesinhos", "Leõezinhos"},
    {"pifaro", "pífaro"},
    {"Pifaro", "Pífaro"},
    {"cubiça", "cobiça"},
    {"Cubiça", "Cobiça"},
    {"saimos", "saímos"},
    {"Saimos", "Saímos"},
    {"aromaticas", "aromáticas"},
    {"Aromaticas", "Aromáticas"},
    {"vêl-o", "vê-lo"},
    {"Vêl-o", "Vê-lo"},
    {"aromatica", "aromática"},
    {"Aromatica", "Aromática"},
    {"efesios", "efésios"},
    {"Efesios", "Efésios"},
    {"brazeiros", "braseiros"},
    {"Brazeiros", "Braseiros"},
    {"alem", "além"},
    {"Alem", "Além"},
    {"destruiste", "destruíste"},
    {"Destruiste", "Destruíste"},
    {"espivitadores", "espevitadores"},
    {"Espivitadores", "Espevitadores"},
    {"proconsul", "procônsul"},
    {"Proconsul", "Procônsul"},
    {"afectuosamente", "afetuosamente"},
    {"Afectuosamente", "Afetuosamente"},
    {"anuncíamos", "anunciamos"},
    {"Anuncíamos", "Anunciamos"},
    {"exhortar", "exortar"},
    {"Exhortar", "Exortar"},
    {"brazeiro", "braseiro"},
    {"Brazeiro", "Braseiro"},
    {"cananea", "cananeia"},
    {"Cananea", "Cananeia"},
    {"pertência", "pertencia"},
    {"Pertência", "Pertencia"},
    {"hebreas", "hebreias"},
    {"Hebreas", "Hebreias"},
    {"constituiram", "constituíram"},
    {"Constituiram", "Constituíram"},
    {"himno", "hino"},
    {"Himno", "Hino"},
    {"saissem", "saíssem"},
    {"Saissem", "Saíssem"},
    {"cubiçarás", "cobiçarás"},
    {"Cubiçarás", "Cobiçarás"},
    {"carbunculo", "carbúnculo"},
    {"Carbunculo", "Carbúnculo"},
    {"pavez", "pavês"},
    {"Pavez", "Pavês"},
    {"cireneo", "cireneu"},
    {"Cireneo", "Cireneu"},
    {"rodeam", "rodeiam"},
    {"Rodeam", "Rodeiam"},
    {"confíeis", "confieis"},
    {"Confíeis", "Confieis"},
    {"aquem", "aquém"},
    {"Aquem", "Aquém"},
    {"caiste", "caíste"},
    {"Caiste", "Caíste"},
    {"pavezes", "paveses"},
    {"Pavezes", "Paveses"},
    {"matemô-lo", "matemo-lo"},
    {"Matemô-lo", "Matemo-lo"},
    {"sanguinaria", "sanguinária"},
    {"Sanguinaria", "Sanguinária"},
    {"exhortamos", "exortamos"},
    {"Exhortamos", "Exortamos"},
    {"leval-os-á", "levá-los-á"},
    {"Leval-os-á", "Levá-los-á"},
    {"cubiçosos", "cobiçosos"},
    {"Cubiçosos", "Cobiçosos"},
    {"circunvisinhas", "circunvizinhas"},
    {"Circunvisinhas", "Circunvizinhas"},
    {"revéz", "revés"},
    {"Revéz", "Revés"},
    {"cubices", "cobices"},
    {"Cubices", "Cobices"},
    {"tinhamos", "tínhamos"},
    {"Tinhamos", "Tínhamos"},
    {"sêlo", "selo"},
    {"Sêlo", "Selo"},
    {"prohibais", "proibais"},
    {"Prohibais", "Proibais"},
    {"cordeis", "cordéis"},
    {"Cordeis", "Cordéis"},
    {"seguí-lo", "segui-lo"},
    {"Seguí-lo", "Segui-lo"},
    {"destruisse", "destruísse"},
    {"Destruisse", "Destruísse"},
    {"inuteis", "inúteis"},
    {"Inuteis", "Inúteis"},
    {"himnos", "hinos"},
    {"Himnos", "Hinos"},
    {"pôrem", "porem"},
    {"Pôrem", "Porem"},
    {"dificeis", "difíceis"},
    {"Dificeis", "Difíceis"},
    {"possuirem", "possuírem"},
    {"Possuirem", "Possuírem"},
    {"satrapas", "sátrapas"},
    {"Satrapas", "Sátrapas"},
    {"esmiuça", "esmiúça"},
    {"Esmiuça", "Esmiúça"},
    {"grangeou", "granjeou"},
    {"Grangeou", "Granjeou"},
    {"antehontem", "anteontem"},
    {"Antehontem", "Anteontem"},
    {"simfonia", "sinfonia"},
    {"Simfonia", "Sinfonia"},
    {"nóra", "nora"},
    {"Nóra", "Nora"},
    {"dextros", "destros"},
    {"Dextros", "Destros"},
    {"saisse", "saísse"},
    {"Saisse", "Saísse"},
    {"espadoa", "espádua"},
    {"Espadoa", "Espádua"},
    {"furia", "fúria"},
    {"Furia", "Fúria"},
    {"efrateo", "efrateu"},
    {"Efrateo", "Efrateu"},
    {"destruissem", "destruíssem"},
    {"Destruissem", "Destruíssem"},
    {"opéra", "opera"},
    {"Opéra", "Opera"},
    {"siriaco", "siríaco"},
    {"Siriaco", "Siríaco"},
    {"caira", "caíra"},
    {"Caira", "Caíra"},
    {"libios", "líbios"},
    {"Libios", "Líbios"},
    {"pôpa", "popa"},
    {"Pôpa", "Popa"},
    {"exhortava", "exortava"},
    {"Exhortava", "Exortava"},
    {"hetea", "heteia"},
    {"Hetea", "Heteia"},
    {"ponteagudas", "pontiagudas"},
    {"Ponteagudas", "Pontiagudas"},
    {"circunvisinha", "circunvizinha"},
    {"Circunvisinha", "Circunvizinha"},
    {"sobresairam", "sobressaíram"},
    {"Sobresairam", "Sobressaíram"},
    {"exhortai-vos", "exortai-vos"},
    {"Exhortai-vos", "Exortai-vos"},
    {"ácêrca", "acerca"},
    {"Ácêrca", "Acerca"},
    {"prosigo", "prossigo"},
    {"Prosigo", "Prossigo"},
    {"destruia", "destruía"},
    {"Destruia", "Destruía"},
    {"tôa", "toa"},
    {"Tôa", "Toa"},
    {"triumfar", "triunfar"},
    {"Triumfar", "Triunfar"},
    {"pozêsseis", "pusésseis"},
    {"Pozêsseis", "Pusésseis"},
    {"chôro", "choro"},
    {"Chôro", "Choro"},
    {"deminuirá", "diminuirá"},
    {"Deminuirá", "Diminuirá"},
    {"bahia", "baía"},
    {"Bahia", "Baía"},
    {"vehemência", "veemência"},
    {"Vehemência", "Veemência"},
    {"eguas", "éguas"},
    {"Eguas", "Éguas"},
    {"cairem", "caírem"},
    {"Cairem", "Caírem"},
    {"exhortando-os", "exortando-os"},
    {"Exhortando-os", "Exortando-os"},
    {"gloría-se", "gloria-se"},
    {"Gloría-se", "Gloria-se"},
    {"hebrea", "hebreia"},
    {"Hebrea", "Hebreia"},
    {"despojemô-los", "despojemo-los"},
    {"Despojemô-los", "Despojemo-los"},
    {"destruira", "destruíra"},
    {"Destruira", "Destruíra"},
    {"infrutifera", "infrutífera"},
    {"Infrutifera", "Infrutífera"},
    {"coherdeiros", "coerdeiros"},
    {"Coherdeiros", "Coerdeiros"},
    {"constituiste", "constituíste"},
    {"Constituiste", "Constituíste"},
    {"afecto", "afeto"},
    {"Afecto", "Afeto"},
    {"prohibimos", "proibimos"},
    {"Prohibimos", "Proibimos"},
    {"leôa", "leoa"},
    {"Leôa", "Leoa"},
    {"cantámos-vos", "cantamos-vos"},
    {"Cantámos-vos", "Cantamos-vos"},
    {"alforges", "alforjes"},
    {"Alforges", "Alforjes"},
    {"efectuar", "efetuar"},
    {"Efectuar", "Efetuar"},
    {"lidios", "lídios"},
    {"Lidios", "Lídios"},
    {"prestimo", "préstimo"},
    {"Prestimo", "Préstimo"},
    {"cenaculos", "cenáculos"},
    {"Cenaculos", "Cenáculos"},
    {"notorias", "notórias"},
    {"Notorias", "Notórias"},
    {"recrea", "recreia"},
    {"Recrea", "Recreia"},
    {"arabe", "árabe"},
    {"Arabe", "Árabe"},
    {"âncias", "ânsias"},
    {"Âncias", "Ânsias"},
    {"prôa", "proa"},
    {"Prôa", "Proa"},
    {"frutiferos", "frutíferos"},
    {"Frutiferos", "Frutíferos"},
    {"resoará", "ressoará"},
    {"Resoará", "Ressoará"},
    {"sêlos", "selos"},
    {"Sêlos", "Selos"},
    {"frutifero", "frutífero"},
    {"Frutifero", "Frutífero"},
    {"roe", "rói"},
    {"Roe", "Rói"},
    {"gloría", "gloria"},
    {"Gloría", "Gloria"},
    {"sobresaltados", "sobressaltados"},
    {"Sobresaltados", "Sobressaltados"},
    {"desterco", "esterco"},
    {"Desterco", "Esterco"},
    {"proseguirei", "prosseguirei"},
    {"Proseguirei", "Prosseguirei"},
    {"ishmaelita", "ismaelita"},
    {"Ishmaelita", "Ismaelita"},
    {"incômodes", "incomodes"},
    {"Incômodes", "Incomodes"},
    {"grangeei", "granjeei"},
    {"Grangeei", "Granjeei"},
    {"judaismo", "judaísmo"},
    {"Judaismo", "Judaísmo"},
    {"quéda", "queda"},
    {"Quéda", "Queda"},
    {"descairam", "decaíram"},
    {"Descairam", "Decaíram"},
    {"impuzer", "impuser"},
    {"Impuzer", "Impuser"},
    {"agata", "ágata"},
    {"Agata", "Ágata"},
    {"creíamos", "creiamos"},
    {"Creíamos", "Creiamos"},
    {"caisse", "caísse"},
    {"Caisse", "Caísse"},
    {"envíeis", "envieis"},
    {"Envíeis", "Envieis"},
    {"crédor", "credor"},
    {"Crédor", "Credor"},
    {"extasis", "êxtase"},
    {"Extasis", "Êxtase"},
    {"envíamos", "enviamos"},
    {"Envíamos", "Enviamos"},
    {"assocíeis", "associeis"},
    {"Assocíeis", "Associeis"},
    {"distribuirem", "distribuírem"},
    {"Distribuirem", "Distribuírem"},
    {"contribuiram", "contribuíram"},
    {"Contribuiram", "Contribuíram"},
    {"feiticerias", "feitiçarias"},
    {"Feiticerias", "Feitiçarias"},
    {"mórmente", "mormente"},
    {"Mórmente", "Mormente"},
    {"leviticos", "levíticos"},
    {"Leviticos", "Levíticos"},
    {"peze", "pese"},
    {"Peze", "Pese"},
    {"retens", "reténs"},
    {"Retens", "Reténs"},
    {"menea", "meneia"},
    {"Menea", "Meneia"},
    {"mólhos", "molhos"},
    {"Mólhos", "Molhos"},
    {"arduas", "árduas"},
    {"Arduas", "Árduas"},
    {"mortifera", "mortífera"},
    {"Mortifera", "Mortífera"},
    {"braza", "brasa"},
    {"Braza", "Brasa"},
    {"exhortar-vos", "exortar-vos"},
    {"Exhortar-vos", "Exortar-vos"},
    {"cubiçoso", "cobiçoso"},
    {"Cubiçoso", "Cobiçoso"},
    {"feril-os-á", "feri-los-á"},
    {"Feril-os-á", "Feri-los-á"},
    {"podêssemos", "pudéssemos"},
    {"Podêssemos", "Pudéssemos"},
    {"anuncíeis", "anuncieis"},
    {"Anuncíeis", "Anuncieis"},
    {"caires", "caíres"},
    {"Caires", "Caíres"},
    {"diminuida", "diminuída"},
    {"Diminuida", "Diminuída"},
    {"podera", "pudera"},
    {"Podera", "Pudera"},
    {"orfãosinho", "orfãozinho"},
    {"Orfãosinho", "Orfãozinho"},
    {"sobreedificados", "sobre-edificados"},
    {"Sobreedificados", "Sobre-edificados"},
    {"pôres", "pores"},
    {"Pôres", "Pores"},
    {"diminuim", "diminuem"},
    {"Diminuim", "Diminuem"},
    {"purpurea", "purpúrea"},
    {"Purpurea", "Purpúrea"},
    {"exhortando", "exortando"},
    {"Exhortando", "Exortando"},
    {"enfastíeis", "enfastieis"},
    {"Enfastíeis", "Enfastieis"},
    {"detens", "deténs"},
    {"Detens", "Deténs"},
    {"quisereis", "quiserdes"},
    {"Quisereis", "Quiserdes"},
    {"desarreiguemô-la", "desarraiguemo-la"},
    {"Desarreiguemô-la", "Desarraiguemo-la"},
    {"noticíeis", "noticieis"},
    {"Noticíeis", "Noticieis"},
    {"destruimos", "destruímos"},
    {"Destruimos", "Destruímos"},
    {"destrui-a", "destruí-a"},
    {"Destrui-a", "Destruí-a"},
    {"impôz", "impôs"},
    {"Impôz", "Impôs"},
    {"desviemô-los", "desviemo-los"},
    {"Desviemô-los", "Desviemo-los"},
    {"adheriu", "aderiu"},
    {"Adheriu", "Aderiu"},
    {"posaam", "possam"},
    {"Posaam", "Possam"},
    {"ateem", "atêm"},
    {"Ateem", "Atêm"},
    {"reverenceie", "reverencie"},
    {"Reverenceie", "Reverencie"},
    {"passeam", "passeiam"},
    {"Passeam", "Passeiam"},
    {"jurámos-lhes", "juramos-lhes"},
    {"Jurámos-lhes", "Juramos-lhes"},
    {"rogozijando-se", "regozijando-se"},
    {"Rogozijando-se", "Regozijando-se"},
    {"concluida", "concluída"},
    {"Concluida", "Concluída"},
    {"practicai", "praticai"},
    {"Practicai", "Praticai"},
    {"negoceia", "negocia"},
    {"Negoceia", "Negocia"},
    {"lagôas", "lagoas"},
    {"Lagôas", "Lagoas"},
    {"vendamô-lo", "vendamo-lo"},
    {"Vendamô-lo", "Vendamo-lo"},
    {"destruimô-las", "destruímo-las"},
    {"Destruimô-las", "Destruímo-las"},
    {"cordealmente", "cordialmente"},
    {"Cordealmente", "Cordialmente"},
    {"praticos", "práticos"},
    {"Praticos", "Práticos"},
    {"aflictíssimo", "aflitíssimo"},
    {"Aflictíssimo", "Aflitíssimo"},
    {"prostituiram", "prostituíram"},
    {"Prostituiram", "Prostituíram"},
    {"deshonestidades", "desonestidades"},
    {"Deshonestidades", "Desonestidades"},
    {"multiplice", "múltiplice"},
    {"Multiplice", "Múltiplice"},
    {"investiguemô-los", "investiguemo-los"},
    {"Investiguemô-los", "Investiguemo-los"},
    {"heteas", "heteias"},
    {"Heteas", "Heteias"},
    {"incircumcisa", "incircuncisa"},
    {"Incircumcisa", "Incircuncisa"},
    {"odorifera", "odorífera"},
    {"Odorifera", "Odorífera"},
    {"aligera", "alígera"},
    {"Aligera", "Alígera"},
    {"efectuarás", "efetuarás"},
    {"Efectuarás", "Efetuarás"},
    {"triumfou", "triunfou"},
    {"Triumfou", "Triunfou"},
    {"contentaramos", "contentáramos"},
    {"Contentaramos", "Contentáramos"},
    {"levéda", "leveda"},
    {"Levéda", "Leveda"},
    {"deprezo", "desprezo"},
    {"Deprezo", "Desprezo"},
    {"grangear", "granjear"},
    {"Grangear", "Granjear"},
    {"triumfem", "triunfem"},
    {"Triumfem", "Triunfem"},
    {"afectos", "afetos"},
    {"Afectos", "Afetos"},
    {"inprecações", "imprecações"},
    {"Inprecações", "Imprecações"},
    {"visseis", "vísseis"},
    {"Visseis", "Vísseis"},
    {"constituistes", "constituístes"},
    {"Constituistes", "Constituístes"},
    {"possuistes", "possuístes"},
    {"Possuistes", "Possuístes"},
    {"cubiçar", "cobiçar"},
    {"Cubiçar", "Cobiçar"},
    {"distribuia", "distribuía"},
    {"Distribuia", "Distribuía"},
    {"opéro", "opero"},
    {"Opéro", "Opero"},
    {"dáes", "dais"},
    {"Dáes", "Dais"},
    {"proseguiremos", "prosseguiremos"},
    {"Proseguiremos", "Prosseguiremos"},
    {"perseguil-os-hei", "persegui-los-ei"},
    {"Perseguil-os-hei", "Persegui-los-ei"},
    {"sobresalte", "sobressalte"},
    {"Sobresalte", "Sobressalte"},
    {"excluida", "excluída"},
    {"Excluida", "Excluída"},
    {"géra", "gera"},
    {"Géra", "Gera"},
    {"confeção", "confecção"},
    {"Confeção", "Confecção"},
    {"exhorto-vos", "exorto-vos"},
    {"Exhorto-vos", "Exorto-vos"},
    {"susteem", "sustêm"},
    {"Susteem", "Sustêm"},
    {"efectuará", "efetuará"},
    {"Efectuará", "Efetuará"},
    {"grangeiam", "granjeiam"},
    {"Grangeiam", "Granjeiam"},
    {"siro-fenicia", "siro-fenícia"},
    {"Siro-fenicia", "Siro-fenícia"},
    {"cortemô-lo", "cortemo-lo"},
    {"Cortemô-lo", "Cortemo-lo"},
    {"virámo-nos", "viramo-nos"},
    {"Virámo-nos", "Viramo-nos"},
    {"vasarei", "vazarei"},
    {"Vasarei", "Vazarei"},
    {"rez", "rês"},
    {"Rez", "Rês"},
    {"reflectindo", "refletindo"},
    {"Reflectindo", "Refletindo"},
    {"exhortaram", "exortaram"},
    {"Exhortaram", "Exortaram"},
    {"sirvamô-los", "sirvamo-los"},
    {"Sirvamô-los", "Sirvamo-los"},
    {"proconsules", "procônsules"},
    {"Proconsules", "Procônsules"},
    {"sirios", "sírios"},
    {"Sirios", "Sírios"},
    {"possuiam", "possuíam"},
    {"Possuiam", "Possuíam"},
    {"pôe-no", "põe-no"},
    {"Pôe-no", "Põe-no"},
    {"desd'as", "desde as"},
    {"Desd'as", "Desde as"},
    {"bemfeitores", "benfeitores"},
    {"Bemfeitores", "Benfeitores"},
    {"cairam-lhe", "caíram-lhe"},
    {"Cairam-lhe", "Caíram-lhe"},
    {"varôa", "varoa"},
    {"Varôa", "Varoa"},
    {"sára", "sara"},
    {"Sára", "Sara"},
    {"esbofetéar", "esbofetear"},
    {"Esbofetéar", "Esbofetear"},
    {"odorifero", "odorífero"},
    {"Odorifero", "Odorífero"},
    {"siriaca", "siríaca"},
    {"Siriaca", "Siríaca"},
    {"ameacemô-los", "ameacemo-los"},
    {"Ameacemô-los", "Ameacemo-los"},
    {"ornatorias", "ornatórias"},
    {"Ornatorias", "Ornatórias"},
    {"presbiterio", "presbitério"},
    {"Presbiterio", "Presbitério"},
    {"sobreveiu-lhes", "sobreveio-lhes"},
    {"Sobreveiu-lhes", "Sobreveio-lhes"},
    {"destruisses", "destruísses"},
    {"Destruisses", "Destruísses"},
    {"apanhal-os-á", "apanhá-los-á"},
    {"Apanhal-os-á", "Apanhá-los-á"},
    {"decaiste", "decaíste"},
    {"Decaiste", "Decaíste"},
    {"deter-vos-híeis", "deter-vos-íeis"},
    {"Deter-vos-híeis", "Deter-vos-íeis"},
    {"rogámos-lhe", "rogamos-lhe"},
    {"Rogámos-lhe", "Rogamos-lhe"},
    {"conveiu", "conveio"},
    {"Conveiu", "Conveio"},
    {"sitía", "sitia"},
    {"Sitía", "Sitia"},
    {"distribuiam", "distribuíam"},
    {"Distribuiam", "Distribuíam"},
    {"possuissem", "possuíssem"},
    {"Possuissem", "Possuíssem"},
    {"ferteis", "férteis"},
    {"Ferteis", "Férteis"},
    {"condiscipulos", "condiscípulos"},
    {"Condiscipulos", "Condiscípulos"},
    {"sobria", "sóbria"},
    {"Sobria", "Sóbria"},
    {"passea", "passeia"},
    {"Passea", "Passeia"},
    {"incircumcidado", "incircunciso"},
    {"Incircumcidado", "Incircunciso"},
    {"pôl-a-hei", "pô-la-ei"},
    {"Pôl-a-hei", "Pô-la-ei"},
    {"dir-vos-hão", "dir-vos-ão"},
    {"Dir-vos-hão", "Dir-vos-ão"},
    {"proseguiram", "prosseguiram"},
    {"Proseguiram", "Prosseguiram"},
    {"voltar-mos", "voltarmos"},
    {"Voltar-mos", "Voltarmos"},
    {"sobresaltada", "sobressaltada"},
    {"Sobresaltada", "Sobressaltada"},
    {"pôl-os-hei", "pô-los-ei"},
    {"Pôl-os-hei", "Pô-los-ei"},
    {"deshonras", "desonras"},
    {"Deshonras", "Desonras"},
    {"vasou", "vazou"},
    {"Vasou", "Vazou"},
    {"afligil-os-hão", "afligi-los-ão"},
    {"Afligil-os-hão", "Afligi-los-ão"},
    {"ten", "teu"},
    {"Ten", "Teu"},
    {"encurvande", "encurvando"},
    {"Encurvande", "Encurvando"},
    {"deshonrastes", "desonrastes"},
    {"Deshonrastes", "Desonrastes"},
    {"móra", "mora"},
    {"Móra", "Mora"},
    {"esperamô-lo", "esperamo-lo"},
    {"Esperamô-lo", "Esperamo-lo"},
    {"caissem", "caíssem"},
    {"Caissem", "Caíssem"},
    {"descahiam", "descaíam"},
    {"Descahiam", "Descaíam"},
    {"médo", "medo"},
    {"Médo", "Medo"},
    {"choviscos", "chuviscos"},
    {"Choviscos", "Chuviscos"},
    {"vasaste", "vazaste"},
    {"Vasaste", "Vazaste"},
    {"crêsse", "cresse"},
    {"Crêsse", "Cresse"},
    {"avisinhará", "avizinhará"},
    {"Avisinhará", "Avizinhará"},
    {"enviemô-los", "enviemo-los"},
    {"Enviemô-los", "Enviemo-los"},
    {"dalem", "além"},
    {"Dalem", "Além"},
    {"cubiçastes", "cobiçastes"},
    {"Cubiçastes", "Cobiçastes"},
    {"confortadgos", "confortados"},
    {"Confortadgos", "Confortados"},
    {"frecháda", "flechada"},
    {"Frecháda", "Flechada"},
    {"convezes", "conveses"},
    {"Convezes", "Conveses"},
    {"aformosea", "aformoseia"},
    {"Aformosea", "Aformoseia"},
    {"chronista", "cronista"},
    {"Chronista", "Cronista"},
    {"sobresahia", "sobressaía"},
    {"Sobresahia", "Sobressaía"},
    {"prohibido", "proibido"},
    {"Prohibido", "Proibido"},
    {"prosigamos", "prossigamos"},
    {"Prosigamos", "Prossigamos"},
    {"dir-lhe-hei", "dir-lhe-ei"},
    {"Dir-lhe-hei", "Dir-lhe-ei"},
    {"prohibia", "proibia"},
    {"Prohibia", "Proibia"},
    {"curuja", "coruja"},
    {"Curuja", "Coruja"},
    {"dictava-me", "ditava-me"},
    {"Dictava-me", "Ditava-me"},
    {"desposorios", "desposórios"},
    {"Desposorios", "Desposórios"},
    {"vol", "vo-lo"},
    {"Vol", "Vo-lo"},
    {"reflectirás", "refletirás"},
    {"Reflectirás", "Refletirás"},
    {"assenhoream", "assenhoreiam"},
    {"Assenhoream", "Assenhoreiam"},
    {"contámos-lhos", "contamos-lhos"},
    {"Contámos-lhos", "Contamos-lhos"},
    {"siriacos", "siríacos"},
    {"Siriacos", "Siríacos"},
    {"cerquemô-las", "cerquemo-las"},
    {"Cerquemô-las", "Cerquemo-las"},
    {"efectuados", "efetuados"},
    {"Efectuados", "Efetuados"},
    {"contradicto", "contradito"},
    {"Contradicto", "Contradito"},
    {"expuz-lhe", "expus-lhe"},
    {"Expuz-lhe", "Expus-lhe"},
    {"disséra", "dissera"},
    {"Disséra", "Dissera"},
    {"circunvisinhos", "circunvizinhos"},
    {"Circunvisinhos", "Circunvizinhos"},
    {"arduo", "árduo"},
    {"Arduo", "Árduo"},
    {"licaonica", "licaônica"},
    {"Licaonica", "Licaônica"},
    {"infrutiferas", "infrutíferas"},
    {"Infrutiferas", "Infrutíferas"},
    {"receiou", "receou"},
    {"Receiou", "Receou"},
    {"tomámô-lo", "tomamo-lo"},
    {"Tomámô-lo", "Tomamo-lo"},
    {"desposorio", "desposório"},
    {"Desposorio", "Desposório"},
    {"aflictiva", "aflitiva"},
    {"Aflictiva", "Aflitiva"},
    {"refrea", "refreia"},
    {"Refrea", "Refreia"},
    {"pol-o-hão", "pô-lo-ão"},
    {"Pol-o-hão", "Pô-lo-ão"},
    {"possuiste", "possuíste"},
    {"Possuiste", "Possuíste"},
    {"dromedaria", "dromedária"},
    {"Dromedaria", "Dromedária"},
    {"dir-te-hão", "dir-te-ão"},
    {"Dir-te-hão", "Dir-te-ão"},
    {"cohabitai", "coabitai"},
    {"Cohabitai", "Coabitai"},
    {"concluiam", "concluíam"},
    {"Concluiam", "Concluíam"},
    {"resicados", "ressecados"},
    {"Resicados", "Ressecados"},
    {"dizéndo", "dizendo"},
    {"Dizéndo", "Dizendo"},
    {"decairem", "decaírem"},
    {"Decairem", "Decaírem"},
    {"habitalas", "habitá-las"},
    {"Habitalas", "Habitá-las"},
    {"opoz", "opôs"},
    {"Opoz", "Opôs"},
    {"galatas", "gálatas"},
    {"Galatas", "Gálatas"},
    {"porteiros-móres", "porteiros-mores"},
    {"Porteiros-móres", "Porteiros-mores"},
    {"est'", "estas"},
    {"Est'", "Estas"},
    {"echo", "eco"},
    {"Echo", "Eco"},
    {"médos", "medos"},
    {"Médos", "Medos"},
    {"quaesquer", "quaisquer"},
    {"Quaesquer", "Quaisquer"},
    {"prostituiste", "prostituíste"},
    {"Prostituiste", "Prostituíste"},
    {"destituidos", "destituídos"},
    {"Destituidos", "Destituídos"},
    {"abroga-se", "ab-roga-se"},
    {"Abroga-se", "Ab-roga-se"},
    {"directamente", "diretamente"},
    {"Directamente", "Diretamente"},
    {"cohibiram", "coibiram"},
    {"Cohibiram", "Coibiram"},
    {"vasaram", "vazaram"},
    {"Vasaram", "Vazaram"},
    {"prohibisse", "proibisse"},
    {"Prohibisse", "Proibisse"},
    {"exhortes", "exortes"},
    {"Exhortes", "Exortes"},
    {"sobreveiu-lhe", "sobreveio-lhe"},
    {"Sobreveiu-lhe", "Sobreveio-lhe"},
    {"sobresair", "sobressair"},
    {"Sobresair", "Sobressair"},
    {"recaisse", "recaísse"},
    {"Recaisse", "Recaísse"},
    {"deshonrada", "desonrada"},
    {"Deshonrada", "Desonrada"},
    {"babilonica", "babilônica"},
    {"Babilonica", "Babilônica"},
    {"levitico", "levítico"},
    {"Levitico", "Levítico"},
    {"pósinhos", "pozinhos"},
    {"Pósinhos", "Pozinhos"},
    {"comerãs", "comerás"},
    {"Comerãs", "Comerás"},
    {"mesuros", "mesmos"},
    {"Mesuros", "Mesmos"},
    {"triumfo", "triunfo"},
    {"Triumfo", "Triunfo"},
    {"caimos", "caímos"},
    {"Caimos", "Caímos"},
    {"efectuado", "efetuado"},
    {"Efectuado", "Efetuado"},
    {"reconcilíeis", "reconcilieis"},
    {"Reconcilíeis", "Reconcilieis"},
    {"buscámô-lo", "buscamo-lo"},
    {"Buscámô-lo", "Buscamo-lo"},
    {"deshonrais", "desonrais"},
    {"Deshonrais", "Desonrais"},
    {"filacterias", "filactérias"},
    {"Filacterias", "Filactérias"},
    {"despedeçarei", "despedaçarei"},
    {"Despedeçarei", "Despedaçarei"},
    {"macãs", "maçãs"},
    {"Macãs", "Maçãs"},
    {"prohibe-lho", "proíbe-lho"},
    {"Prohibe-lho", "Proíbe-lho"},
    {"saisses", "saísses"},
    {"Saisses", "Saísses"},
    {"desarreiguemô-los", "desarraiguemo-los"},
    {"Desarreiguemô-los", "Desarraiguemo-los"},
    {"abenção", "bênção"},
    {"Abenção", "Bênção"},
    {"reunam", "reúnam"},
    {"Reunam", "Reúnam"},
    {"adheriam", "aderiam"},
    {"Adheriam", "Aderiam"},
    {"diabolica", "diabólica"},
    {"Diabolica", "Diabólica"},
    {"assentaram-ce", "assentaram-se"},
    {"Assentaram-ce", "Assentaram-se"},
    {"expuz", "expus"},
    {"Expuz", "Expus"},
    {"comissarios", "comissários"},
    {"Comissarios", "Comissários"},
    {"abstem", "abstêm"},
    {"Abstem", "Abstêm"},
    {"manuseis", "manuseies"},
    {"Manuseis", "Manuseies"},
    {"divídido", "dividido"},
    {"Divídido", "Dividido"},
    {"contrar", "contra"},
    {"Contrar", "Contra"},
    {"exhortavam", "exortavam"},
    {"Exhortavam", "Exortavam"},
    {"repartamô-lo", "repartamo-lo"},
    {"Repartamô-lo", "Repartamo-lo"},
    {"destruires", "destruíres"},
    {"Destruires", "Destruíres"},
    {"jubileo", "jubileu"},
    {"Jubileo", "Jubileu"},
    {"atribue", "atribui"},
    {"Atribue", "Atribui"},
    {"pranteam", "pranteiam"},
    {"Pranteam", "Pranteiam"},
    {"deshabitadas", "desabitadas"},
    {"Deshabitadas", "Desabitadas"},
    {"restituires", "restituíres"},
    {"Restituires", "Restituíres"},
    {"vomitalas-ha", "vomitá-las-á"},
    {"Vomitalas-ha", "Vomitá-las-á"},
    {"mármoreas", "marmóreas"},
    {"Mármoreas", "Marmóreas"},
    {"extinguí-la", "extingui-la"},
    {"Extinguí-la", "Extingui-la"},
    {"pôe-o", "põe-o"},
    {"Pôe-o", "Põe-o"},
    {"proveiu", "proveu"},
    {"Proveiu", "Proveu"},
    {"pôem", "põem"},
    {"Pôem", "Põem"},
    {"sacrilegos", "sacrílegos"},
    {"Sacrilegos", "Sacrílegos"},
    {"exhortando-o", "exortando-o"},
    {"Exhortando-o", "Exortando-o"},
    {"prohibiu", "proibiu"},
    {"Prohibiu", "Proibiu"},
    {"alimarias", "alimárias"},
    {"Alimarias", "Alimárias"},
    {"parvoices", "parvoíces"},
    {"Parvoices", "Parvoíces"},
    {"prohibira", "proibira"},
    {"Prohibira", "Proibira"},
    {"entaboladas", "entabuladas"},
    {"Entaboladas", "Entabuladas"},
    {"altisonantes", "altissonantes"},
    {"Altisonantes", "Altissonantes"},
    {"efectua", "efetua"},
    {"Efectua", "Efetua"},
    {"efectuo", "efetuo"},
    {"Efectuo", "Efetuo"},
    {"sobrias", "sóbrias"},
    {"Sobrias", "Sóbrias"},
    {"sofocam", "sufocam"},
    {"Sofocam", "Sufocam"},
    {"adoptou", "adotou"},
    {"Adoptou", "Adotou"},
    {"des-te", "deste"},
    {"Des-te", "Deste"},
    {"prohibe", "proíbe"},
    {"Prohibe", "Proíbe"},
    {"expôl-os-hão", "expô-los-ão"},
    {"Expôl-os-hão", "Expô-los-ão"},
    {"destrui-os", "destruí-os"},
    {"Destrui-os", "Destruí-os"},
    {"distribuisse", "distribuísse"},
    {"Distribuisse", "Distribuísse"},
    {"espadoas", "espáduas"},
    {"Espadoas", "Espáduas"},
    {"parvoice", "parvoíce"},
    {"Parvoice", "Parvoíce"},
    {"arripiado", "arrepiado"},
    {"Arripiado", "Arrepiado"},
    {"soborno", "suborno"},
    {"Soborno", "Suborno"},
    {"golosos", "gulosos"},
    {"Golosos", "Gulosos"},
    {"cedula", "cédula"},
    {"Cedula", "Cédula"},
    {"deshonestidade", "desonestidade"},
    {"Deshonestidade", "Desonestidade"},
    {"geracões", "gerações"},
    {"Geracões", "Gerações"},
    {"pêga", "pega"},
    {"Pêga", "Pega"},
    {"cubiçará", "cobiçará"},
    {"Cubiçará", "Cobiçará"},
    {"colirio", "colírio"},
    {"Colirio", "Colírio"},
    {"inhabitável", "inabitável"},
    {"Inhabitável", "Inabitável"},
    {"convência", "convencia"},
    {"Convência", "Convencia"},
    {"proseguir", "prosseguir"},
    {"Proseguir", "Prosseguir"},
    {"obstrói", "obstrui"},
    {"Obstrói", "Obstrui"},
    {"disvelo", "desvelo"},
    {"Disvelo", "Desvelo"},
    {"deshonrarem", "desonrarem"},
    {"Deshonrarem", "Desonrarem"},
    {"destrui-lo-ei", "destruí-lo-ei"},
    {"Destrui-lo-ei", "Destruí-lo-ei"},
    {"engola", "engula"},
    {"Engola", "Engula"},
    {"aiuda", "ainda"},
    {"Aiuda", "Ainda"},
    {"cubiçaram", "cobiçaram"},
    {"Cubiçaram", "Cobiçaram"},
    {"môa", "moa"},
    {"Môa", "Moa"},
    {"firamô-lo", "firamo-lo"},
    {"Firamô-lo", "Firamo-lo"},
    {"faxas", "faixas"},
    {"Faxas", "Faixas"},
    {"rodeiou", "rodeou"},
    {"Rodeiou", "Rodeou"},
    {"gloríeis", "glorieis"},
    {"Gloríeis", "Glorieis"},
    {"fazemô-lo", "fazemo-lo"},
    {"Fazemô-lo", "Fazemo-lo"},
    {"mandragoras", "mandrágoras"},
    {"Mandragoras", "Mandrágoras"},
    {"meis", "meio"},
    {"Meis", "Meio"},
    {"vestuario", "vestuário"},
    {"Vestuario", "Vestuário"},
    {"resurgirão", "ressurgirão"},
    {"Resurgirão", "Ressurgirão"},
    {"triumfa", "triunfa"},
    {"Triumfa", "Triunfa"},
    {"exhortou", "exortou"},
    {"Exhortou", "Exortou"},
    {"lancemô-lo", "lancemo-lo"},
    {"Lancemô-lo", "Lancemo-lo"},
    {"leôas", "leoas"},
    {"Leôas", "Leoas"},
    {"incircumcisas", "incircuncisas"},
    {"Incircumcisas", "Incircuncisas"},
    {"restituissem", "restituíssem"},
    {"Restituissem", "Restituíssem"},
    {"restituirem", "restituírem"},
    {"Restituirem", "Restituírem"},
    {"deshonre", "desonre"},
    {"Deshonre", "Desonre"},
    {"despertemô-lo", "despertemo-lo"},
    {"Despertemô-lo", "Despertemo-lo"},
    {"moe", "mói"},
    {"Moe", "Mói"},
    {"trazeim'os", "trazei-mos"},
    {"Trazeim'os", "Trazei-mos"},
    {"importunanado-o", "importunando-o"},
    {"Importunanado-o", "Importunando-o"},
    {"conducta", "conduta"},
    {"Conducta", "Conduta"},
    {"israhita", "israelita"},
    {"Israhita", "Israelita"},
    {"redemi-me", "redimi-me"},
    {"Redemi-me", "Redimi-me"},
    {"creai-os", "criai-os"},
    {"Creai-os", "Criai-os"},
    {"possuim", "possuem"},
    {"Possuim", "Possuem"},
    {"pégadas", "pegadas"},
    {"Pégadas", "Pegadas"},
    {"amplicíssimo", "amplíssimo"},
    {"Amplicíssimo", "Amplíssimo"},
    {"exhortávamos", "exortávamos"},
    {"Exhortávamos", "Exortávamos"},
    {"destruistes", "destruístes"},
    {"Destruistes", "Destruístes"},
    {"pifaros", "pífaros"},
    {"Pifaros", "Pífaros"},
    {"povosinho", "povozinho"},
    {"Povosinho", "Povozinho"},
    {"reune-te", "reúne-te"},
    {"Reune-te", "Reúne-te"},
    {"anciosos", "ansiosos"},
    {"Anciosos", "Ansiosos"},
    {"rôlo", "rolo"},
    {"Rôlo", "Rolo"},
    {"dôa", "doa"},
    {"Dôa", "Doa"},
    {"enchete", "enche-te"},
    {"Enchete", "Enche-te"},
    {"exhalar", "exalar"},
    {"Exhalar", "Exalar"},
    {"abrogou", "ab-rogou"},
    {"Abrogou", "Ab-rogou"},
    {"socoresse", "socorresse"},
    {"Socoresse", "Socorresse"},
    {"dar-vos-híamos", "dar-vos-íamos"},
    {"Dar-vos-híamos", "Dar-vos-íamos"},
    {"incircumcisãoh", "incircuncisão"},
    {"Incircumcisãoh", "Incircuncisão"},
    {"celebremô-lo", "celebremo-lo"},
    {"Celebremô-lo", "Celebremo-lo"},
    {"cérca", "cerca"},
    {"Cérca", "Cerca"},
    {"anhelais", "anelais"},
    {"Anhelais", "Anelais"},
    {"prosiga", "prossiga"},
    {"Prosiga", "Prossiga"},
    {"epicureos", "epicureus"},
    {"Epicureos", "Epicureus"},
    {"pórros", "porros"},
    {"Pórros", "Porros"},
    {"queimemô-los", "queimemo-los"},
    {"Queimemô-los", "Queimemo-los"},
    {"constituissem", "constituíssem"},
    {"Constituissem", "Constituíssem"},
    {"matemê-lo", "matemo-lo"},
    {"Matemê-lo", "Matemo-lo"},
    {"possuamô-la", "possuamo-la"},
    {"Possuamô-la", "Possuamo-la"},
    {"acquisição", "aquisição"},
    {"Acquisição", "Aquisição"},
    {"pêz", "pez"},
    {"Pêz", "Pez"},
    {"faziamos-lhe", "fazíamos-lhe"},
    {"Faziamos-lhe", "Fazíamos-lhe"},
    {"zêlo", "zelo"},
    {"Zêlo", "Zelo"},
    {"cincoentenarios", "cinquentenários"},
    {"Cincoentenarios", "Cinquentenários"},
    {"entregar-mô-lo", "entregar-mo-lo"},
    {"Entregar-mô-lo", "Entregar-mo-lo"},
    {"queimal-os-á", "queimá-los-á"},
    {"Queimal-os-á", "Queimá-los-á"},
    {"viêsseis", "viésseis"},
    {"Viêsseis", "Viésseis"},
    {"cambaleam", "cambaleiam"},
    {"Cambaleam", "Cambaleiam"},
    {"cubicemos", "cobicemos"},
    {"Cubicemos", "Cobicemos"},
    {"filosofos", "filósofos"},
    {"Filosofos", "Filósofos"},
    {"buscal-os-á", "buscá-los-á"},
    {"Buscal-os-á", "Buscá-los-á"},
    {"indignaes-vos", "indignais-vos"},
    {"Indignaes-vos", "Indignais-vos"},
    {"foramos", "fôramos"},
    {"Foramos", "Fôramos"},
    {"cubicei", "cobicei"},
    {"Cubicei", "Cobicei"},
    {"fêl-os", "fê-los"},
    {"Fêl-os", "Fê-los"},
    {"inutil", "inútil"},
    {"Inutil", "Inútil"},
    {"ultima", "última"},
    {"Ultima", "Última"},
    {"ultimas", "últimas"},
    {"Ultimas", "Últimas"},
    {"ultimo", "último"},
    {"Ultimo", "Último"},
    {"ultimos", "últimos"},
    {"Ultimos", "Últimos"},
    {"Asia", "Ásia"},
    {"Africa", "África"},
    {"eramos", "éramos"},
    {"Eramos", "Éramos"},
    {"ruido", "ruído"},
    {"Ruido", "Ruído"},
        };
        int total_zona = (int)(sizeof(zona_invisivel) / sizeof(zona_invisivel[0]));
        for (int z = 0; z < total_zona; z++)
            substituir_palavra_inteira(texto, zona_invisivel[z][0], zona_invisivel[z][1]);

        /* Casos que dependem do sentido: só nas frases em que é nome */
        substituir_texto(texto, "em duvida", "em dúvida");         substituir_texto(texto, "Sem duvida", "Sem dúvida");
        substituir_texto(texto, "grande duvida", "grande dúvida");
        substituir_texto(texto, "havia interprete", "havia intérprete"); substituir_texto(texto, "um interprete", "um intérprete");
        substituir_texto(texto, "houver interprete", "houver intérprete");
        substituir_texto(texto, "pouco celebre", "pouco célebre");
        /* "-mor" não leva acento hoje: copeiro-mor, padeiro-mor, mordomo-mor */
        substituir_texto(texto, "-mór", "-mor");
        /* "porem" é verbo (para porem); só aqui é "porém" */
        substituir_texto(texto, "olhos porem", "olhos porém");
        /* At 27:16: passado do verbo poder */
        substituir_texto(texto, "apenas podemos ganhar", "apenas pudemos ganhar");
    }

    /* 11. NOMES PRÓPRIOS (revisão de 27/11/2026)
       Adaptação ortográfica própria ao português (th, ph, y, ch=k, k, consoantes dobradas,
       finais -b/-d/-th com -e, acentos do Acordo) e formas de uso geral na língua.
       Nenhuma lista de edição moderna foi copiada. Na dúvida, ficou a forma de 1911.
       Fonte: biblia-build/nomes/nomes_todos.tsv */
    {
        static const char *nomes_proprios[][2] = {
    {"Pocheret-hat-sebaim", "Poquerete-hat-sebaim"},
    {"Beerot-Bene-jaakan", "Beerote-Bene-jaacan"},
    {"Ashterot-karnaim", "Asterote-carnaim"},
    {"Esperal-os-híeis", "Esperá-los-íeis"},
    {"Merodach-baladan", "Merodaque-baladan"},
    {"Almon-diblataim", "Almom-diblataim"},
    {"Chefar-haamonai", "Quefar-haamonai"},
    {"Helkat-hazzurim", "Helcat-hazurim"},
    {"Pocheret-zebaim", "Poquerete-zebaim"},
    {"Quibrot-hataava", "Quibrote-hataava"},
    {"Sela-hamahlecot", "Sela-hamahlecote"},
    {"Shave-kiriataim", "Shave-quiriataim"},
    {"Tiglat-pilneser", "Tiglate-pilneser"},
    {"Tilgat-pilneser", "Tilgate-pilneser"},
    {"Abel-bet-maaca", "Abel-bete-maaca"},
    {"Bet-hamarcabot", "Bet-hamarcabote"},
    {"Hazer-hatichon", "Hazer-haticon"},
    {"Joseb-bashebet", "Josebe-bashebete"},
    {"Kibrot-hataava", "Quibrote-hataava"},
    {"Magor-missabib", "Magor-missabibe"},
    {"Tiglat-pileser", "Tiglate-pileser"},
    {"Chislot-tabor", "Quislote-tabor"},
    {"Conjurámo-nos", "Conjuramo-nos"},
    {"Estribaes-vos", "Estribais-vos"},
    {"Evil-merodach", "Evil-merodaque"},
    {"Exhortamo-vos", "Exortamo-vos"},
    {"Gerut-chimham", "Gerut-quimham"},
    {"Kiriat-jearim", "Quiriate-jearim"},
    {"Kiriot-hesron", "Quiriote-hesron"},
    {"Misrefot-main", "Misrefote-maim"},
    {"Nabucodonozor", "Nabucodonosor"},
    {"Quibrot-taava", "Quibrote-taava"},
    {"Quiriat-huzot", "Quiriate-huzote"},
    {"Quiriat-sefer", "Quiriate-sefer"},
    {"Zafnat-paneah", "Zafnate-paneah"},
    {"Abel-keramim", "Abel-queramim"},
    {"Adram-melech", "Adram-meleque"},
    {"Belteshazzar", "Beltessazar"},
    {"Bet-marcabot", "Bet-marcabote"},
    {"Chedorlaomer", "Quedorlaomer"},
    {"Evil-merodac", "Evil-merodaque"},
    {"feril-os-hei", "feri-los-ei"},
    {"Feril-os-hei", "Feri-los-ei"},
    {"Hazzelelponi", "Hazelelponi"},
    {"Hor-hagidgad", "Hor-hagidgade"},
    {"Jabez-gilead", "Jabez-gileade"},
    {"Jabéz-gilead", "Jabez-gileade"},
    {"Keren-hapuch", "Queren-hapuque"},
    {"Kiriat-sefer", "Quiriate-sefer"},
    {"Natan-melech", "Natã-meleque"},
    {"Nebus-hasban", "Nebus-hasbã"},
    {"Quiriat-arba", "Quiriate-arba"},
    {"Ramot-gilead", "Ramote-gileade"},
    {"Rimon-metoar", "Rimom-metoar"},
    {"Sihor-libnat", "Sihor-libnate"},
    {"Simron-meron", "Simrom-merom"},
    {"Timnat-heres", "Timnate-heres"},
    {"Timnat-serah", "Timnate-sera"},
    {"Traguemô-los", "Traguemo-los"},
    {"Abel-mehola", "Abel-meola"},
    {"Adoni-bezek", "Adoni-bezeque"},
    {"Anam-melech", "Anam-meleque"},
    {"Asdot-pisga", "Asdote-pisga"},
    {"Atarot-adar", "Atarote-adar"},
    {"Atrot-sofan", "Atrote-sofan"},
    {"Azmot-tabor", "Azmote-tabor"},
    {"Baalat-beer", "Baalate-beer"},
    {"Baal-hermon", "Baal-hermom"},
    {"Bene-jaakan", "Bene-jaacan"},
    {"Bet-azmavet", "Bete-azmavete"},
    {"Bet-jesimot", "Bet-jesimote"},
    {"Conductores", "Condutores"},
    {"Ebed-melech", "Ebede-meleque"},
    {"Eseon-geber", "Ezion-geber"},
    {"Esion-geber", "Ezion-geber"},
    {"Ezeon-geber", "Ezion-geber"},
    {"Hassar-sual", "Hasar-sual"},
    {"Hassar-susa", "Hasar-susa"},
    {"Jeberechias", "Jeberequias"},
    {"Jusab-hesed", "Jusabe-hesede"},
    {"Kir-hareset", "Quir-haresete"},
    {"Kiriat-arba", "Quiriate-arba"},
    {"Kiriat-arim", "Quiriate-arim"},
    {"Kiriat-baal", "Quiriate-baal"},
    {"Kiriat-sana", "Quiriate-sana"},
    {"Litostrótos", "Litostrotos"},
    {"Melchisedec", "Melquisedeque"},
    {"Melchizedec", "Melquisedeque"},
    {"Nebuzaradan", "Nebuzaradã"},
    {"Obedeecemos", "Obedecemos"},
    {"Ramat-lechi", "Ramate-lequi"},
    {"Ramat-mispe", "Ramate-mispe"},
    {"Rimon-parez", "Rimom-perez"},
    {"Rimon-perez", "Rimom-perez"},
    {"Sucot-benot", "Sucote-benote"},
    {"Taanat-silo", "Taanate-silo"},
    {"Timnat-sera", "Timnate-sera"},
    {"Tocámos-vos", "Tocamos-vos"},
    {"Adonizedek", "Adonizedeque"},
    {"Adramelech", "Adrameleque"},
    {"Aholibamah", "Aholibama"},
    {"Alonbachut", "Alombacute"},
    {"Amalekitas", "Amalequitas"},
    {"Aquimelech", "Aquimeleque"},
    {"Aristobulo", "Aristóbulo"},
    {"Asar-hadon", "Asar-hadom"},
    {"Baal-berit", "Baal-berite"},
    {"Baal-hamon", "Baal-hamom"},
    {"Baal-zebub", "Baal-zebube"},
    {"Baal-zefon", "Baal-zefom"},
    {"Bamot-baal", "Bamote-baal"},
    {"Bat-arabim", "Bate-arabim"},
    {"Belshazzar", "Belsazar"},
    {"Bene-berak", "Bene-beraque"},
    {"Bet-acerem", "Bete-acerem"},
    {"Bet-cherem", "Bet-querem"},
    {"Bet-lebaot", "Bet-lebaote"},
    {"Bet-rechob", "Bet-recobe"},
    {"Concluimos", "Concluímos"},
    {"Filadelfia", "Filadélfia"},
    {"Hamolechet", "Hamolequet"},
    {"Hasarmavet", "Hazarmavet"},
    {"Hermogenes", "Hermógenes"},
    {"Hierapolis", "Hierápolis"},
    {"Isbi-benob", "Isbi-benobe"},
    {"Jesua-joab", "Jesua-joabe"},
    {"Lachai-roi", "Lahai-roi"},
    {"Machnadbai", "Macnadbai"},
    {"Mahalaleel", "Mahalalel"},
    {"Malchi-sua", "Malquisua"},
    {"Mehetabeel", "Mehetabel"},
    {"Merib-baal", "Meribe-baal"},
    {"Mezezabeel", "Mesezabeel"},
    {"Migdal-gad", "Migdal-gade"},
    {"Padan-aram", "Padã-arã"},
    {"Pahat-moab", "Paate-moabe"},
    {"Pi-hahirot", "Pi-hairote"},
    {"Pozeram-te", "Puseram-te"},
    {"Prohibindo", "Proibindo"},
    {"Proseguiam", "Prosseguiam"},
    {"Regemelech", "Regemeleque"},
    {"Rehobot-Ir", "Reobote-Ir"},
    {"Samotracia", "Samotrácia"},
    {"Senacherib", "Senaqueribe"},
    {"Senaquerib", "Senaqueribe"},
    {"Tiberiades", "Tiberíades"},
    {"Uzzen-sera", "Uzen-sera"},
    {"Abed-nego", "Abede-nego"},
    {"Abi-albon", "Abi-albom"},
    {"Achitofel", "Aquitofel"},
    {"Ahisamach", "Ahisamaque"},
    {"Alamelech", "Alameleque"},
    {"Amaldiçôa", "Amaldiçoa"},
    {"Andronico", "Andrônico"},
    {"Antiochia", "Antioquia"},
    {"Apromptai", "Aprontai"},
    {"Armagedon", "Armagedom"},
    {"Asincrito", "Assíncrito"},
    {"Baal-meon", "Baal-meom"},
    {"Bakbakkar", "Bacbacar"},
    {"Bakbukias", "Bacbuquias"},
    {"Baracheel", "Baraqueel"},
    {"Barachias", "Baraquias"},
    {"Ben-deker", "Ben-dequer"},
    {"Ben-hadad", "Ben-hadade"},
    {"Berechias", "Berequias"},
    {"Bet-araba", "Bete-araba"},
    {"Bet-arbel", "Bete-arbel"},
    {"Bet-lehem", "Belém"},
    {"Bet-palet", "Bet-pelete"},
    {"Bet-pelet", "Bet-pelete"},
    {"Capernaum", "Cafarnaum"},
    {"Carchemis", "Carquemis"},
    {"Chenanias", "Quenanias"},
    {"Decapolis", "Decápolis"},
    {"Dibon-gad", "Dibon-gade"},
    {"Emekkesis", "Emequesis"},
    {"Exhaustos", "Exaustos"},
    {"Genezaret", "Genesaré"},
    {"Ismachias", "Ismaquias"},
    {"Jeheiakim", "Jeheiaquim"},
    {"Jehizkias", "Jehizquias"},
    {"Jehoiarib", "Jehoiaribe"},
    {"Jekabseel", "Jecabseel"},
    {"Jerubeset", "Jerubesete"},
    {"Jeshoaias", "Jeshoaías"},
    {"Josbekasa", "Josbecasa"},
    {"Kir-heres", "Quir-heres"},
    {"Kiriataim", "Quiriataim"},
    {"Machbanai", "Macbanai"},
    {"Machpelah", "Macpela"},
    {"Malachias", "Malaquias"},
    {"Malchiram", "Malquiram"},
    {"Malchisua", "Malquisua"},
    {"Mardocheo", "Mardoqueu"},
    {"Mefiboset", "Mefibosete"},
    {"Mesezabel", "Mesezabeel"},
    {"Mesilemit", "Mesilemite"},
    {"Mesilemot", "Mesilemote"},
    {"Mesulemet", "Mesulemete"},
    {"Metusalah", "Matusalém"},
    {"Mikametat", "Micmetate"},
    {"Nafat-dor", "Nafate-dor"},
    {"Nafot-dor", "Nafote-dor"},
    {"Nicopolis", "Nicópolis"},
    {"Obedeecei", "Obedecei"},
    {"Peres-uza", "Perez-uza"},
    {"Petachias", "Petaquias"},
    {"Protector", "Protetor"},
    {"Sechanias", "Secanias"},
    {"Semachias", "Semaquias"},
    {"Semiramot", "Semiramote"},
    {"Shigionot", "Shigionote"},
    {"Tachemoni", "Taquemoni"},
    {"Tachfanes", "Tahpanhes"},
    {"Tachpenes", "Tacpenes"},
    {"Zerachias", "Zeraquias"},
    {"Abikeila", "Abiqueila"},
    {"Abinadab", "Abinadabe"},
    {"Achinoam", "Aquinoam"},
    {"Adonikam", "Adonicam"},
    {"Ahasuero", "Assuero"},
    {"Ahimelek", "Ahimeleque"},
    {"Ahinadab", "Ahinadabe"},
    {"Amomtoei", "Amontoei"},
    {"Amorrheo", "Amorreu"},
    {"Amorrheu", "Amorreu"},
    {"Anacarat", "Anacarate"},
    {"Apolonia", "Apolônia"},
    {"Archelau", "Arquelau"},
    {"Areopago", "Areópago"},
    {"Arimatea", "Arimateia"},
    {"Asquelon", "Ascalom"},
    {"Baal-gad", "Baal-gade"},
    {"Baalpeor", "Baal-peor"},
    {"Barrabas", "Barrabás"},
    {"Bartimeo", "Bartimeu"},
    {"Bat-seba", "Bate-seba"},
    {"Bechorat", "Becorate"},
    {"Beelzebú", "Beelzebu"},
    {"Benchail", "Benquail"},
    {"Benhadad", "Ben-hadade"},
    {"Benzohet", "Benzohete"},
    {"Bet-anat", "Bete-anate"},
    {"Bet-anot", "Bete-anote"},
    {"Bet-aram", "Bete-aram"},
    {"Bet-aven", "Bete-aven"},
    {"Bet-eked", "Bet-equede"},
    {"Bet-emek", "Bet-emeque"},
    {"Birzavit", "Birzavite"},
    {"Chaldeus", "Caldeus"},
    {"Chanaana", "Canaana"},
    {"Chelubai", "Quelubai"},
    {"Chenaana", "Quenaana"},
    {"Chesulot", "Quesulote"},
    {"Chineret", "Cinerete"},
    {"Chinerot", "Cinerote"},
    {"Chorazin", "Corazin"},
    {"Cubiçais", "Cobiçais"},
    {"Dalmacia", "Dalmácia"},
    {"Dionisio", "Dionísio"},
    {"Elifelet", "Elifelete"},
    {"Elihoref", "Elihorefe"},
    {"Filistia", "Filístia"},
    {"Filologo", "Filólogo"},
    {"Gomorrah", "Gomorra"},
    {"Gomorrha", "Gomorra"},
    {"Grangeai", "Granjeai"},
    {"grangeai", "granjeai"},
    {"Haldrach", "Haldraque"},
    {"Hamelech", "Hameleque"},
    {"Jecholia", "Jecolia"},
    {"Jehezkel", "Jehezquel"},
    {"Jekamias", "Jecamias"},
    {"Jekutiel", "Jecutiel"},
    {"Jesebeab", "Jesebeabe"},
    {"Jochebed", "Joquebede"},
    {"Jogbehah", "Jogbeha"},
    {"Kadmoneo", "Cadmoneu"},
    {"Kehelata", "Quehelata"},
    {"Kirheres", "Quir-heres"},
    {"Leviatan", "Leviatã"},
    {"Machbena", "Macbena"},
    {"Machpela", "Macpela"},
    {"Mahaziot", "Mahaziote"},
    {"Malchias", "Malquias"},
    {"Malchiel", "Malquiel"},
    {"Matusala", "Matusalém"},
    {"Melchias", "Melquias"},
    {"Merodach", "Merodaque"},
    {"Messulam", "Mesulam"},
    {"Metusala", "Matusalém"},
    {"Michaias", "Micaías"},
    {"Mikmetat", "Micmetate"},
    {"Mikneias", "Micneias"},
    {"Miniamin", "Miniamim"},
    {"Misperet", "Misperete"},
    {"Mitredat", "Mitredate"},
    {"Mitsraim", "Mizraim"},
    {"Naftuhim", "Naftuim"},
    {"Nahaliel", "Naaliel"},
    {"Nahamani", "Naamani"},
    {"Nahasson", "Naassom"},
    {"Nehemias", "Neemias"},
    {"Nehustan", "Neustã"},
    {"Petahias", "Petaías"},
    {"Prochoro", "Prócoro"},
    {"Puteolos", "Putéolos"},
    {"Quedemot", "Quedemote"},
    {"Rahmeses", "Rameses"},
    {"Ramesses", "Rameses"},
    {"Reelaias", "Reelaías"},
    {"Rehabias", "Reabias"},
    {"Restitue", "Restitui"},
    {"Saalabin", "Saalabim"},
    {"Sabtecha", "Sabteca"},
    {"Sahasima", "Saasima"},
    {"Salechet", "Salequete"},
    {"Sanbalat", "Sambalate"},
    {"Sanherib", "Sanheribe"},
    {"Seleucia", "Selêucia"},
    {"Semeiais", "Semeais"},
    {"Senazzar", "Senazar"},
    {"Shelomit", "Selomite"},
    {"Shemaias", "Semaías"},
    {"Sheshach", "Sesaque"},
    {"Sibechai", "Sibecai"},
    {"Sidonios", "Sidônios"},
    {"Sintiche", "Síntique"},
    {"Sostenes", "Sóstenes"},
    {"Tabrimon", "Tabrimom"},
    {"Tanhumet", "Tanhumete"},
    {"Tarshish", "Társis"},
    {"Tel-abib", "Tel-abibe"},
    {"Trogilio", "Trogílio"},
    {"Zedekias", "Zedequias"},
    {"Abiasaf", "Abiasafe"},
    {"Achaico", "Acaico"},
    {"Achmeta", "Acmeta"},
    {"Ahieser", "Ahiezer"},
    {"Aholiab", "Aholiabe"},
    {"Ahuzzam", "Ahuzam"},
    {"Ahuzzat", "Ahuzate"},
    {"Aijalon", "Aijalom"},
    {"Akrabim", "Acrabim"},
    {"Almodad", "Almodade"},
    {"Amomtoa", "Amontoa"},
    {"Apolion", "Apoliom"},
    {"A'quele", "Aquele"},
    {"Archipo", "Arquipo"},
    {"Ascalon", "Ascalom"},
    {"Askelon", "Ascalom"},
    {"Asmavet", "Azmavete"},
    {"Astarot", "Astarote"},
    {"Astoret", "Astorete"},
    {"Azmavet", "Azmavete"},
    {"Azrikam", "Azricam"},
    {"Barabás", "Barrabás"},
    {"Basemat", "Basemate"},
    {"Batseba", "Bate-seba"},
    {"Bat-sua", "Bate-sua"},
    {"Behemot", "Beemote"},
    {"Beracha", "Beraca"},
    {"Berodac", "Berodaque"},
    {"Besemat", "Basemate"},
    {"Betaven", "Bete-aven"},
    {"Betfage", "Betfagé"},
    {"Betoron", "Bet-horon"},
    {"Bet-sur", "Bet-zur"},
    {"Bukkias", "Buquias"},
    {"Chefira", "Quefira"},
    {"Chelbon", "Quelbon"},
    {"Cheluhi", "Queluhi"},
    {"Chenani", "Quenani"},
    {"Chileab", "Quileabe"},
    {"Chilion", "Quilion"},
    {"Chimham", "Quimham"},
    {"Chislon", "Quislon"},
    {"Chitlis", "Quitlis"},
    {"Cineret", "Cinerete"},
    {"Cinerot", "Cinerote"},
    {"Cirenio", "Cirênio"},
    {"Claudia", "Cláudia"},
    {"Daberat", "Daberate"},
    {"Dabeset", "Dabesete"},
    {"Damaris", "Dâmaris"},
    {"Ebiasaf", "Ebiasafe"},
    {"Eleazer", "Eleasar"},
    {"Eliasaf", "Eliasafe"},
    {"Eltekon", "Eltecon"},
    {"Epéneto", "Epêneto"},
    {"Euticho", "Êutico"},
    {"Hachila", "Haquila"},
    {"Hadoran", "Hadoram"},
    {"Havilah", "Havila"},
    {"Henadad", "Henadade"},
    {"Hezroma", "Hezrona"},
    {"Hidekel", "Hidequel"},
    {"Himeneo", "Himeneu"},
    {"instrue", "instrui"},
    {"Instrue", "Instrui"},
    {"Ishmael", "Ismael"},
    {"Jaakoba", "Jaacoba"},
    {"Jamlech", "Jamleque"},
    {"Jekamam", "Jecamam"},
    {"Jeremat", "Jeremate"},
    {"Jeremot", "Jeremote"},
    {"Jerimot", "Jerimote"},
    {"Jesaias", "Jesaías"},
    {"Joachaz", "Joacaz"},
    {"Joachim", "Joaquim"},
    {"Joachin", "Joaquim"},
    {"Joiarib", "Joiaribe"},
    {"Jojarib", "Joiaribe"},
    {"Jokdeam", "Jocdeam"},
    {"Jokmeam", "Jocmeam"},
    {"Jokneam", "Jocneam"},
    {"Jokteel", "Jocteel"},
    {"Jonadab", "Jonadabe"},
    {"Jonatan", "Jônatas"},
    {"Jorkeam", "Jorqueam"},
    {"Josadac", "Josadaque"},
    {"Josadak", "Josadaque"},
    {"Josebat", "Josebate"},
    {"Jozabad", "Jozabade"},
    {"Jozabat", "Jozabade"},
    {"Jupiter", "Júpiter"},
    {"Kabseel", "Cabseel"},
    {"Kadmiel", "Cadmiel"},
    {"Kazerim", "Cazerim"},
    {"Kedemot", "Quedemote"},
    {"Kelaias", "Quelaías"},
    {"Kelitas", "Quelitas"},
    {"Kenezeo", "Quenezeu"},
    {"Kesalon", "Quesalon"},
    {"Kibsaim", "Quibsaim"},
    {"Kolaias", "Colaías"},
    {"Koritas", "Coritas"},
    {"Kusaias", "Cusaías"},
    {"Lapidot", "Lapidote"},
    {"Maaseia", "Maaseias"},
    {"Mahalat", "Mahalate"},
    {"Mahalon", "Mahlon"},
    {"Manahat", "Manahate"},
    {"Maresha", "Maresa"},
    {"Masreka", "Masreca"},
    {"Mechona", "Mecona"},
    {"Melichu", "Melicu"},
    {"Meraias", "Meraías"},
    {"Meraiot", "Meraiote"},
    {"Meremot", "Meremote"},
    {"Meribah", "Meribá"},
    {"Mesobab", "Mesobabe"},
    {"Mesulum", "Mesulam"},
    {"Mezahab", "Mezahabe"},
    {"Michaia", "Micaia"},
    {"Micheas", "Miqueas"},
    {"Michmas", "Micmás"},
    {"Mihamin", "Miamim"},
    {"Miseria", "Miséria"},
    {"Moserot", "Moserote"},
    {"Nahalal", "Naalal"},
    {"Nahalol", "Naalol"},
    {"Naharai", "Naarai"},
    {"Naharat", "Naarate"},
    {"Napoles", "Nápoles"},
    {"Nebaiot", "Nebaiote"},
    {"Nebajot", "Nebaiote"},
    {"Nebalat", "Nebalate"},
    {"Neftoah", "Neftoa"},
    {"Nehusta", "Neusta"},
    {"Niniveh", "Nínive"},
    {"Nisroch", "Nisroque"},
    {"Olimpia", "Olímpia"},
    {"Onesimo", "Onésimo"},
    {"Pedaias", "Pedaías"},
    {"Pelaias", "Pelaías"},
    {"Pereseo", "Pereseu"},
    {"Pergamo", "Pérgamo"},
    {"Pibeset", "Pibesete"},
    {"Piraton", "Piratom"},
    {"Pisidia", "Pisídia"},
    {"Pozéste", "Puseste"},
    {"Queriot", "Queriote"},
    {"Rabsaké", "Rabsaqué"},
    {"Rafidim", "Refidim"},
    {"Refaias", "Refaías"},
    {"Rehobot", "Reobote"},
    {"Sachias", "Saquias"},
    {"Sadrach", "Sadraque"},
    {"Sahalim", "Saalim"},
    {"Sefarad", "Sefarade"},
    {"Selofad", "Selofade"},
    {"Selomit", "Selomite"},
    {"Selomot", "Selomote"},
    {"Semaias", "Semaías"},
    {"Seminit", "Seminite"},
    {"Seraias", "Seraías"},
    {"Sibolet", "Sibolete"},
    {"Soferet", "Soferete"},
    {"Taanach", "Taanaque"},
    {"Taberah", "Tabera"},
    {"Tertulo", "Tértulo"},
    {"Tichico", "Tíquico"},
    {"Tirhaká", "Tirhacá"},
    {"Trofimo", "Trófimo"},
    {"UFARSIN", "UFARSIM"},
    {"Zabulon", "Zebulom"},
    {"Zarefat", "Zarefate"},
    {"Zebulon", "Zebulom"},
    {"Zererat", "Zererate"},
    {"Zohelet", "Zohelete"},
    {"Abadon", "Abadom"},
    {"Abidah", "Abida"},
    {"Abihud", "Abihude"},
    {"Abisag", "Abisague"},
    {"Abitud", "Abitude"},
    {"Achaia", "Acaia"},
    {"Achbor", "Acbor"},
    {"Achsaf", "Acsafe"},
    {"Achzib", "Aczibe"},
    {"Adamah", "Adama"},
    {"Ahihud", "Ahihude"},
    {"Ahikam", "Ahicam"},
    {"Ahilud", "Ahilude"},
    {"Ahimot", "Ahimote"},
    {"Ahitub", "Ahitube"},
    {"Aialon", "Aijalom"},
    {"Ajalon", "Aijalom"},
    {"Alamet", "Alamete"},
    {"Alamot", "Alamote"},
    {"Alemet", "Alemete"},
    {"Amalek", "Amaleque"},
    {"Amelek", "Amaleque"},
    {"Anatot", "Anatote"},
    {"Arioch", "Arioque"},
    {"Arubot", "Arubote"},
    {"Asenat", "Asenate"},
    {"Atarot", "Atarote"},
    {"Baalat", "Baalate"},
    {"Baalin", "Baalim"},
    {"Bakbuk", "Bacbuque"},
    {"Bariah", "Baria"},
    {"Baslit", "Baslite"},
    {"Baslut", "Baslute"},
    {"Basmat", "Basemate"},
    {"Bealot", "Bealote"},
    {"Bearot", "Bearote"},
    {"Becher", "Bequer"},
    {"Beerot", "Beerote"},
    {"Beriah", "Beria"},
    {"Berish", "Beris"},
    {"Bet-el", "Betel"},
    {"Betzur", "Bet-zur"},
    {"Bichri", "Bicri"},
    {"Bidkar", "Bidcar"},
    {"Bildad", "Bildade"},
    {"Bilhah", "Bilha"},
    {"Bochim", "Boquim"},
    {"Boscat", "Boscate"},
    {"Boskat", "Boscate"},
    {"Bosrah", "Bosra"},
    {"Calneh", "Calne"},
    {"Carmel", "Carmelo"},
    {"Chamoz", "Camos"},
    {"Chebar", "Quebar"},
    {"Chelal", "Quelal"},
    {"Chelba", "Quelba"},
    {"Chelub", "Quelube"},
    {"Cheran", "Queran"},
    {"Cheret", "Querete"},
    {"Cherub", "Querube"},
    {"Chesed", "Quesede"},
    {"Chesil", "Quesil"},
    {"Chezib", "Quezibe"},
    {"Chidon", "Quidon"},
    {"Cholon", "Colon"},
    {"Darkon", "Darcon"},
    {"Debora", "Débora"},
    {"Diblat", "Diblate"},
    {"Diclah", "Dicla"},
    {"Didimo", "Dídimo"},
    {"Dishan", "Disan"},
    {"Dishon", "Dison"},
    {"Dobrat", "Dobrate"},
    {"Eisque", "Eis que"},
    {"Elidad", "Elidade"},
    {"Elifsz", "Elifaz"},
    {"Elkana", "Elcana"},
    {"Elteke", "Elteque"},
    {"Eshban", "Esban"},
    {"Estmoa", "Estemoa"},
    {"Evodia", "Evódia"},
    {"Fichol", "Ficol"},
    {"Finéas", "Fineas"},
    {"Galaad", "Gileade"},
    {"Galeed", "Galeede"},
    {"Gibeah", "Gibea"},
    {"Gilead", "Gileade"},
    {"Goliat", "Golias"},
    {"Goshen", "Gósen"},
    {"Gudgod", "Gudgode"},
    {"Hakkos", "Hacos"},
    {"Hakufa", "Hacufa"},
    {"Hanoch", "Hanoque"},
    {"Hassub", "Hassube"},
    {"Hatach", "Hataque"},
    {"Havvim", "Havim"},
    {"Helkai", "Helcai"},
    {"Helkat", "Helcat"},
    {"Iarmut", "Jarmut"},
    {"Icabod", "Icabode"},
    {"Idumea", "Idumeia"},
    {"Jaakan", "Jaacan"},
    {"Jachan", "Jacan"},
    {"Jachin", "Jaquin"},
    {"Janoah", "Janoa"},
    {"Jaroah", "Jaroa"},
    {"Jefone", "Jefoné"},
    {"Jeriot", "Jeriote"},
    {"Jisbac", "Jisbaque"},
    {"Jisbak", "Jisbaque"},
    {"Johana", "Joana"},
    {"Joksan", "Jocsan"},
    {"Joktan", "Joctan"},
    {"Jotbat", "Jotbate"},
    {"Juchal", "Jucal"},
    {"JUDEOS", "JUDEUS"},
    {"Kareat", "Careate"},
    {"Kedmot", "Quedmote"},
    {"Kelita", "Quelita"},
    {"Kemuel", "Quemuel"},
    {"Keriot", "Queriote"},
    {"Ketura", "Quetura"},
    {"Kilmad", "Quilmade"},
    {"Kiriat", "Quiriate"},
    {"Kision", "Quision"},
    {"Kisjon", "Quision"},
    {"Kitron", "Quitron"},
    {"Lachis", "Laquis"},
    {"Lebaot", "Lebaote"},
    {"Lisias", "Lísias"},
    {"Maacah", "Maaca"},
    {"Maacat", "Maacate"},
    {"Maacha", "Maaca"},
    {"Maarat", "Maarate"},
    {"Machir", "Maquir"},
    {"Machla", "Macla"},
    {"Machli", "Maheli"},
    {"Mahali", "Maheli"},
    {"Makeda", "Maqueda"},
    {"Maluch", "Maluque"},
    {"Mashit", "Mashite"},
    {"Massah", "Massá"},
    {"Matred", "Matrede"},
    {"Mefaat", "Mefaate"},
    {"Melchi", "Melqui"},
    {"Melech", "Meleque"},
    {"Memfis", "Mênfis"},
    {"Mesach", "Mesaque"},
    {"Mesech", "Meseque"},
    {"Michal", "Mical"},
    {"Michri", "Micri"},
    {"Micmas", "Micmás"},
    {"Midian", "Midiã"},
    {"Mifkad", "Mifcade"},
    {"Migron", "Migrom"},
    {"Miklot", "Miclote"},
    {"Milcah", "Milca"},
    {"Milkom", "Milcom"},
    {"Misgab", "Misgabe"},
    {"Mispah", "Mispa"},
    {"Mispeh", "Mispe"},
    {"Mizpah", "Mispa"},
    {"Mizzah", "Miza"},
    {"Mnason", "Mnasom"},
    {"Molech", "Moloque"},
    {"Moloch", "Moloque"},
    {"Naaman", "Naamã"},
    {"Naaran", "Naarã"},
    {"Naason", "Naassom"},
    {"Nachon", "Nacom"},
    {"Nachor", "Naor"},
    {"Nekoda", "Necoda"},
    {"Nemrod", "Ninrode"},
    {"Nesiag", "Nesiague"},
    {"Nesiah", "Nesia"},
    {"Nibsan", "Nibsã"},
    {"Nimrod", "Ninrode"},
    {"Ninive", "Nínive"},
    {"Ochran", "Ocrã"},
    {"Orfãos", "Órfãos"},
    {"Parnah", "Parna"},
    {"Paruah", "Parua"},
    {"Pasach", "Pasaque"},
    {"Paseah", "Pasea"},
    {"Pashur", "Pasur"},
    {"Pekaia", "Pecaia"},
    {"Peseah", "Pesea"},
    {"Pôe-na", "Põe-na"},
    {"Porcio", "Pórcio"},
    {"Publio", "Públio"},
    {"Quenat", "Quenate"},
    {"Raamah", "Raama"},
    {"Rachal", "Racal"},
    {"Rachel", "Raquel"},
    {"Reaias", "Reaías"},
    {"Rechab", "Recabe"},
    {"Rechob", "Recobe"},
    {"Rehuel", "Reuel"},
    {"Remfan", "Renfã"},
    {"Rhegio", "Régio"},
    {"Rhehum", "Reum"},
    {"Rhodes", "Rodes"},
    {"Rohega", "Roega"},
    {"Ruhama", "Ruama"},
    {"Sabtah", "Sabta"},
    {"Salcha", "Salca"},
    {"Samhut", "Samhute"},
    {"Samlah", "Samla"},
    {"Sargon", "Sargom"},
    {"Saruch", "Saruque"},
    {"Scheba", "Sheba"},
    {"Sechem", "Sequém"},
    {"Sechen", "Sequém"},
    {"Seirat", "Seirate"},
    {"Semaia", "Semaía"},
    {"Sesach", "Sesaque"},
    {"Shalum", "Salum"},
    {"Shelah", "Selá"},
    {"Shelef", "Selefe"},
    {"Shilem", "Silem"},
    {"Shiloh", "Siló"},
    {"Shinab", "Shinabe"},
    {"Shinar", "Sinear"},
    {"Shobab", "Sobabe"},
    {"Shobal", "Sobal"},
    {"Sichem", "Siquém"},
    {"Sichor", "Sicor"},
    {"Sicron", "Sicrom"},
    {"Sifmot", "Sifmote"},
    {"Siklag", "Siclag"},
    {"Silchi", "Silqui"},
    {"Simeat", "Simeate"},
    {"Simeon", "Simeão"},
    {"Simrat", "Simrate"},
    {"Simret", "Simrete"},
    {"Simron", "Simrom"},
    {"Sirion", "Siriom"},
    {"Sisera", "Sísera"},
    {"Smirna", "Esmirna"},
    {"Sobach", "Sobaque"},
    {"Taanak", "Taanaque"},
    {"Tabaot", "Tabaote"},
    {"Tachat", "Taquate"},
    {"Talmon", "Talmom"},
    {"Tanach", "Tanaque"},
    {"Tartak", "Tartaque"},
    {"Tibhat", "Tibhate"},
    {"Timnah", "Timna"},
    {"Timnat", "Timnate"},
    {"Tochen", "Toquen"},
    {"Tokhat", "Tocate"},
    {"Ussiel", "Uziel"},
    {"Uzziel", "Uziel"},
    {"Zachur", "Zacur"},
    {"Zalmon", "Zalmom"},
    {"Zarchi", "Zarqui"},
    {"Zecher", "Zequer"},
    {"Zibeon", "Zibeom"},
    {"Zichri", "Zicri"},
    {"Zifion", "Zifiom"},
    {"Zifron", "Zifrom"},
    {"Ziklag", "Siclag"},
    {"Zilpah", "Zilpa"},
    {"Zipora", "Zípora"},
    {"Achab", "Acabe"},
    {"Achor", "Acor"},
    {"Acsaf", "Acsafe"},
    {"Aczib", "Aczibe"},
    {"Admah", "Adma"},
    {"Adnah", "Adna"},
    {"Afeka", "Afeca"},
    {"Ahoah", "Ahoa"},
    {"Ahrah", "Ahra"},
    {"Akkub", "Acube"},
    {"Alfeo", "Alfeu"},
    {"Almon", "Almom"},
    {"Amnon", "Amnom"},
    {"Apfia", "Afia"},
    {"Ardon", "Ardom"},
    {"Arfad", "Arpade"},
    {"Argob", "Argobe"},
    {"Arieh", "Arie"},
    {"Arkeu", "Arqueu"},
    {"Arnon", "Arnom"},
    {"Arpad", "Arpade"},
    {"Arvad", "Arvade"},
    {"Asdod", "Asdode"},
    {"Aserá", "Asera"},
    {"Asgad", "Azgade"},
    {"Asher", "Aser"},
    {"Ashur", "Asur"},
    {"Asmon", "Azmom"},
    {"Assás", "Assaz"},
    {"Asson", "Assom"},
    {"Asvat", "Asvate"},
    {"Atrot", "Atrote"},
    {"Azaka", "Azeca"},
    {"Azbuk", "Azbuque"},
    {"Azeka", "Azeca"},
    {"Azgad", "Azgade"},
    {"Azmon", "Azmom"},
    {"Baása", "Baasa"},
    {"Baiit", "Baiite"},
    {"Balac", "Balaque"},
    {"Balak", "Balaque"},
    {"Bamah", "Bama"},
    {"Bamot", "Bamote"},
    {"Barac", "Baraque"},
    {"Bedad", "Bedade"},
    {"Behon", "Behom"},
    {"Belah", "Bela"},
    {"Bered", "Berede"},
    {"Berit", "Berite"},
    {"Bezek", "Bezeque"},
    {"Bukki", "Buqui"},
    {"Caleb", "Calebe"},
    {"Carit", "Carite"},
    {"Catat", "Catate"},
    {"Chloe", "Cloé"},
    {"Chuza", "Cuza"},
    {"Dalet", "Dalete"},
    {"Démas", "Demas"},
    {"Dikla", "Dicla"},
    {"Dinah", "Diná"},
    {"Dofka", "Dofca"},
    {"Efrat", "Efrate"},
    {"Ekron", "Ecron"},
    {"Eldad", "Eldade"},
    {"Eldah", "Elda"},
    {"Elead", "Eleade"},
    {"Eliab", "Eliabe"},
    {"Eliad", "Eliade"},
    {"Elika", "Elica"},
    {"Emaus", "Emaús"},
    {"Eneas", "Eneias"},
    {"Erech", "Ereque"},
    {"Galio", "Gálio"},
    {"Gareb", "Garebe"},
    {"Gazer", "Gezer"},
    {"Geser", "Gezer"},
    {"Golan", "Golã"},
    {"Gosen", "Gósen"},
    {"Hacub", "Hacube"},
    {"Hadad", "Hadade"},
    {"Hadid", "Hadide"},
    {"Hagab", "Hagabe"},
    {"Halah", "Hala"},
    {"Harad", "Harade"},
    {"Harod", "Harode"},
    {"Hasub", "Hassube"},
    {"Heleb", "Helebe"},
    {"Heled", "Helede"},
    {"Helek", "Heleque"},
    {"Hirah", "Hira"},
    {"Hobab", "Hobabe"},
    {"Hobah", "Hoba"},
    {"Hucok", "Hucoque"},
    {"Hukok", "Hucoque"},
    {"Iftah", "Ifta"},
    {"Ikkes", "Iques"},
    {"Imnah", "Imna"},
    {"India", "Índia"},
    {"Isbah", "Isba"},
    {"Iscah", "Isca"},
    {"Ishod", "Ishode"},
    {"Isrel", "Israel"},
    {"Jabok", "Jaboque"},
    {"Jafet", "Jafé"},
    {"Jahas", "Jahaz"},
    {"Jakim", "Jaquim"},
    {"Jarah", "Jara"},
    {"Jareb", "Jarebe"},
    {"Jarib", "Jaribe"},
    {"Jasib", "Jasibe"},
    {"Jasub", "Jasube"},
    {"Jehud", "Jehude"},
    {"Jerah", "Jerá"},
    {"Jered", "Jerede"},
    {"Jesse", "Jessé"},
    {"Jetet", "Jetete"},
    {"Jeush", "Jeús"},
    {"Jobab", "Jobabe"},
    {"Johat", "Johate"},
    {"Jokim", "Joquim"},
    {"Judit", "Judite"},
    {"Julia", "Júlia"},
    {"Junia", "Júnia"},
    {"Kades", "Cades"},
    {"Kalai", "Calai"},
    {"Katan", "Catan"},
    {"Kedar", "Quedar"},
    {"Kedes", "Quedes"},
    {"Kedma", "Quedma"},
    {"Keila", "Queila"},
    {"Kenas", "Quenaz"},
    {"Kenat", "Quenate"},
    {"Kenaz", "Quenaz"},
    {"Kenez", "Quenaz"},
    {"Keros", "Queros"},
    {"Kison", "Quison"},
    {"Kitim", "Quitim"},
    {"Kohat", "Coate"},
    {"Korah", "Corá"},
    {"Lahad", "Lahade"},
    {"Lakum", "Lacum"},
    {"Lebeo", "Lebeu"},
    {"Lecha", "Leca"},
    {"Lechi", "Lequi"},
    {"Lidia", "Lídia"},
    {"Likhi", "Liqui"},
    {"Lucio", "Lúcio"},
    {"Luhit", "Luhite"},
    {"Maaka", "Maaca"},
    {"Machi", "Maqui"},
    {"Mahat", "Mahate"},
    {"Mahon", "Mahlon"},
    {"Makas", "Macas"},
    {"Maoch", "Maoque"},
    {"Marah", "Mara"},
    {"Marot", "Marote"},
    {"Matãa", "Matana"},
    {"Medad", "Medade"},
    {"Merab", "Merabe"},
    {"Mered", "Merede"},
    {"Micah", "Mica"},
    {"Midin", "Midim"},
    {"Milka", "Milca"},
    {"Minit", "Minite"},
    {"Mitka", "Mitca"},
    {"Molid", "Molide"},
    {"Nabot", "Nabote"},
    {"Nadab", "Nadabe"},
    {"Nahas", "Naás"},
    {"Nahat", "Naate"},
    {"Nahor", "Naor"},
    {"Nahum", "Naum"},
    {"Naiot", "Naiote"},
    {"Nebat", "Nebate"},
    {"Necho", "Neco"},
    {"Nefeg", "Nefegue"},
    {"Nehum", "Neum"},
    {"Nekeb", "Nequebe"},
    {"Netan", "Netã"},
    {"Nezib", "Nezibe"},
    {"Nimfa", "Ninfa"},
    {"Nobah", "Noba"},
    {"Nodab", "Nodabe"},
    {"Nofah", "Nofa"},
    {"Nogah", "Noga"},
    {"Nogat", "Nogate"},
    {"Ornan", "Ornã"},
    {"Padan", "Padã"},
    {"Padon", "Padom"},
    {"Paran", "Parã"},
    {"Pecod", "Pecode"},
    {"Peleg", "Pelegue"},
    {"Pelet", "Pelete"},
    {"Pinon", "Pinom"},
    {"Pison", "Pisom"},
    {"Piton", "Pitom"},
    {"Polux", "Pólux"},
    {"Rabit", "Rabite"},
    {"Racat", "Racate"},
    {"Racon", "Racom"},
    {"Raema", "Raama"},
    {"Ramah", "Rama"},
    {"Ramat", "Ramate"},
    {"Ramot", "Ramote"},
    {"Recab", "Recabe"},
    {"Recob", "Recobe"},
    {"Refah", "Refa"},
    {"Rehob", "Reobe"},
    {"Rehum", "Reum"},
    {"Rehun", "Reum"},
    {"Rekab", "Recabe"},
    {"Rekem", "Requem"},
    {"Remet", "Remete"},
    {"Resef", "Resefe"},
    {"Resin", "Resim"},
    {"Rezon", "Rezom"},
    {"Rhesa", "Resa"},
    {"Rhode", "Rode"},
    {"Rifat", "Rifate"},
    {"Rimon", "Rimom"},
    {"Safan", "Safã"},
    {"Safat", "Safate"},
    {"Safon", "Safom"},
    {"Salem", "Salém"},
    {"Samot", "Samote"},
    {"Saraf", "Sarafe"},
    {"Sarid", "Saride"},
    {"Saron", "Sarom"},
    {"Sasak", "Sasaque"},
    {"Sebat", "Sebate"},
    {"Segub", "Segube"},
    {"Sehon", "Seom"},
    {"Selah", "Selá"},
    {"Seled", "Selede"},
    {"Selef", "Selefe"},
    {"Sered", "Serede"},
    {"Serug", "Serugue"},
    {"Shuah", "Suah"},
    {"Shuni", "Suni"},
    {"Sidon", "Sidom"},
    {"Sihon", "Seom"},
    {"Sisak", "Sisaque"},
    {"Sobab", "Sobabe"},
    {"Sobek", "Sobeque"},
    {"Socho", "Soco"},
    {"Sochó", "Socó"},
    {"Socoh", "Soco"},
    {"Sofac", "Sofaque"},
    {"Sorec", "Soreque"},
    {"Sucot", "Sucote"},
    {"Tabat", "Tabate"},
    {"Tafat", "Tafate"},
    {"Tahat", "Tahate"},
    {"TEKEL", "TEQUEL"},
    {"Tekoa", "Tecoa"},
    {"Terah", "Terá"},
    {"Tikva", "Ticva"},
    {"Tilon", "Tilom"},
    {"Tofet", "Tofete"},
    {"Tolad", "Tolade"},
    {"Tsora", "Zora"},
    {"Vaheb", "Vahebe"},
    {"Zabad", "Zabade"},
    {"Zabud", "Zabude"},
    {"Zadoc", "Zadoque"},
    {"Zadok", "Zadoque"},
    {"Zalaf", "Zalafe"},
    {"Zedad", "Zedade"},
    {"Zefat", "Zefate"},
    {"Zefon", "Zefom"},
    {"Zelek", "Zeleque"},
    {"Zered", "Zerede"},
    {"Zeret", "Zerete"},
    {"Zidon", "Sidom"},
    {"Zohet", "Zohete"},
    {"Abib", "Abibe"},
    {"Acab", "Acabe"},
    {"Acad", "Acade"},
    {"Adah", "Ada"},
    {"Adon", "Adom"},
    {"Afek", "Afeque"},
    {"Afik", "Afique"},
    {"Agag", "Agague"},
    {"Ahab", "Acabe"},
    {"Aiat", "Aiate"},
    {"Ajah", "Aja"},
    {"Alef", "Álefe"},
    {"Alon", "Alom"},
    {"Alot", "Alote"},
    {"Amad", "Amade"},
    {"Amok", "Amoque"},
    {"Anab", "Anabe"},
    {"Anat", "Anate"},
    {"Anok", "Anoque"},
    {"Anub", "Anube"},
    {"Apio", "Ápio"},
    {"Arad", "Arade"},
    {"Arod", "Arode"},
    {"Asaf", "Asafe"},
    {"Avit", "Avite"},
    {"Azza", "Aza"},
    {"Chen", "Quen"},
    {"Chun", "Cun"},
    {"Doeg", "Doegue"},
    {"Ebad", "Ebade"},
    {"Ebed", "Ebede"},
    {"Echi", "Equi"},
    {"Efod", "Éfode"},
    {"Ehud", "Eúde"},
    {"Eker", "Equer"},
    {"Elat", "Elate"},
    {"Elef", "Elefe"},
    {"Elod", "Elode"},
    {"Elot", "Elote"},
    {"Enac", "Enaque"},
    {"Enak", "Enaque"},
    {"Esek", "Eseque"},
    {"Ezzi", "Ezi"},
    {"Gaas", "Gaás"},
    {"Giah", "Gia"},
    {"Goah", "Goa"},
    {"Irad", "Irade"},
    {"Jake", "Jaque"},
    {"Jehu", "Jeú"},
    {"Jeus", "Jeús"},
    {"Joab", "Joabe"},
    {"Joah", "Joá"},
    {"Joed", "Joede"},
    {"Juda", "Judá"},
    {"Kina", "Quina"},
    {"Kisi", "Quisi"},
    {"Koat", "Coate"},
    {"Kore", "Core"},
    {"Leah", "Lia"},
    {"Meah", "Meá"},
    {"Moab", "Moabe"},
    {"Nain", "Naim"},
    {"Obot", "Obote"},
    {"Oded", "Odede"},
    {"Ohad", "Oade"},
    {"Onan", "Onã"},
    {"Oreb", "Orebe"},
    {"Peka", "Peca"},
    {"Puah", "Pua"},
    {"Rehu", "Reú"},
    {"Saaf", "Saafe"},
    {"Saba", "Sabá"},
    {"Sion", "Siom"},
    {"Uzza", "Uza"},
    {"Uzzi", "Uzi"},
    {"Zeeb", "Zeebe"},
    {"Bet", "Bete"},
    {"Cub", "Cube"},
    {"Gad", "Gade"},
    {"Gob", "Gobe"},
    {"Hod", "Hode"},
    {"Iob", "Iobe"},
    {"Job", "Jó"},
    {"Kir", "Quir"},
    {"Kis", "Quis"},
    {"Kos", "Cós"},
    {"Kós", "Cós"},
    {"Lod", "Lode"},
    {"Lot", "Ló"},
    {"Lud", "Lude"},
    {"Mea", "Meá"},
    {"Nob", "Nobe"},
    {"Nod", "Node"},
    {"Nof", "Nofe"},
    {"Ólá", "Olá"},
    {"Poz", "Pôs"},
    {"Put", "Pute"},
    {"Saf", "Safe"},
    {"Suf", "Sufe"},
    {"Tob", "Tobe"},
    {"Zif", "Zife"},
    {"Zin", "Zim"},
    {"Ziv", "Zive"},
    {"Zuf", "Zufe"},
    {"Ed", "Ede"},
    {"Og", "Ogue"},
    {"On", "Om"},
        };
        int total_nomes = (int)(sizeof(nomes_proprios) / sizeof(nomes_proprios[0]));
        for (int z = 0; z < total_nomes; z++)
            substituir_palavra_inteira(texto, nomes_proprios[z][0], nomes_proprios[z][1]);
    }

    /* 12. REVISÃO DOS VERSÍCULOS (27/11/2026)
       Revisão de todos os 31.104 versículos (Claude Sonnet 5.5) com segunda opinião (Claude Opus 5.5).
       Tipo 1 = palavra inteira, quando TODAS as ocorrências na Bíblia foram aprovadas.
       Tipo 0 = trecho com as palavras vizinhas, que só existe nos versículos aprovados.
       Fonte: biblia-build/versos_rev/opus/decisoes*.tsv */
    {
        static const struct { const char *antigo; const char *novo; int inteira; } revisao[] = {
    {"Eis que veem dias", "Eis que vêm dias", 0}, {"eis que veem dias", "eis que vêm dias", 0},
    {"capiteis", "capitéis", 1},
    {"e as suas utensílios em casa, não desça a tomá-las; e,", "e os seus utensílios em casa, não desça a tomá-los; e,", 0},
    {"Porque eis que dias veem, diz o Senhor, em", "Porque eis que dias vêm, diz o Senhor, em", 0},
    {"deleitando-se seus enganos, em ainda que", "deleitando-se em seus enganos, ainda que", 0},
    {"disse-lhe: Porque não fizeste inquirição", "disse-lhe: Por que não fizeste inquirição", 0},
    {"disse-lhes: Porque me tentais? trazei-me", "disse-lhes: Por que me tentais? trazei-me", 0},
    {"e me lançaste para traz das tuas costas,", "e me lançaste para trás das tuas costas,", 0},
    {"lhe disse: Porque me chamas bom? ninguém", "lhe disse: Por que me chamas bom? ninguém", 0},
    {"os globos dos capiteis que estavam sobre", "os globos dos capitéis que estavam sobre", 0},
    {"disse Jesus: Em verdade vos digo que um", "disse Jesus: Em verdade vós digo que um", 0},
    {"disse: Dá-me uma benção; pois me deste", "disse: Dá-me uma bênção; pois me deste", 0},
    {"me lançaste para traz das tuas costas;", "me lançaste para trás das tuas costas;", 0},
    {"assolação, nem tão pouco haverá coisa", "assolação, nem tampouco haverá coisa", 0},
    {"fortaleza; porque me rejeitas? porque", "fortaleza; por que me rejeitas? porque", 0},
    {"isso eis que dias veem, diz o Senhor,", "isso eis que dias vêm, diz o Senhor,", 0},
    {"oferecia oferta voluntaria ao Senhor:", "oferecia oferta voluntária ao Senhor:", 0},
    {"subsistirá, nem tão pouco acontecerá.", "subsistirá, nem tampouco acontecerá.", 0},
    {"alguma das vossas utensílios; porque", "alguma dos vossos utensílios; porque", 0},
    {"coração: Porque me sobrevieram estas", "coração: Por que me sobrevieram estas", 0},
    {"Deus, porque me desamparaste? porque", "Deus, por que me desamparaste? porque", 0},
    {"oferta voluntaria de holocaustos, ou", "oferta voluntária de holocaustos, ou", 0},
    {"temor, nem tão pouco vos assombreis.", "temor, nem tampouco vos assombreis.", 0},
    {"estavam: Tiras os deuses estranhos,", "estavam: Tirai os deuses estranhos,", 0},
    {"cala-te, porque te feririam? então", "cala-te, por que te feririam? então", 0},
    {"desamparaste? porque te alongas do", "desamparaste? por que te alongas do", 0},
    {"excederão as bençãos de meus pais,", "excederão as bênçãos de meus pais,", 0},
    {"oferta voluntaria ao Senhor, então", "oferta voluntária ao Senhor, então", 0},
    {"porque lançaste para traz das tuas", "porque lançaste para trás das tuas", 0},
    {"Senhor, nem tão pouco vos rebeleis", "Senhor, nem tampouco vos rebeleis", 0},
    {"deserto? porque nos fizeste isto,", "deserto? por que nos fizeste isto,", 0},
    {"disse-lhes: Porque temeis, homens", "disse-lhes: Por que temeis, homens", 0},
    {"dizer: Porque o perseguimos? Pois", "dizer: Por que o perseguimos? Pois", 0},
    {"estes: Porque não acabastes vossa", "estes: Por que não acabastes vossa", 0},
    {"nem tão pouco quem manifeste, nem", "nem tampouco quem manifeste, nem", 0},
    {"peleja: nem tão pouco a impiedade", "peleja: nem tampouco a impiedade", 0},
    {"terra, o aos sacerdotes, dizendo:", "terra, e aos sacerdotes, dizendo:", 0},
    {"vestido, porque andais solícitos?", "vestido, por que andais solícitos?", 0},
    {"Arnom, porque o não recuperastes", "Arnom, por que o não recuperastes", 0},
    {"casa; porque te entremeterias no", "casa; por que te entremeterias no", 0},
    {"Desperta, porque dormes, Senhor?", "Desperta, por que dormes, Senhor?", 0},
    {"disse: Porque contendeis comigo?", "disse: Por que contendeis comigo?", 0},
    {"Eis que dias veem, diz o Senhor,", "Eis que dias vêm, diz o Senhor,", 0}, {"eis que dias veem, diz o Senhor,", "eis que dias vêm, diz o Senhor,", 0},
    {"fosse, porque se não angustiaria", "fosse, por que se não angustiaria", 0},
    {"herança, come o Senhor ordenara,", "herança, como o Senhor ordenara,", 0},
    {"mensageiros que veem a Jerusalém", "mensageiros que vêm a Jerusalém", 0},
    {"palavra: assim ficou mulhendo, e", "palavra: assim ficou a mulher, e", 0},
    {"profetas, tão pouco acreditarão,", "profetas, tampouco acreditarão,", 0},
    {"rejeitas? porque ando lamentando", "rejeitas? por que ando lamentando", 0},
    {"segundo, nem tão pouco filho nem", "segundo, nem tampouco filho nem", 0},
    {"sempre? porque nos desampararias", "sempre? por que nos desampararias", 0},
    {"todo o de mais sangue derramarás", "todo o demais sangue derramarás", 0},
    {"dele; tão pouco os descendentes", "dele; tampouco os descendentes", 0},
    {"disse: Porque nos fizeste subir", "disse: Por que nos fizeste subir", 0},
    {"disseram-lhe: Porque lhes falas", "disseram-lhe: Por que lhes falas", 0},
    {"espírito; nem tão pouco tem ele", "espírito; nem tampouco tem ele", 0},
    {"nenhuma, nem tão pouco têm eles", "nenhuma, nem tampouco têm eles", 0},
    {"a preza de abundantes despojos", "a presa de abundantes despojos", 0},
    {"anunciar? porque razão pereceu", "anunciar? por que razão pereceu", 0},
    {"chamar-te? porque não vieste a", "chamar-te? por que não vieste a", 0},
    {"cobrir os capiteis que estavam", "cobrir os capitéis que estavam", 0},
    {"consolação a que lhes causava.", "consolação que lhes causava.", 0},
    {"Conta todo e primogênito macho", "Conta todo o primogênito macho", 0},
    {"devoção voluntaria, humildade,", "devoção voluntária, humildade,", 0},
    {"disse-lhes: Porque fazeis tais", "disse-lhes: Por que fazeis tais", 0},
    {"disseram: Porque fizemos isso,", "disseram: Por que fizemos isso,", 0},
    {"Israel, nem tão pouco dos reis", "Israel, nem tampouco dos reis", 0},
    {"nem tão pouco dos entendidos o", "nem tampouco dos entendidos o", 0},
    {"nem tão pouco dos prudentes as", "nem tampouco dos prudentes as", 0},
    {"ou injuria por injuria; antes,", "ou injúria por injúria; antes,", 0},
    {"pois, porque morreríamos? pois", "pois, por que morreríamos? pois", 0},
    {"Rocha: Porque te esqueceste de", "Rocha: Por que te esqueceste de", 0},
    {"saber porque contendes comigo.", "saber por que contendes comigo.", 0},
    {"Samaria; e Benjamin a Gileade.", "Samaria; e Benjamim a Gileade.", 0},
    {"segundo as sua gerações, foram", "segundo as suas gerações, foram", 0},
    {"Senhor? Porque te escondes nos", "Senhor? Por que te escondes nos", 0},
    {"Abraão, da tribo de Benjamin.", "Abraão, da tribo de Benjamim.", 0},
    {"Acazias: Traição ha, Acazias.", "Acazias: Traição há, Acazias.", 0},
    {"apostasia tão continua? retem", "apostasia tão contínua? retem", 0},
    {"contínua? retem o engano, não", "contínua? retém o engano, não", 0},
    {"dada subsistência continua, a", "dada subsistência contínua, a", 0},
    {"deles: porque não lhas destes", "deles: por que não lhas destes", 0},
    {"Deus? porque seremos desleais", "Deus? por que seremos desleais", 0},
    {"disse: Porque buscaste deuses", "disse: Por que buscaste deuses", 0},
    {"disse: Porque perguntas assim", "disse: Por que perguntas assim", 0},
    {"disse-lhe: Porque achei graça", "disse-lhe: Por que achei graça", 0},
    {"disserem: Porque suspiras tu?", "disserem: Por que suspiras tu?", 0},
    {"Dizendo: Porque jejuamos nós,", "Dizendo: Por que jejuamos nós,", 0},
    {"escribas: Porque não andam os", "escribas: Por que não andam os", 0},
    {"filhas, porque iríeis comigo?", "filhas, por que iríeis comigo?", 0},
    {"fino, e purpura, e escarlata;", "fino, e púrpura, e escarlata;", 0},
    {"Josué, e disserem: Porque com", "Josué, e disseram: Porque com", 0},
    {"perguntaram-lhe: Pois que? És", "perguntaram-lhe: Pois quê? És", 0},
    {"Porque te glórias na malícia,", "Porque te glorias na malícia,", 0},
    {"pronunciarás a benção sobre o", "pronunciarás a bênção sobre o", 0},
    {"Senhor, nem tão pouco a obra,", "Senhor, nem tampouco a obra,", 0},
    {"a perseverança e suplica por", "a perseverança e súplica por", 0},
    {"agora, nem tão pouco haverá.", "agora, nem tampouco haverá.", 0},
    {"angústia! porque serias como", "angústia! por que serias como", 0},
    {"azul, e purpura, e carmesim,", "azul, e púrpura, e carmesim,", 0},
    {"caminhos? Porque endureces o", "caminhos? Por que endureces o", 0},
    {"disse: Porque caminho passou", "disse: Por que caminho passou", 0},
    {"dizendo: Porque jejuamos nós", "dizendo: Por que jejuamos nós", 0},
    {"E porque me chamais, Senhor,", "E por que me chamais, Senhor,", 0},
    {"e porque te perturbas dentro", "e por que te perturbas dentro", 0},
    {"escuridão? porque pois diz o", "escuridão? por que pois diz o", 0},
    {"gentios, a nossa vista, pela", "gentios, à nossa vista, pela", 0},
    {"Judá: Porque subistes contra", "Judá: Por que subistes contra", 0},
    {"mal, nem tão pouco têm poder", "mal, nem tampouco têm poder", 0},
    {"médico? porque pois não teve", "médico? por que pois não teve", 0},
    {"oferta voluntaria, oferecida", "oferta voluntária, oferecida", 0},
    {"para os capiteis que estavam", "para os capitéis que estavam", 0},
    {"porém senão lha restituíres,", "porém se não lha restituíres,", 0},
    {"Porém vos não vos detenhais;", "Porém vós não vos detenhais;", 0},
    {"povo? porque te assentas só,", "povo? por que te assentas só,", 0},
    {"superior de Benjamin, a qual", "superior de Benjamim, a qual", 0},
    {"vem, e enviar-te-hei a eles.", "vem, e enviar-te-ei a eles.", 0},
    {"Aarão, porque fazeis cessar", "Aarão, por que fazeis cessar", 0},
    {"bosques de Arabia passareis", "bosques de Arábia passareis", 0},
    {"carro? porque se demoram os", "carro? por que se demoram os", 0},
    {"companheiro: Porque obrou o", "companheiro: Por que obrou o", 0},
    {"contaminei em os seus dons,", "contaminei nos seus dons,", 0},
    {"contemplam, o Deus, Senhor:", "contemplam, ó Deus, Senhor:", 0},
    {"despiram-lhe a purpura, e o", "despiram-lhe a púrpura, e o", 0},
    {"Deus: Porque transgredis os", "Deus: Por que transgredis os", 0},
    {"disse: Porque quebrantais o", "disse: Por que quebrantais o", 0},
    {"disse-lhe: O mulher! grande", "disse-lhe: Ó mulher! grande", 0},
    {"disser: Porque fazeis isso?", "disser: Por que fazeis isso?", 0},
    {"disseram-lhe: Ha ainda para", "disseram-lhe: Há ainda para", 0},
    {"disseram-lhe: O nossa irmã,", "disseram-lhe: Ó nossa irmã,", 0},
    {"disserdes: Porque nos fez o", "disserdes: Por que nos fez o", 0},
    {"dizendo: Porque fazes assim", "dizendo: Por que fazes assim", 0},
    {"entesourem para si mesmo um", "entesourem para si mesmos um", 0},
    {"Filho, porque fizeste assim", "Filho, por que fizeste assim", 0},
    {"filhos de Benjamin, do meio", "filhos de Benjamim, do meio", 0},
    {"folha da corôa de santidade", "folha da coroa de santidade", 0},
    {"forno: em escorias de prata", "forno: em escórias de prata", 0},
    {"Israel, porque sucedeu isto", "Israel, por que sucedeu isto", 0},
    {"levando a corôa de espinhos", "levando a coroa de espinhos", 0},
    {"médico, nem tão pouco há em", "médico, nem tampouco há em", 0},
    {"mim? porque ando lamentando", "mim? por que ando lamentando", 0},
    {"Moisés: Porque nos fizestes", "Moisés: Por que nos fizestes", 0},
    {"parte: Porque o não podemos", "parte: Por que o não podemos", 0},
    {"pela meia noite suspeitaram", "pela meia-noite suspeitaram", 0},
    {"penas de varias cores, veio", "penas de várias cores, veio", 0},
    {"Porque nos assentamos aqui?", "Por que nos assentamos aqui?", 0},
    {"receberá a benção do Senhor", "receberá a bênção do Senhor", 0},
    {"seu pagem de armas: Arranca", "seu pajem de armas: Arranca", 0},
    {"Sião? porque nos feriste de", "Sião? por que nos feriste de", 0},
    {"tiraria a preza ao valente?", "tiraria a presa ao valente?", 0},
    {"virados para traz, cobriram", "virados para trás, cobriram", 0},
    {"agora, nem tão pouco há de", "agora, nem tampouco há de", 0},
    {"apanhar a preza: e devorou", "apanhar a presa: e devorou", 0},
    {"coração voluntariamente se", "coração voluntáriamente se", 0},
    {"de varias concupiscências;", "de várias concupiscências;", 0},
    {"de, encurvando me, desatar", "de, encurvando-me, desatar", 0},
    {"depois pacifica, moderada,", "depois pacífica, moderada,", 0},
    {"disse: Porque ajuizais mal", "disse: Por que ajuizais mal", 0},
    {"disse: Porque não reparais", "disse: Por que não reparais", 0},
    {"dizendo: Donde veem a este", "dizendo: Donde vêm a este", 0},
    {"estes que veem voando como", "estes que vêm voando como", 0},
    {"estranhos por preza, e aos", "estranhos por presa, e aos", 0},
    {"Israel: Porque nos feriu o", "Israel: Por que nos feriu o", 0},
    {"manjares de ciumes, oferta", "manjares de ciúmes, oferta", 0},
    {"mãos: porque pois subistes", "mãos: por que pois subistes", 0},
    {"nações, que ás de possuir,", "nações, que hás de possuir,", 0},
    {"nem tão pouco dos sábios o", "nem tampouco dos sábios o", 0},
    {"nem tão pouco quem ouça as", "nem tampouco quem ouça as", 0},
    {"Portanto, tornar-me-hei, e", "Portanto, tornar-me-ei, e", 0},
    {"protestarei contia ti; Sou", "protestarei contra ti; Sou", 0},
    {"quando vos entregaram, não", "quando vos entregarem, não", 0},
    {"quanto neles ha, jubilarão", "quanto neles há, jubilarão", 0},
    {"rei, nem tão pouco no mais", "rei, nem tampouco no mais", 0},
    {"segundo a benção do Senhor", "segundo a bênção do Senhor", 0},
    {"Senhor! porque fizeste mal", "Senhor! por que fizeste mal", 0},
    {"seu pagem de armas estavam", "seu pajem de armas estavam", 0},
    {"seu pagem de armas feriram", "seu pajem de armas feriram", 0},
    {"tornastes em escorias, por", "tornastes em escórias, por", 0},
    {"tornaste-te para traz; por", "tornaste-te para trás; por", 0},
    {"Tu, que te glórias na lei,", "Tu, que te glorias na lei,", 0},
    {"voltaram para traz, caíram", "voltaram para trás, caíram", 0},
    {"voz. Porque fizestes isto?", "voz. Por que fizestes isto?", 0},
    {"cabeças, a corôa de vossa", "cabeças, a coroa de vossa", 0},
    {"com bençãos do abismo que", "com bênçãos do abismo que", 0},
    {"comigo? porque tentais ao", "comigo? por que tentais ao", 0},
    {"conosco, porque tudo isto", "conosco, por que tudo isto", 0},
    {"da meia noite, havendo-se", "da meia-noite, havendo-se", 0},
    {"dar em nós mesmo exemplo,", "dar em nós mesmos exemplo,", 0},
    {"das suas dizimas resgatar", "das suas dízimas resgatar", 0},
    {"de todos os moveis da tua", "de todos os móveis da tua", 0},
    {"de varias cores, despojos", "de várias cores, despojos", 0},
    {"descerá ante dos olhos de", "descerá diante dos olhos de", 0},
    {"discípulos: Porque come o", "discípulos: Por que come o", 0},
    {"disseram: Porque fazeis o", "disseram: Por que fazeis o", 0},
    {"dizeis: Porque não levará", "dizeis: Por que não levará", 0},
    {"dizendo: Porque viriam os", "dizendo: Por que viriam os", 0},
    {"durará a corôa de geração", "durará a coroa de geração", 0},
    {"e azul celeste, pendentes", "e azul-celeste, pendentes", 0},
    {"e Eleal, as e Quiriataim;", "e Eleale, e Quiriataim;", 0},
    {"E os capiteis que estavam", "E os capitéis que estavam", 0},
    {"e perpetua alegria haverá", "e perpétua alegria haverá", 0},
    {"e porque te entremeterias", "e por que te entremeterias", 0},
    {"fino e purpura, e argolas", "fino e púrpura, e argolas", 0},
    {"fortes; e veem, e devoram", "fortes; e vêm, e devoram", 0},
    {"gordura que neles ha, que", "gordura que neles há, que", 0},
    {"homens? porque fizeste de", "homens? por que fizeste de", 0},
    {"instrumentos de musica; e", "instrumentos de música; e", 0},
    {"irmão, ás de servir-me de", "irmão, hás de servir-me de", 0},
    {"isso? Porque afligimos as", "isso? Por que afligimos as", 0},
    {"juízos. Porque prospera o", "juízos. Por que prospera o", 0},
    {"manjares dos ciumes, porá", "manjares dos ciúmes, porá", 0},
    {"manjares, duas dizimas de", "manjares, duas dízimas de", 0},
    {"Misma, e De uma, e Masca,", "Misma, e Dumá, e Masca,", 0},
    {"Misma, e De uma, e Massa,", "Misma, e Duma, e Massa,", 0},
    {"Nem tão pouco apresenteis", "Nem tampouco apresenteis", 0},
    {"nem tão pouco derramareis", "nem tampouco derramareis", 0},
    {"nem tão pouco pretendemos", "nem tampouco pretendemos", 0},
    {"nisto; porque cresceria o", "nisto; por que cresceria o", 0},
    {"o espírito de ciumes vier", "o espírito de ciúmes vier", 0},
    {"olhando para traz, disse:", "olhando para trás, disse:", 0},
    {"pela vida o homicida, que", "pela vida do homicida, que", 0},
    {"Porque contendeis comigo?", "Por que contendeis comigo?", 0},
    {"Porque me perseguis assim", "Por que me perseguis assim", 0},
    {"povo; se a ás de redimir,", "povo; se a hás de redimir,", 0},
    {"povo? porque me enviaste?", "povo? por que me enviaste?", 0},
    {"quem ás de enviar comigo:", "quem hás de enviar comigo:", 0},
    {"retiraram-se para traz, e", "retiraram-se para trás, e", 0},
    {"semente? Porque tem então", "semente? Por que tem então", 0},
    {"será chamada adultera, se", "será chamada adúltera, se", 0},
    {"a varias concupiscências", "a várias concupiscências", 0},
    {"aonde ás de entrar; para", "aonde hás de entrar; para", 0},
    {"apressou era prepará-la.", "apressou em prepará-la.", 0},
    {"casa? porque pois veio a", "casa? por que pois veio a", 0},
    {"certamente ás de reinar,", "certamente hás de reinar,", 0},
    {"com bençãos dos peitos e", "com bênçãos dos peitos e", 0},
    {"comiam os primeiras sete", "comiam as primeiras sete", 0},
    {"disseram aos Cabeças dos", "disseram aos cabeças dos", 0},
    {"dizendo: Porque comeis e", "dizendo: Por que comeis e", 0},
    {"dizendo: Porque dizem os", "dizendo: Por que dizem os", 0},
    {"e ha-te já valorosamente", "e há-te já valorosamente", 0},
    {"e porque te perturbas em", "e por que te perturbas em", 0},
    {"e solver duvidas: agora,", "e solver dúvidas: agora,", 0},
    {"entre a purpura, e entre", "entre a púrpura, e entre", 0},
    {"esquivar-se de ali entre", "esquivar-se dali entre", 0},
    {"filhas dous homens desta", "filhas dos homens desta", 0},
    {"gente sabia e entendida.", "gente sábia e entendida.", 0},
    {"globos dos capiteis, que", "globos dos capitéis, que", 0},
    {"Israel: A tarde sabereis", "Israel: À tarde sabereis", 0},
    {"Jefté: Porque passaste a", "Jefté: Por que passaste a", 0},
    {"Jordão voltou para traz.", "Jordão voltou para trás.", 0},
    {"maravilhas de aquele que", "maravilhas daquele que", 0},
    {"mulher tiver ciumes, por", "mulher tiver ciúmes, por", 0},
    {"multidão doa anos ensine", "multidão dos anos ensine", 0},
    {"nenhum vos fiéis; porque", "nenhum vos fieis; porque", 0},
    {"o meio dia, profetizaram", "o meio-dia, profetizaram", 0},
    {"oferta voluntaria, assim", "oferta voluntária, assim", 0},
    {"Porque te desvias tanto,", "Por que te desvias tanto,", 0},
    {"que estilo na Judeia, em", "que estão na Judeia, em", 0},
    {"que retem violentamente,", "que retém violentamente,", 0},
    {"recebe a benção de Deus:", "recebe a bênção de Deus:", 0},
    {"Samuel: Vai te deitar, e", "Samuel: Vai-te deitar, e", 0},
    {"seu pagem de armas atrás", "seu pajem de armas atrás", 0},
    {"terra de Benjamin, e das", "terra de Benjamim, e das", 0},
    {"tornaram-se para traz, e", "tornaram-se para trás, e", 0},
    {"a minha benção sobre os", "a minha bênção sobre os", 0},
    {"ajuntam veem a ti: vivo", "ajuntam vêm a ti: vivo", 0},
    {"ajuntaram, e veem a ti:", "ajuntaram, e vêm a ti:", 0},
    {"alma voluntaria; porque", "alma voluntária; porque", 0},
    {"Antes, porque-vos tenho", "Antes, porque vos tenho", 0},
    {"Arab, e De uma, e Esan,", "Arab, e Duma, e Esan,", 0},
    {"arrebatam a preza, para", "arrebatam a presa, para", 0},
    {"bases de baixo de outra", "bases debaixo de outra", 0},
    {"com bençãos dos céus de", "com bênçãos dos céus de", 0},
    {"com perpetua inimizade,", "com perpétua inimizade,", 0},
    {"coxos roubarão a preza.", "coxos roubarão a presa.", 0},
    {"da corôa de soberba dos", "da coroa de soberba dos", 0},
    {"Dan porque se deteve em", "Dan por que se deteve em", 0},
    {"de manjares continua; a", "de manjares contínua; a", 0},
    {"dentro dos tuas portas;", "dentro das tuas portas;", 0},
    {"disse: Porque pede esta", "disse: Por que pede esta", 0},
    {"disser: Ha aqui alguém?", "disser: Há aqui alguém?", 0},
    {"disseram: Ha alguma das", "disseram: Há alguma das", 0},
    {"disserem: Porque fala o", "disserem: Por que fala o", 0},
    {"do Todo Poderoso, caído", "do Todo-Poderoso, caído", 0},
    {"e a continua oferta dos", "e a contínua oferta dos", 0},
    {"e apiedar-me-hei deles.", "e apiedar-me-ei deles.", 0},
    {"E porque me não perdoas", "E por que me não perdoas", 0},
    {"errei grandissimamente.", "errei grandíssimamente.", 0},
    {"espírito se angústia em", "espírito se angustia em", 0},
    {"espirituais em os ares.", "espirituais nos ares.", 0},
    {"estranha distilam favos", "estranha destilam favos", 0},
    {"excelência perpetua, um", "excelência perpétua, um", 0},
    {"Hai para traz, olharam,", "Hai para trás, olharam,", 0},
    {"homem, te constitui por", "homem, te constituí por", 0},
    {"Judá, que veem a adorar", "Judá, que vêm a adorar", 0},
    {"Moisés: Porque clamas a", "Moisés: Por que clamas a", 0},
    {"nem tão pouco desejarão", "nem tampouco desejarão", 0},
    {"nem tão pouco esperarão", "nem tampouco esperarão", 0},
    {"o espírito de ciumes, e", "o espírito de ciúmes, e", 0},
    {"passar deste mando para", "passar deste mundo para", 0},
    {"Peso contra Arabia. Nos", "Peso contra Arábia. Nos", 0},
    {"porém tão pouco estavam", "porém tampouco estavam", 0},
    {"Porém tão pouco ouviram", "Porém tampouco ouviram", 0},
    {"porque adulteras são, e", "porque adúlteras são, e", 0},
    {"Porque estás abatida, ó", "Por que estás abatida, ó", 0},
    {"porque hei-de enviar-te", "porque hei de enviar-te", 0},
    {"Porque pois descorajais", "Por que pois descorajais", 0},
    {"porque te glórias, como", "porque te glorias, como", 0},
    {"profetas virão para ti,", "profetas viram para ti,", 0},
    {"quando hei-de trabalhar", "quando hei de trabalhar", 0},
    {"que retem violentamente", "que retém violentamente", 0},
    {"roubar as utensílios do", "roubar os utensílios do", 0},
    {"Senhor: porque pois não", "Senhor: por que pois não", 0},
    {"também fábrica dele uma", "também fabrica dele uma", 0},
    {"tempos porque, os que o", "tempos por que, os que o", 0},
    {"terão perpetua alegria.", "terão perpétua alegria.", 0},
    {"toda a benção e louvor.", "toda a bênção e louvor.", 0},
    {"todos os moveis da casa", "todos os móveis da casa", 0},
    {"tornará para traz sobre", "tornará para trás sobre", 0},
    {"uma mulher sabia gritou", "uma mulher sábia gritou", 0},
    {"vistes: porque pois vos", "vistes: por que pois vos", 0},
    {"voltem para traz, todos", "voltem para trás, todos", 0},
    {"zombaria de aqueles que", "zombaria daqueles que", 0},
    {"A corôa de soberba dos", "A coroa de soberba dos", 0},
    {"adversário não ha, nem", "adversário não há, nem", 0},
    {"alguma: tão pouco virá", "alguma: tampouco virá", 0},
    {"apressurou-se à preza.", "apressurou-se à presa.", 0},
    {"arrebata a preza: eles", "arrebata a presa: eles", 0},
    {"arrebatarão a preza, e", "arrebatarão a presa, e", 0},
    {"as bençãos espirituais", "as bênçãos espirituais", 0},
    {"benignidade sustem ele", "benignidade sustém ele", 0},
    {"confundir as sabias; e", "confundir as sábias; e", 0},
    {"contigo de aqui trinta", "contigo daqui trinta", 0},
    {"de moveis apetecíveis.", "de móveis apetecíveis.", 0},
    {"de varias enfermidades", "de várias enfermidades", 0},
    {"deles: porque pois vos", "deles: por que pois vos", 0},
    {"disse: Eis-aqui, tenho", "disse: Eis aqui, tenho", 0},
    {"disse: Ha acaso alguma", "disse: Há acaso alguma", 0},
    {"disse: Porque é que me", "disse: Por que é que me", 0},
    {"dizeis: Porque? porque", "dizeis: Por quê? porque", 0},
    {"dizendo: Eis-aqui, nós", "dizendo: Eis aqui, nós", 0},
    {"do candieiro alumiarão", "do candeeiro alumiarão", 0},
    {"e congregar-vos-hei de", "e congregar-vos-ei de", 0},
    {"E dar-lhes-hei coração", "E dar-lhes-ei coração", 0},
    {"e solvendo duvidas, ao", "e solvendo dúvidas, ao", 0},
    {"e todos o moradores de", "e todos os moradores de", 0},
    {"eles: Porque jejuam os", "eles: Por que jejuam os", 0},
    {"em perpetua possessão,", "em perpétua possessão,", 0},
    {"espessura doe ramos de", "espessura dos ramos de", 0},
    {"estrangeiros: sustem o", "estrangeiros: sustém o", 0},
    {"família das helekitas:", "família dos helekitas:", 0},
    {"família doa hamulitas.", "família dos hamulitas.", 0},
    {"grade: Porque tarda em", "grade: Por que tarda em", 0},
    {"imagem dos ciumes, que", "imagem dos ciúmes, que", 0},
    {"incorruptível corôa de", "incorruptível coroa de", 0},
    {"juízo das adulteras, e", "juízo das adúlteras, e", 0},
    {"lei dos ciumes, quando", "lei dos ciúmes, quando", 0},
    {"lei para traz das suas", "lei para trás das suas", 0},
    {"lhes ás de fazer, para", "lhes hás de fazer, para", 0},
    {"manjares dos ciumes da", "manjares dos ciúmes da", 0},
    {"mãos distilavam mirra,", "mãos destilavam mirra,", 0},
    {"mim em escorias: todos", "mim em escórias: todos", 0},
    {"muito povo ha; faze-os", "muito povo há; faze-os", 0},
    {"Nem tão pouco cortarás", "Nem tampouco cortarás", 0},
    {"nem tão pouco deixarei", "nem tampouco deixarei", 0},
    {"Nem tão pouco Ezequias", "Nem tampouco Ezequias", 0},
    {"oferta voluntaria, mas", "oferta voluntária, mas", 0},
    {"oferta voluntaria, sem", "oferta voluntária, sem", 0},
    {"ordena a benção e vida", "ordena a bênção e vida", 0},
    {"os dois capiteis sobre", "os dois capitéis sobre", 0},
    {"pai? porque pois agora", "pai? por que pois agora", 0},
    {"panela ao luz, põe-na,", "panela ao lume, põe-na,", 0},
    {"para traz deles; antes", "para trás deles; antes", 0},
    {"Peso de De uma. Dão-me", "Peso de Duma. Dão-me", 0},
    {"Porque escreves contra", "Por que escreves contra", 0},
    {"Porque nos rejeitarias", "Por que nos rejeitarias", 0},
    {"Porque somos estimados", "Por que somos estimados", 0},
    {"que ás de padecer. Eis", "que hás de padecer. Eis", 0},
    {"que me ás de dar, pois", "que me hás de dar, pois", 0},
    {"que neles ha, dizendo:", "que neles há, dizendo:", 0},
    {"seu pagem de armas lhe", "seu pajem de armas lhe", 0},
    {"seu pagem de armas que", "seu pajem de armas que", 0},
    {"sua oferta manjares, e", "sua oferta de manjares, e", 0},
    {"Tão pouco desapossarei", "Tampouco desapossarei", 0},
    {"tenha preza? levantará", "tenha presa? levantará", 0},
    {"tornar: porque pois me", "tornar: por que pois me", 0},
    {"tornaram-se para traz.", "tornaram-se para trás.", 0},
    {"uma corôa corruptível,", "uma coroa corruptível,", 0},
    {"voltou-se para traz, e", "voltou-se para trás, e", 0},
    {"à meia noite ouviu-se", "à meia-noite ouviu-se", 0},
    {"a oração e suplica em", "a oração e súplica em", 0},
    {"achado de aqueles que", "achado daqueles que", 0},
    {"agora a ás de dar, e,", "agora a hás de dar, e,", 0},
    {"Ana, porque choras? e", "Ana, por que choras? e", 0},
    {"ao meio dia, dizendo:", "ao meio-dia, dizendo:", 0},
    {"aquentando-se ao luz.", "aquentando-se ao lume.", 0},
    {"assolação perpetua; o", "assolação perpétua; o", 0},
    {"cadeira para traz, da", "cadeira para trás, da", 0},
    {"carne vivo a na fé do", "carne vivo-a na fé do", 0},
    {"com varias tentações.", "com várias tentações.", 0},
    {"confusão perpetua que", "confusão perpétua que", 0},
    {"Davi: Ha ainda alguém", "Davi: Há ainda alguém", 0},
    {"De aqui em diante não", "Daqui em diante não", 0},
    {"disse Jeremias: Ha. E", "disse Jeremias: Há. E", 0},
    {"disse: Porque já três", "disse: Por que já três", 0},
    {"disso veem ainda dois", "disso vêm ainda dois", 0},
    {"e ao meio dia orarei;", "e ao meio-dia orarei;", 0},
    {"e porque não comes? e", "e por que não comes? e", 0},
    {"E porque nos fizestes", "E por que nos fizestes", 0},
    {"e porque somente este", "e por que somente este", 0},
    {"E porque trouxestes a", "E por que trouxestes a", 0},
    {"e tão pouco pelejaram", "e tampouco pelejaram", 0},
    {"esmeralda, purpura, e", "esmeralda, púrpura, e", 0},
    {"esta suplica, estando", "esta súplica, estando", 0},
    {"fazes as povo? por que", "fazes ao povo? por que", 0},
    {"fazia continua oração", "fazia contínua oração", 0},
    {"inimizade perpetua, e", "inimizade perpétua, e", 0},
    {"Jacó um sacrificou na", "Jacó um sacrifício na", 0},
    {"Jerusalém ha, próximo", "Jerusalém há, próximo", 0},
    {"lâmpadas, o queimara.", "lâmpadas, o queimará.", 0},
    {"longe doa sidônios, e", "longe dos sidônios, e", 0},
    {"mas tem-o o marido; e", "mas tem-no o marido; e", 0},
    {"minha suplica perante", "minha súplica perante", 0},
    {"morte. Ha pecado para", "morte. Há pecado para", 0},
    {"não será adultera, se", "não será adúltera, se", 0},
    {"nem tão pouco comerei", "nem tampouco comerei", 0},
    {"nem tão pouco desejei", "nem tampouco desejei", 0},
    {"nem tão pouco pediste", "nem tampouco pediste", 0},
    {"nem tão pouco segundo", "nem tampouco segundo", 0},
    {"nenhum rei ha, grande", "nenhum rei há, grande", 0},
    {"nos chamas te, quando", "nos chamaste, quando", 0},
    {"oferta voluntaria, no", "oferta voluntária, no", 0},
    {"oferta voluntaria, ou", "oferta voluntária, ou", 0},
    {"olhou para traz dele,", "olhou para trás dele,", 0},
    {"ou porque não ficou a", "ou por que não ficou a", 0},
    {"por corôa gloriosa, e", "por coroa gloriosa, e", 0},
    {"Porque se amotinam as", "Por que se amotinam as", 0},
    {"Porque te esquecerias", "Por que te esquecerias", 0},
    {"Porque, ó Senhor, nos", "Por que, ó Senhor, nos", 0},
    {"Que veem estes fazer?", "Que vêm estes fazer?", 0},
    {"querida de aquela que", "querida daquela que", 0},
    {"respondia a si mesmo:", "respondia a si mesma:", 0},
    {"saltérios, alaudes, e", "saltérios, alaúdes, e", 0},
    {"Senhor todo poderoso.", "Senhor todo-poderoso.", 0},
    {"seu pagem de armas, e", "seu pajem de armas, e", 0},
    {"sorte de musica, para", "sorte de música, para", 0},
    {"sua corôa, lançando-a", "sua coroa, lançando-a", 0},
    {"também à suas tendas,", "também às suas tendas,", 0},
    {"te constitui) perante", "te constituí) perante", 0},
    {"tornaram para traz, e", "tornaram para trás, e", 0},
    {"trevas nos veem; pelo", "trevas nos vêm; pelo", 0},
    {"vestido da purpura, e", "vestido da púrpura, e", 0},
    {"vestido de purpura, e", "vestido de púrpura, e", 0},
    {"virá a benção do bem.", "virá a bênção do bem.", 0},
    {"voltarão para traz os", "voltarão para trás os", 0},
    {"a continua oferta de", "a contínua oferta de", 0},
    {"afogava a preza para", "afogava a presa para", 0},
    {"água; veem às cavas,", "água; vêm às cavas,", 0},
    {"alguma de aquilo que", "alguma daquilo que", 0},
    {"ao meio dia, perante", "ao meio-dia, perante", 0},
    {"Barnabé à Antioquia.", "Barnabé a Antioquia.", 0},
    {"clamarão: Ha medo de", "clamarão: Há medo de", 0},
    {"da casa o Senhor, da", "da casa do Senhor, da", 0},
    {"da meia noite, Paulo", "da meia-noite, Paulo", 0},
    {"Daniel de purpura, e", "Daniel de púrpura, e", 0},
    {"de se por o sol: Que", "de se pôr o sol: Que", 0},
    {"de todas a bandas em", "de todas as bandas em", 0},
    {"de varias cores, que", "de várias cores, que", 0},
    {"do candieiro acendeu", "do candeeiro acendeu", 0},
    {"e a benção do Senhor", "e a bênção do Senhor", 0},
    {"e ante ontem, quando", "e anteontem, quando", 0},
    {"e mostrar-te-hei que", "e mostrar-te-ei que", 0},
    {"eis que veem dias, e", "eis que vêm dias, e", 0},
    {"eles veem a ti, como", "eles vêm a ti, como", 0},
    {"em varias tentações:", "em várias tentações:", 0},
    {"Então os Cabeças dos", "Então os cabeças dos", 0},
    {"espiritual. Ha corpo", "espiritual. Há corpo", 0},
    {"filhos do Israel não", "filhos de Israel não", 0},
    {"justiça ha-de julgar", "justiça há de julgar", 0},
    {"Mas, quanto tu deres", "Mas, quando tu deres", 0},
    {"minha suplica diante", "minha súplica diante", 0},
    {"mulher adultera que,", "mulher adúltera que,", 0},
    {"mulher adultera: ela", "mulher adúltera: ela", 0},
    {"na mão e Acabe, para", "na mão de Acabe, para", 0},
    {"nem tão pouco devias", "nem tampouco devias", 0},
    {"nem tão pouco olhará", "nem tampouco olhará", 0},
    {"nem tão pouco passes", "nem tampouco passes", 0},
    {"nós fossemos fracos,", "nós fôssemos fracos,", 0},
    {"nós? porque pois nos", "nós? por que pois nos", 0},
    {"nossa suplica diante", "nossa súplica diante", 0},
    {"o ouriço cacheiro, e", "o ouriço-cacheiro, e", 0},
    {"olharão para traz: e", "olharão para trás: e", 0},
    {"onde teus ungido uma", "onde tens ungido uma", 0},
    {"por oferta pacifica;", "por oferta pacífica;", 0},
    {"Porque pois dizes, ó", "Por que pois dizes, ó", 0},
    {"Porque te arrebata o", "Por que te arrebata o", 0},
    {"povo em os olhos dos", "povo nos olhos dos", 0},
    {"que hei-de herdá-la?", "que hei de herdá-la?", 0},
    {"que, no non o ano do", "que, no nono ano do", 0},
    {"saltérios, alaudes e", "saltérios, alaúdes e", 0},
    {"sarna em os magos, e", "sarna nos magos, e", 0},
    {"Saul para traz, Davi", "Saul para trás, Davi", 0},
    {"se voltou para traz.", "se voltou para trás.", 0},
    {"sorte de musica, vos", "sorte de música, vos", 0},
    {"também eu disso: Não", "também eu disse: Não", 0},
    {"Tão pouco apliques o", "Tampouco apliques o", 0},
    {"tarde de aquele dia,", "tarde daquele dia,", 0},
    {"torno para traz, não", "torno para trás, não", 0},
    {"tu me ás amado antes", "tu me hás amado antes", 0},
    {"Vira-te para traz de", "Vira-te para trás de", 0},
    {"vossa suplica diante", "vossa súplica diante", 0},
    {"ao meio dia; porque", "ao meio-dia; porque", 0},
    {"aos seus Cabeças, e", "aos seus cabeças, e", 0},
    {"coisas moveis, como", "coisas móveis, como", 0},
    {"com alaudes, harpas", "com alaúdes, harpas", 0},
    {"Cornélio: Ha quatro", "Cornélio: Há quatro", 0},
    {"daquela benção, com", "daquela bênção, com", 0},
    {"Deus: os Cabeças de", "Deus: os cabeças de", 0},
    {"dirá: Pois porque o", "dirá: Pois por que o", 0},
    {"disse: Ha paz, Jeú?", "disse: Há paz, Jeú?", 0},
    {"disse: O néscios, e", "disse: Ó néscios, e", 0},
    {"e a preza do tirano", "e a presa do tirano", 0},
    {"e alaudes, tamboris", "e alaúdes, tamboris", 0},
    {"e apiedar-me-hei de", "e apiedar-me-ei de", 0},
    {"e ir-me-hei embora;", "e ir-me-ei embora;", 0},
    {"E livrar-te-hei das", "E livrar-te-ei das", 0},
    {"e mostrar-te-hei as", "e mostrar-te-ei as", 0},
    {"e o seus estatutos,", "e os seus estatutos,", 0},
    {"E porque atentas tu", "E por que atentas tu", 0},
    {"E porque reparas tu", "E por que reparas tu", 0},
    {"e varias maravilhas", "e várias maravilhas", 0},
    {"enchia de prezas as", "enchia de presas as", 0},
    {"estas bençãos virão", "estas bênçãos virão", 0},
    {"falta nelas ha; não", "falta nelas há; não", 0},
    {"fino e purpura, e a", "fino e púrpura, e a", 0},
    {"foste prospera, até", "foste próspera, até", 0},
    {"Ha acaso iniquidade", "Há acaso iniquidade", 0},
    {"imagem de ciumes na", "imagem de ciúmes na", 0},
    {"mas tem-o a mulher.", "mas tem-no a mulher.", 0},
    {"mim cara trazeres à", "mim para trazeres à", 0},
    {"nem tão pouco ainda", "nem tampouco ainda", 0},
    {"nem tão pouco desde", "nem tampouco desde", 0},
    {"nem tão pouco foste", "nem tampouco foste", 0},
    {"Nem tão pouco podem", "Nem tampouco podem", 0},
    {"nem tão pouco sabem", "nem tampouco sabem", 0},
    {"olhos, dar-te-hei a", "olhos, dar-te-ei a", 0},
    {"os meus moveis, que", "os meus móveis, que", 0},
    {"para que pára sobre", "para que para sobre", 0},
    {"Pilatos: Eis-aqui o", "Pilatos: Eis aqui o", 0},
    {"Pois que? Pecaremos", "Pois quê? Pecaremos", 0},
    {"Porque não me matou", "Por que não me matou", 0},
    {"Porque se tiraria o", "Por que se tiraria o", 0},
    {"possessão perpetua.", "possessão perpétua.", 0},
    {"postos por preza, e", "postos por presa, e", 0},
    {"que veem sete anos,", "que vêm sete anos,", 0},
    {"que, em o que falam", "que, no que falam", 0},
    {"real azul celeste e", "real azul-celeste e", 0},
    {"retirou para traz a", "retirou para trás a", 0},
    {"retirou para traz o", "retirou para trás o", 0},
    {"seu pagem de armas.", "seu pajem de armas.", 0},
    {"seus capiteis assim", "seus capitéis assim", 0},
    {"sorte de musica, se", "sorte de música, se", 0},
    {"também à requerido.", "também é requerido.", 0},
    {"todas as dizimas de", "todas as dízimas de", 0},
    {"todas as dizimas do", "todas as dízimas do", 0},
    {"tua benção. (Selá).", "tua bênção. (Selá).", 0},
    {"uma mulher sabia, e", "uma mulher sábia, e", 0},
    {"vejo? E o homens da", "vejo? E os homens da", 0},
    {"voltaram para traz.", "voltaram para trás.", 0},
    {"voltaste para traz?", "voltaste para trás?", 0},
    {"vos constitui, para", "vos constituí, para", 0},
    {"a corôa; deram-lhe", "a coroa; deram-lhe", 0},
    {"à preza, moabitas!", "à presa, moabitas!", 0},
    {"a sua preza, ainda", "a sua presa, ainda", 0},
    {"Achis: Porque? que", "Achis: Por quê? que", 0},
    {"Arnom; eis-aqui na", "Arnom; eis aqui na", 0},
    {"bem? porque veio a", "bem? por que veio a", 0},
    {"caiam para traz, e", "caiam para trás, e", 0},
    {"cobre era forma de", "cobre em forma de", 0},
    {"com alaudes, sobre", "com alaúdes, sobre", 0},
    {"coração para traz.", "coração para trás.", 0},
    {"de duas dizimas de", "de duas dízimas de", 0},
    {"de varias cores, e", "de várias cores, e", 0},
    {"disse: Que ha, que", "disse: Que há, que", 0},
    {"disse: Tão pouco a", "disse: Tampouco a", 0},
    {"disseram: E réu de", "disseram: É réu de", 0},
    {"disserem: Ha paz e", "disserem: Há paz e", 0},
    {"doutrinas varias e", "doutrinas várias e", 0},
    {"e de num testifica", "e de mim testifica", 0},
    {"e de purpura, e de", "e de púrpura, e de", 0},
    {"e ensinar-te-hei a", "e ensinar-te-ei a", 0},
    {"e mostrar-te-hei o", "e mostrar-te-ei o", 0},
    {"E o senhor dissera", "E o Senhor dissera", 0},
    {"e porque acenam os", "e por que acenam os", 0},
    {"Eis-aqui, bendizei", "Eis aqui, bendizei", 0},
    {"entre as prezas, e", "entre as presas, e", 0},
    {"era Cabeça da casa", "era cabeça da casa", 0},
    {"escarnecer de nim;", "escarnecer de mim;", 0},
    {"falta nele ha; não", "falta nele há; não", 0},
    {"geração adultera e", "geração adúltera e", 0},
    {"Ha diferença entre", "Há diferença entre", 0},
    {"jornadas ha, desde", "jornadas há, desde", 0},
    {"mas nenhumas ha, e", "mas nenhumas há, e", 0},
    {"meio doe filhos de", "meio dos filhos de", 0},
    {"meu bem querido da", "meu bem-querido da", 0},
    {"minha benção. Mais", "minha bênção. Mais", 0},
    {"moinhos o moia, ou", "moinhos o moía, ou", 0},
    {"mulheres sabias de", "mulheres sábias de", 0},
    {"não me exército em", "não me exercito em", 0},
    {"nem tão pouco diga", "nem tampouco diga", 0},
    {"Nem tão pouco está", "Nem tampouco está", 0},
    {"nem tão pouco para", "nem tampouco para", 0},
    {"o corvo marinho, e", "o corvo-marinho, e", 0},
    {"oferta pacifica se", "oferta pacífica se", 0},
    {"oferta pacifica, o", "oferta pacífica, o", 0},
    {"oferta voluntaria.", "oferta voluntária.", 0},
    {"olhar para traz de", "olhar para trás de", 0},
    {"olhes para traz de", "olhes para trás de", 0},
    {"os seus Cabeças, e", "os seus cabeças, e", 0},
    {"pão nem água ha: e", "pão nem água há: e", 0},
    {"para as as aldeias", "para as aldeias", 0},
    {"pela meia noite, o", "pela meia-noite, o", 0},
    {"Pois que? julga-se", "Pois quê? julga-se", 0},
    {"Porque dais coices", "Por que dais coices", 0},
    {"Porque serias como", "Por que serias como", 0},
    {"quanto nela ha; os", "quanto nela há; os", 0},
    {"quanto neles ha, e", "quanto neles há, e", 0},
    {"que ás de livrar a", "que hás de livrar a", 0},
    {"que ás de vir, que", "que hás de vir, que", 0},
    {"que profetiza vos?", "que vos profetiza?", 0},
    {"que veem do norte:", "que vêm do norte:", 0},
    {"sua suplica diante", "sua súplica diante", 0},
    {"terra de Benjamin:", "terra de Benjamim:", 0},
    {"vigias veem de uma", "vigias vêm de uma", 0},
    {"voltem para traz e", "voltem para trás e", 0},
    {"à meia noite, que", "à meia-noite, que", 0},
    {"Acaso não veem de", "Acaso não vêm de", 0},
    {"anos, que veem, e", "anos, que vêm, e", 0},
    {"ao meio dia andem", "ao meio-dia andem", 0},
    {"ao meio dia Elias", "ao meio-dia Elias", 0},
    {"ao meio dia, como", "ao meio-dia, como", 0},
    {"céu o os consuma,", "céu e os consuma,", 0},
    {"comerá a preza, e", "comerá a presa, e", 0},
    {"crê em Ele, tenha", "crê nele, tenha", 0},
    {"da preza subiste,", "da presa subiste,", 0},
    {"de varias doenças", "de várias doenças", 0},
    {"dia, era que eles", "dia, em que eles", 0},
    {"disse: Eis-aqui o", "disse: Eis aqui o", 0},
    {"e aos Cabeças dos", "e aos cabeças dos", 0},
    {"e dar-te-hei dois", "e dar-te-ei dois", 0},
    {"e far-te-hei bem;", "e far-te-ei bem;", 0},
    {"e ir-me-hei; pois", "e ir-me-ei; pois", 0},
    {"e porque está mal", "e por que está mal", 0},
    {"E porque o Senhor", "E por que o Senhor", 0},
    {"e tão pouco fazem", "e tampouco fazem", 0},
    {"e tiver ciumes de", "e tiver ciúmes de", 0},
    {"e tornar-me-hei a", "e tornar-me-ei a", 0},
    {"e varias abluções", "e várias abluções", 0},
    {"e veem descendo e", "e vêm descendo e", 0},
    {"é): Ha conselho e", "é): Há conselho e", 0},
    {"ele para traz, os", "ele para trás, os", 0},
    {"Esaú; o filhos de", "Esaú; os filhos de", 0},
    {"Hamã: Tão pouco a", "Hamã: Tampouco a", 0},
    {"hoje tomastes tão", "hoje tornastes tão", 0},
    {"mão sobro os teus", "mão sobre os teus", 0},
    {"mas benção haverá", "mas bênção haverá", 0},
    {"minha benção, que", "minha bênção, que", 0},
    {"nem tão pouco meu", "nem tampouco meu", 0},
    {"Nem tão pouco vos", "Nem tampouco vos", 0},
    {"nós e ministrada:", "nós é ministrada:", 0},
    {"nuvens distilam o", "nuvens destilam o", 0},
    {"o laço veem sobre", "o laço vêm sobre", 0},
    {"o meio dia; ainda", "o meio-dia; ainda", 0},
    {"olhará para traz.", "olhará para trás.", 0},
    {"ouvi: Quem ha, de", "ouvi: Quem há, de", 0},
    {"para traz, fez-me", "para trás, fez-me", 0},
    {"para traz: terror", "para trás: terror", 0},
    {"pomba, a verse as", "pomba, a ver se as", 0},
    {"porém sabias, bem", "porém sábias, bem", 0},
    {"Porque blasfema o", "Por que blasfema o", 0},
    {"Porque ciumes são", "Porque ciúmes são", 0},
    {"Porque enfeitas o", "Por que enfeitas o", 0},
    {"Porque escondes a", "Por que escondes a", 0},
    {"Porque escondes o", "Por que escondes o", 0},
    {"Porque ficaste tu", "Por que ficaste tu", 0},
    {"provoca a ciumes.", "provoca a ciúmes.", 0},
    {"que hei-de fazer,", "que hei de fazer,", 0},
    {"que veem para vós", "que vêm para vós", 0},
    {"que vo-lo o tenho", "que vo-lo tenho", 0},
    {"retiro para traz.", "retiro para trás.", 0},
    {"Se ás de livrar a", "Se hás de livrar a", 0},
    {"Senhor: Se-não me", "Senhor: Se não me", 0},
    {"Tão pouco expeliu", "Tampouco expeliu", 0},
    {"Tão pouco Naftali", "Tampouco Naftali", 0},
    {"tornar para traz?", "tornar para trás?", 0},
    {"Tu me ás de dar a", "Tu me hás de dar a", 0},
    {"vossas bençãos; e", "vossas bênçãos; e", 0},
    {"A benção, quando", "A bênção, quando", 0},
    {"à meia noite, se", "à meia-noite, se", 0},
    {"a preza daqueles", "a presa daqueles", 0},
    {"como o meio dia.", "como o meio-dia.", 0},
    {"da Media; porque", "da Média; porque", 0},
    {"de duas dizimas.", "de duas dízimas.", 0},
    {"de obra prima se", "de obra-prima se", 0},
    {"de varias cores.", "de várias cores.", 0},
    {"de varias cores;", "de várias cores;", 0},
    {"dois capiteis de", "dois capitéis de", 0},
    {"e a purpura, e o", "e a púrpura, e o", 0},
    {"e adulteras, não", "e adúlteras, não", 0},
    {"e espantai vos e", "e espantai-vos e", 0},
    {"E far-te-hei uma", "E far-te-ei uma", 0},
    {"e não o ha; pela", "e não o há; pela", 0},
    {"é obra prima dos", "é obra-prima dos", 0},
    {"e os Cabeças dos", "e os cabeças dos", 0},
    {"e tão pouco aqui", "e tampouco aqui", 0},
    {"e tão pouco está", "e tampouco está", 0},
    {"ele para traz de", "ele para trás de", 0},
    {"então morrera só", "então morrerá só", 0},
    {"ferida me doe, e", "ferida me dói, e", 0},
    {"fiéis bençãos de", "fiéis bênçãos de", 0},
    {"foram cito mil e", "foram oito mil e", 0},
    {"grande ao luz, e", "grande ao lume, e", 0},
    {"homem a hora que", "homem à hora que", 0},
    {"laço, veem sobre", "laço, vêm sobre", 0},
    {"minha suplica; o", "minha súplica; o", 0},
    {"não ás mister de", "não hás mister de", 0},
    {"Nem tão pouco eu", "Nem tampouco eu", 0},
    {"nem tão pouco os", "nem tampouco os", 0},
    {"Nem tão pouco os", "Nem tampouco os", 0},
    {"oferta pacifica.", "oferta pacífica.", 0},
    {"Os capiteis pois", "Os capitéis pois", 0},
    {"por sabias, para", "por sábias, para", 0},
    {"Porque lhe ás de", "Porque lhe hás de", 0},
    {"proveito ha, mas", "proveito há, mas", 0},
    {"que ás de comer,", "que hás de comer,", 0},
    {"que ás de falar.", "que hás de falar.", 0},
    {"que ás de fazer,", "que hás de fazer,", 0},
    {"que ás de fazer.", "que hás de fazer.", 0},
    {"que ás de fazer;", "que hás de fazer;", 0},
    {"que veem as aves", "que vêm as aves", 0},
    {"que veem dias em", "que vêm dias em", 0},
    {"quem há e ir por", "quem há de ir por", 0},
    {"Senhor sustem os", "Senhor sustém os", 0},
    {"será prospera, a", "será próspera, a", 0},
    {"suave musica não", "suave música não", 0},
    {"Tão pouco pequei", "Tampouco pequei", 0},
    {"terra sera cheia", "terra será cheia", 0},
    {"tu ás de enviar.", "tu hás de enviar.", 0},
    {"tuas escorias; e", "tuas escórias; e", 0},
    {"um os Cabeças do", "um os cabeças do", 0},
    {"volte para traz.", "volte para trás.", 0},
    {"vós a benção e a", "vós a bênção e a", 0},
    {"À cerca do qual", "Acerca do qual", 0},
    {"À meia noite eu", "À meia-noite eu", 0},
    {"À meia noite me", "À meia-noite me", 0},
    {"à meia noite os", "à meia-noite os", 0},
    {"à meia noite, e", "à meia-noite, e", 0},
    {"Acaso ha, entre", "Acaso há, entre", 0},
    {"ao meio dia, de", "ao meio-dia, de", 0},
    {"aquela corôa, e", "aquela coroa, e", 0},
    {"das adulteras e", "das adúlteras e", 0},
    {"de preza: todos", "de presa: todos", 0},
    {"de purpura, que", "de púrpura, que", 0},
    {"de seu pais: em", "de seus pais: em", 0},
    {"disse: Com que?", "disse: Com quê?", 0},
    {"do bode vive, e", "do bode vivo, e", 0},
    {"do candieiro de", "do candeeiro de", 0},
    {"dons aos homem.", "dons aos homens.", 0},
    {"e à tuas casas;", "e às tuas casas;", 0},
    {"e adultera pede", "e adúltera pede", 0},
    {"E constitui ali", "E constituí ali", 0},
    {"e dar-te-hei um", "e dar-te-ei um", 0},
    {"e diminui a tua", "e diminuí a tua", 0},
    {"é mais sabia do", "é mais sábia do", 0},
    {"e purpura; obra", "e púrpura; obra", 0},
    {"e tão pouco vos", "e tampouco vos", 0},
    {"e te glórias em", "e te glorias em", 0},
    {"Egito sustem, e", "Egito sustêm, e", 0},
    {"em bençãos, mas", "em bênçãos, mas", 0},
    {"fazia, veem ter", "fazia, vêm ter", 0},
    {"fino nela ha, o", "fino nela há, o", 0},
    {"grande corôa de", "grande coroa de", 0},
    {"Ha alguma coisa", "Há alguma coisa", 0},
    {"há benção nele;", "há bênção nele;", 0},
    {"lugar a cima de", "lugar acima de", 0},
    {"lugar e este; e", "lugar a este; e", 0},
    {"luz perpetua, e", "luz perpétua, e", 0},
    {"mais sabias das", "mais sábias das", 0},
    {"não ás de dizer", "não hás de dizer", 0},
    {"Nem tão pouco é", "Nem tampouco é", 0},
    {"nem tão pouco o", "nem tampouco o", 0},
    {"ou ao janela, e", "ou à janela, e", 0},
    {"Pai, possui por", "Pai, possuí por", 0},
    {"Pela benção dos", "Pela bênção dos", 0},
    {"Pois que? Somos", "Pois quê? Somos", 0},
    {"por corações se", "por caracóis se", 0},
    {"por preza, como", "por presa, como", 0},
    {"porém não a ha.", "porém não a há.", 0},
    {"porque ás de ir", "porque hás de ir", 0},
    {"Porque dirão as", "Por que dirão as", 0},
    {"Porque estás ao", "Por que estás ao", 0},
    {"que ás de fazer", "que hás de fazer", 0},
    {"que distilem os", "que destilem os", 0},
    {"que em ti ha, a", "que em ti há, a", 0},
    {"que nela ha, se", "que nela há, se", 0},
    {"que nele ha, no", "que nele há, no", 0},
    {"que neles ha, e", "que neles há, e", 0},
    {"sairdes de ali.", "sairdes dali.", 0},
    {"sua injuria com", "sua injúria com", 0},
    {"temor ha, porém", "temor há, porém", 0},
    {"ti, a Benjamin.", "ti, a Benjamim.", 0},
    {"tirava a preza.", "tirava a presa.", 0},
    {"três dizimas de", "três dízimas de", 0},
    {"tua benção será", "tua bênção será", 0},
    {"tua suplica que", "tua súplica que", 0},
    {"uma benção: não", "uma bênção: não", 0},
    {"Veem contra mim", "Vêm contra mim", 0},
    {"verdade em nos.", "verdade em nós.", 0},
    {"a corôa deste,", "a coroa deste,", 0},
    {"a suplica, que", "a súplica, que", 0},
    {"a tua preza, e", "a tua presa, e", 0},
    {"alguns ha, dos", "alguns há, dos", 0},
    {"ao meio dia um", "ao meio-dia um", 0},
    {"ao meio dia, e", "ao meio-dia, e", 0},
    {"Ao meio dia, ó", "Ao meio-dia, ó", 0},
    {"ao meio dia: e", "ao meio-dia: e", 0},
    {"as escorias, e", "as escórias, e", 0},
    {"com alaudes, e", "com alaúdes, e", 0},
    {"da corôa serão", "da coroa serão", 0},
    {"da Media dirão", "da Média dirão", 0},
    {"das bençãos de", "das bênçãos de", 0},
    {"de escorias de", "de escórias de", 0},
    {"de purpura, da", "de púrpura, da", 0},
    {"de purpura, e,", "de púrpura, e,", 0},
    {"de rubins, mas", "de rubis, mas", 0},
    {"do meio dia ao", "do meio-dia ao", 0},
    {"e alaudes para", "e alaúdes para", 0},
    {"e corôa, estai", "e coroa, estai", 0},
    {"e dar-te-hei a", "e dar-te-ei a", 0},
    {"em escorias, o", "em escórias, o", 0},
    {"em purpura, em", "em púrpura, em", 0},
    {"eu tão pouco a", "eu tampouco a", 0},
    {"glória veem de", "glória vêm de", 0},
    {"Ha uma geração", "Há uma geração", 0},
    {"juízo sustem a", "juízo sustém a", 0},
    {"meus ciumes se", "meus ciúmes se", 0},
    {"minha suplica.", "minha súplica.", 0},
    {"nele ha: este,", "nele há: este,", 0},
    {"o candieiro, e", "o candeeiro, e", 0},
    {"Porque? Porque", "Por quê? Porque", 0},
    {"que nela ha, e", "que nela há, e", 0},
    {"que nele ha, e", "que nele há, e", 0},
    {"rei: Ha paz? E", "rei: Há paz? E", 0},
    {"só benção, meu", "só bênção, meu", 0},
    {"sua corôa; aos", "sua coroa; aos", 0},
    {"sua suplica, e", "sua súplica, e", 0},
    {"sua suplica, ó", "sua súplica, ó", 0},
    {"Tão pouco Aser", "Tampouco Aser", 0},
    {"Tão pouco para", "Tampouco para", 0},
    {"uma benção até", "uma bênção até", 0},
    {"uma benção, em", "uma bênção, em", 0},
    {"vós ha, vede-o", "vós há, vede-o", 0},
    {"a benção, ela", "a bênção, ela", 0},
    {"a benção, que", "a bênção, que", 0},
    {"a corôa real,", "a coroa real,", 0},
    {"a corôa; esta", "a coroa; esta", 0},
    {"à meia noite.", "à meia-noite.", 0},
    {"a pratica até", "a prática até", 0},
    {"a preza: seus", "a presa: seus", 0},
    {"ainda retem a", "ainda retém a", 0},
    {"As bençãos de", "As bênçãos de", 0},
    {"com alaudes e", "com alaúdes e", 0},
    {"de purpura, o", "de púrpura, o", 0},
    {"de purpura. E", "de púrpura. E", 0},
    {"diga: Ha paz?", "diga: Há paz?", 0},
    {"Donde veem as", "Donde vêm as", 0},
    {"e a adultera.", "e a adúltera.", 0},
    {"e o meio dia.", "e o meio-dia.", 0},
    {"e purpura das", "e púrpura das", 0},
    {"Eis-aí, pois,", "Eis aí, pois,", 0},
    {"em ciumes com", "em ciúmes com", 0},
    {"em purpura, e", "em púrpura, e", 0},
    {"está sobro as", "está sobre as", 0},
    {"Ha uns homens", "Há uns homens", 0},
    {"homem ha, que", "homem há, que", 0},
    {"homem retem a", "homem retém a", 0},
    {"Nada ha, fora", "Nada há, fora", 0},
    {"nome ha, dado", "nome há, dado", 0},
    {"para preza, e", "para presa, e", 0},
    {"pela preza, e", "pela presa, e", 0},
    {"Porque saí da", "Por que saí da", 0},
    {"que lhe ás de", "que lhe hás de", 0},
    {"quem ha, como", "quem há, como", 0},
    {"tu ciumes por", "tu ciúmes por", 0},
    {"tu lhes ás de", "tu lhes hás de", 0},
    {"tu preza para", "tu presa para", 0},
    {"uma benção do", "uma bênção do", 0},
    {"uma benção no", "uma bênção no", 0},
    {"a benção que", "a bênção que", 0},
    {"a corôa real", "a coroa real", 0},
    {"a purpura, o", "a púrpura, o", 0},
    {"a purpura: o", "a púrpura: o", 0},
    {"a suplica do", "a súplica do", 0},
    {"à suplica do", "à súplica do", 0},
    {"açoite ha, e", "açoite há, e", 0},
    {"ao meio dia.", "ao meio-dia.", 0},
    {"ao meio dia:", "ao meio-dia:", 0},
    {"aqui veem os", "aqui vêm os", 0},
    {"da Arabia, e", "da Arábia, e", 0},
    {"da Media, os", "da Média, os", 0},
    {"de musica de", "de música de", 0},
    {"de musica, e", "de música, e", 0},
    {"de purpura e", "de púrpura e", 0},
    {"de rubins, e", "de rubis, e", 0},
    {"do peço, que", "do poço, que", 0},
    {"E Hamã disse", "E Hámã disse", 0},
    {"e o outra da", "e o outro da", 0},
    {"ele retem as", "ele retém as", 0},
    {"em purpura e", "em púrpura e", 0},
    {"Ha muito que", "Há muito que", 0},
    {"muita preza.", "muita presa.", 0},
    {"o sustem com", "o sustém com", 0},
    {"os rubins, e", "os rubis, e", 0},
    {"os rubins; e", "os rubis; e", 0},
    {"para traz, é", "para trás, é", 0},
    {"Pois que? Se", "Pois quê? Se", 0},
    {"que te ás de", "que te hás de", 0},
    {"Saul sairá à", "Saul saíra à", 0},
    {"sua corôa, e", "sua coroa, e", 0},
    {"sua preza, e", "sua presa, e", 0},
    {"tu ás de ser", "tu hás de ser", 0},
    {"uma corôa de", "uma coroa de", 0},
    {"uma corôa, e", "uma coroa, e", 0},
    {"uma pratica.", "uma prática.", 0},
    {"vos fiéis em", "vos fieis em", 0},
    {"a Arabia, e", "a Arábia, e", 0},
    {"A benção do", "A bênção do", 0},
    {"à benção do", "à bênção do", 0},
    {"a corôa com", "a coroa com", 0},
    {"A corôa dos", "A coroa dos", 0},
    {"a corôa que", "a coroa que", 0},
    {"da corôa, e", "da coroa, e", 0},
    {"de purpura.", "de púrpura.", 0},
    {"de rezes, e", "de reses, e", 0},
    {"donde veem?", "donde vêm?", 0},
    {"e purpura é", "e púrpura é", 0},
    {"em benção a", "em bênção a", 0},
    {"Ha caminho,", "Há caminho,", 0},
    {"há preza; e", "há presa; e", 0},
    {"Ha tempo de", "Há tempo de", 0},
    {"Ha um homem", "Há um homem", 0},
    {"nada retem.", "nada retém.", 0},
    {"não benção.", "não bênção.", 0},
    {"não retem a", "não retém a", 0},
    {"ou corôa de", "ou coroa de", 0},
    {"Pois que? O", "Pois quê? O", 0},
    {"que retem a", "que retém a", 0},
    {"que retem o", "que retém o", 0},
    {"sua benção.", "sua bênção.", 0},
    {"te corôa de", "te coroa de", 0},
    {"tua benção.", "tua bênção.", 0},
    {"uma benção.", "uma bênção.", 0},
    {"a corôa da", "a coroa da", 0},
    {"a corôa do", "a coroa do", 0},
    {"a corôa, e", "a coroa, e", 0},
    {"a musica e", "a música e", 0},
    {"a preza: o", "a presa: o", 0},
    {"a preza; e", "a presa; e", 0},
    {"a Tadeo, e", "a Tadeu, e", 0},
    {"da Media e", "da Média e", 0},
    {"de musica.", "de música.", 0},
    {"de rubins.", "de rubis.", 0},
    {"e espia os", "e expia os", 0},
    {"em benção.", "em bênção.", 0},
    {"Ha caminho", "Há caminho", 0},
    {"Ha de ser,", "Há de ser,", 0},
    {"Ha mal que", "Há mal que", 0},
    {"Ha, porém,", "Há, porém,", 0},
    {"mui sabia.", "mui sábia.", 0},
    {"Pai. Ha um", "Pai. Há um", 0},
    {"por corôa.", "por coroa.", 0},
    {"ser preza?", "ser presa?", 0},
    {"sua corôa.", "sua coroa.", 0},
    {"tua corôa.", "tua coroa.", 0},
    {"a Eser, e", "a Aser, e", 0},
    {"a preza e", "a presa e", 0},
    {"Arabia, e", "Arábia, e", 0},
    {"da Media:", "da Média:", 0},
    {"de preza.", "de presa.", 0},
    {"e Media e", "e Média e", 0},
    {"e retem a", "e retém a", 0},
    {"Ha alguns", "Há alguns", 0},
    {"Ha um mal", "Há um mal", 0},
    {"Ha um rio", "Há um rio", 0},
    {"Ha um tal", "Há um tal", 0},
    {"neles ha;", "neles há;", 0},
    {"outro ha.", "outro há.", 0},
    {"terra ha.", "terra há.", 0},
    {"Ha outro", "Há outro", 0},
    {"Ha um só", "Há um só", 0},
    {"nela ha.", "nela há.", 0},
    {"Ha ouro", "Há ouro", 0},
    {"Ha, por", "Há, por", 0},
    {"Ha só", "Há só", 0},
    {"Compadecer-me-hei", "Compadecer-me-ei", 1},
    {"considerar-te-hão", "considerar-te-ão", 1},
    {"incorrutibilidade", "incorruptibilidade", 1},
    {"restituir-vos-hei", "restituir-vos-ei", 1},
    {"anunciar-vos-hei", "anunciar-vos-ei", 1},
    {"converter-me-hei", "converter-me-ei", 1},
    {"perguntar-te-hei", "perguntar-te-ei", 1},
    {"abençoar-te-hei", "abençoar-te-ei", 1},
    {"abrir-se-vos-ha", "abrir-se-vos-á", 1},
    {"castigar-te-hei", "castigar-te-ei", 1},
    {"desposar-te-hei", "desposar-te-ei", 1},
    {"Ensinar-vos-hei", "Ensinar-vos-ei", 1},
    {"enviar-vô-lo-ei", "enviar-vo-lo-ei", 1},
    {"levantar-me-hei", "levantar-me-ei", 1},
    {"prantear-te-hão", "prantear-te-ão", 1},
    {"rejeitar-te-hei", "rejeitar-te-ei", 1},
    {"alegrar-me-hei", "alegrar-me-ei", 1},
    {"entregar-mo-lo", "entregá-lo", 1},
    {"exaltar-te-hei", "exaltar-te-ei", 1},
    {"mostrar-to-hei", "mostrar-to-ei", 1},
    {"queixar-me-hei", "queixar-me-ei", 1},
    {"ultra-passarão", "ultrapassarão", 1},
    {"vomitar-te-hei", "vomitar-te-ei", 1},
    {"aborreceram-o", "aborreceram-no", 1},
    {"chegar-me-hei", "chegar-me-ei", 1},
    {"coloquintidas", "coloquíntidas", 1},
    {"dar-se-vos-ha", "dar-se-vos-á", 1},
    {"despojaram-as", "despojaram-nas", 1},
    {"encontraram-o", "encontraram-no", 1},
    {"guiar-vos-hei", "guiar-vos-ei", 1},
    {"inexcrutáveis", "inescrutáveis", 1},
    {"Lançar-te-hão", "Lançar-te-ão", 1},
    {"servil-os-hão", "servi-los-ão", 1},
    {"sobre-maneira", "sobremaneira", 1},
    {"Tornar-me-hei", "Tornar-me-ei", 1},
    {"ajuntaram-as", "ajuntaram-nas", 1},
    {"alvoraçadora", "alvoroçadora", 1},
    {"anti-cristos", "anticristos", 1},
    {"dar-se-te-ha", "dar-se-te-á", 1},
    {"expectadores", "espectadores", 1},
    {"gozar-me-hei", "gozar-me-ei", 1},
    {"inexcrutável", "inescrutável", 1},
    {"louvar-te-ha", "louvar-te-á", 1},
    {"matar-me-hão", "matar-me-ão", 1},
    {"Ouvir-te-hei", "Ouvir-te-ei", 1},
    {"pôr-lhes-hei", "pôr-lhes-ei", 1},
    {"regar-te-hei", "regar-te-ei", 1},
    {"sepultaram-o", "sepultaram-no", 1},
    {"ser-lhes-hei", "ser-lhes-ei", 1},
    {"anti-cristo", "anticristo", 1},
    {"dar-lhe-hei", "dar-lhe-ei", 1},
    {"destilharão", "desfilharão", 1},
    {"far-lhe-hei", "far-lhe-ei", 1},
    {"Far-vos-hei", "Far-vos-ei", 1},
    {"interpretes", "intérpretes", 1},
    {"lançaram-as", "lançaram-nas", 1},
    {"pirhatonita", "piratonita", 1},
    {"ser-lhe-hei", "ser-lhe-ei", 1},
    {"voluntarias", "voluntárias", 1},
    {"adoraram-o", "adoraram-no", 1},
    {"amalekitas", "amalequitas", 1},
    {"ante-ontem", "anteontem", 1},
    {"corrutores", "corruptores", 1},
    {"Dar-te-hei", "Dar-te-ei", 1},
    {"distilante", "destilante", 1},
    {"distilarão", "destilarão", 1},
    {"far-te-hão", "far-te-ão", 1},
    {"feriram-os", "feriram-nos", 1},
    {"lançaram-a", "lançaram-na", 1},
    {"lançaram-o", "lançaram-no", 1},
    {"machinaram", "maquinaram", 1},
    {"magnificas", "magníficas", 1},
    {"pôr-te-hei", "pôr-te-ei", 1},
    {"seguiram-o", "seguiram-no", 1},
    {"ser-me-hão", "ser-me-ão", 1},
    {"servos-hão", "ser-vos-ão", 1},
    {"tributaria", "tributária", 1},
    {"vanglorias", "vanglórias", 1},
    {"venderam-o", "venderam-no", 1},
    {"amalekita", "amalequita", 1},
    {"archeiros", "arqueiros", 1},
    {"co-eleita", "coeleita", 1},
    {"contraria", "contrária", 1},
    {"deleitara", "deleitará", 1},
    {"exhaustos", "exaustos", 1},
    {"glória-se", "gloria-se", 1},
    {"individuo", "indivíduo", 1},
    {"industria", "indústria", 1},
    {"insignias", "insígnias", 1},
    {"Ir-me-hei", "Ir-me-ei", 1},
    {"kohatitas", "coatitas", 1},
    {"koratitas", "coratitas", 1},
    {"ladroices", "ladroíces", 1},
    {"magnifica", "magnífica", 1},
    {"pacificas", "pacíficas", 1},
    {"perpetuas", "perpétuas", 1},
    {"piramides", "pirâmides", 1},
    {"puseram-o", "puseram-no", 1},
    {"recamaras", "recâmaras", 1},
    {"subtileza", "sutileza", 1},
    {"sustem-me", "sustém-me", 1},
    {"tiraram-o", "tiraram-no", 1},
    {"tomaram-o", "tomaram-no", 1},
    {"Trabalhao", "Trabalhai", 1},
    {"zoratisas", "zoratitas", 1},
    {"arrateis", "arráteis", 1},
    {"asdotita", "asdodita", 1},
    {"astucias", "astúcias", 1},
    {"cireneos", "cireneus", 1},
    {"colectas", "coletas", 1},
    {"corrutos", "corruptos", 1},
    {"delicias", "delícias", 1},
    {"deslisar", "deslizar", 1},
    {"dessemos", "déssemos", 1},
    {"estatuas", "estátuas", 1},
    {"estereis", "estéreis", 1},
    {"excepção", "exceção", 1},
    {"exceptua", "excetua", 1},
    {"fartaras", "fartarás", 1},
    {"hospedes", "hóspedes", 1},
    {"idolatra", "idólatra", 1},
    {"injurias", "injúrias", 1},
    {"leviatan", "leviatã", 1},
    {"machinas", "máquinas", 1},
    {"Mercurio", "Mercúrio", 1},
    {"por-se-á", "pôr-se-á", 1},
    {"presagio", "presságio", 1},
    {"recamara", "recâmara", 1},
    {"servos-á", "ser-vos-á", 1},
    {"suplicas", "súplicas", 1},
    {"anekins", "enaquins", 1},
    {"Bençãos", "Bênçãos", 1},
    {"caíamos", "caiamos", 1},
    {"colecta", "coleta", 1},
    {"colonia", "colônia", 1},
    {"corruto", "corrupto", 1},
    {"cosidos", "cozidos", 1},
    {"dividas", "dívidas", 1},
    {"escoria", "escória", 1},
    {"espirou", "expirou", 1},
    {"Excepto", "Exceto", 1},
    {"fabulas", "fábulas", 1},
    {"guizado", "guisado", 1},
    {"laminas", "lâminas", 1},
    {"licitas", "lícitas", 1},
    {"lubrica", "lúbrica", 1},
    {"maculas", "máculas", 1},
    {"magicas", "mágicas", 1},
    {"olfacto", "olfato", 1},
    {"peco-te", "peço-te", 1},
    {"publica", "pública", 1},
    {"Saíamos", "Saiamos", 1},
    {"saíamos", "saiamos", 1},
    {"sais-te", "saíste", 1},
    {"secundo", "segundo", 1},
    {"sosterá", "susterá", 1},
    {"tesoiro", "tesouro", 1},
    {"ulceras", "úlceras", 1},
    {"viram-o", "viram-no", 1},
    {"abstêm", "abstém", 1},
    {"asimos", "ázimos", 1},
    {"cassia", "cássia", 1},
    {"corôas", "coroas", 1},
    {"cosida", "cozida", 1},
    {"dizima", "dízima", 1},
    {"guizar", "guisar", 1},
    {"halito", "hálito", 1},
    {"impias", "ímpias", 1},
    {"intima", "íntima", 1},
    {"Johnan", "Johanan", 1},
    {"kineos", "queneus", 1},
    {"Lazaro", "Lázaro", 1},
    {"magica", "mágica", 1},
    {"nodoas", "nódoas", 1},
    {"propôr", "propor", 1},
    {"sereia", "sereis", 1},
    {"tisica", "tísica", 1},
    {"argue", "argui", 1},
    {"Corôa", "Coroa", 1},
    {"cousa", "coisa", 1},
    {"emfim", "enfim", 1},
    {"Fieis", "Fiéis", 1},
    {"impia", "ímpia", 1},
    {"impôr", "impor", 1},
    {"locar", "lugar", 1},
    {"Logar", "Lugar", 1},
    {"o[o?]", "o", 1},
    {"Orion", "Órion", 1},
    {"orion", "Órion", 1},
    {"pul-a", "pu-la", 1},
    {"regoa", "régua", 1},
    {"Retem", "Retém", 1},
    {"sesta", "sexta", 1},
    {"Vô-lo", "Vo-lo", 1},
    {"opôr", "opor", 1},
    {"crú", "cru", 1},
    {"Más", "Mas", 1},
    {"poz", "pôs", 1},
        };
        int total_rev = (int)(sizeof(revisao) / sizeof(revisao[0]));
        for (int z = 0; z < total_rev; z++) {
            if (revisao[z].inteira) substituir_palavra_inteira(texto, revisao[z].antigo, revisao[z].novo);
            else substituir_texto(texto, revisao[z].antigo, revisao[z].novo);
        }
    }
    /* Mesóclise -hei/-hão depois de pronome (louvar-te-hei -> louvar-te-ei, Ser-vos-hão -> Ser-vos-ão).
       Fica depois da parte 12 para não mudar o texto que as frases da revisão procuram. */
    {
        static const char *pron_meso[] = {"me", "te", "se", "nos", "vos", "lhe", "lhes"};
        static const char *fim_meso[][2] = {{"hei", "ei"}, {"hão", "ão"}};
        for (int p = 0; p < 7; p++)
            for (int f = 0; f < 2; f++) {
                char ant[24], nov[24];
                snprintf(ant, sizeof(ant), "-%s-%s", pron_meso[p], fim_meso[f][0]);
                snprintf(nov, sizeof(nov), "-%s-%s", pron_meso[p], fim_meso[f][1]);
                substituir_texto(texto, ant, nov);
            }
    }
    converter_infinitivo_lusitano_para_gerundio(texto);
}

void limpar_texto_verso(const char *origem, char *destino) {
    int i = 0;
    int pos = 0;

    while (origem[i] != '\0' && origem[i] != '\n' && origem[i] != '\r') {
        if (origem[i] == '*') {
            i++;
            continue;
        }

        if (origem[i] == '[') {
            int j = i + 1;
            if (origem[j] == '[' || origem[j] == '^' || isdigit((unsigned char)origem[j])) {
                while (origem[j] != '\0' && origem[j] != ']') j++;
                if (origem[j] == '\0') {          /* '[' sem ']': mantem o texto */
                    destino[pos++] = origem[i++];
                    continue;
                }
                if (origem[j] == ']') j++;
                if (origem[j] == ']') j++;

                if (origem[j] == '(') {
                    while (origem[j] != '\0' && origem[j] != ')') j++;
                    if (origem[j] == ')') j++;
                }

                if (pos > 0 && destino[pos - 1] == ' ' && origem[j] == ' ') {
                    j++;
                }

                i = j;
                continue;
            }
        }

        destino[pos++] = origem[i++];
    }
    destino[pos] = '\0';
}

/* -----------------------------------------------------------------------------
- 4. LEITURA E EXIBIÇÃO DE CAPÍTULOS (.md) ESTILO PÁGINA CENTRALIZADA
- ----------------------------------------------------------------------------- */

FILE *abrir_arquivo_capitulo(const char *pasta_livro, int capitulo, char *caminho_out) {
    sprintf(caminho_out, "%s/%02d.md", pasta_livro, capitulo);
    FILE *f = fopen(caminho_out, "r");

    if (f == NULL) {
        sprintf(caminho_out, "%s/%03d.md", pasta_livro, capitulo);
        f = fopen(caminho_out, "r");
    }
    return f;
}

/* Retorna: 1 = continuar | 0 = voltar ('0') | 2 = reimprimir capítulo inteiro ('T') */
int verificar_pausa_pagina(int *linhas_na_pagina, int *modo_continuo, const char *nome_livro, int capitulo) {
    if (*modo_continuo) return 1;

    int cols = 80, rows = 24;
    obter_dimensoes_terminal(&cols, &rows);
    int limite_linhas = (rows > 8) ? (rows - 5) : 18;

    if (*linhas_na_pagina >= limite_linhas) {
        int margem = calcular_margem_esquerda(LARGURA_PAGINA);
        imprimir_separador(LARGURA_PAGINA, '-');
        imprimir_espacos(margem);
        printf("%s[ENTER] Próxima página  |  [T] Ler capítulo inteiro  |  [0] Voltar: %s",
               COR_DIM, COR_RESET);
        fflush(stdout);

        int ch = ler_comando_tecla();
        if (ch == '0') {
            return 0;
        }
        if (ch == 't' || ch == 'T') {
            *modo_continuo = 1;
            return 2;
        }

        limpar_tela();
        char cabecalho[160];
        snprintf(cabecalho, sizeof(cabecalho), "%s — Capítulo %d (continuação)", nome_livro, capitulo);
        imprimir_separador(LARGURA_PAGINA, '=');
        printf("%s", COR_TITULO);
        imprimir_linha_centralizada(cabecalho, LARGURA_PAGINA);
        printf("%s", COR_RESET);
        imprimir_separador(LARGURA_PAGINA, '=');
        printf("\n");
        *linhas_na_pagina = 4;
    }
    return 1;
}

void imprimir_verso_formatado(const char *versiculo_num, const char *texto, int *linhas_impressas) {
    int margem = calcular_margem_esquerda(LARGURA_PAGINA);
    char prefixo[32];
    snprintf(prefixo, sizeof(prefixo), "[%s] ", versiculo_num);
    int larg_prefixo = comprimento_visivel_utf8(prefixo);

    imprimir_espacos(margem);
    printf("%s%s%s", COR_VERSO, prefixo, COR_RESET);

    int col_atual = larg_prefixo;
    const char *p = texto;

    while (*p != '\0') {
        while (*p == ' ') p++;
        if (*p == '\0') break;

        const char *fim_palavra = p;
        int larg_palavra = 0;
        while (*fim_palavra != '\0' && *fim_palavra != ' ') {
            if ((*fim_palavra & 0xC0) != 0x80) {
                larg_palavra++;
            }
            fim_palavra++;
        }

        if (col_atual > larg_prefixo && (col_atual + 1 + larg_palavra) > LARGURA_PAGINA) {
            putchar('\n');
            (*linhas_impressas)++;
            imprimir_espacos(margem + larg_prefixo);
            col_atual = larg_prefixo;
        } else if (col_atual > larg_prefixo) {
            putchar(' ');
            col_atual++;
        }

        fwrite(p, 1, (size_t)(fim_palavra - p), stdout);
        col_atual += larg_palavra;
        p = fim_palavra;
    }

    printf("\n\n");
    (*linhas_impressas) += 2;
}

void imprimir_subtitulo_formatado(const char *texto, int *linhas_impressas) {
    int margem = calcular_margem_esquerda(LARGURA_PAGINA);
    imprimir_espacos(margem);
    printf("%s%s(%s)%s\n\n", COR_TITULO, COR_ITALICO, texto, COR_RESET);
    (*linhas_impressas) += 2;
}

/* Retorna 1 se leu até o fim (pede ENTER no final), ou 0 se usuário apertou '0' (volta direto) */
int ler_capitulo(const char *nome_livro, const char *pasta_livro, int capitulo) {
    char caminho_ficheiro[200];
    FILE *ficheiro = abrir_arquivo_capitulo(pasta_livro, capitulo, caminho_ficheiro);

    if (ficheiro == NULL) {
        int margem = calcular_margem_esquerda(LARGURA_PAGINA);
        printf("\n");
        imprimir_espacos(margem);
        printf("(Erro: Não foi possível encontrar o arquivo '%s'.)\n", caminho_ficheiro);
        return 1;
    }

    int modo_continuo = 0;

reiniciar_leitura:
    rewind(ficheiro);
    limpar_tela();

    char cabecalho[160];
    snprintf(cabecalho, sizeof(cabecalho), "%s — Capítulo %d", nome_livro, capitulo);

    imprimir_separador(LARGURA_PAGINA, '=');
    printf("%s", COR_TITULO);
    imprimir_linha_centralizada(cabecalho, LARGURA_PAGINA);
    printf("%s", COR_RESET);
    imprimir_separador(LARGURA_PAGINA, '=');
    printf("\n");

    char linha[MAX_LINHA];
    char texto_limpo[MAX_LINHA];
    int linhas_na_pagina = 4;

    int primeira_linha = 1;
    while (fgets(linha, sizeof(linha), ficheiro) != NULL) {
        size_t tam_linha = strlen(linha);
        if (tam_linha > 0 && linha[tam_linha - 1] != '\n' && !feof(ficheiro)) {
            int ch; /* linha maior que MAX_LINHA: descarta o resto, para nao virar versiculo falso */
            while ((ch = fgetc(ficheiro)) != '\n' && ch != EOF) {}
        }
        if (primeira_linha && (unsigned char)linha[0] == 0xEF &&
            (unsigned char)linha[1] == 0xBB && (unsigned char)linha[2] == 0xBF) {
            memmove(linha, linha + 3, strlen(linha + 3) + 1); /* tira o BOM do UTF-8 */
        }
        primeira_linha = 0;

        if (strncmp(linha, "---", 3) == 0) {
            break;
        }

        if (linha[0] == '#' || linha[0] == '[') {
            continue;
        }

        int idx = 0;
        while (linha[idx] == '*' || linha[idx] == ' ') {
            idx++;
        }

        if (linha[idx] >= '1' && linha[idx] <= '9') {
            int acao = verificar_pausa_pagina(&linhas_na_pagina, &modo_continuo, nome_livro, capitulo);
            if (acao == 0) {
                fclose(ficheiro);
                return 0;
            }
            if (acao == 2) {
                goto reiniciar_leitura;
            }

            char versiculo_num[20];
            int v_pos = 0;

            while (linha[idx] >= '0' && linha[idx] <= '9' && v_pos < 19) {
                versiculo_num[v_pos++] = linha[idx++];
            }
            versiculo_num[v_pos] = '\0';

            while (linha[idx] == '*' || linha[idx] == ' ') {
                idx++;
            }

            limpar_texto_verso(&linha[idx], texto_limpo);
            atualizar_ortografia(texto_limpo);
            imprimir_verso_formatado(versiculo_num, texto_limpo, &linhas_na_pagina);
        }
        else if (strncmp(linha, "**", 2) == 0 && linha[idx] != '\0' && linha[idx] != '\n' && linha[idx] != '\r') {
            int acao = verificar_pausa_pagina(&linhas_na_pagina, &modo_continuo, nome_livro, capitulo);
            if (acao == 0) {
                fclose(ficheiro);
                return 0;
            }
            if (acao == 2) {
                goto reiniciar_leitura;
            }

            limpar_texto_verso(&linha[idx], texto_limpo);
            atualizar_ortografia(texto_limpo);
            imprimir_subtitulo_formatado(texto_limpo, &linhas_na_pagina);
        }
    }

    imprimir_separador(LARGURA_PAGINA, '=');
    fclose(ficheiro);
    return 1;
}

/* -----------------------------------------------------------------------------
- 5. FUNÇÃO PRINCIPAL: main
- ----------------------------------------------------------------------------- */

int main(void) {
    configurar_terminal();

    int opcao_principal = 0;
    int opcao_livro = 0;
    int opcao_capitulo = 0;

    do {
        limpar_tela();
        exibir_data_hora();

        int margem = calcular_margem_esquerda(LARGURA_PAGINA);

        imprimir_separador(LARGURA_PAGINA, '=');
        printf("%s", COR_TITULO);
        imprimir_linha_centralizada("BÍBLIA DIGITAL ALMEIDA 1911 EM PT-BR (DOMÍNIO PÚBLICO)", LARGURA_PAGINA);
        imprimir_linha_centralizada("Atualização Ortográfica e Gramatical realizada em 11/2026", LARGURA_PAGINA);
        printf("%s%s", COR_RESET, COR_DIM);
        imprimir_linha_centralizada("Motor de navegação para Windows  |  Autor: Romulo CSR", LARGURA_PAGINA);
        imprimir_linha_centralizada("Download free: github.com/RomuloCSR", LARGURA_PAGINA);
        imprimir_linha_centralizada("Colabore espontaneamente com esta obra via PIX: 21980934118", LARGURA_PAGINA);
        printf("%s", COR_RESET);
        imprimir_separador(LARGURA_PAGINA, '=');
        printf("\n");

        imprimir_espacos(margem + 4); printf("1 - Antigo Testamento\n");
        imprimir_espacos(margem + 4); printf("2 - Novo Testamento\n");
        imprimir_espacos(margem + 4); printf("3 - Sair\n\n");
        imprimir_espacos(margem + 4); printf("%sZoom da fonte: Ctrl + = / - (ou Ctrl + roda do mouse)%s\n\n", COR_DIM, COR_RESET);
        imprimir_espacos(margem + 4); printf("Escolha uma opção: ");

        if (scanf("%d", &opcao_principal) != 1) {
            limpar_buffer_entrada();
            continue;
        }
        limpar_buffer_entrada();

        if (opcao_principal == 1 || opcao_principal == 2) {
            const char **nomes = (opcao_principal == 1) ? nomes_at : nomes_nt;
            const char **pastas = (opcao_principal == 1) ? pastas_at : pastas_nt;
            const int *max_caps = (opcao_principal == 1) ? capitulos_at : capitulos_nt;
            int total_livros = (opcao_principal == 1) ? 39 : 27;

            limpar_tela();
            imprimir_separador(LARGURA_PAGINA, '=');
            printf("%s", COR_TITULO);
            imprimir_linha_centralizada((opcao_principal == 1) ? "ANTIGO TESTAMENTO" : "NOVO TESTAMENTO", LARGURA_PAGINA);
            printf("%s", COR_RESET);
            imprimir_separador(LARGURA_PAGINA, '=');
            printf("\n");

            for (int i = 0; i < total_livros; i++) {
                if (i % 3 == 0) {
                    imprimir_espacos(margem);
                }
                imprimir_coluna(i + 1, nomes[i]);
                if ((i + 1) % 3 == 0) {
                    printf("\n");
                }
            }

            if (total_livros % 3 != 0) {
                printf("\n");
            }

            printf("\n");
            imprimir_espacos(margem); printf(" 0 - Voltar\n");
            imprimir_espacos(margem); printf(" Escolha um livro: ");
            if (scanf("%d", &opcao_livro) != 1) {
                limpar_buffer_entrada();
                continue;
            }
            limpar_buffer_entrada();

            if (opcao_livro >= 1 && opcao_livro <= total_livros) {
                int total_do_livro = max_caps[opcao_livro - 1];

                printf("\n");
                imprimir_espacos(margem);
                printf("%s selecionado. Digite o capítulo (1 a %d): ",
                       nomes[opcao_livro - 1], total_do_livro);

                if (scanf("%d", &opcao_capitulo) == 1 &&
                    opcao_capitulo >= 1 && opcao_capitulo <= total_do_livro) {
                    limpar_buffer_entrada();
                    int leu_ate_o_fim = ler_capitulo(nomes[opcao_livro - 1], pastas[opcao_livro - 1], opcao_capitulo);
                    if (leu_ate_o_fim) {
                        pausar_tela();
                    }
                } else {
                    limpar_buffer_entrada();
                    printf("\n");
                    imprimir_espacos(margem);
                    printf("Capítulo inválido! O livro de %s possui capítulos de 1 a %d.\n",
                           nomes[opcao_livro - 1], total_do_livro);
                    pausar_tela();
                }
            } else if (opcao_livro != 0) {
                printf("\n");
                imprimir_espacos(margem);
                printf("Opção de livro inválida!\n");
                pausar_tela();
            }

        } else if (opcao_principal == 3) {
            int margem_saida = calcular_margem_esquerda(LARGURA_PAGINA);
            printf("\n");
            imprimir_espacos(margem_saida);
            printf("Saindo do aplicativo...\n");
        } else {
            int margem_inv = calcular_margem_esquerda(LARGURA_PAGINA);
            printf("\n");
            imprimir_espacos(margem_inv);
            printf("Opção inválida! Tente novamente.\n");
            pausar_tela();
        }

    } while (opcao_principal != 3);

    return 0;
}
