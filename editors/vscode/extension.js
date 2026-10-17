// =============================================================================
// extension.js - the VS Code side of the Axiom language server.
//
// A small Language Server Protocol client with no dependencies: it starts
// `axiom lsp`, speaks JSON-RPC to it over stdin/stdout, keeps it told about
// open .ax documents, and maps its answers onto VS Code's provider API.
// =============================================================================
'use strict';

const vscode = require('vscode');
const cp = require('child_process');
const fs = require('fs');
const path = require('path');

const SELECTOR = { language: 'axiom' };

let output;      // the "Axiom" output channel
let client;      // the running Client, or null
let diagnostics; // DiagnosticCollection
let restarts = []; // times of recent unexpected exits

// -----------------------------------------------------------------------------
// JSON-RPC over a child process's stdio
// -----------------------------------------------------------------------------
class Client {
    constructor(exe, args, cwd) {
        this.nextId = 1;
        this.pending = new Map(); // id -> {resolve, reject, method}
        this.buf = Buffer.alloc(0);
        this.stopping = false;
        this.proc = cp.spawn(exe, args, { cwd, stdio: ['pipe', 'pipe', 'pipe'] });
        this.proc.stdout.on('data', (d) => this.onData(d));
        this.proc.stderr.on('data', (d) => output.append(d.toString()));
        this.proc.on('error', (e) => {
            output.appendLine(`axiom lsp: ${e.message}`);
            vscode.window.showErrorMessage(
                `Could not start the Axiom language server (${exe}): ${e.message}. ` +
                'Set "axiom.path" to the axiom compiler.');
        });
        this.exited = new Promise((resolve) => this.proc.on('exit', (code, signal) => {
            for (const p of this.pending.values()) {
                p.reject(new Error('the Axiom language server exited'));
            }
            this.pending.clear();
            resolve({ code, signal });
        }));
    }

    trace(dir, msg) {
        if (vscode.workspace.getConfiguration('axiom').get('trace.server')) {
            output.appendLine(`${dir} ${JSON.stringify(msg)}`);
        }
    }

    write(msg) {
        if (this.proc.exitCode !== null || !this.proc.stdin.writable) {
            return;
        }
        this.trace('->', msg);
        const body = Buffer.from(JSON.stringify(msg), 'utf8');
        this.proc.stdin.write(`Content-Length: ${body.length}\r\n\r\n`);
        this.proc.stdin.write(body);
    }

    request(method, params, token) {
        const id = this.nextId++;
        const p = new Promise((resolve, reject) => {
            this.pending.set(id, { resolve, reject, method });
        });
        this.write({ jsonrpc: '2.0', id, method, params });
        if (token) {
            token.onCancellationRequested(() => {
                if (this.pending.has(id)) {
                    this.write({ jsonrpc: '2.0', method: '$/cancelRequest', params: { id } });
                }
            });
        }
        return p;
    }

    notify(method, params) {
        this.write({ jsonrpc: '2.0', method, params });
    }

    onData(chunk) {
        this.buf = Buffer.concat([this.buf, chunk]);
        for (;;) {
            const sep = this.buf.indexOf('\r\n\r\n');
            if (sep < 0) {
                return;
            }
            const header = this.buf.slice(0, sep).toString('ascii');
            const m = /Content-Length:\s*(\d+)/i.exec(header);
            if (!m) {
                output.appendLine(`axiom lsp: bad header: ${header}`);
                this.buf = this.buf.slice(sep + 4);
                continue;
            }
            const len = parseInt(m[1], 10);
            if (this.buf.length < sep + 4 + len) {
                return;
            }
            const body = this.buf.slice(sep + 4, sep + 4 + len).toString('utf8');
            this.buf = this.buf.slice(sep + 4 + len);
            let msg;
            try {
                msg = JSON.parse(body);
            } catch (e) {
                output.appendLine(`axiom lsp: bad JSON: ${body}`);
                continue;
            }
            this.trace('<-', msg);
            this.dispatch(msg);
        }
    }

    dispatch(msg) {
        if (msg.id !== undefined && msg.method === undefined) {
            const p = this.pending.get(msg.id);
            if (!p) {
                return;
            }
            this.pending.delete(msg.id);
            if (msg.error) {
                output.appendLine(`axiom lsp: ${p.method}: ${msg.error.message}`);
                p.resolve(null);
            } else {
                p.resolve(msg.result);
            }
            return;
        }
        if (msg.method === 'textDocument/publishDiagnostics') {
            publishDiagnostics(msg.params);
        } else if (msg.method === 'window/logMessage' || msg.method === 'window/showMessage') {
            output.appendLine(msg.params.message);
        }
        if (msg.id !== undefined) {
            // a request from the server we don't support
            this.write({ jsonrpc: '2.0', id: msg.id, result: null });
        }
    }

    async stop() {
        this.stopping = true;
        try {
            await Promise.race([
                this.request('shutdown', null),
                new Promise((r) => setTimeout(r, 2000)),
            ]);
            this.notify('exit', null);
        } catch (e) {
            // already gone
        }
        setTimeout(() => {
            if (this.proc.exitCode === null) {
                this.proc.kill();
            }
        }, 1000);
    }
}

// -----------------------------------------------------------------------------
// Conversions between LSP and VS Code types
// -----------------------------------------------------------------------------
function toRange(r) {
    return new vscode.Range(r.start.line, r.start.character, r.end.line, r.end.character);
}

function toLocation(l) {
    return new vscode.Location(vscode.Uri.parse(l.uri), toRange(l.range));
}

function toLocations(x) {
    if (!x) {
        return null;
    }
    if (Array.isArray(x)) {
        return x.map(toLocation);
    }
    return toLocation(x);
}

function textDocument(doc) {
    return { uri: doc.uri.toString() };
}

function position(doc, pos) {
    return { textDocument: textDocument(doc), position: { line: pos.line, character: pos.character } };
}

function publishDiagnostics(params) {
    const uri = vscode.Uri.parse(params.uri);
    const severities = [
        null,
        vscode.DiagnosticSeverity.Error,
        vscode.DiagnosticSeverity.Warning,
        vscode.DiagnosticSeverity.Information,
        vscode.DiagnosticSeverity.Hint,
    ];
    diagnostics.set(uri, params.diagnostics.map((d) => {
        const v = new vscode.Diagnostic(toRange(d.range), d.message, severities[d.severity || 1]);
        v.source = d.source || 'axiom';
        if (d.code) {
            v.code = d.code;
        }
        if (d.relatedInformation) {
            v.relatedInformation = d.relatedInformation.map((ri) =>
                new vscode.DiagnosticRelatedInformation(toLocation(ri.location), ri.message));
        }
        return v;
    }));
}

function toSymbol(s) {
    const sym = new vscode.DocumentSymbol(s.name, s.detail || '', s.kind - 1, toRange(s.range),
                                          toRange(s.selectionRange));
    sym.children = (s.children || []).map(toSymbol);
    return sym;
}

function toCompletion(c) {
    const item = new vscode.CompletionItem(c.label, c.kind ? c.kind - 1 : undefined);
    item.detail = c.detail;
    if (c.documentation) {
        item.documentation = new vscode.MarkdownString(
            typeof c.documentation === 'string' ? c.documentation : c.documentation.value);
    }
    if (c.insertText) {
        item.insertText = c.insertTextFormat === 2 ? new vscode.SnippetString(c.insertText) : c.insertText;
    }
    if (c.sortText) {
        item.sortText = c.sortText;
    }
    if (c.filterText) {
        item.filterText = c.filterText;
    }
    return item;
}

function toEdit(e) {
    return vscode.TextEdit.replace(toRange(e.range), e.newText);
}

// -----------------------------------------------------------------------------
// Starting the server
// -----------------------------------------------------------------------------
function exists(p) {
    try {
        return fs.statSync(p).isFile();
    } catch (e) {
        return false;
    }
}

function findAxiom() {
    const configured = vscode.workspace.getConfiguration('axiom').get('path');
    if (configured) {
        return configured.replace(/^~(?=[\\/])/, process.env.HOME || process.env.USERPROFILE || '~');
    }
    const exe = process.platform === 'win32' ? 'axiom.exe' : 'axiom';
    for (const f of vscode.workspace.workspaceFolders || []) {
        const p = path.join(f.uri.fsPath, 'cmd', 'axiom', 'build', exe);
        if (exists(p)) {
            return p;
        }
    }
    return 'axiom';
}

function sendOpen(doc) {
    if (client && doc.languageId === 'axiom') {
        client.notify('textDocument/didOpen', {
            textDocument: { uri: doc.uri.toString(), languageId: 'axiom', version: doc.version, text: doc.getText() },
        });
    }
}

async function start(context) {
    if (!vscode.workspace.getConfiguration('axiom').get('enableLanguageServer')) {
        return;
    }
    const exe = findAxiom();
    const args = ['lsp', ...vscode.workspace.getConfiguration('axiom').get('serverArgs')];
    const folders = vscode.workspace.workspaceFolders || [];
    const cwd = folders.length ? folders[0].uri.fsPath : undefined;
    output.appendLine(`starting ${exe} ${args.join(' ')}`);
    const c = new Client(exe, args, cwd);
    client = c;
    c.exited.then(({ code, signal }) => {
        if (c.stopping || client !== c) {
            return;
        }
        output.appendLine(`axiom lsp exited (code ${code}${signal ? ', signal ' + signal : ''})`);
        client = null;
        const now = Date.now();
        restarts = restarts.filter((t) => now - t < 3 * 60 * 1000);
        restarts.push(now);
        if (restarts.length <= 4) {
            start(context);
        } else {
            vscode.window.showErrorMessage(
                'The Axiom language server keeps exiting; see the Axiom output. Use "Axiom: Restart Language Server" to try again.');
        }
    });
    const init = await c.request('initialize', {
        processId: process.pid,
        clientInfo: { name: 'vscode-axiom', version: '0.1.0' },
        rootUri: folders.length ? folders[0].uri.toString() : null,
        workspaceFolders: folders.map((f) => ({ uri: f.uri.toString(), name: f.name })),
        capabilities: {
            textDocument: {
                hover: { contentFormat: ['markdown', 'plaintext'] },
                completion: { completionItem: { snippetSupport: true } },
                documentSymbol: { hierarchicalDocumentSymbolSupport: true },
                publishDiagnostics: { relatedInformation: true },
            },
        },
    });
    if (!init || client !== c) {
        return;
    }
    c.notify('initialized', {});
    for (const doc of vscode.workspace.textDocuments) {
        sendOpen(doc);
    }
}

async function stop() {
    const c = client;
    client = null;
    if (c) {
        await c.stop();
    }
    diagnostics.clear();
}

// -----------------------------------------------------------------------------
// Activation
// -----------------------------------------------------------------------------
function activate(context) {
    output = vscode.window.createOutputChannel('Axiom');
    diagnostics = vscode.languages.createDiagnosticCollection('axiom');
    context.subscriptions.push(output, diagnostics);

    const req = async (method, params, token) => {
        if (!client) {
            return null;
        }
        try {
            return await client.request(method, params, token);
        } catch (e) {
            return null;
        }
    };

    context.subscriptions.push(
        vscode.workspace.onDidOpenTextDocument(sendOpen),
        vscode.workspace.onDidChangeTextDocument((e) => {
            if (client && e.document.languageId === 'axiom' && e.contentChanges.length) {
                client.notify('textDocument/didChange', {
                    textDocument: { uri: e.document.uri.toString(), version: e.document.version },
                    contentChanges: [{ text: e.document.getText() }],
                });
            }
        }),
        vscode.workspace.onDidSaveTextDocument((doc) => {
            if (client && doc.languageId === 'axiom') {
                client.notify('textDocument/didSave', { textDocument: textDocument(doc) });
            }
        }),
        vscode.workspace.onDidCloseTextDocument((doc) => {
            if (client && doc.languageId === 'axiom') {
                client.notify('textDocument/didClose', { textDocument: textDocument(doc) });
                diagnostics.delete(doc.uri);
            }
        }),

        vscode.languages.registerHoverProvider(SELECTOR, {
            async provideHover(doc, pos, token) {
                const h = await req('textDocument/hover', position(doc, pos), token);
                if (!h || !h.contents) {
                    return null;
                }
                const md = new vscode.MarkdownString(
                    typeof h.contents === 'string' ? h.contents : h.contents.value);
                return new vscode.Hover(md, h.range ? toRange(h.range) : undefined);
            },
        }),
        vscode.languages.registerDefinitionProvider(SELECTOR, {
            async provideDefinition(doc, pos, token) {
                return toLocations(await req('textDocument/definition', position(doc, pos), token));
            },
        }),
        vscode.languages.registerTypeDefinitionProvider(SELECTOR, {
            async provideTypeDefinition(doc, pos, token) {
                return toLocations(await req('textDocument/typeDefinition', position(doc, pos), token));
            },
        }),
        vscode.languages.registerReferenceProvider(SELECTOR, {
            async provideReferences(doc, pos, ctx, token) {
                const p = position(doc, pos);
                p.context = { includeDeclaration: ctx.includeDeclaration };
                return toLocations(await req('textDocument/references', p, token));
            },
        }),
        vscode.languages.registerDocumentHighlightProvider(SELECTOR, {
            async provideDocumentHighlights(doc, pos, token) {
                const hs = await req('textDocument/documentHighlight', position(doc, pos), token);
                return (hs || []).map((h) => new vscode.DocumentHighlight(toRange(h.range), (h.kind || 1) - 1));
            },
        }),
        vscode.languages.registerRenameProvider(SELECTOR, {
            async provideRenameEdits(doc, pos, newName, token) {
                const p = position(doc, pos);
                p.newName = newName;
                const we = await req('textDocument/rename', p, token);
                if (!we) {
                    return null;
                }
                const edit = new vscode.WorkspaceEdit();
                for (const uri of Object.keys(we.changes || {})) {
                    edit.set(vscode.Uri.parse(uri), we.changes[uri].map(toEdit));
                }
                return edit;
            },
        }),
        vscode.languages.registerDocumentSymbolProvider(SELECTOR, {
            async provideDocumentSymbols(doc, token) {
                const syms = await req('textDocument/documentSymbol', { textDocument: textDocument(doc) }, token);
                return (syms || []).map(toSymbol);
            },
        }),
        vscode.languages.registerCompletionItemProvider(SELECTOR, {
            async provideCompletionItems(doc, pos, token) {
                const r = await req('textDocument/completion', position(doc, pos), token);
                if (!r) {
                    return null;
                }
                const items = Array.isArray(r) ? r : r.items;
                return new vscode.CompletionList(items.map(toCompletion), !Array.isArray(r) && r.isIncomplete);
            },
        }, '.', '@'),
        vscode.languages.registerSignatureHelpProvider(SELECTOR, {
            async provideSignatureHelp(doc, pos, token) {
                const r = await req('textDocument/signatureHelp', position(doc, pos), token);
                if (!r || !r.signatures || !r.signatures.length) {
                    return null;
                }
                const sh = new vscode.SignatureHelp();
                sh.signatures = r.signatures.map((s) => {
                    const si = new vscode.SignatureInformation(s.label,
                        s.documentation ? new vscode.MarkdownString(s.documentation.value || s.documentation) : undefined);
                    si.parameters = (s.parameters || []).map((p) => new vscode.ParameterInformation(p.label));
                    return si;
                });
                sh.activeSignature = r.activeSignature || 0;
                sh.activeParameter = r.activeParameter || 0;
                return sh;
            },
        }, '(', ','),
        vscode.languages.registerDocumentFormattingEditProvider(SELECTOR, {
            async provideDocumentFormattingEdits(doc, opts, token) {
                const edits = await req('textDocument/formatting', {
                    textDocument: textDocument(doc),
                    options: { tabSize: opts.tabSize, insertSpaces: opts.insertSpaces },
                }, token);
                return (edits || []).map(toEdit);
            },
        }),

        vscode.commands.registerCommand('axiom.restartServer', async () => {
            await stop();
            restarts = [];
            await start(context);
        }),
        vscode.commands.registerCommand('axiom.showOutput', () => output.show()),
        vscode.workspace.onDidChangeConfiguration(async (e) => {
            if (e.affectsConfiguration('axiom.path') || e.affectsConfiguration('axiom.serverArgs') ||
                e.affectsConfiguration('axiom.enableLanguageServer')) {
                await stop();
                await start(context);
            }
        }),
    );
    start(context);
}

function deactivate() {
    return stop();
}

module.exports = { activate, deactivate };
