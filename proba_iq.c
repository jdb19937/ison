/*
 * proba_iq.c — probationes instrumenti iq
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int errores = 0;

static void adfirma(int cond, const char *nomen)
{
    if (cond) {
        printf("  bene: %s\n", nomen);
    } else {
        printf("  MALUM: %s\n", nomen);
        errores++;
    }
}

/* scribit chordam in plicam temporariam, reddit viam */
static char *scribe_plicam_temp(const char *praefixum, const char *contentum)
{
    char *via = malloc(256);
    snprintf(via, 256, "/tmp/%s_XXXXXX", praefixum);
    int fd = mkstemp(via);
    if (fd < 0) {
        free(via);
        return NULL;
    }

    FILE *f = fdopen(fd, "w");
    fputs(contentum, f);
    fclose(f);
    return via;
}

/* currit mandatum, captat stdout in buf, reddit exitum */
static int curre_capta(const char *mandatum, char *buf, size_t mag)
{
    FILE *p = popen(mandatum, "r");
    if (!p)
        return -1;
    size_t lon = 0;
    size_t n;
    while ((n = fread(buf + lon, 1, mag - lon - 1, p)) > 0)
        lon += n;
    buf[lon] = '\0';
    int r    = pclose(p);
    return WEXITSTATUS(r);
}

/* ================================================================
 * probationes
 * ================================================================ */

static void proba_objectum_simplex(void)
{
    printf("objectum simplex:\n");

    char *via = scribe_plicam_temp("iq_simp", "{\"a\":\"b\",\"c\":1}");
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "    \"a\": \"b\"") != NULL, "clavis a indenta 4 spatiis");
    adfirma(strstr(buf, "    \"c\": 1") != NULL, "clavis c indenta 4 spatiis");
    adfirma(buf[0] == '{', "incipit cum {");
    adfirma(strstr(buf, "}\n") != NULL, "finitur cum }");

    remove(via);
    free(via);
}

static void proba_objectum_nidum(void)
{
    printf("objectum nidum:\n");

    char *via = scribe_plicam_temp(
        "iq_nid",
        "{\"x\":{\"y\":{\"z\":42}}}"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "    \"x\": {\n") != NULL, "x aperitur in linea sua");
    adfirma(
        strstr(buf, "        \"y\": {\n") != NULL,
        "y indenta 8 spatiis"
    );
    adfirma(
        strstr(buf, "            \"z\": 42\n") != NULL,
        "z indenta 12 spatiis"
    );

    remove(via);
    free(via);
}

static void proba_indicem(void)
{
    printf("index (array):\n");

    char *via = scribe_plicam_temp(
        "iq_idx",
        "{\"res\":[1,2,3]}"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "    \"res\": [\n") != NULL, "index aperitur");
    adfirma(strstr(buf, "        1,\n") != NULL, "elementum 1 indentum");
    adfirma(strstr(buf, "        2,\n") != NULL, "elementum 2 indentum");
    adfirma(strstr(buf, "        3\n") != NULL, "elementum 3 sine virgula");
    adfirma(strstr(buf, "    ]\n") != NULL, "index clauditur");

    remove(via);
    free(via);
}

static void proba_indicem_objectorum(void)
{
    printf("index objectorum:\n");

    char *via = scribe_plicam_temp(
        "iq_iobj",
        "[{\"a\":1},{\"b\":2}]"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(buf[0] == '[', "incipit cum [");
    adfirma(strstr(buf, "    {\n") != NULL, "objectum indentum in indice");
    adfirma(strstr(buf, "        \"a\": 1\n") != NULL, "clavis a in objecto");
    adfirma(strstr(buf, "        \"b\": 2\n") != NULL, "clavis b in objecto");

    remove(via);
    free(via);
}

static void proba_typos_varios(void)
{
    printf("typi varii:\n");

    char *via = scribe_plicam_temp(
        "iq_typ",
        "{\"chorda\":\"salve\",\"numerus\":42,\"fractum\":3.14,"
        "\"verum\":true,\"falsum\":false,\"nihil\":null,"
        "\"negativus\":-7}"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "\"chorda\": \"salve\"") != NULL, "chorda");
    adfirma(strstr(buf, "\"numerus\": 42") != NULL, "numerus");
    adfirma(strstr(buf, "\"fractum\": 3.14") != NULL, "fractum");
    adfirma(strstr(buf, "\"verum\": true") != NULL, "verum");
    adfirma(strstr(buf, "\"falsum\": false") != NULL, "falsum");
    adfirma(strstr(buf, "\"nihil\": null") != NULL, "nihil");
    adfirma(strstr(buf, "\"negativus\": -7") != NULL, "negativus");

    remove(via);
    free(via);
}

static void proba_viam(void)
{
    printf("via punctata:\n");

    char *via = scribe_plicam_temp(
        "iq_via",
        "{\"persona\":{\"nomen\":\"Marcus\",\"aetas\":30},"
        "\"urbs\":\"Roma\"}"
    );
    char buf[4096];
    char mandatum[512];

    /* .persona — subobjectum */
    snprintf(mandatum, sizeof(mandatum), "./iq .persona %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));
    adfirma(r == 0, ".persona exitus 0");
    adfirma(strstr(buf, "\"nomen\": \"Marcus\"") != NULL, ".persona habet nomen");
    adfirma(strstr(buf, "\"aetas\": 30") != NULL, ".persona habet aetatem");
    adfirma(strstr(buf, "urbs") == NULL, ".persona non habet urbem");

    /* .urbs — valor simplex */
    snprintf(mandatum, sizeof(mandatum), "./iq .urbs %s", via);
    r = curre_capta(mandatum, buf, sizeof(buf));
    adfirma(r == 0, ".urbs exitus 0");
    adfirma(strstr(buf, "\"Roma\"") != NULL, ".urbs reddit Romam");

    /* .persona.nomen — via profunda */
    snprintf(mandatum, sizeof(mandatum), "./iq .persona.nomen %s", via);
    r = curre_capta(mandatum, buf, sizeof(buf));
    adfirma(r == 0, ".persona.nomen exitus 0");
    adfirma(strstr(buf, "\"Marcus\"") != NULL, ".persona.nomen reddit Marcum");

    /* .persona.aetas — numerus per viam */
    snprintf(mandatum, sizeof(mandatum), "./iq .persona.aetas %s", via);
    r = curre_capta(mandatum, buf, sizeof(buf));
    adfirma(r == 0, ".persona.aetas exitus 0");
    adfirma(strstr(buf, "30") != NULL, ".persona.aetas reddit 30");

    remove(via);
    free(via);
}

static void proba_viam_non_inventam(void)
{
    printf("via non inventa:\n");

    char *via = scribe_plicam_temp("iq_noinv", "{\"a\":1}");
    char mandatum[512];

    snprintf(
        mandatum, sizeof(mandatum),
        "./iq .b.c.d %s >/dev/null 2>&1", via
    );
    int r = system(mandatum);
    adfirma(WEXITSTATUS(r) == 1, "exitus 1 pro via non inventa");

    remove(via);
    free(via);
}

static void proba_compactam(void)
{
    printf("forma compacta:\n");

    char *via = scribe_plicam_temp(
        "iq_comp",
        "{\n  \"a\" : \"b\" ,\n  \"c\" : [ 1 , 2 ]\n}\n"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq -c %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(
        strcmp(buf, "{\"a\":\"b\",\"c\":[1,2]}\n") == 0,
        "omnia spatia remota"
    );

    remove(via);
    free(via);
}

static void proba_stdin(void)
{
    printf("lectio ex stdin:\n");

    char buf[4096];

    int r = curre_capta(
        "echo '{\"x\":1}' | ./iq",
        buf, sizeof(buf)
    );

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "    \"x\": 1") != NULL, "x indenta ex stdin");
}

static void proba_stdin_cum_via(void)
{
    printf("stdin cum via:\n");

    char buf[4096];

    int r = curre_capta(
        "echo '{\"a\":{\"b\":\"salve\"}}' | ./iq .a.b",
        buf, sizeof(buf)
    );

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "\"salve\"") != NULL, "via per stdin");
}

static void proba_objectum_vacuum(void)
{
    printf("objectum vacuum:\n");

    char *via = scribe_plicam_temp("iq_vac", "{}");
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strcmp(buf, "{}\n") == 0, "objectum vacuum reddit {}");

    remove(via);
    free(via);
}

static void proba_indicem_vacuum(void)
{
    printf("index vacuus:\n");

    char *via = scribe_plicam_temp("iq_ivac", "{\"res\":[]}");
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq .res %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strcmp(buf, "[]\n") == 0, "index vacuus reddit []");

    remove(via);
    free(via);
}

static void proba_effugia(void)
{
    printf("chordae cum effugiis:\n");

    char *via = scribe_plicam_temp(
        "iq_eff",
        "{\"msg\":\"salve\\nmunde\",\"via\":\"c:\\\\dir\"}"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strstr(buf, "\\n") != NULL, "\\n servatur in chorda");
    adfirma(strstr(buf, "\\\\") != NULL, "\\\\ servatur in chorda");
}

static void proba_plicam_non_inventam(void)
{
    printf("plica non inventa:\n");

    int r = system("./iq /tmp/non_exsistens_iq_proba.ison >/dev/null 2>&1");
    adfirma(WEXITSTATUS(r) == 1, "exitus 1 pro plica non inventa");
}

static void proba_compactam_cum_via(void)
{
    printf("compacta cum via:\n");

    char *via = scribe_plicam_temp(
        "iq_cvia",
        "{\"a\":{\"b\":1,\"c\":2}}"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq -c .a %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(
        strcmp(buf, "{\"b\":1,\"c\":2}\n") == 0,
        "compacta subobiecti"
    );

    remove(via);
    free(via);
}

static void proba_viam_cum_indice(void)
{
    printf("via cum indice:\n");

    char *via = scribe_plicam_temp(
        "iq_vidx",
        "{\"nomina\":[\"Marcus\",\"Lucia\",\"Gaius\"]}"
    );
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq .nomina[1] %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, ".nomina[1] exitus 0");
    adfirma(strstr(buf, "\"Lucia\"") != NULL, ".nomina[1] reddit Luciam");

    snprintf(mandatum, sizeof(mandatum), "./iq .nomina[0] %s", via);
    r = curre_capta(mandatum, buf, sizeof(buf));
    adfirma(strstr(buf, "\"Marcus\"") != NULL, ".nomina[0] reddit Marcum");

    remove(via);
    free(via);
}

static void proba_idempotentiam(void)
{
    printf("idempotentia:\n");

    /* iq formatam iam formatam non mutare debet */
    const char *formata =
        "{\n"
        "    \"a\": 1,\n"
        "    \"b\": {\n"
        "        \"c\": \"salve\"\n"
        "    }\n"
        "}\n";

    char *via = scribe_plicam_temp("iq_idem", formata);
    char buf[4096];
    char mandatum[512];

    snprintf(mandatum, sizeof(mandatum), "./iq %s", via);
    int r = curre_capta(mandatum, buf, sizeof(buf));

    adfirma(r == 0, "exitus 0");
    adfirma(strcmp(buf, formata) == 0, "iq bis applicata idem reddit");

    remove(via);
    free(via);
}

/* ================================================================
 * principale
 * ================================================================ */

int main(void)
{
    printf("--- probationes iq ---\n\n");

    proba_objectum_simplex();
    proba_objectum_nidum();
    proba_indicem();
    proba_indicem_objectorum();
    proba_typos_varios();
    proba_viam();
    proba_viam_non_inventam();
    proba_compactam();
    proba_stdin();
    proba_stdin_cum_via();
    proba_objectum_vacuum();
    proba_indicem_vacuum();
    proba_effugia();
    proba_plicam_non_inventam();
    proba_compactam_cum_via();
    proba_viam_cum_indice();
    proba_idempotentiam();

    printf("\n");
    if (errores == 0)
        printf("omnes probationes transierunt.\n");
    else
        printf("%d probatio(nes) ceciderunt!\n", errores);

    return errores ? 1 : 0;
}
