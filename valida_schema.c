/*
 * valida_schema.c — validat datum ISON vel ISONL contra schema.
 *
 * Usus:
 *   valida_schema <schema.ison> <datum.ison>
 *   valida_schema <schema.ison> <datum.isonl>
 *   cat datum.isonl | valida_schema <schema.ison> -
 *
 * Reddit 0 si validum, 1 si invalidum, 2 si error.
 */

#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *nomen_programma;

static void usus(void)
{
    fprintf(
        stderr,
        "Usus: %s <schema.ison> <datum.ison|datum.isonl|->\n"
        "\n"
        "  Validat datum ISON vel ISONL contra schema.\n"
        "  '-' pro stdin.\n",
        nomen_programma
    );
}

/* legit totam stdin in memoriam */
static char *lege_stdin(void)
{
    size_t mag = 0, cap = 4096;
    char *buf  = malloc(cap);
    if (!buf)
        return NULL;

    size_t n;
    while ((n = fread(buf + mag, 1, cap - mag, stdin)) > 0) {
        mag += n;
        if (mag == cap) {
            cap *= 2;
            char *novum = realloc(buf, cap);
            if (!novum) {
                free(buf);
                return NULL;
            }
            buf = novum;
        }
    }
    buf[mag] = '\0';
    return buf;
}

/* inspicit num datum plures lineas habeat (ISONL) */
static int est_isonl(const char *datum)
{
    const char *p = strchr(datum, '\n');
    if (!p)
        return 0;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
        p++;
    return *p != '\0';
}

int main(int argc, char **argv)
{
    nomen_programma = argv[0];

    if (argc != 3) {
        usus();
        return 2;
    }

    schema_t s;
    if (schema_lege_plicam(argv[1], &s) != 0) {
        fprintf(stderr, "error: schema legi non potest: %s\n", argv[1]);
        return 2;
    }

    char *datum;
    if (strcmp(argv[2], "-") == 0)
        datum = lege_stdin();
    else
        datum = ison_lege_plicam(argv[2]);

    if (!datum) {
        fprintf(stderr, "error: datum legi non potest: %s\n", argv[2]);
        return 2;
    }

    int exitus;

    if (est_isonl(datum)) {
        int errores = schema_valida_isonl(&s, datum);
        if (errores == 0) {
            printf("validum.\n");
            exitus = 0;
        } else {
            printf("%d error(es).\n", errores);
            exitus = 1;
        }
    } else {
        ison_par_t pp[64];
        int n = ison_lege(datum, pp, 64);
        if (n < 0) {
            fprintf(stderr, "error: datum ISON legi non potest\n");
            free(datum);
            return 2;
        }
        char err[256];
        if (schema_valida(&s, pp, n, err, sizeof(err)) == 0) {
            printf("validum.\n");
            exitus = 0;
        } else {
            fprintf(stderr, "%s\n", err);
            exitus = 1;
        }
    }

    free(datum);
    return exitus;
}
