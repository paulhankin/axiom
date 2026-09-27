// diag.c - diagnostics.
#include "diag.h"

#include <stdarg.h>

static Vec(Diag *) diags;
static int nerrors;

static Diag *diag_new(int sev, Span sp, const char *code, const char *fmt, va_list ap) {
    Diag *d = NEW(Diag);
    d->severity = sev;
    d->span = sp;
    d->code = code;
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    d->msg = arena_strndup(buf, strlen(buf));
    d->seq = diags.len;
    vec_push(diags, d);
    if (sev == SEV_ERROR) nerrors++;
    return d;
}

Diag *diag_error(Span sp, const char *code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    Diag *d = diag_new(SEV_ERROR, sp, code, fmt, ap);
    va_end(ap);
    return d;
}

Diag *diag_warning(Span sp, const char *code, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    Diag *d = diag_new(SEV_WARNING, sp, code, fmt, ap);
    va_end(ap);
    return d;
}

void diag_note(Diag *d, Span sp, const char *fmt, ...) {
    if (!d) return;
    va_list ap;
    va_start(ap, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    DiagNote n = {arena_strndup(buf, strlen(buf)), sp};
    vec_push(d->notes, n);
}

void diag_types(Diag *d, const char *expected, const char *found) {
    if (!d) return;
    d->expected = expected;
    d->found = found;
}

DiagFix *diag_fix(Diag *d, const char *description, const char *applicability) {
    DiagFix *f = NEW(DiagFix);
    f->description = description;
    f->applicability = applicability;
    if (d) vec_push(d->fixes, f);
    return f;
}

void diag_fix_edit(DiagFix *f, Span sp, const char *replacement) {
    DiagEdit e = {sp, replacement};
    vec_push(f->edits, e);
}

int diag_error_count(void) { return nerrors; }

static int diag_cmp(const void *a, const void *b) {
    const Diag *x = *(Diag *const *)a, *y = *(Diag *const *)b;
    const char *fx = g_files.data[x->span.file]->display, *fy = g_files.data[y->span.file]->display;
    int c = strcmp(fx, fy);
    if (c) return c;
    if (x->span.start != y->span.start) return x->span.start < y->span.start ? -1 : 1;
    c = strcmp(x->code, y->code);
    if (c) return c;
    return x->seq - y->seq;
}

static void json_str(FILE *out, const char *s) {
    fputc('"', out);
    for (; s && *s; s++) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
        case '"': fputs("\\\"", out); break;
        case '\\': fputs("\\\\", out); break;
        case '\n': fputs("\\n", out); break;
        case '\r': fputs("\\r", out); break;
        case '\t': fputs("\\t", out); break;
        default:
            if (c < 0x20) fprintf(out, "\\u%04x", c);
            else fputc(c, out);
        }
    }
    fputc('"', out);
}

static void json_span(FILE *out, Span sp) {
    int sl, sc, el, ec;
    span_linecol(sp, &sl, &sc, &el, &ec);
    fprintf(out,
            "{\"start_line\":%d,\"start_col\":%d,\"end_line\":%d,\"end_col\":%d,"
            "\"start_byte\":%u,\"end_byte\":%u}",
            sl, sc, el, ec, sp.start, sp.end);
}

// The full source line containing the start of the span.
static char *context_line(Span sp) {
    SourceFile *f = g_files.data[sp.file];
    uint32_t s = sp.start, e = sp.start;
    while (s > 0 && f->text[s - 1] != '\n') s--;
    while (e < f->len && f->text[e] != '\n' && f->text[e] != '\r') e++;
    while (s < e && (f->text[s] == ' ' || f->text[s] == '\t')) s++;
    return arena_strndup(f->text + s, e - s);
}

static const char *sev_name(int s) { return s == SEV_ERROR ? "error" : s == SEV_WARNING ? "warning" : "note"; }

void diag_emit(bool json, FILE *out) {
    if (diags.len) qsort(diags.data, (size_t)diags.len, sizeof(Diag *), diag_cmp);
    if (json) {
        fprintf(out, "{\"schema\":\"phalange-diagnostics/1\",\"status\":\"%s\",\"diagnostics\":[",
                nerrors ? "error" : "ok");
        for (int i = 0; i < diags.len; i++) {
            Diag *d = diags.data[i];
            if (i) fputc(',', out);
            fprintf(out, "\n  {\"code\":");
            json_str(out, d->code);
            fprintf(out, ",\"severity\":\"%s\",\"message\":", sev_name(d->severity));
            json_str(out, d->msg);
            fprintf(out, ",\"file\":");
            json_str(out, g_files.data[d->span.file]->display);
            fprintf(out, ",\"span\":");
            json_span(out, d->span);
            fprintf(out, ",\"context\":");
            json_str(out, context_line(d->span));
            if (d->expected) {
                fprintf(out, ",\"expected\":");
                json_str(out, d->expected);
            }
            if (d->found) {
                fprintf(out, ",\"found\":");
                json_str(out, d->found);
            }
            fprintf(out, ",\"notes\":[");
            for (int j = 0; j < d->notes.len; j++) {
                if (j) fputc(',', out);
                fprintf(out, "{\"message\":");
                json_str(out, d->notes.data[j].msg);
                fprintf(out, ",\"file\":");
                json_str(out, g_files.data[d->notes.data[j].span.file]->display);
                fprintf(out, ",\"span\":");
                json_span(out, d->notes.data[j].span);
                fputc('}', out);
            }
            fprintf(out, "],\"fixes\":[");
            for (int j = 0; j < d->fixes.len; j++) {
                DiagFix *f = d->fixes.data[j];
                if (j) fputc(',', out);
                fprintf(out, "{\"description\":");
                json_str(out, f->description);
                fprintf(out, ",\"applicability\":\"%s\",\"edits\":[", f->applicability);
                for (int k = 0; k < f->edits.len; k++) {
                    if (k) fputc(',', out);
                    fprintf(out, "{\"file\":");
                    json_str(out, g_files.data[f->edits.data[k].span.file]->display);
                    fprintf(out, ",\"start_byte\":%u,\"end_byte\":%u,\"replacement\":", f->edits.data[k].span.start,
                            f->edits.data[k].span.end);
                    json_str(out, f->edits.data[k].replacement);
                    fputc('}', out);
                }
                fprintf(out, "]}");
            }
            fprintf(out, "]}");
        }
        fprintf(out, "%s]}\n", diags.len ? "\n" : "");
        return;
    }
    for (int i = 0; i < diags.len; i++) {
        Diag *d = diags.data[i];
        int sl, sc, el, ec;
        span_linecol(d->span, &sl, &sc, &el, &ec);
        fprintf(out, "%s:%d:%d: %s[%s]: %s\n", g_files.data[d->span.file]->display, sl, sc, sev_name(d->severity),
                d->code, d->msg);
        fprintf(out, "    %s\n", context_line(d->span));
        if (d->expected) fprintf(out, "    expected: %s\n       found: %s\n", d->expected, d->found ? d->found : "?");
        for (int j = 0; j < d->notes.len; j++) {
            span_linecol(d->notes.data[j].span, &sl, &sc, &el, &ec);
            fprintf(out, "  %s:%d:%d: note: %s\n", g_files.data[d->notes.data[j].span.file]->display, sl, sc,
                    d->notes.data[j].msg);
        }
        for (int j = 0; j < d->fixes.len; j++) fprintf(out, "  fix (%s): %s\n", d->fixes.data[j]->applicability,
                                                        d->fixes.data[j]->description);
    }
}

void diag_emit_fatal(bool json, const char *code, const char *msg) {
    if (json) {
        printf("{\"schema\":\"phalange-diagnostics/1\",\"status\":\"error\",\"diagnostics\":[{\"code\":");
        json_str(stdout, code);
        printf(",\"severity\":\"error\",\"message\":");
        json_str(stdout, msg);
        printf(",\"notes\":[],\"fixes\":[]}]}\n");
    } else {
        fprintf(stderr, "phalange: error[%s]: %s\n", code, msg);
    }
}
