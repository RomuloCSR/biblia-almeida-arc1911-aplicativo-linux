/* =============================================================================
- PROJETO: Bíblia Almeida ARC-1911 — Aplicativo para Linux
- OBJETIVO: Janela GTK + WebKitGTK usando o MESMO motor em C (motor/motor.biblia.c)
-           e a mesma tela (ui/index.html). O texto sai igual, letra por letra.
- AUTOR: Romulo CSR (GitHub: RomuloCSR)
- COMPILAR: ./compilar.sh
- =============================================================================
 */

#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <webkit2/webkit2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

/* ---------------------------------------------------------------------------
- 1. KERNEL DA BÍBLIA (28/11/2026)
-    Antes o app ligava o motor.biblia.c inteiro. Agora usa o kernel (kernel/),
-    gerado a partir do mesmo motor: o texto sai igual, byte a byte, ~12x mais rápido.
-    As regras continuam sendo escritas no motor/motor.biblia.c; o kernel é gerado dele.
-    Livros: 0..38 = Antigo Testamento, 39..65 = Novo Testamento.
- --------------------------------------------------------------------------- */
#include "../kernel/biblia_kernel.h"

static int livro_global(int testamento, int livro) {
    if (testamento == 1 && livro >= 0 && livro < 39) return livro;
    if (testamento == 2 && livro >= 0 && livro < 27) return 39 + livro;
    return -1;
}

static GtkWidget *g_janela = NULL;
static WebKitWebView *g_web = NULL;
static char g_pasta_app[PATH_MAX];
static char g_pasta_dados[PATH_MAX + 32];

static const char *URL_GITHUB = "https://github.com/RomuloCSR";

/* ---------------------------------------------------------------------------
- 2. UTILITÁRIOS
- --------------------------------------------------------------------------- */
static void json_texto(GString *s, const char *t) {
    g_string_append_c(s, '"');
    for (; *t; t++) {
        unsigned char c = (unsigned char)*t;
        switch (c) {
            case '"':  g_string_append(s, "\\\""); break;
            case '\\': g_string_append(s, "\\\\"); break;
            case '\n': g_string_append(s, "\\n"); break;
            case '\r': break;
            case '\t': g_string_append_c(s, ' '); break;
            default:
                if (c < 0x20) g_string_append_c(s, ' ');
                else g_string_append_c(s, (char)c);
        }
    }
    g_string_append_c(s, '"');
}

static void pasta_do_programa(void) {
    ssize_t n = readlink("/proc/self/exe", g_pasta_app, sizeof(g_pasta_app) - 1);
    if (n <= 0) { strcpy(g_pasta_app, "."); return; }
    g_pasta_app[n] = '\0';
    char *barra = strrchr(g_pasta_app, '/');
    if (barra) *barra = '\0';
}

/* Modo portátil: dados ao lado do programa (pendrive). Se não der, ~/.local/share */
static void escolher_pasta_dados(void) {
    snprintf(g_pasta_dados, sizeof(g_pasta_dados), "%s/dados", g_pasta_app);
    mkdir(g_pasta_dados, 0755);
    char teste[PATH_MAX + 64];
    snprintf(teste, sizeof(teste), "%s/.teste", g_pasta_dados);
    FILE *f = fopen(teste, "w");
    if (f) { fclose(f); remove(teste); return; }
    snprintf(g_pasta_dados, sizeof(g_pasta_dados), "%s/BibliaARC1911", g_get_user_data_dir());
    g_mkdir_with_parents(g_pasta_dados, 0755);
}

/* ---------------------------------------------------------------------------
- 3. LEITURA DE CAPÍTULO (28/11/2026: feita pelo kernel, bk_capitulo)
-    O kernel lê as linhas do .md como o terminal (tira BOM, ignora linha longa demais),
-    limpa o Markdown e atualiza a ortografia. Aqui só montamos o JSON de cada item.
-    Sem buffers static: pode ser chamado de qualquer thread.
- --------------------------------------------------------------------------- */
typedef struct { GString *s; gboolean primeiro; } SaidaJson;

static int item_json(void *ctx, int tipo, int numero, const char *texto, size_t tamanho_texto) {
    (void)tamanho_texto;
    SaidaJson *o = (SaidaJson *)ctx;
    if (!o->primeiro) g_string_append_c(o->s, ',');
    if (tipo == BK_VERSO) {
        char num[16];
        g_snprintf(num, sizeof(num), "%d", numero);
        g_string_append(o->s, "{\"n\":"); json_texto(o->s, num);
        g_string_append(o->s, ",\"t\":"); json_texto(o->s, texto);
    } else {
        g_string_append(o->s, "{\"s\":"); json_texto(o->s, texto);
    }
    g_string_append_c(o->s, '}');
    o->primeiro = FALSE;
    return 0;
}

static char *capitulo_json(int t, int b, int c) {
    int g = livro_global(t, b);
    const char *pasta = g >= 0 ? bk_livro_pasta(g) : NULL;
    int total = g >= 0 ? bk_livro_capitulos(g) : 0;

    GString *s = g_string_new(NULL);
    g_string_append_printf(s, "{\"tipo\":\"capitulo\",\"t\":%d,\"b\":%d,\"c\":%d,\"versos\":[", t, b, c);
    if (!pasta || c < 1 || c > total) {
        g_string_append(s, "],\"erro\":\"Capítulo inválido.\"}");
        return g_string_free(s, FALSE);
    }

    char caminho[PATH_MAX + 64];
    gchar *conteudo = NULL;
    gsize tamanho = 0;
    snprintf(caminho, sizeof(caminho), "%s/textos/%s/%02d.md", g_pasta_app, pasta, c);
    if (!g_file_get_contents(caminho, &conteudo, &tamanho, NULL)) {
        snprintf(caminho, sizeof(caminho), "%s/textos/%s/%03d.md", g_pasta_app, pasta, c);
        if (!g_file_get_contents(caminho, &conteudo, &tamanho, NULL)) {
            g_string_append(s, "],\"erro\":\"Arquivo do capítulo não encontrado. Verifique a pasta textos.\"}");
            return g_string_free(s, FALSE);
        }
    }

    SaidaJson o = { s, TRUE };
    bk_capitulo(conteudo, (size_t)tamanho, item_json, &o);
    g_free(conteudo);
    g_string_append(s, "]}");
    return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------
- 4. ESTADO SALVO (onde parou, tamanho da letra, tema)
- --------------------------------------------------------------------------- */
typedef struct { int t, b, c, fonte; char tema[16]; } Estado;

static void arquivo_estado(char *out, size_t n) { g_snprintf(out, (gulong)n, "%s/estado.txt", g_pasta_dados); }

static Estado ler_estado(void) {
    Estado e = {1, 0, 1, 20, "claro"};
    char arq[PATH_MAX + 16];
    arquivo_estado(arq, sizeof(arq));
    FILE *f = fopen(arq, "r");
    if (f) {
        Estado l;
        if (fscanf(f, "%d %d %d %d %15s", &l.t, &l.b, &l.c, &l.fonte, l.tema) == 5) e = l;
        fclose(f);
    }
    if (e.t != 1 && e.t != 2) e.t = 1;
    int max_livro = e.t == 1 ? 39 : 27;
    if (e.b < 0 || e.b >= max_livro) e.b = 0;
    int max_cap = bk_livro_capitulos(livro_global(e.t, e.b));
    if (e.c < 1 || e.c > max_cap) e.c = 1;
    if (e.fonte < 14 || e.fonte > 40) e.fonte = 20;
    if (strcmp(e.tema, "claro") && strcmp(e.tema, "sepia") && strcmp(e.tema, "escuro")) strcpy(e.tema, "claro");
    return e;
}

static void salvar_estado(int t, int b, int c, int fonte, const char *tema) {
    char arq[PATH_MAX + 16];
    arquivo_estado(arq, sizeof(arq));
    FILE *f = fopen(arq, "w");
    if (!f) return;
    const char *ok = (!strcmp(tema, "sepia") || !strcmp(tema, "escuro")) ? tema : "claro";
    fprintf(f, "%d %d %d %d %s\n", t, b, c, fonte, ok);
    fclose(f);
}

static char *init_json(void) {
    GString *s = g_string_new("{\"tipo\":\"init\",\"at\":[");
    for (int i = 0; i < 39; i++) {
        if (i) g_string_append_c(s, ',');
        g_string_append(s, "{\"n\":"); json_texto(s, bk_livro_nome(i));
        g_string_append_printf(s, ",\"c\":%d}", bk_livro_capitulos(i));
    }
    g_string_append(s, "],\"nt\":[");
    for (int i = 0; i < 27; i++) {
        if (i) g_string_append_c(s, ',');
        g_string_append(s, "{\"n\":"); json_texto(s, bk_livro_nome(39 + i));
        g_string_append_printf(s, ",\"c\":%d}", bk_livro_capitulos(39 + i));
    }
    Estado e = ler_estado();
    g_string_append_printf(s, "],\"estado\":{\"t\":%d,\"b\":%d,\"c\":%d,\"fonte\":%d,\"tema\":", e.t, e.b, e.c, e.fonte);
    json_texto(s, e.tema);
    g_string_append(s, "}}");
    return g_string_free(s, FALSE);
}

/* ---------------------------------------------------------------------------
- 5. ENVIAR (WhatsApp, Telegram, E-mail, X) e COPIAR
-    O texto completo sempre vai também para a área de transferência.
- --------------------------------------------------------------------------- */
static void copiar_texto(const char *texto) {
    GtkClipboard *cb = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(cb, texto, -1);
    gtk_clipboard_store(cb);
}

static void abrir_url(const char *url) {
    GError *err = NULL;
    if (!g_app_info_launch_default_for_uri(url, NULL, &err)) {
        if (err) g_error_free(err);
    }
}

static void compartilhar(const char *destino, const char *ref, const char *texto) {
    copiar_texto(texto);
    gchar *corpo_cod = g_uri_escape_string(texto, NULL, FALSE);
    gchar *corpo = NULL;
    if (strlen(corpo_cod) > 1800) {
        g_free(corpo_cod);
        corpo = g_strdup_printf("%s\n\n(O texto completo foi copiado. Cole aqui com Ctrl+V.)\n\n"
                                "Bíblia Almeida ARC-1911 — https://github.com/RomuloCSR\n"
                                "Doação espontânea para a obra via PIX: 21980934118", ref);
        corpo_cod = g_uri_escape_string(corpo, NULL, FALSE);
    }
    gchar *url = NULL;
    if (!strcmp(destino, "whatsapp")) {
        url = g_strdup_printf("https://wa.me/?text=%s", corpo_cod);
    } else if (!strcmp(destino, "telegram")) {
        url = g_strdup_printf("https://t.me/share/url?url=%s&text=%s", "https%3A%2F%2Fgithub.com%2FRomuloCSR", corpo_cod);
    } else if (!strcmp(destino, "email")) {
        gchar *assunto = g_strdup_printf("%s - Bíblia Almeida ARC-1911", ref);
        gchar *a = g_uri_escape_string(assunto, NULL, FALSE);
        url = g_strdup_printf("mailto:?subject=%s&body=%s", a, corpo_cod);
        g_free(a); g_free(assunto);
    } else if (!strcmp(destino, "x")) {
        gchar *curto = g_strdup_printf("%s — Bíblia Almeida ARC-1911 (texto copiado, cole com Ctrl+V)", ref);
        gchar *c = g_uri_escape_string(curto, NULL, FALSE);
        url = g_strdup_printf("https://x.com/intent/post?text=%s&url=%s", c, "https%3A%2F%2Fgithub.com%2FRomuloCSR");
        g_free(c); g_free(curto);
    }
    if (url) abrir_url(url);
    g_free(url); g_free(corpo_cod); g_free(corpo);
}

/* ---------------------------------------------------------------------------
- 6. MENSAGENS DA TELA PARA O MOTOR
-    init | cap|t|b|c | salvar|t|b|c|fonte|tema | copiar|texto | enviar|dest|ref|texto | abrir|github
- --------------------------------------------------------------------------- */
static void enviar_para_tela(const char *json) {
    GString *js = g_string_new("window.__bibliaReceber(");
    g_string_append(js, json);
    g_string_append(js, ");");
    webkit_web_view_evaluate_javascript(g_web, js->str, -1, NULL, NULL, NULL, NULL, NULL);
    g_string_free(js, TRUE);
}

static void tratar_mensagem(const char *msg) {
    if (!strcmp(msg, "init")) {
        char *j = init_json(); enviar_para_tela(j); g_free(j);
        return;
    }
    if (!strncmp(msg, "cap|", 4)) {
        gchar **p = g_strsplit(msg, "|", 4);
        if (g_strv_length(p) == 4) {
            char *j = capitulo_json(atoi(p[1]), atoi(p[2]), atoi(p[3]));
            enviar_para_tela(j); g_free(j);
        }
        g_strfreev(p);
        return;
    }
    if (!strncmp(msg, "salvar|", 7)) {
        gchar **p = g_strsplit(msg, "|", 6);
        if (g_strv_length(p) == 6) salvar_estado(atoi(p[1]), atoi(p[2]), atoi(p[3]), atoi(p[4]), p[5]);
        g_strfreev(p);
        return;
    }
    if (!strncmp(msg, "copiar|", 7)) { copiar_texto(msg + 7); return; }
    if (!strncmp(msg, "enviar|", 7)) {
        gchar **p = g_strsplit(msg, "|", 4);
        if (g_strv_length(p) == 4) compartilhar(p[1], p[2], p[3]);
        g_strfreev(p);
        return;
    }
    if (!strcmp(msg, "abrir|github")) { abrir_url(URL_GITHUB); return; }
}

static void ao_receber(WebKitUserContentManager *m, WebKitJavascriptResult *r, gpointer d) {
    (void)m; (void)d;
    JSCValue *v = webkit_javascript_result_get_js_value(r);
    if (jsc_value_is_string(v)) {
        char *s = jsc_value_to_string(v);
        tratar_mensagem(s);
        g_free(s);
    }
}

/* Links externos abrem no navegador, nunca dentro do aplicativo */
static gboolean ao_decidir(WebKitWebView *w, WebKitPolicyDecision *dec, WebKitPolicyDecisionType tipo, gpointer d) {
    (void)w; (void)d;
    if (tipo == WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION) {
        WebKitNavigationAction *a = webkit_navigation_policy_decision_get_navigation_action(WEBKIT_NAVIGATION_POLICY_DECISION(dec));
        const char *uri = webkit_uri_request_get_uri(webkit_navigation_action_get_request(a));
        if (uri && (g_str_has_prefix(uri, "http://") || g_str_has_prefix(uri, "https://") || g_str_has_prefix(uri, "mailto:"))) {
            abrir_url(uri);
            webkit_policy_decision_ignore(dec);
            return TRUE;
        }
    }
    return FALSE;
}

/* Ponte: a tela foi feita para o WebView2 (window.chrome.webview).
   Aqui criamos o mesmo objeto, ligado ao WebKitGTK. */
static const char *PONTE_JS =
    "(function(){"
    "  var ouvintes=[];"
    "  window.chrome = window.chrome || {};"
    "  window.chrome.webview = {"
    "    postMessage: function(s){ window.webkit.messageHandlers.biblia.postMessage(String(s)); },"
    "    addEventListener: function(tipo, fn){ if(tipo==='message') ouvintes.push(fn); }"
    "  };"
    "  window.__bibliaReceber = function(obj){ ouvintes.forEach(function(fn){ fn({data:obj}); }); };"
    "})();";

/* ---------------------------------------------------------------------------
- 7. JANELA
- --------------------------------------------------------------------------- */
static void ativar(GtkApplication *app, gpointer d) {
    (void)d;
    g_janela = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(g_janela), "Bíblia Almeida ARC-1911");

    /* Tamanho que cabe na tela (sem esconder os botões de baixo) */
    int larg = 1000, alt = 760;
    GdkDisplay *disp = gdk_display_get_default();
    GdkMonitor *mon = disp ? gdk_display_get_primary_monitor(disp) : NULL;
    if (!mon && disp) mon = gdk_display_get_monitor(disp, 0);
    if (mon) {
        GdkRectangle area;
        gdk_monitor_get_workarea(mon, &area);
        larg = area.width * 70 / 100;
        alt = area.height * 85 / 100;
        if (larg > 1100) larg = 1100;
        if (larg < 420) larg = area.width;
    }
    gtk_window_set_default_size(GTK_WINDOW(g_janela), larg, alt);
    gtk_window_set_position(GTK_WINDOW(g_janela), GTK_WIN_POS_CENTER);
    gtk_widget_set_size_request(g_janela, 380, 480);

    char icone[PATH_MAX + 32];
    snprintf(icone, sizeof(icone), "%s/ui/icone.png", g_pasta_app);
    if (g_file_test(icone, G_FILE_TEST_EXISTS)) gtk_window_set_icon_from_file(GTK_WINDOW(g_janela), icone, NULL);

    WebKitUserContentManager *ucm = webkit_user_content_manager_new();
    webkit_user_content_manager_add_script(ucm, webkit_user_script_new(PONTE_JS,
        WEBKIT_USER_CONTENT_INJECT_TOP_FRAME, WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START, NULL, NULL));
    g_signal_connect(ucm, "script-message-received::biblia", G_CALLBACK(ao_receber), NULL);
    webkit_user_content_manager_register_script_message_handler(ucm, "biblia");

    g_web = WEBKIT_WEB_VIEW(webkit_web_view_new_with_user_content_manager(ucm));
    WebKitSettings *cfg = webkit_web_view_get_settings(g_web);
    webkit_settings_set_enable_developer_extras(cfg, FALSE);
    webkit_settings_set_enable_write_console_messages_to_stdout(cfg, FALSE);
    g_signal_connect(g_web, "decide-policy", G_CALLBACK(ao_decidir), NULL);

    char html_arq[PATH_MAX + 32];
    snprintf(html_arq, sizeof(html_arq), "%s/ui/index.html", g_pasta_app);
    gchar *html = NULL;
    if (!g_file_get_contents(html_arq, &html, NULL, NULL)) {
        GtkWidget *dlg = gtk_message_dialog_new(GTK_WINDOW(g_janela), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
            GTK_BUTTONS_OK, "Arquivo ui/index.html não encontrado ao lado do programa.");
        gtk_dialog_run(GTK_DIALOG(dlg));
        gtk_widget_destroy(dlg);
        g_application_quit(G_APPLICATION(app));
        return;
    }
    webkit_web_view_load_html(g_web, html, "file:///biblia/");
    g_free(html);

    gtk_container_add(GTK_CONTAINER(g_janela), GTK_WIDGET(g_web));
    gtk_widget_show_all(g_janela);
}

int main(int argc, char **argv) {
    /* Desenho por software: evita avisos e telas pretas em máquinas sem placa de vídeo própria */
    g_setenv("WEBKIT_DISABLE_COMPOSITING_MODE", "1", FALSE);
    g_setenv("WEBKIT_DISABLE_DMABUF_RENDERER", "1", FALSE);
    pasta_do_programa();
    escolher_pasta_dados();
    GtkApplication *app = gtk_application_new("br.com.romulocsr.biblia1911", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(app, "activate", G_CALLBACK(ativar), NULL);
    int r = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return r;
}
