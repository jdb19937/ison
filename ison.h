/*
 * ison.h — ISON auxiliaria
 *
 * Scriptor aedificat objecta ISON.
 * Lector paria clavis-valor ex objecto plano extrahit.
 * Navigator per viam punctatam ("a.b[0].c") navigat.
 */

#ifndef ISON_H
#define ISON_H

#include <stddef.h>

/* --- scriptor --- */

typedef struct ison_scriptor ison_scriptor_t;

ison_scriptor_t *ison_scriptor_crea(void);
void ison_scriptor_adde(ison_scriptor_t *js, const char *clavis,
                        const char *valor);
void ison_scriptor_adde_crudum(ison_scriptor_t *js, const char *clavis,
                               const char *valor);
char *ison_scriptor_fini(ison_scriptor_t *js);

/* --- lector parium (objectum planum) --- */

typedef struct ison_par {
    char clavis[64];
    char valor[64];
} ison_par_t;

int ison_lege(const char *ison, ison_par_t *pares, int max_pares);

/* --- effugere --- */

char *ison_effuge(const char *textus);

/* --- navigator per viam --- */

/*
 * naviga ISON per viam punctatam et extrahe valorem.
 * via exempla: "usage.input_tokens", "output[0].content[0].text",
 *              "choices[0].message.content", "error.message"
 *
 * ison_da_chordam: reddit chordam allocatam (vocans liberet), vel NULL.
 * ison_da_numerum: reddit numerum, vel 0 si non inventum.
 */
char *ison_da_chordam(const char *ison, const char *via);
long ison_da_numerum(const char *ison, const char *via);

/*
 * ison_da_fractum: reddit valorem ut double (0.0 si non inventum).
 * ison_da_f: idem cum praeferentia si via non inventa.
 * ison_da_n: reddit valorem ut long cum praeferentia.
 * Utramque formam tractant: valores quotati ("5778") et non quotati (5778).
 */
double ison_da_fractum(const char *ison, const char *via);
double ison_da_f(const char *ison, const char *via, double praef);
long   ison_da_n(const char *ison, const char *via, long praef);

/*
 * ison_pares_f/n/s: accessores pro tabula ison_par_t.
 * ison_pares_s reddit "" si clavis non inventa (nulla allocatio).
 */
double      ison_pares_f(const ison_par_t *pp, int n,
                         const char *clavis, double praef);
long        ison_pares_n(const ison_par_t *pp, int n,
                         const char *clavis, long praef);
const char *ison_pares_s(const ison_par_t *pp, int n, const char *clavis);

/*
 * ison_da_crudum: reddit valorem crudum (objectum, array, chordam, numerum)
 * ut chordam allocatam. vocans liberet per free().
 */
char *ison_da_crudum(const char *ison, const char *via);

/*
 * ison_compacta: reddit ISON in forma compacta (una linea, sine spatiis
 * superfluis extra chordas). vocans liberet per free(). reddit NULL si error.
 */
char *ison_compacta(const char *ison);

/*
 * ison_claves: extrahe claves primi gradus objecti ISON.
 * scribit in tabulam clavium, reddit numerum clavium.
 */
int ison_claves(const char *ison, char claves[][64], int max);

/* --- plicae --- */

/*
 * ison_lege_plicam — legit plicam integrum in memoriam.
 * vocans liberet per free(). reddit NULL si error.
 */
char *ison_lege_plicam(const char *via);

/* --- ISONL --- */

/*
 * ison_pro_quaque_linea — iterat per lineas ISONL.
 * pro quaque linea non vacua, legit paria et vocat functorem.
 * reddit numerum linearum processarum.
 */
typedef void (*ison_linea_functor_t)(const ison_par_t *pp, int n, void *ctx);
int ison_pro_quaque_linea(const char *isonl, ison_linea_functor_t f, void *ctx);

/*
 * ison_pro_quaque_linea_s — ut supra, sed functor lineam crudissimam accipit.
 * Utile ubi linea objecta imbrisa continet quae ison_lege transilit.
 */
typedef void (*ison_linea_s_functor_t)(const char *linea, void *ctx);
int ison_pro_quaque_linea_s(const char *isonl, ison_linea_s_functor_t f, void *ctx);

/* --- schema --- */

#define SCHEMA_CAMPI_MAX 32

/* typus campi */
typedef enum {
    TYPUS_CHORDA = 0,    /* chorda (string) */
    TYPUS_NUMERUS,       /* integer ("integer") */
    TYPUS_FRACTUM        /* numerus fractus ("number" vel "fractum") */
} typus_t;

/* unus campus schematis */
typedef struct {
    char nomen[64];
    typus_t typus;
    int necessarium;        /* 1 si in "required" */
} schema_campus_t;

/* schema integrum */
typedef struct {
    char titulus[128];
    schema_campus_t campi[SCHEMA_CAMPI_MAX];
    int num_campi;
} schema_t;

/*
 * schema_lege — legit schema ex chorda ISON.
 * reddit 0 si bene, -1 si error.
 */
int schema_lege(const char *ison, schema_t *s);

/*
 * schema_lege_plicam — legit schema ex plicae.
 * reddit 0 si bene, -1 si error.
 */
int schema_lege_plicam(const char *via, schema_t *s);

/*
 * schema_valida — validat paria contra schema.
 * reddit 0 si validum. si invalidum, scribit errorem in buf et reddit -1.
 */
int schema_valida(const schema_t *s, const ison_par_t *pp, int n,
                  char *error, size_t mag);

/*
 * schema_valida_isonl — validat plicam ISONL contra schema.
 * scribit errores ad stderr. reddit numerum errorum.
 */
int schema_valida_isonl(const schema_t *s, const char *isonl);

#endif /* ISON_H */
