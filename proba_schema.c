/*
 * proba_schema.c — probationes validationis schematis
 */

#include "ison.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static void proba_lectionem(void)
{
    printf("lectio schematis:\n");

    const char *sch =
        "{\"titulus\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    int r = schema_lege(sch, &s);
    adfirma(r == 0, "schema lecta");
    adfirma(s.num_campi == 2, "duo campi");
    adfirma(strcmp(s.titulus, "Proba") == 0, "titulus");
    adfirma(s.campi[0].necessarium == 1, "nomen necessarium");
    adfirma(s.campi[1].necessarium == 0, "aetas non necessarium");
    adfirma(s.campi[1].typus == TYPUS_NUMERUS, "aetas typus numerus");
}

static void proba_validum(void)
{
    printf("datum validum:\n");

    const char *sch =
        "{\"titulus\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];

    const char *bonum = "{\"nomen\":\"Marcus\",\"aetas\":\"30\"}";
    int n = ison_lege(bonum, pp, 8);
    adfirma(schema_valida(&s, pp, n, err, sizeof(err)) == 0, "omnia bona");

    /* campus non necessarius omissus */
    const char *sine_aetate = "{\"nomen\":\"Marcus\"}";
    n = ison_lege(sine_aetate, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) == 0,
        "campus optionalis omissus"
    );

    /* numerus negativus */
    const char *neg = "{\"nomen\":\"Marcus\",\"aetas\":\"-5\"}";
    n = ison_lege(neg, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) == 0,
        "numerus negativus validus"
    );
}

static void proba_campum_deestem(void)
{
    printf("campus necessarius deest:\n");

    const char *sch =
        "{\"titulus\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];

    const char *deest = "{\"aetas\":\"30\"}";
    int n = ison_lege(deest, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicitur"
    );
    adfirma(strstr(err, "nomen") != NULL, "error nominat campum");
}

static void proba_typum_malum(void)
{
    printf("typus malus:\n");

    const char *sch =
        "{\"titulus\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];

    const char *non_num = "{\"nomen\":\"Marcus\",\"aetas\":\"xxx\"}";
    int n = ison_lege(non_num, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "integer invalidus"
    );
    adfirma(strstr(err, "aetas") != NULL, "error nominat campum");
}

static void proba_campum_ignotum(void)
{
    printf("campus ignotus:\n");

    const char *sch =
        "{\"titulus\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];

    const char *ignotum = "{\"nomen\":\"Marcus\",\"color\":\"ruber\"}";
    int n = ison_lege(ignotum, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicitur"
    );
    adfirma(strstr(err, "color") != NULL, "error nominat campum ignotum");
}

static void proba_fractum(void)
{
    printf("typus fractus:\n");

    const char *sch =
        "{\"titulus\":\"Mensura\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"pondus\":{\"type\":\"number\"}},"
        "\"required\":[\"nomen\",\"pondus\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];

    const char *bonum = "{\"nomen\":\"Ferrum\",\"pondus\":\"3.14\"}";
    int n = ison_lege(bonum, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) == 0,
        "fractum validum"
    );

    const char *exp = "{\"nomen\":\"Ferrum\",\"pondus\":\"-2.5e3\"}";
    n = ison_lege(exp, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) == 0,
        "fractum cum exponente"
    );

    const char *malum = "{\"nomen\":\"Ferrum\",\"pondus\":\"abc\"}";
    n = ison_lege(malum, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "fractum invalidum"
    );
}

static void proba_isonl_validationem(void)
{
    printf("isonl validatio:\n");

    const char *sch =
        "{\"titulus\":\"Proba\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    schema_t s;
    schema_lege(sch, &s);

    const char *bonum =
        "{\"nomen\":\"Marcus\",\"aetas\":\"30\"}\n"
        "{\"nomen\":\"Lucia\",\"aetas\":\"25\"}\n";
    adfirma(schema_valida_isonl(&s, bonum) == 0, "isonl validum");

    const char *malum =
        "{\"nomen\":\"Marcus\",\"aetas\":\"30\"}\n"
        "{\"aetas\":\"xxx\"}\n";
    adfirma(schema_valida_isonl(&s, malum) > 0, "isonl cum erroribus");
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

static void proba_omnes_necessarios(void)
{
    printf("omnes campi necessarii:\n");

    const char *sch =
        "{\"titulus\":\"Strictum\","
        "\"properties\":{\"a\":{\"type\":\"string\"},"
        "\"b\":{\"type\":\"integer\"},"
        "\"c\":{\"type\":\"number\"}},"
        "\"required\":[\"a\",\"b\",\"c\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];
    int n;

    /* omnes adsunt */
    const char *plenum = "{\"a\":\"x\",\"b\":\"1\",\"c\":\"2.5\"}";
    n = ison_lege(plenum, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) == 0,
        "omnes campi praesentes acceptantur"
    );

    /* unus deest */
    const char *sine_b = "{\"a\":\"x\",\"c\":\"2.5\"}";
    n = ison_lege(sine_b, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicit cum b deest"
    );
    adfirma(strstr(err, "b") != NULL, "error nominat b");

    /* duo desunt */
    const char *solum_a = "{\"a\":\"x\"}";
    n = ison_lege(solum_a, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicit cum b et c desunt"
    );

    /* vacuum */
    n = 0;
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicit datum vacuum"
    );
}

static void proba_objecta_nidata(void)
{
    printf("objecta nidata:\n");

    const char *sch =
        "{\"titulus\":\"Nidus\","
        "\"properties\":{\"alpha\":{\"type\":\"string\"},"
        "\"beta\":{\"type\":\"string\"}},"
        "\"required\":[\"alpha\",\"beta\"]}";

    schema_t s;
    schema_lege(sch, &s);
    ison_par_t pp[8];
    char err[256];
    int n;

    /* valor est obiectum — clavis debet in paribus manere */
    const char *cum_obj =
        "{\"alpha\":\"salve\",\"beta\":{\"x\":\"1\"}}";
    n = ison_lege(cum_obj, pp, 8);
    adfirma(n == 2, "ison_lege numerat campos cum objectis");
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) == 0,
        "acceptat valorem obiectum"
    );

    /* campus necessarius est obiectum sed deest */
    const char *sine_beta = "{\"alpha\":\"salve\"}";
    n = ison_lege(sine_beta, pp, 8);
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicit cum campo necessario deest"
    );

    /* campus ignotus est obiectum */
    const char *ignotum_obj =
        "{\"alpha\":\"salve\",\"beta\":\"vale\",\"gamma\":{\"z\":\"9\"}}";
    n = ison_lege(ignotum_obj, pp, 8);
    adfirma(n == 3, "ison_lege numerat campum ignotum obiectum");
    adfirma(
        schema_valida(&s, pp, n, err, sizeof(err)) != 0,
        "reicit campum ignotum obiectum"
    );

    /* valor est index */
    const char *cum_idx =
        "{\"alpha\":\"salve\",\"beta\":[1,2,3]}";
    n = ison_lege(cum_idx, pp, 8);
    adfirma(n == 2, "ison_lege numerat campos cum indicibus");
}

static void proba_ison_multilineum(void)
{
    printf("binarium cum ISON multilineo:\n");

    const char *schema_ison =
        "{\"titulus\":\"Nidus\","
        "\"properties\":{\"a\":{\"type\":\"string\"},"
        "\"b\":{\"type\":\"string\"}},"
        "\"required\":[\"a\",\"b\"]}";

    /* ISON multilineum validum — non est ISONL */
    const char *datum_bonum =
        "{\n"
        "  \"a\": \"salve\",\n"
        "  \"b\": \"vale\"\n"
        "}\n";

    /* ISON multilineum cum campis ignotis — debet fallere */
    const char *datum_malum =
        "{\n"
        "  \"x\": \"salve\",\n"
        "  \"y\": \"vale\"\n"
        "}\n";

    char *via_schema = scribe_plicam_temp("sch_ml", schema_ison);
    char *via_bonum  = scribe_plicam_temp("bon_ml", datum_bonum);
    char *via_malum  = scribe_plicam_temp("mal_ml", datum_malum);

    char mandatum[512];
    int r;

    snprintf(
        mandatum, sizeof(mandatum),
        "./valida_schema %s %s >/dev/null 2>&1", via_schema, via_bonum
    );
    r = system(mandatum);
    adfirma(
        WEXITSTATUS(r) == 0,
        "ISON multilineum validum acceptatur"
    );

    snprintf(
        mandatum, sizeof(mandatum),
        "./valida_schema %s %s >/dev/null 2>&1", via_schema, via_malum
    );
    r = system(mandatum);
    adfirma(
        WEXITSTATUS(r) == 1,
        "ISON multilineum invalidum reicitur"
    );

    remove(via_schema);
    remove(via_bonum);
    remove(via_malum);
    free(via_schema);
    free(via_bonum);
    free(via_malum);
}

static void proba_binarium(void)
{
    printf("binarium valida_schema:\n");

    const char *schema_ison =
        "{\"titulus\":\"Persona\","
        "\"properties\":{\"nomen\":{\"type\":\"string\"},"
        "\"aetas\":{\"type\":\"integer\"}},"
        "\"required\":[\"nomen\"]}";

    const char *datum_bonum = "{\"nomen\":\"Marcus\",\"aetas\":\"30\"}";
    const char *datum_malum = "{\"aetas\":\"xxx\"}";

    char *via_schema = scribe_plicam_temp("schema", schema_ison);
    char *via_bonum  = scribe_plicam_temp("bonum", datum_bonum);
    char *via_malum  = scribe_plicam_temp("malum", datum_malum);

    adfirma(via_schema && via_bonum && via_malum, "plicae temporariae creatae");

    char mandatum[512];
    int r;

    /* datum validum */
    snprintf(
        mandatum, sizeof(mandatum),
        "./valida_schema %s %s >/dev/null 2>&1", via_schema, via_bonum
    );
    r = system(mandatum);
    adfirma(WEXITSTATUS(r) == 0, "binarium accipit datum validum");

    /* datum invalidum */
    snprintf(
        mandatum, sizeof(mandatum),
        "./valida_schema %s %s >/dev/null 2>&1", via_schema, via_malum
    );
    r = system(mandatum);
    adfirma(WEXITSTATUS(r) == 1, "binarium reicit datum invalidum");

    /* sine argumentis */
    r = system("./valida_schema >/dev/null 2>&1");
    adfirma(WEXITSTATUS(r) == 2, "binarium sine argumentis reddit 2");

    remove(via_schema);
    remove(via_bonum);
    remove(via_malum);
    free(via_schema);
    free(via_bonum);
    free(via_malum);
}

int main(void)
{
    printf("--- probationes schematis ---\n\n");

    proba_lectionem();
    proba_validum();
    proba_campum_deestem();
    proba_typum_malum();
    proba_campum_ignotum();
    proba_fractum();
    proba_isonl_validationem();
    proba_omnes_necessarios();
    proba_objecta_nidata();
    proba_ison_multilineum();
    proba_binarium();

    printf("\n");
    if (errores == 0)
        printf("omnes probationes transierunt.\n");
    else
        printf("%d probatio(nes) ceciderunt!\n", errores);

    return errores ? 1 : 0;
}
