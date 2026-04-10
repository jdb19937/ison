/*
 * iq — instrumentum ad ISON formandum
 *
 * usus:
 *   iq [via] [plica]     — lege ISON ex plica (vel stdin) et forma
 *   iq -c [plica]        — compacta (una linea)
 *
 * via exempla: ".", ".nomen", ".ubi.res.temperatura"
 */

#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INDENTUM 4

/* ================================================================
 * forma — ISON pulchre imprimere
 * ================================================================ */

static void forma(const char *p, int gradus, FILE *ex);

static const char *transili_spatia(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    return p;
}

static void scribe_indentum(int gradus, FILE *ex)
{
    for (int i = 0; i < gradus * INDENTUM; i++)
        fputc(' ', ex);
}

/* transili chordam, scribe eam ad ex. reddit indicem post chordam. */
static const char *scribe_chordam(const char *p, FILE *ex)
{
    if (*p != '"')
        return p;
    fputc('"', ex);
    p++;
    while (*p && *p != '"') {
        if (*p == '\\') {
            fputc(*p++, ex);
            if (*p)
                fputc(*p++, ex);
        } else {
            fputc(*p++, ex);
        }
    }
    if (*p == '"') {
        fputc('"', ex);
        p++;
    }
    return p;
}

/* scribe valorem literalem (numerus, true, false, null). */
static const char *scribe_literalem(const char *p, FILE *ex)
{
    while (
        *p && *p != ',' && *p != '}' && *p != ']' &&
        *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r'
    )
        fputc(*p++, ex);
    return p;
}

static void forma(const char *p, int gradus, FILE *ex)
{
    p = transili_spatia(p);

    if (*p == '{') {
        p = transili_spatia(p + 1);
        if (*p == '}') {
            fprintf(ex, "{}");
            return;
        }
        fprintf(ex, "{\n");
        int primus = 1;
        while (*p && *p != '}') {
            if (!primus) {
                fprintf(ex, ",\n");
            }
            primus = 0;
            p      = transili_spatia(p);

            /* clavis */
            scribe_indentum(gradus + 1, ex);
            p = scribe_chordam(p, ex);

            p = transili_spatia(p);
            if (*p == ':')
                p++;
            p = transili_spatia(p);

            fprintf(ex, ": ");

            /* valor */
            forma(p, gradus + 1, ex);
            p = transili_spatia(p);
            p = nav_transili_valorem(p);
            p = transili_spatia(p);
            if (*p == ',')
                p = transili_spatia(p + 1);
        }
        fprintf(ex, "\n");
        scribe_indentum(gradus, ex);
        fprintf(ex, "}");

    } else if (*p == '[') {
        p = transili_spatia(p + 1);
        if (*p == ']') {
            fprintf(ex, "[]");
            return;
        }
        fprintf(ex, "[\n");
        int primus = 1;
        while (*p && *p != ']') {
            if (!primus) {
                fprintf(ex, ",\n");
            }
            primus = 0;
            p      = transili_spatia(p);

            scribe_indentum(gradus + 1, ex);
            forma(p, gradus + 1, ex);
            p = nav_transili_valorem(p);
            p = transili_spatia(p);
            if (*p == ',')
                p = transili_spatia(p + 1);
        }
        fprintf(ex, "\n");
        scribe_indentum(gradus, ex);
        fprintf(ex, "]");

    } else if (*p == '"') {
        scribe_chordam(p, ex);

    } else {
        scribe_literalem(p, ex);
    }
}

/* ================================================================
 * lege omnia ex stdin
 * ================================================================ */

static char *lege_stdin(void)
{
    size_t cap = 4096, lon = 0;
    char *buf  = malloc(cap);
    if (!buf)
        return NULL;
    size_t n;
    while ((n = fread(buf + lon, 1, cap - lon - 1, stdin)) > 0) {
        lon += n;
        if (lon + 1 >= cap) {
            cap *= 2;
            char *novum = realloc(buf, cap);
            if (!novum) {
                free(buf);
                return NULL;
            }
            buf = novum;
        }
    }
    buf[lon] = '\0';
    return buf;
}

/* ================================================================
 * principale
 * ================================================================ */

int main(int argc, char **argv)
{
    const char *via   = ".";
    const char *plica = NULL;
    int compacte      = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            compacte = 1;
        } else if (
            strcmp(argv[i], "-h") == 0 ||
            strcmp(argv[i], "--auxilium") == 0
        ) {
            fprintf(
                stderr,
                "usus: iq [optiones] [via] [plica]\n"
                "  via    — via punctata (e.g. '.nomen', '.ubi.res')\n"
                "  plica  — plica ISON (praeferentia: stdin)\n"
                "  -c     — forma compacta\n"
                "  -h     — hoc auxilium\n"
            );
            return 0;
        } else if (argv[i][0] == '.') {
            via = argv[i];
        } else {
            plica = argv[i];
        }
    }

    /* lege ISON */
    char *ison;
    if (plica) {
        ison = ison_lege_plicam(plica);
        if (!ison) {
            fprintf(stderr, "iq: non possum legere plicam: %s\n", plica);
            return 1;
        }
    } else {
        ison = lege_stdin();
        if (!ison) {
            fprintf(stderr, "iq: error legendi stdin\n");
            return 1;
        }
    }

    /* naviga per viam */
    const char *datum = ison;
    if (strcmp(via, ".") != 0) {
        /* remove '.' initialem */
        const char *v = via;
        if (*v == '.')
            v++;
        char *crudum = ison_da_crudum(ison, v);
        if (!crudum) {
            fprintf(stderr, "iq: via non inventa: %s\n", via);
            free(ison);
            return 1;
        }
        free(ison);
        ison  = crudum;
        datum = ison;
    }

    if (compacte) {
        char *comp = ison_compacta(datum);
        if (comp) {
            printf("%s\n", comp);
            free(comp);
        }
    } else {
        forma(datum, 0, stdout);
        printf("\n");
    }

    free(ison);
    return 0;
}
