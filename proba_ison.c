/*
 * proba_ison.c — probationes bibliothecae ISON
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ison.h"

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

/* --- scriptor --- */

static void proba_scriptorem(void)
{
    printf("scriptor:\n");

    ison_scriptor_t *js = ison_scriptor_crea();
    ison_scriptor_adde(js, "nomen", "Mundus");
    ison_scriptor_adde_crudum(js, "numerus", "42");
    char *res = ison_scriptor_fini(js);

    adfirma(res != NULL, "fini non nullum");
    adfirma(strstr(res, "\"nomen\"") != NULL, "clavis nomen");
    adfirma(strstr(res, "\"Mundus\"") != NULL, "valor Mundus");
    adfirma(strstr(res, "42") != NULL, "numerus crudus");
    free(res);
}

/* --- lector --- */

static void proba_lectorem(void)
{
    printf("lector:\n");

    const char *ison = "{\"a\":\"1\",\"b\":\"2\"}";
    ison_par_t pp[8];
    int n = ison_lege(ison, pp, 8);

    adfirma(n == 2, "duo paria");
    adfirma(strcmp(pp[0].clavis, "a") == 0, "clavis a");
    adfirma(strcmp(pp[0].valor, "1") == 0, "valor 1");
    adfirma(strcmp(pp[1].clavis, "b") == 0, "clavis b");
}

/* --- navigator --- */

static void proba_navigatorem(void)
{
    printf("navigator:\n");

    const char *ison = "{\"x\":{\"y\":\"salve\"},\"n\":123}";

    char *ch = ison_da_chordam(ison, "x.y");
    adfirma(ch != NULL && strcmp(ch, "salve") == 0, "da_chordam x.y");
    free(ch);

    long num = ison_da_numerum(ison, "n");
    adfirma(num == 123, "da_numerum n");

    double f = ison_da_f(ison, "deest", 9.5);
    adfirma(f == 9.5, "da_f praeferentia");

    long nl = ison_da_n(ison, "deest", -1);
    adfirma(nl == -1, "da_n praeferentia");
}

/* --- effugium --- */

static void proba_effugium(void)
{
    printf("effugium:\n");

    char *e = ison_effuge("salve\tmunde");
    adfirma(e != NULL, "effuge non nullum");
    adfirma(strstr(e, "\\t") != NULL, "tab effugit");
    free(e);
}

/* --- compacta --- */

static void proba_compactam(void)
{
    printf("compacta:\n");

    const char *ison = "{ \"a\" : \"b\" , \"c\" : 1 }";
    char *c = ison_compacta(ison);
    adfirma(c != NULL, "compacta non nulla");
    adfirma(strcmp(c, "{\"a\":\"b\",\"c\":1}") == 0, "spatia remota");
    free(c);
}

/* --- claves --- */

static void proba_claves(void)
{
    printf("claves:\n");

    const char *ison = "{\"alpha\":1,\"beta\":2,\"gamma\":3}";
    char cl[8][64];
    int n = ison_claves(ison, cl, 8);

    adfirma(n == 3, "tres claves");
    adfirma(strcmp(cl[0], "alpha") == 0, "clavis alpha");
    adfirma(strcmp(cl[1], "beta") == 0, "clavis beta");
    adfirma(strcmp(cl[2], "gamma") == 0, "clavis gamma");
}

/* --- crudum --- */

static void proba_crudum(void)
{
    printf("crudum:\n");

    const char *ison = "{\"obj\":{\"a\":1}}";
    char *c = ison_da_crudum(ison, "obj");
    adfirma(c != NULL, "crudum non nullum");
    adfirma(strstr(c, "\"a\"") != NULL, "objectum interiorem");
    free(c);
}

/* --- schema --- */

static void proba_schema(void)
{
    printf("schema:\n");

    const char *sch =
        "{\"title\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    int r = schema_lege(sch, &s);
    adfirma(r == 0, "schema lecta");
    adfirma(s.num_campi == 2, "duo campi");

    const char *bonum = "{\"nomen\":\"Marcus\",\"aetas\":\"30\"}";
    ison_par_t pp[8];
    int n = ison_lege(bonum, pp, 8);
    char err[256];
    adfirma(schema_valida(&s, pp, n, err, sizeof(err)) == 0, "validum");

    const char *malum = "{\"aetas\":\"xxx\"}";
    n = ison_lege(malum, pp, 8);
    adfirma(schema_valida(&s, pp, n, err, sizeof(err)) != 0, "invalidum");
}

/* --- ISONL --- */

static int linea_numerus;
static void functor_numera(const ison_par_t *pp, int n, void *ctx)
{
    (void)pp;
    (void)n;
    (void)ctx;
    linea_numerus++;
}

static void proba_isonl(void)
{
    printf("isonl:\n");

    const char *isonl = "{\"a\":\"1\"}\n{\"b\":\"2\"}\n{\"c\":\"3\"}\n";
    linea_numerus = 0;
    int r = ison_pro_quaque_linea(isonl, functor_numera, NULL);

    adfirma(r == 3, "tres lineae processae");
    adfirma(linea_numerus == 3, "functor ter vocatus");
}

/* --- principale --- */

int main(void)
{
    printf("--- probationes ISON ---\n\n");

    proba_scriptorem();
    proba_lectorem();
    proba_navigatorem();
    proba_effugium();
    proba_compactam();
    proba_claves();
    proba_crudum();
    proba_schema();
    proba_isonl();

    printf("\n");
    if (errores == 0)
        printf("omnes probationes transierunt.\n");
    else
        printf("%d probatio(nes) ceciderunt!\n", errores);

    return errores ? 1 : 0;
}
