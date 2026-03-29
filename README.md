# ison

**A complete JSON library in one source file and one header. 871 lines. Zero dependencies. Parses, navigates, constructs, validates, and streams JSONL — without ever allocating memory proportional to document size.**

## Why ison Exists

Every other JSON library wants to parse your document into a tree. They allocate nodes, build hash tables, copy strings, and hand you a pointer to a structure that's often larger than the document itself. Then they ask you to walk that tree, one key at a time, to find the single value you actually wanted.

ison doesn't do any of that. The dot-path navigator scans the raw source string directly — `"response.choices[0].message.content"` — and returns exactly the value you asked for, in exactly one call, with exactly zero allocations beyond the result itself. You can query a fifty-megabyte JSON document with the memory footprint of a `printf`.

## What's In The Box

| Component | What It Does |
|---|---|
| **Writer** | Builds JSON objects pair by pair, returns a finished string |
| **Reader** | Extracts key-value pairs from a flat object in a single pass |
| **Navigator** | Reaches arbitrarily nested values via dot-path syntax (`"a.b[0].c"`) |
| **Schema** | Validates data against a schema definition — types, required fields, constraints |
| **JSONL** | Iterates line-delimited JSON streams, one object at a time |

## Building

```bash
make
```

That's it. A C99 compiler and `ar`. No configuration step. No cmake. No autotools. No package manager.

## Usage

```c
#include "ison.h"

/* write JSON */
ison_scriptor_t *js = ison_scriptor_crea();
ison_scriptor_adde(js, "nomen", "Marcus");
ison_scriptor_adde_crudum(js, "aetas", "42");
char *res = ison_scriptor_fini(js);
/* res = {"nomen": "Marcus", "aetas": 42} */
free(res);

/* navigate to a nested value by path */
char *v = ison_da_chordam(ison, "responsio.textus");
long n  = ison_da_numerum(ison, "usus.signa_in");

/* read flat key-value pairs */
ison_par_t pp[16];
int n = ison_lege(ison, pp, 16);

/* validate against a schema */
schema_t s;
schema_lege_plicam("schema.ison", &s);
schema_valida(&s, pp, n, err, sizeof(err));
```

## The Rust Port

A complete, faithful Rust translation lives in `cancer/`. Zero external dependencies. Full API parity with the C implementation.

## License

Free. Use however you like.
