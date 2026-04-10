/*
 * ison.c — implementatio ISON auxiliarium
 */

#include "ison.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ================================================================
 * scriptor
 * ================================================================ */

struct ison_scriptor {
    char  *data;
    size_t mag;     /* bytes scripti (sine NUL terminali) */
    size_t cap;     /* capacitas allocata */
    int    numerus; /* paria addita */
};

static int scriptor_cresc(ison_scriptor_t *js, size_t opus)
{
    if (js->mag + opus + 1 <= js->cap)
        return 0;
    size_t nova = js->cap * 2;
    if (nova < js->mag + opus + 1)
        nova = js->mag + opus + 1;
    char *p = realloc(js->data, nova);
    if (!p)
        return -1;
    js->data = p;
    js->cap  = nova;
    return 0;
}

static void scriptor_cat(ison_scriptor_t *js, const char *s, size_t n)
{
    if (scriptor_cresc(js, n) < 0)
        return;
    memcpy(js->data + js->mag, s, n);
    js->mag += n;
    js->data[js->mag] = '\0';
}

static void scriptor_chordam(ison_scriptor_t *js, const char *s)
{
    scriptor_cat(js, "\"", 1);
    for (const char *p = s; *p; p++) {
        char buf[8];
        size_t n;
        switch (*p) {
        case '"':
            buf[0] = '\\';
            buf[1] = '"';
            n      = 2;
            break;
        case '\\':
            buf[0] = '\\';
            buf[1] = '\\';
            n      = 2;
            break;
        case '\n':
            buf[0] = '\\';
            buf[1] = 'n';
            n      = 2;
            break;
        case '\r':
            buf[0] = '\\';
            buf[1] = 'r';
            n      = 2;
            break;
        case '\t':
            buf[0] = '\\';
            buf[1] = 't';
            n      = 2;
            break;
        default:
            if ((unsigned char)*p < 0x20) {
                n = (size_t)snprintf(
                    buf, sizeof(buf),
                    "\\u%04x", (unsigned char)*p
                );
            } else {
                buf[0] = *p;
                n      = 1;
            }
        }
        scriptor_cat(js, buf, n);
    }
    scriptor_cat(js, "\"", 1);
}

ison_scriptor_t *ison_scriptor_crea(void)
{
    ison_scriptor_t *js = calloc(1, sizeof(*js));
    if (!js)
        return NULL;
    js->cap  = 256;
    js->data = malloc(js->cap);
    if (!js->data) {
        free(js);
        return NULL;
    }
    js->data[0] = '{';
    js->data[1] = '\0';
    js->mag     = 1;
    return js;
}

void ison_scriptor_adde(
    ison_scriptor_t *js, const char *clavis,
    const char *valor
) {
    if (!js)
        return;
    if (js->numerus > 0)
        scriptor_cat(js, ", ", 2);
    scriptor_chordam(js, clavis);
    scriptor_cat(js, ": ", 2);
    scriptor_chordam(js, valor);
    js->numerus++;
}

void ison_scriptor_adde_crudum(
    ison_scriptor_t *js, const char *clavis,
    const char *valor
) {
    if (!js)
        return;
    if (js->numerus > 0)
        scriptor_cat(js, ", ", 2);
    scriptor_chordam(js, clavis);
    scriptor_cat(js, ": ", 2);
    scriptor_cat(js, valor, strlen(valor));
    js->numerus++;
}

char *ison_scriptor_fini(ison_scriptor_t *js)
{
    if (!js)
        return NULL;
    scriptor_cat(js, "}", 1);
    char *res = js->data;
    free(js);
    return res;
}

/* ================================================================
 * lector
 * ================================================================ */

/* declaratio anticipata */
const char *nav_transili_valorem(const char *p);

static const char *transili_spatia(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    return p;
}

/*
 * lege chordam ISON (p ad '"' initialem).
 * scribit in buf[mag], reddit indicem post '"' terminalem.
 */
static const char *lege_chordam(const char *p, char *buf, size_t mag)
{
    if (*p != '"')
        return NULL;
    p++;

    size_t i = 0;
    while (*p && *p != '"') {
        char c;
        if (*p == '\\') {
            p++;
            if (!*p)
                return NULL;
            switch (*p) {
            case '"':
                c = '"';
                break;
            case '\\':
                c = '\\';
                break;
            case '/':
                c = '/';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'u':
                if (p[1] && p[2] && p[3] && p[4])
                    p += 4;
                c = '?';
                break;
            default:
                c = *p;
                break;
            }
        } else {
            c = *p;
        }
        if (i + 1 < mag)
            buf[i++] = c;
        p++;
    }
    buf[i] = '\0';

    if (*p == '"')
        p++;
    return p;
}

int ison_lege(const char *ison, ison_par_t *pares, int max_pares)
{
    const char *p = transili_spatia(ison);
    if (*p != '{')
        return -1;
    p++;

    int n = 0;
    while (n < max_pares) {
        p = transili_spatia(p);
        if (*p == '}' || *p == '\0')
            break;

        if (n > 0) {
            if (*p == ',')
                p++;
            p = transili_spatia(p);
        }

        if (*p != '"')
            break;

        /* clavis */
        char clavis[64];
        p = lege_chordam(p, clavis, sizeof(clavis));
        if (!p)
            return -1;

        p = transili_spatia(p);
        if (*p != ':')
            return -1;
        p++;
        p = transili_spatia(p);

        /* valor */
        char valor[64];
        if (*p == '"') {
            p = lege_chordam(p, valor, sizeof(valor));
            if (!p)
                return -1;
        } else if (*p == '{' || *p == '[') {
            /* transili objecta et indices */
            valor[0] = *p;
            valor[1] = '\0';
            p        = nav_transili_valorem(p);
        } else {
            /* numeri, true, false, null */
            size_t i = 0;
            while (
                *p && *p != ',' && *p != '}' &&
                *p != ' ' && *p != '\t' && *p != '\n' &&
                i < sizeof(valor) - 1
            )
                valor[i++] = *p++;
            valor[i] = '\0';
        }

        memcpy(pares[n].clavis, clavis, sizeof(pares[n].clavis));
        memcpy(pares[n].valor, valor, sizeof(pares[n].valor));
        n++;
    }

    return n;
}

/* ================================================================
 * auxiliaria: effugere, extrahere, quaerere
 * ================================================================ */

char *ison_effuge(const char *textus)
{
    size_t mag = 1;
    for (const char *p = textus; *p; p++) {
        switch (*p) {
        case '"':
        case '\\':
            mag += 2;
            break;
        case '\n':
        case '\r':
        case '\t':
            mag += 2;
            break;
        default:
            if ((unsigned char)*p < 0x20)
                mag += 6;
            else
                mag += 1;
        }
    }

    char *res = malloc(mag);
    if (!res)
        return NULL;
    char *q = res;

    for (const char *p = textus; *p; p++) {
        switch (*p) {
        case '"':
            *q++ = '\\';
            *q++ = '"';
            break;
        case '\\':
            *q++ = '\\';
            *q++ = '\\';
            break;
        case '\n':
            *q++ = '\\';
            *q++ = 'n';
            break;
        case '\r':
            *q++ = '\\';
            *q++ = 'r';
            break;
        case '\t':
            *q++ = '\\';
            *q++ = 't';
            break;
        default:
            if ((unsigned char)*p < 0x20)
                q += sprintf(q, "\\u%04x", (unsigned char)*p);
            else
                *q++ = *p;
            break;
        }
    }
    *q = '\0';
    return res;
}

/* ================================================================
 * navigator: proprius ISON parser cum recursivo descensu
 * ================================================================ */

/* transili chordam ISON (p ad '"' initialem). reddit post '"' terminalem. */
static const char *nav_transili_chordam(const char *p)
{
    if (*p != '"')
        return p;
    p++;
    while (*p) {
        if (*p == '\\') {
            p += 2;
            continue;
        }
        if (*p == '"')  { return p + 1; }
        p++;
    }
    return p;
}

/* transili quemlibet valorem ISON. reddit post valorem. */
const char *nav_transili_valorem(const char *p)
{
    p = transili_spatia(p);
    switch (*p) {
    case '"':
        return nav_transili_chordam(p);
    case '{': {
            p++;
            p = transili_spatia(p);
            while (*p && *p != '}') {
                p = nav_transili_chordam(transili_spatia(p)); /* clavis */
                p = transili_spatia(p);
                if (*p == ':')
                    p++;
                const char *ante = p;
                p = nav_transili_valorem(p);                  /* valor */
                if (p == ante)
                    return p; /* valor invalidus */
                p = transili_spatia(p);
                if (*p == ',')
                    p = transili_spatia(p + 1);
            }
            if (*p == '}')
                p++;
            return p;
        }
    case '[': {
            p++;
            p = transili_spatia(p);
            while (*p && *p != ']') {
                const char *ante = p;
                p = nav_transili_valorem(p);
                if (p == ante)
                    return p; /* valor invalidus */
                p = transili_spatia(p);
                if (*p == ',')
                    p = transili_spatia(p + 1);
            }
            if (*p == ']')
                p++;
            return p;
        }
    case 't': return p + 4; /* true */
    case 'f': return p + 5; /* false */
    case 'n': return p + 4; /* null */
    default: /* numerus */
        if (*p == '-')
            p++;
        while (
            (*p >= '0' && *p <= '9') || *p == '.' ||
            *p == 'e' || *p == 'E' || *p == '+' || *p == '-'
        )
            p++;
        return p;
    }
}

/* quaere clavem in objecto. p ad '{'. reddit indicem ad valorem. */
static const char *nav_in_objecto(
    const char *p, const char *clavis,
    size_t clon
) {
    p = transili_spatia(p);
    if (*p != '{')
        return NULL;
    p = transili_spatia(p + 1);

    while (*p && *p != '}') {
        if (*p != '"')
            return NULL;
        /* compara clavem */
        const char *k      = p + 1;
        const char *kfinis = k;
        while (*kfinis && *kfinis != '"') {
            if (*kfinis == '\\')
                kfinis++;
            kfinis++;
        }
        size_t klon = (size_t)(kfinis - k);
        p = kfinis + 1; /* post '"' */
        p = transili_spatia(p);
        if (*p == ':')
            p = transili_spatia(p + 1);

        if (klon == clon && memcmp(k, clavis, clon) == 0)
            return p; /* valor inventus */

        p = nav_transili_valorem(p);
        p = transili_spatia(p);
        if (*p == ',')
            p = transili_spatia(p + 1);
    }
    return NULL;
}

/* quaere indicem in array. p ad '['. reddit indicem ad valorem. */
static const char *nav_in_indice(const char *p, int index)
{
    p = transili_spatia(p);
    if (*p != '[')
        return NULL;
    p = transili_spatia(p + 1);

    for (int i = 0; i < index; i++) {
        if (*p == ']' || !*p)
            return NULL;
        p = nav_transili_valorem(p);
        p = transili_spatia(p);
        if (*p == ',')
            p = transili_spatia(p + 1);
    }

    return (*p && *p != ']') ? p : NULL;
}

/* naviga per viam punctatam: "a.b[0].c" */
static const char *ison_naviga(const char *ison, const char *via)
{
    const char *p = transili_spatia(ison);

    while (*via) {
        if (*via == '.') {
            via++;
            continue;
        }

        if (*via == '[') {
            via++;
            int idx = 0;
            while (*via >= '0' && *via <= '9')
                idx = idx * 10 + (*via++ - '0');
            if (*via == ']')
                via++;
            p = nav_in_indice(p, idx);
        } else {
            const char *vfinis = via;
            while (*vfinis && *vfinis != '.' && *vfinis != '[')
                vfinis++;
            p   = nav_in_objecto(p, via, (size_t)(vfinis - via));
            via = vfinis;
        }

        if (!p)
            return NULL;
    }

    return p;
}

/* extrahe chordam ab indice (p ad '"'). allocat, vocans liberet. */
static char *nav_extrahe_chordam(const char *p)
{
    if (*p != '"')
        return NULL;
    p++;

    size_t cap = 256, lon = 0;
    char *res  = malloc(cap);
    if (!res)
        return NULL;

    while (*p && *p != '"') {
        char c;
        if (*p == '\\') {
            p++;
            if (!*p)
                break;
            switch (*p) {
            case '"':
                c = '"';
                break;
            case '\\':
                c = '\\';
                break;
            case '/':
                c = '/';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'u':
                if (p[1] && p[2] && p[3] && p[4])
                    p += 4;
                c = '?';
                break;
            default:
                c = *p;
                break;
            }
        } else {
            c = *p;
        }

        if (lon + 2 >= cap) {
            cap *= 2;
            char *novum = realloc(res, cap);
            if (!novum) {
                free(res);
                return NULL;
            }
            res = novum;
        }
        res[lon++] = c;
        p++;
    }
    res[lon] = '\0';
    return res;
}

char *ison_da_chordam(const char *ison, const char *via)
{
    const char *p = ison_naviga(ison, via);
    if (!p)
        return NULL;
    p = transili_spatia(p);
    return nav_extrahe_chordam(p);
}

long ison_da_numerum(const char *ison, const char *via)
{
    const char *p = ison_naviga(ison, via);
    if (!p)
        return 0;
    p = transili_spatia(p);
    return strtol(p, NULL, 10);
}

char *ison_da_crudum(const char *ison, const char *via)
{
    const char *p = ison_naviga(ison, via);
    if (!p)
        return NULL;
    p = transili_spatia(p);
    const char *finis = nav_transili_valorem(p);
    if (finis <= p)
        return NULL;
    size_t lon = (size_t)(finis - p);
    char *res  = malloc(lon + 1);
    if (!res)
        return NULL;
    memcpy(res, p, lon);
    res[lon] = '\0';
    return res;
}

char *ison_compacta(const char *ison)
{
    if (!ison)
        return NULL;
    size_t cap = strlen(ison) + 1;
    char *res  = malloc(cap);
    if (!res)
        return NULL;
    size_t n      = 0;
    int in_chorda = 0;
    for (const char *p = ison; *p; p++) {
        if (in_chorda) {
            res[n++] = *p;
            if (*p == '\\' && *(p+1)) {
                res[n++] = *++p;
            } else if (*p == '"') {
                in_chorda = 0;
            }
        } else {
            if (*p == '"') {
                in_chorda = 1;
                res[n++]  = *p;
            } else if (
                *p == ' ' || *p == '\t' ||
                *p == '\n' || *p == '\r'
            ) {
                /* omitte */
            } else {
                res[n++] = *p;
            }
        }
    }
    res[n] = '\0';
    return res;
}

/* ================================================================
 * accessores cum praeferentiis et fractis
 * ================================================================ */

double ison_da_fractum(const char *ison, const char *via)
{
    const char *p = ison_naviga(ison, via);
    if (!p)
        return 0.0;
    p = transili_spatia(p);
    if (*p == '"') {
        /* valor in chorda: extrahe et converte */
        char buf[64];
        lege_chordam(p, buf, sizeof(buf));
        return strtod(buf, NULL);
    }
    return strtod(p, NULL);
}

double ison_da_f(const char *ison, const char *via, double praef)
{
    const char *p = ison_naviga(ison, via);
    if (!p)
        return praef;
    p = transili_spatia(p);
    if (*p == '"') {
        char buf[64];
        lege_chordam(p, buf, sizeof(buf));
        return strtod(buf, NULL);
    }
    return strtod(p, NULL);
}

long ison_da_n(const char *ison, const char *via, long praef)
{
    const char *p = ison_naviga(ison, via);
    if (!p)
        return praef;
    p = transili_spatia(p);
    if (*p == '"') {
        char buf[64];
        lege_chordam(p, buf, sizeof(buf));
        return strtol(buf, NULL, 10);
    }
    return strtol(p, NULL, 10);
}

/* --- pares accessores --- */

double ison_pares_f(
    const ison_par_t *pp, int n,
    const char *clavis, double praef
) {
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return strtod(pp[i].valor, NULL);
    return praef;
}

long ison_pares_n(
    const ison_par_t *pp, int n,
    const char *clavis, long praef
) {
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return strtol(pp[i].valor, NULL, 10);
    return praef;
}

const char *ison_pares_s(const ison_par_t *pp, int n, const char *clavis)
{
    for (int i = 0; i < n; i++)
        if (strcmp(pp[i].clavis, clavis) == 0)
            return pp[i].valor;
    return "";
}

/* ================================================================
 * plicae
 * ================================================================ */

char *ison_lege_plicam(const char *via)
{
    FILE *f = fopen(via, "rb");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long mag = ftell(f);
    if (mag < 0) {
        fclose(f);
        return NULL;
    }
    fseek(f, 0, SEEK_SET);
    char *buf = malloc((size_t)mag + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    size_t lecta = fread(buf, 1, (size_t)mag, f);
    fclose(f);
    buf[lecta] = '\0';
    return buf;
}

/* ================================================================
 * ISONL
 * ================================================================ */

int ison_pro_quaque_linea(const char *isonl, ison_linea_functor_t f, void *ctx)
{
    int n         = 0;
    const char *p = isonl;
    while (*p) {
        /* transili spatia et lineas vacuas */
        while (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t')
            p++;
        if (!*p)
            break;

        /* inveni finem lineae */
        const char *finis = p;
        while (*finis && *finis != '\n' && *finis != '\r')
            finis++;

        /* copia lineae in buffer temporarium */
        size_t lon  = (size_t)(finis - p);
        char *linea = malloc(lon + 1);
        if (!linea)
            break;
        memcpy(linea, p, lon);
        linea[lon] = '\0';

        /* lege paria */
        ison_par_t pares[32];
        int np = ison_lege(linea, pares, 32);
        if (np > 0)
            f(pares, np, ctx);
        free(linea);
        n++;

        p = finis;
    }
    return n;
}

int ison_pro_quaque_linea_s(const char *isonl, ison_linea_s_functor_t f, void *ctx)
{
    int n         = 0;
    const char *p = isonl;
    while (*p) {
        while (*p == '\n' || *p == '\r' || *p == ' ' || *p == '\t')
            p++;
        if (!*p)
            break;

        const char *finis = p;
        while (*finis && *finis != '\n' && *finis != '\r')
            finis++;

        size_t lon  = (size_t)(finis - p);
        char *linea = malloc(lon + 1);
        if (!linea)
            break;
        memcpy(linea, p, lon);
        linea[lon] = '\0';

        f(linea, ctx);
        free(linea);
        n++;

        p = finis;
    }
    return n;
}

int ison_claves(const char *ison, char claves[][64], int max)
{
    const char *p = transili_spatia(ison);
    if (*p != '{')
        return 0;
    p = transili_spatia(p + 1);

    int n = 0;
    while (*p && *p != '}' && n < max) {
        if (n > 0) {
            if (*p == ',')
                p = transili_spatia(p + 1);
        }
        if (*p != '"')
            break;
        char buf[64];
        p = lege_chordam(p, buf, sizeof(buf));
        if (!p)
            break;
        memcpy(claves[n], buf, 64);
        n++;
        p = transili_spatia(p);
        if (*p == ':')
            p = transili_spatia(p + 1);
        p = nav_transili_valorem(p);
        p = transili_spatia(p);
    }
    return n;
}

/* ================================================================
 * schema — lector et validator
 * ================================================================ */

/* auxiliarium: lege indicem "required" et nota campos */
static void lege_necessarios(const char *ison, schema_t *s)
{
    char *req_crudum = ison_da_crudum(ison, "required");
    if (!req_crudum || req_crudum[0] != '[') {
        free(req_crudum);
        return;
    }
    for (int i = 0; i < s->num_campi; i++) {
        const char *p = req_crudum + 1;
        while (*p) {
            while (*p && (*p == ' ' || *p == ',' || *p == '\n'))
                p++;
            if (*p == ']')
                break;
            if (*p == '"') {
                const char *ini = p + 1;
                const char *fin = strchr(ini, '"');
                if (!fin)
                    break;
                size_t lon = (size_t)(fin - ini);
                if (
                    lon == strlen(s->campi[i].nomen) &&
                    memcmp(ini, s->campi[i].nomen, lon) == 0
                ) {
                    s->campi[i].necessarium = 1;
                    break;
                }
                p = fin + 1;
            } else {
                break;
            }
        }
    }
    free(req_crudum);
}

int schema_lege(const char *ison, schema_t *s)
{
    memset(s, 0, sizeof(*s));

    /* titulus */
    char *tit = ison_da_chordam(ison, "titulus");
    if (tit) {
        snprintf(s->titulus, sizeof(s->titulus), "%s", tit);
        free(tit);
    }

    /* verifica claves summi gradus */
    char claves[32][64];
    int nk = ison_claves(ison, claves, 32);
    for (int i = 0; i < nk; i++) {
        if (
            strcmp(claves[i], "titulus") != 0 &&
            strcmp(claves[i], "properties") != 0 &&
            strcmp(claves[i], "required") != 0 &&
            strcmp(claves[i], "oneOf") != 0
        )
            return -1;
    }

    char *prop_crudum  = ison_da_crudum(ison, "properties");
    char *oneof_crudum = ison_da_crudum(ison, "oneOf");

    /* debet habere aut properties aut oneOf, non ambo */
    if (prop_crudum && oneof_crudum) {
        free(prop_crudum);
        free(oneof_crudum);
        return -1;
    }
    if (!prop_crudum && !oneof_crudum)
        return -1;

    if (prop_crudum) {
        /* --- modus PROPRIETATES --- */
        s->modus = SCHEMA_PROPRIETATES;

        char nomina[SCHEMA_CAMPI_MAX][64];
        int nc = ison_claves(prop_crudum, nomina, SCHEMA_CAMPI_MAX);

        for (int i = 0; i < nc && s->num_campi < SCHEMA_CAMPI_MAX; i++) {
            schema_campus_t *c = &s->campi[s->num_campi];
            snprintf(c->nomen, sizeof(c->nomen), "%s", nomina[i]);

            /* verifica claves proprietatis */
            char prop_via[128];
            snprintf(prop_via, sizeof(prop_via), "properties.%s", nomina[i]);
            char *prop_json = ison_da_crudum(ison, prop_via);
            if (prop_json) {
                char pk[8][64];
                int npk = ison_claves(prop_json, pk, 8);
                for (int k = 0; k < npk; k++) {
                    if (
                        strcmp(pk[k], "type") != 0 &&
                        strcmp(pk[k], "$ref") != 0 &&
                        strcmp(pk[k], "items") != 0
                    ) {
                        free(prop_json);
                        free(prop_crudum);
                        return -1;
                    }
                }
                free(prop_json);
            }

            /* typus campi */
            char via[128];
            snprintf(via, sizeof(via), "properties.%s.type", nomina[i]);
            char *typ = ison_da_chordam(ison, via);
            if (typ) {
                if (strcmp(typ, "integer") == 0)
                    c->typus = TYPUS_NUMERUS;
                else if (
                    strcmp(typ, "number") == 0 ||
                    strcmp(typ, "fractum") == 0
                )
                    c->typus = TYPUS_FRACTUM;
                else if (strcmp(typ, "string") == 0)
                    c->typus = TYPUS_CHORDA;
                else if (strcmp(typ, "array") == 0) {
                    c->typus = TYPUS_SERIES;
                    char via_ref[128];
                    snprintf(
                        via_ref, sizeof(via_ref),
                        "properties.%s.items.$ref", nomina[i]
                    );
                    char *ref = ison_da_chordam(ison, via_ref);
                    if (!ref) {
                        /* array sine items.$ref */
                        free(typ);
                        free(prop_crudum);
                        return -1;
                    }
                    snprintf(c->ref, sizeof(c->ref), "%s", ref);
                    free(ref);
                } else {
                    /* typus ignotus */
                    free(typ);
                    free(prop_crudum);
                    return -1;
                }
                free(typ);
            } else {
                /* sine type — quaere $ref */
                char via_ref[128];
                snprintf(
                    via_ref, sizeof(via_ref),
                    "properties.%s.$ref", nomina[i]
                );
                char *ref = ison_da_chordam(ison, via_ref);
                if (!ref) {
                    /* nec type nec $ref */
                    free(prop_crudum);
                    return -1;
                }
                c->typus = TYPUS_OBJECTUM;
                snprintf(c->ref, sizeof(c->ref), "%s", ref);
                free(ref);
            }

            s->num_campi++;
        }
        free(prop_crudum);

        lege_necessarios(ison, s);
    } else {
        /* --- modus UNUM_EX --- */
        s->modus = SCHEMA_UNUM_EX;
        free(oneof_crudum);

        /* lege $ref ex quaque voce oneOf */
        for (int i = 0; i < SCHEMA_ONEOF_MAX; i++) {
            char via[64];
            snprintf(via, sizeof(via), "oneOf[%d].$ref", i);
            char *ref = ison_da_chordam(ison, via);
            if (!ref)
                break;
            snprintf(
                s->oneof_ref[s->num_oneof], 128, "%s", ref
            );
            s->num_oneof++;
            free(ref);
        }
        if (s->num_oneof == 0)
            return -1;

        /* verifica quod quaeque vox oneOf solum $ref habeat */
        for (int i = 0; i < s->num_oneof; i++) {
            char via[64];
            snprintf(via, sizeof(via), "oneOf[%d]", i);
            char *entry = ison_da_crudum(ison, via);
            if (entry) {
                char ek[8][64];
                int nek = ison_claves(entry, ek, 8);
                for (int k = 0; k < nek; k++) {
                    if (strcmp(ek[k], "$ref") != 0) {
                        free(entry);
                        return -1;
                    }
                }
                free(entry);
            }
        }

    }

    return 0;
}

int schema_lege_plicam(const char *via, schema_t *s)
{
    char *ison = ison_lege_plicam(via);
    if (!ison)
        return -1;
    int res = schema_lege(ison, s);
    free(ison);
    if (res != 0)
        return res;

    /* serva directorium pro $ref resolutione */
    snprintf(s->directorium, sizeof(s->directorium), "%s", via);
    char *ult = strrchr(s->directorium, '/');
    if (ult)
        *(ult + 1) = '\0';
    else
        s->directorium[0] = '\0';

    return 0;
}

/* est valor numerus integer? */
static int est_numerus(const char *v)
{
    if (!*v)
        return 0;
    const char *p = v;
    if (*p == '-')
        p++;
    if (!*p)
        return 0;
    while (*p) {
        if (!isdigit((unsigned char)*p))
            return 0;
        p++;
    }
    return 1;
}

/* est valor numerus fractus (vel integer)? */
static int est_fractum(const char *v)
{
    if (!*v)
        return 0;
    const char *p = v;
    if (*p == '-')
        p++;
    if (!*p)
        return 0;
    int cifrae = 0;
    while (isdigit((unsigned char)*p)) {
        cifrae++;
        p++;
    }
    if (*p == '.') {
        p++;
        while (isdigit((unsigned char)*p)) {
            cifrae++;
            p++;
        }
    }
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-')
            p++;
        while (isdigit((unsigned char)*p))
            p++;
    }
    return cifrae > 0 && *p == '\0';
}

/* auxiliarium: valida sub-objectum contra schema referentem */
static int valida_ref(
    const schema_t *s, const char *campus_nomen, const char *ref,
    const char *crudum, char *error, size_t mag
) {
    char via_ref[384];
    snprintf(via_ref, sizeof(via_ref), "%s%s", s->directorium, ref);

    schema_t sub;
    if (schema_lege_plicam(via_ref, &sub) != 0) {
        snprintf(
            error, mag,
            "campus \"%s\": schema referens legi non potest: %s",
            campus_nomen, ref
        );
        return -1;
    }

    ison_par_t epp[64];
    int en = ison_lege(crudum, epp, 64);
    if (en < 0) {
        snprintf(
            error, mag, "campus \"%s\": ISON legi non potest",
            campus_nomen
        );
        return -1;
    }

    char sub_err[256];
    if (schema_valida(&sub, epp, en, crudum, sub_err, sizeof(sub_err)) != 0) {
        snprintf(error, mag, "campus \"%s\": %s", campus_nomen, sub_err);
        return -1;
    }
    return 0;
}

/* valida modum PROPRIETATES */
static int valida_proprietates(
    const schema_t *s, const ison_par_t *pp, int n,
    const char *datum, char *error, size_t mag
) {
    /* verifica campos necessarios */
    for (int i = 0; i < s->num_campi; i++) {
        if (!s->campi[i].necessarium)
            continue;
        int inventum = 0;
        for (int j = 0; j < n; j++) {
            if (strcmp(pp[j].clavis, s->campi[i].nomen) == 0) {
                inventum = 1;
                break;
            }
        }
        if (!inventum) {
            snprintf(
                error, mag, "campus necessarius deest: \"%s\"",
                s->campi[i].nomen
            );
            return -1;
        }
    }

    /* verifica typum cuiusque campi in dato */
    for (int j = 0; j < n; j++) {
        const schema_campus_t *c = NULL;
        for (int i = 0; i < s->num_campi; i++) {
            if (strcmp(s->campi[i].nomen, pp[j].clavis) == 0) {
                c = &s->campi[i];
                break;
            }
        }

        if (!c) {
            snprintf(error, mag, "campus ignotus: \"%s\"", pp[j].clavis);
            return -1;
        }

        if (c->typus == TYPUS_NUMERUS && !est_numerus(pp[j].valor)) {
            snprintf(
                error, mag,
                "campus \"%s\": expectatur integer, datum \"%s\"",
                c->nomen, pp[j].valor
            );
            return -1;
        }
        if (c->typus == TYPUS_FRACTUM && !est_fractum(pp[j].valor)) {
            snprintf(
                error, mag,
                "campus \"%s\": expectatur numerus fractus, datum \"%s\"",
                c->nomen, pp[j].valor
            );
            return -1;
        }

        /* TYPUS_OBJECTUM — validat sub-objectum contra $ref */
        if (c->typus == TYPUS_OBJECTUM && datum && c->ref[0]) {
            char *crudum = ison_da_crudum(datum, c->nomen);
            if (!crudum || crudum[0] != '{') {
                snprintf(
                    error, mag,
                    "campus \"%s\": expectatur objectum", c->nomen
                );
                free(crudum);
                return -1;
            }
            int r = valida_ref(s, c->nomen, c->ref, crudum, error, mag);
            free(crudum);
            if (r != 0)
                return -1;
        }

        /* TYPUS_SERIES — validat elementa seriei contra $ref */
        if (c->typus == TYPUS_SERIES && datum && c->ref[0]) {
            char *crudum = ison_da_crudum(datum, c->nomen);
            if (!crudum || crudum[0] != '[') {
                snprintf(
                    error, mag,
                    "campus \"%s\": expectatur series", c->nomen
                );
                free(crudum);
                return -1;
            }

            /* verifica schema referens */
            char via_ref[384];
            snprintf(
                via_ref, sizeof(via_ref), "%s%s",
                s->directorium, c->ref
            );
            schema_t sub;
            if (schema_lege_plicam(via_ref, &sub) != 0) {
                snprintf(
                    error, mag,
                    "campus \"%s\": schema referens legi non potest: %s",
                    c->nomen, c->ref
                );
                free(crudum);
                return -1;
            }

            /* itera per elementa seriei */
            const char *p = crudum + 1;
            int idx       = 0;
            while (*p) {
                while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
                    p++;
                if (*p == ']' || *p == '\0')
                    break;
                if (*p == ',') {
                    p++;
                    continue;
                }
                if (*p == '{') {
                    const char *ini = p;
                    const char *fin = nav_transili_valorem(p);
                    size_t lon      = (size_t)(fin - ini);
                    char *elem      = malloc(lon + 1);
                    memcpy(elem, ini, lon);
                    elem[lon] = '\0';

                    char lbl[128];
                    snprintf(lbl, sizeof(lbl), "%s[%d]", c->nomen, idx);

                    int r = valida_ref(
                        s, lbl, c->ref, elem, error, mag
                    );
                    free(elem);
                    if (r != 0) {
                        free(crudum);
                        return -1;
                    }
                    p = fin;
                    idx++;
                } else {
                    break;
                }
            }
            free(crudum);
        }
    }

    return 0;
}

/* valida modum UNUM_EX */
static int valida_unum_ex(
    const schema_t *s, const ison_par_t *pp, int n,
    const char *datum, char *error, size_t mag
) {
    /* verifica omnes $ref resolvi possunt */
    for (int i = 0; i < s->num_oneof; i++) {
        char via_ref[384];
        snprintf(
            via_ref, sizeof(via_ref), "%s%s",
            s->directorium, s->oneof_ref[i]
        );
        schema_t sub;
        if (schema_lege_plicam(via_ref, &sub) != 0) {
            snprintf(
                error, mag,
                "oneOf: schema legi non potest: %s", s->oneof_ref[i]
            );
            return -1;
        }
    }

    /* proba quodque schema — accepta si ullum convenit */
    for (int i = 0; i < s->num_oneof; i++) {
        char via_ref[384];
        snprintf(
            via_ref, sizeof(via_ref), "%s%s",
            s->directorium, s->oneof_ref[i]
        );
        schema_t sub;
        schema_lege_plicam(via_ref, &sub);

        char sub_err[256];
        if (
            schema_valida(
                &sub, pp, n, datum, sub_err, sizeof(sub_err)
            ) == 0
        )
            return 0;
    }

    snprintf(error, mag, "nullum schema ex oneOf convenit");
    return -1;
}

int schema_valida(
    const schema_t *s, const ison_par_t *pp, int n,
    const char *datum, char *error, size_t mag
) {
    if (s->modus == SCHEMA_UNUM_EX)
        return valida_unum_ex(s, pp, n, datum, error, mag);
    return valida_proprietates(s, pp, n, datum, error, mag);
}

/* validator ISONL */

typedef struct {
    const schema_t *schema;
    int linea_num;
    int errores;
} validatio_ctx_t;

static void valida_lineam(const ison_par_t *pp, int n, void *ctx)
{
    validatio_ctx_t *v = ctx;
    v->linea_num++;

    char error[256];
    if (schema_valida(v->schema, pp, n, NULL, error, sizeof(error)) < 0) {
        fprintf(stderr, "  linea %d: %s\n", v->linea_num, error);
        v->errores++;
    }
}

int schema_valida_isonl(const schema_t *s, const char *isonl)
{
    validatio_ctx_t ctx = { .schema = s, .linea_num = 0, .errores = 0 };
    ison_pro_quaque_linea(isonl, valida_lineam, &ctx);
    return ctx.errores;
}
