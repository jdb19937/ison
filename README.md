# ISON

ISON is a JSON library written in C99. It is one source file and one header, with no external dependencies. It compiles on any POSIX system with a C99 compiler.

## What It Does

ISON handles the common operations you need when working with JSON in C:

**Writing.** The scriptor builds JSON objects incrementally. You add key-value pairs one at a time and it handles escaping, commas, and braces. When you're done, you get back a heap-allocated string. This is more pleasant than the alternative, which is `sprintf` into a buffer and hoping you remembered to escape the backslashes.

**Reading.** The lector extracts key-value pairs from flat JSON objects into a simple array of structs. Each struct has a `clavis` and a `valor`, both fixed-size character arrays. This is deliberately limited — it doesn't build a tree, it doesn't handle nesting, it just gives you the top-level pairs. For a surprising number of real-world cases, that's all you need.

**Navigation.** The navigator is where ISON earns its keep. You give it a JSON string and a dot-path like `"choices[0].message.content"` or `"usage.input_tokens"`, and it walks the structure to find the value — descending into objects by key, into arrays by index, through as many levels as the path specifies. It does this without constructing an intermediate tree. It scans forward through the source string, skipping over values it doesn't need, and returns a pointer to the one it does. This means you can extract a single field from a large response without allocating memory proportional to the entire document.

**Schema validation.** You define a schema — field names, types, which fields are required — and validate parsed data against it. When validation fails, you get a human-readable error message saying which field was missing or had the wrong type. This is useful at system boundaries where you need to verify that incoming JSON matches your expectations before you act on it.

**JSONL.** ISON iterates over newline-delimited JSON (one object per line) by calling a function you provide for each line, with the parsed key-value pairs already extracted. Logging pipelines, streaming API responses, batch data files — anything line-oriented falls into this pattern.

**File I/O.** A convenience function reads an entire file into a heap-allocated buffer. Small, but you end up writing this function in every C project anyway.

## Why Zero Dependencies Matters

A JSON library with no dependencies has a specific quality that's easy to understate: it never breaks for reasons outside your control. There is no upstream to release a bad version. There is no transitive dependency to conflict with another library in your project. There is no build system integration to maintain. There is no ABI to track across OS upgrades.

The practical consequence is that ISON behaves like code you wrote yourself. You can read every line of it. You can modify it to fit constraints that a general-purpose library wouldn't anticipate. When something goes wrong — and at three in the morning, something always goes wrong — you are debugging seven hundred lines of straightforward C, not navigating someone else's abstraction layers.

This also means ISON embeds cleanly anywhere C compiles: firmware, CLI tools, daemons, WASM modules, kernel-adjacent utilities. You copy two files into your tree and you're done. The entire integration surface is a header with a few function signatures.

## Why Dot-Path Navigation Matters

Most JSON libraries parse the entire document into a tree of typed nodes, then provide an API for walking that tree. This is general and powerful, but it imposes a cost: you allocate memory for every key, every value, every array element, every nesting level — even the ones you'll never look at.

ISON's navigator takes a different approach. It treats the source JSON as a string and scans through it directly, using a recursive descent parser that knows how to skip over values without extracting them. When you ask for `"response.choices[0].message.content"`, it opens the top-level object, finds the `response` key, opens that object, finds `choices`, enters the array at index 0, opens that object, finds `message`, opens that object, finds `content`, and extracts the string. Everything else in the document is skipped in linear time without allocation.

This matters when you're pulling a single field out of a large API response — which, if you're working with language model APIs, is approximately always. The response might be several kilobytes of metadata, token counts, and nested structures, and you want one string from three levels deep. ISON gets it for you without making you pay for the parts you don't need.

## Working With Generative AI

If you're making HTTP calls to language model APIs from C — which is a reasonable thing to do if you're building CLI tools, agents, or infrastructure that talks to these services — you will spend a lot of time constructing JSON request bodies and pulling values out of JSON response bodies.

ISON's scriptor handles the request side: you build the JSON object field by field, user input gets escaped automatically, and you get back a valid JSON string. The navigator handles the response side: you reach into the nested response with a path and extract the content, the token count, the error message, whatever you need.

JSONL iteration handles the logging and streaming side. Streaming responses from these APIs arrive as newline-delimited JSON chunks. Logging systems that record every request and response use JSONL as a natural format. ISON processes both with a single callback-based iterator.

Schema validation handles the structured output side. When you ask a language model to produce JSON conforming to a specific shape, you need to verify that it actually did before you pass the result to code that assumes the shape is correct. ISON validates the parsed output against a schema and tells you exactly what's wrong if it doesn't match.

## Quick Start

```bash
make                # builds libison.a
```

```c
#include "ison.h"

/* extract a value from nested JSON */
char *text = ison_da_chordam(json, "choices[0].message.content");
long tokens = ison_da_numerum(json, "usage.total_tokens");

/* build a JSON object */
ison_scriptor_t *js = ison_scriptor_crea();
ison_scriptor_adde(js, "model", "claude-opus-4-20250918");
ison_scriptor_adde(js, "prompt", user_input);
char *body = ison_scriptor_fini(js);

/* iterate JSONL */
ison_pro_quaque_linea(data, my_callback, &ctx);

/* validate against a schema */
schema_t s;
schema_lege_plicam("schema.json", &s);
char err[256];
if (schema_valida(&s, pairs, n, err, sizeof(err)) < 0)
    fprintf(stderr, "%s\n", err);
```

To integrate without the static library, copy `ison.c` and `ison.h` into your project and compile them with everything else.

## License

Free. Use however you like.
