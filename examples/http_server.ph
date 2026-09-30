// =============================================================================
// File: http_server.ph
// A small JSON API over HTTP: notes that can be listed, added, read and
// deleted.
//
//   phalange build examples/http_server.ph --target=native -O2 -o notes
//   ./notes &                                  (or build for wasm and run it
//                                               with node runtime/host.mjs)
//   curl localhost:8080/notes -d '{"text": "buy milk"}'
//   curl localhost:8080/notes
//   curl localhost:8080/notes/1
//   curl -X DELETE localhost:8080/notes/1
// =============================================================================

import "std/collections/vec" as vec;
import "std/encoding/json" as json;
import "std/fmt" as fmt;
import "std/io" as io;
import "std/mem" as mem;
import "std/net/http" as http;
import "std/str" as str;
import "std/sync" as sync;
import "std/time" as time;

struct Note {
    id: i64,
    text: []mut u8,
    created: time.Time,
}

/// Shared by every handler thread: lock `mu` to use the rest.
struct App {
    mu: sync.Mutex,
    notes: vec.Vec.[Note],
    next_id: i64,
}

fn write_note(w: *mut json.Writer, n: Note) !void {
    let mut b: [40]u8 = [0; 40];
    w.begin_object()!;
    w.key("id")!;
    w.int(n.id)!;
    w.key("text")!;
    w.string(n.text)!;
    w.key("created")!;
    w.string(n.created.format_rfc3339(b[..]))!;
    w.end_object()!;
}

fn find(app: *mut App, id: i64) !usize {
    for i in 0..app.notes.len {
        if app.notes.get(i).id == id {
            return i;
        }
    }
    return error.NotFound;
}

fn index(app: *mut App, req: *http.Request, res: *mut http.Response) !void {
    let mut b: [40]u8 = [0; 40];
    fmt.write(res.writer(), "notes service; the time is {}\n", [fmt.str(time.now().format_rfc3339(b[..]))])!;
}

fn list(app: *mut App, req: *http.Request, res: *mut http.Response) !void {
    res.header("Content-Type", "application/json")!;
    let mut w = json.Writer.pretty(res.writer(), 2);
    app.mu.lock();
    defer app.mu.unlock();
    w.begin_array()!;
    for n in app.notes.items() {
        write_note(&mut w, n)!;
    }
    w.end_array()!;
}

fn add(app: *mut App, req: *http.Request, res: *mut http.Response) !void {
    let mut doc = json.parse(req.body) catch {
        res.text(400, "expected a JSON object\n")!;
        return;
    };
    defer doc.deinit();
    let t = doc.root.get("text")!.as_str() catch {
        res.text(422, "expected {\"text\": \"...\"}\n")!;
        return;
    };
    let copy = str.dup(t)!;
    app.mu.lock();
    defer app.mu.unlock();
    let note = Note.{ id: app.next_id, text: copy, created: time.now() };
    app.notes.push(note) catch |e| {
        mem.free(copy);
        return e;
    };
    app.next_id += 1;
    res.set_status(201);
    res.header("Content-Type", "application/json")!;
    let mut w = json.Writer.init(res.writer());
    write_note(&mut w, note)!;
}

fn show(app: *mut App, req: *http.Request, res: *mut http.Response) !void {
    let id = fmt.parse_int.[i64](req.param("id")!) catch {
        res.text(400, "bad id\n")!;
        return;
    };
    app.mu.lock();
    defer app.mu.unlock();
    let i = find(app, id) catch {
        res.text(404, "no such note\n")!;
        return;
    };
    res.header("Content-Type", "application/json")!;
    let mut w = json.Writer.init(res.writer());
    write_note(&mut w, app.notes.get(i))!;
}

fn remove(app: *mut App, req: *http.Request, res: *mut http.Response) !void {
    let id = fmt.parse_int.[i64](req.param("id")!) catch {
        res.text(400, "bad id\n")!;
        return;
    };
    app.mu.lock();
    defer app.mu.unlock();
    let i = find(app, id) catch {
        res.text(404, "no such note\n")!;
        return;
    };
    mem.free(app.notes.remove(i).text);
    res.set_status(204);
}

export fn main() i32 {
    let mut app = App.{ mu: sync.Mutex.init(), notes: vec.Vec.[Note].init(), next_id: 1 };
    let mut r = http.Router.[App].init(&mut app);
    r.get("/", index) catch return 1;
    r.get("/notes", list) catch return 1;
    r.post("/notes", add) catch return 1;
    r.get("/notes/{id}", show) catch return 1;
    r.delete("/notes/{id}", remove) catch return 1;
    io.stdout().println("listening on http://localhost:8080") catch return 1;
    r.serve("0.0.0.0", 8080, http.Options.default()) catch |e| {
        fmt.eprint("serve: {}\n", [fmt.str(@error_name(e))]) catch {};
        return 1;
    };
    return 0;
}
