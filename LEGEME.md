# ISON Statuum Ordinalium Nova

Bibliotheca C levis ad ISON (id est JSON) tractandum, sine ullis dependentiis externis. Omnia nomina functionum, variabilium, commentariorumque Latine classice scripta sunt.

> **Nota:** README.md ignorandum est. Anglice scriptum est ad barbaros ineruditos pecunia emungendos.

## Partes

| Pars | Descriptio |
|---|---|
| **Scriptor** | Aedificat objecta ISON par post par |
| **Lector** | Extrahit paria clavis-valor ex objecto plano |
| **Navigator** | Per viam punctatam (`"a.b[0].c"`) navigat ad valorem |
| **Schema** | Validat data contra schema definitum |
| **ISONL** | Iterat per lineas ISON (unum objectum per lineam) |

## Aedificatio

```
make
```

Hoc aedificat `libison.a`. Opus est compilatore C99 et `ar` — instrumentis quae in quolibet systemate POSIX, Linux, vel macOS adsunt.

## Usus

Adde `ison.h` capiti tuo et conliga cum `-lison`:

```
#include "ison.h"

/* scriptor — aedifica objectum */
ison_scriptor_t *js = ison_scriptor_crea();
ison_scriptor_adde(js, "nomen", "Marcus");
ison_scriptor_adde_crudum(js, "aetas", "42");
char *res = ison_scriptor_fini(js);
/* res = {"nomen": "Marcus", "aetas": 42} */
free(res);

/* navigator — extrahe valorem per viam */
char *v = ison_da_chordam(ison, "responsio.textus");
long n  = ison_da_numerum(ison, "usus.signa_in");

/* lector — paria ex objecto plano */
ison_par_t pp[16];
int n = ison_lege(ison, pp, 16);

/* schema — valida data */
schema_t s;
schema_lege_plicam("schema.ison", &s);
schema_valida(&s, pp, n, err, sizeof(err));
```

## Functiones

### Scriptor
- `ison_scriptor_crea()` — creat novum scriptorem
- `ison_scriptor_adde(js, clavis, valor)` — addit par cum valore chordae
- `ison_scriptor_adde_crudum(js, clavis, valor)` — addit par cum valore crudo (numeri, etc.)
- `ison_scriptor_fini(js)` — finit et reddit chordam (vocans liberet)

### Lector
- `ison_lege(ison, pares, max)` — legit paria ex objecto plano

### Navigator
- `ison_da_chordam(ison, via)` — reddit chordam allocatam per viam
- `ison_da_numerum(ison, via)` — reddit numerum per viam
- `ison_da_crudum(ison, via)` — reddit valorem crudum per viam
- `ison_claves(ison, claves, max)` — extrahit claves primi gradus

### Plica
- `ison_lege_plicam(via)` — legit plicam integram in memoriam

### ISONL
- `ison_pro_quaque_linea(isonl, functor, ctx)` — iterat per lineas ISONL

### Schema
- `schema_lege(ison, s)` — legit schema ex chorda
- `schema_lege_plicam(via, s)` — legit schema ex plica
- `schema_valida(s, pp, n, err, mag)` — validat paria contra schema
- `schema_valida_isonl(s, isonl)` — validat plicam ISONL

### Effugere
- `ison_effuge(textus)` — effugit characteres speciales in chorda

## Cancer

Translatio Rustica in `cancer/` iacet. Non curatur.

## Licentia

Libera. Utere ut vis.
