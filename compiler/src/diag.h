// diag.h - diagnostics collection and emission (human / agent-json).
#ifndef AX_DIAG_H
#define AX_DIAG_H

#include "util.h"

enum { SEV_ERROR, SEV_WARNING, SEV_NOTE };

typedef struct DiagNote {
    char *msg;
    Span span;
} DiagNote;

typedef struct DiagEdit {
    Span span;
    const char *replacement;
} DiagEdit;

typedef struct DiagFix {
    const char *description;
    const char *applicability;  // "machine" | "suggestion" | "placeholder"
    Vec(DiagEdit) edits;
} DiagFix;

typedef struct Diag {
    const char *code;
    int severity;
    char *msg;
    Span span;
    const char *expected, *found;
    Vec(DiagNote) notes;
    Vec(DiagFix *) fixes;
    int seq;
} Diag;

Diag *diag_error(Span sp, const char *code, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
Diag *diag_warning(Span sp, const char *code, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void diag_note(Diag *d, Span sp, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void diag_types(Diag *d, const char *expected, const char *found);
DiagFix *diag_fix(Diag *d, const char *description, const char *applicability);
void diag_fix_edit(DiagFix *f, Span sp, const char *replacement);

int diag_error_count(void);
void diag_emit(bool json, FILE *out);
// A fatal error unrelated to source (bad invocation, missing file ...).
void diag_emit_fatal(bool json, const char *code, const char *msg);

#endif
