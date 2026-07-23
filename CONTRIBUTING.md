# Contributing to MetaGráfica

This file is the **rules**. For *why* a rule is what it is, see
[`docs/bitacora.md`](docs/bitacora.md) — the engineering log, which records each decision
with the measurement behind it. Don't re-litigate a decision without reading its entry
first; several of them record the approach that was tried first and did **not** work.

| document | answers |
|---|---|
| [`README.md`](README.md) | what this is, and why it matters |
| [`docs/referencia.md`](docs/referencia.md) | how do I write X? (language reference) |
| **this file** | how do I work on this? |
| [`docs/bitacora.md`](docs/bitacora.md) | why is the rule what it is, and when was it decided? |
| [`especificacion_mg.md`](especificacion_mg.md) | why the language is shaped this way, and where it is going |
| [`PENDIENTES.md`](PENDIENTES.md) | what is left to do |

## Build

```bash
make          # bin/mg (and the man page, if pandoc is installed)
```

A C++14 compiler (`clang++` or `g++`), GNU `make` and **zlib**. libharu is vendored in
`third_party/`. `flex` is needed only if you modify `src/lexer.l`; `pandoc` only for the man
page. The build must stay **warning-free** under `-Wall -Wpedantic -Wsuggest-override`.

## Test — five gates, all of them green

```bash
bash test/run.sh check
# ok=66 fail=0 error=0 psfail=0 c3fail=0 imgfail=0 errfail=0 (err_ok=31)
```

Every change must leave that line green. The five gates catch different classes, and a
change is not verified until you know **which** gate would have caught your bug:

| gate | catches |
|---|---|
| golden bytes (EPS/SVG/PDF) | regressions in output |
| `psfail` | Ghostscript over the EPS — prologue bugs a byte-stable golden cannot see |
| `c3fail` | backend parity — what one backend silently omits |
| `imgfail` | `docs/img/*.svg` (published output) gone stale |
| `errfail` | **negative tests**: `test/errors/*.mg`, which must fail, with the right message |

- `bash test/run.sh capture` re-blesses the goldens. Only after you have **looked at** the
  render and confirmed the change is the one you wanted. Goldens are not in git.
- `bash test/run.sh images` regenerates `docs/img/*.svg`. That is **published** output —
  regenerating it is a deliberate commit, never a side effect. `capture` does not touch it.
- Adding a diagnostic? Add a fixture to `test/errors/`: an `.mg` that must fail, with
  `% EXPECT: <message fragment>` (and optionally `% EXPECT_AT: line:col`) in its header.
  No list to update — the file is the declaration.

## Code style

C++14. Rules, not preferences:

- **No exceptions, no RTTI** (`-fno-rtti -fno-exceptions`). Errors abort via
  `evalError`/`parseError`; state is saved and restored explicitly, not with RAII.
- **STL freely for structure**: `std::unique_ptr` for ownership (raw pointers are
  non-owning), `vector`/`map`/`string` without hesitation.
- **`printf` for output, not iostreams.** The three backends emit a wire format. Streams are
  for *assembling* strings only (`std::ostringstream` in `SVGDisplay`), never for writing
  the file.
- **In headers**: fully qualified `std::` (no namespace-scope `using`), `override` on every
  override, include guards `MG_*_H` (never `__*`), in-class member initializers.
- **Comments and compiler messages are in Spanish.** The code is not.
- New language features go **in the compiler**. No external preprocessors.
- Do not hand-edit `src/lexv3.cpp` or `src/lexmg.cpp` — flex generates them.
  `include/version.h` **is** edited by hand.

## Nothing may fail silently

The most persistent class of bug in this project is the compiler **discarding something
without saying so** — an unknown attribute, a leftover argument, a coordinate with no pair.
It produces a *plausible* figure, so no golden catches it. When you add a construct:

- an argument that binds to nothing is an error, not a no-op;
- a return value that says "I didn't recognize this" must not be dropped;
- if you cannot make it an error, say why in the log entry.

## Adding a primitive

1. `GI_*` enum + subclass in `include/primitives.h`
2. dispatch by name in `src/parserv3.cpp` (`isPrim()` + `PrimStmt`, or its own `Stmt` if it
   takes a block, like `axis`/`compound`/`plot`)
3. `draw(Display&)` calling `Display` virtuals
4. implement those in **all three** backends

`src/lexer.l` only needs touching for new symbols or operators, never for new commands.

## Examples

`examples/` is the live corpus: every file compiles with the current binary and is verified
on every change. It is an **explicit list** in `test/run.sh`, not a glob — a new `.mg` does
not join on its own.

A new example should earn its place: it exercises something no other example does, or it
reproduces a published figure. If it reproduces one, say which, and keep the figure number
only as long as it is faithful.

## Commits

One subject per commit. The message says **what changed and why**, and the *why* is the part
that matters — if the reasoning is durable, it belongs in `docs/bitacora.md` too. Messages
are in Spanish, like the comments.
