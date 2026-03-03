#!/usr/bin/env node
/**
 * TBD-16 Persona Prototype — Development Server
 *
 * Adapted from the dev-server.js by nevvkid (ctag-tbd_hacking).
 *
 * Serves the prototype WebUI from sdcard_image/www-prototype/ and provides
 * real data from sdcard_image/data/ (synthdefinitions, macrodefinitions,
 * macrosoundpresets).
 *
 * Endpoints:
 *   GET  /api/v1/samples                              → file list + directory scan
 *   GET  /api/v1/samples?getconfig=<path>             → serve JSON from data/
 *   GET  /api/v1/samples?listdir=macrodefinitions     → list files in a data subdir
 *   POST /api/v1/samples?action=uploadconfig&path=<p> → save JSON to data/
 *   POST /api/v1/samples?action=manage                → delete files etc.
 *   GET  /api/v1/macroapi                             → current track state
 *   POST /api/v1/macroapi?action=update_track         → set track machine/macro/params
 *   POST /api/v1/macroapi?action=reload               → reload config
 *
 * Usage:
 *   node sdcard_image/www-prototype/tools/dev-server.js [port]
 *
 * Default port: 3001
 */

const http = require('http');
const fs   = require('fs');
const path = require('path');
const url  = require('url');

const PORT = parseInt(process.argv[2], 10) || 3001;
const REPO_ROOT = path.resolve(__dirname, '..', '..', '..');
const WEBROOT = path.resolve(__dirname, '..');
const DATA_DIR = path.resolve(REPO_ROOT, 'sdcard_image', 'data');

// ───────── MIME types ─────────
const MIME = {
  '.html': 'text/html; charset=utf-8',
  '.css':  'text/css; charset=utf-8',
  '.js':   'application/javascript; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg':  'image/svg+xml',
  '.png':  'image/png',
  '.jpg':  'image/jpeg',
  '.ico':  'image/x-icon',
  '.woff': 'font/woff',
  '.woff2':'font/woff2',
};

// ───────── In-memory track state (mock MacroTranslator) ─────────

let synthDefs = null;  // Loaded from synthdefinitions.json
const trackState = []; // 16 tracks: { machine, macro, parameters[] }

function loadSynthDefinitions() {
  const p = path.join(DATA_DIR, 'synthdefinitions.json');
  try {
    synthDefs = JSON.parse(fs.readFileSync(p, 'utf8'));
  } catch (e) {
    console.warn('Could not load synthdefinitions.json:', e.message);
    synthDefs = { tracks: [], machines: [] };
  }

  // Initialize 19 track slots (0-15 instruments, 16-18 FX)
  for (let i = 0; i < 19; i++) {
    const trackDef = synthDefs.tracks.find(t => t.index === i);
    trackState[i] = {
      index: i,
      name: trackDef ? trackDef.name : 'Track ' + (i + 1),
      type: trackDef ? trackDef.type : 'drum',
      machine: '',
      macro: '',
      parameters: [],
    };
  }
}

loadSynthDefinitions();

// ───────── Data directory helpers ─────────

/** List all JSON files in a subdirectory of data/ */
function listDataDir(subdir) {
  const dirPath = path.join(DATA_DIR, subdir);
  try {
    const entries = fs.readdirSync(dirPath, { withFileTypes: true });
    return entries
      .filter(e => e.isFile() && e.name.endsWith('.json'))
      .map(e => {
        const fp = path.join(dirPath, e.name);
        const stat = fs.statSync(fp);
        return {
          name: e.name.replace(/\.json$/i, ''),
          filename: e.name,
          path: subdir,
          size: stat.size,
        };
      });
  } catch (e) {
    return [];
  }
}

/** Load all macro definitions from disk and return as array */
function loadAllMacroDefinitions() {
  const files = listDataDir('macrodefinitions');
  const defs = [];
  for (const f of files) {
    try {
      const data = JSON.parse(fs.readFileSync(path.join(DATA_DIR, 'macrodefinitions', f.filename), 'utf8'));
      defs.push(data);
    } catch (e) {
      // skip
    }
  }
  return defs;
}

/** Load all sound presets from disk and return as array */
function loadAllSoundPresets() {
  const files = listDataDir('macrosoundpresets');
  const presets = [];
  for (const f of files) {
    try {
      const raw = fs.readFileSync(path.join(DATA_DIR, 'macrosoundpresets', f.filename), 'utf8').trim();
      if (!raw) continue; // skip empty files
      const data = JSON.parse(raw);
      if (data.id) presets.push(data);
    } catch (e) {
      // skip invalid files
    }
  }
  return presets;
}

// ───────── API: /api/v1/samples ─────────

function handleSamplesGet(req, res) {
  const parsed = url.parse(req.url, true);
  const q = parsed.query;

  // Get a config file: ?getconfig=synthdefinitions.json or ?getconfig=macrodefinitions/db-allparams.json
  if (q.getconfig) {
    const configPath = path.join(DATA_DIR, q.getconfig);
    // Security: must stay inside DATA_DIR
    if (!configPath.startsWith(DATA_DIR)) {
      return sendJson(res, 403, { error: 'Forbidden' });
    }
    try {
      const raw = fs.readFileSync(configPath, 'utf8');
      res.writeHead(200, {
        'Content-Type': 'application/json; charset=utf-8',
        'Access-Control-Allow-Origin': '*',
        'Cache-Control': 'no-cache',
      });
      return res.end(raw);
    } catch (e) {
      return sendJson(res, 404, { error: 'Not found: ' + q.getconfig });
    }
  }

  // List a data directory: ?listdir=macrodefinitions
  if (q.listdir) {
    const files = listDataDir(q.listdir);
    return sendJson(res, 200, { files });
  }

  // Default: return overview with available files
  const macrodefs = listDataDir('macrodefinitions');
  const soundpresets = listDataDir('macrosoundpresets');
  return sendJson(res, 200, {
    macrodefinitions: macrodefs,
    soundpresets: soundpresets,
    synthdefinitions: synthDefs,
  });
}

function handleSamplesPost(req, res) {
  const parsed = url.parse(req.url, true);
  const action = parsed.query.action || '';

  if (action === 'uploadconfig') {
    const filePath = parsed.query.path;
    if (!filePath) {
      return sendJson(res, 400, { error: 'Missing path parameter' });
    }
    const fullPath = path.join(DATA_DIR, filePath);
    if (!fullPath.startsWith(DATA_DIR)) {
      return sendJson(res, 403, { error: 'Forbidden' });
    }
    readBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Bad request' });
      // Ensure directory exists
      const dir = path.dirname(fullPath);
      fs.mkdirSync(dir, { recursive: true });
      fs.writeFileSync(fullPath, body);
      console.log(`[upload] Wrote ${filePath} (${body.length} bytes)`);
      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'manage') {
    readJsonBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Invalid JSON' });
      if (body.action === 'deleteconfig') {
        const fullPath = path.join(DATA_DIR, body.path);
        if (fullPath.startsWith(DATA_DIR) && fs.existsSync(fullPath)) {
          fs.unlinkSync(fullPath);
          console.log(`[delete] Removed ${body.path}`);
        }
      }
      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'reload') {
    console.log('[reload] Reloading synth definitions...');
    loadSynthDefinitions();
    sendJson(res, 200, { ok: true });
    return;
  }

  sendJson(res, 400, { error: 'Unknown action: ' + action });
}

// ───────── API: /api/v1/macroapi ─────────

function handleMacroApiGet(req, res) {
  // Return current track state
  const tracks = trackState.slice(0, 19).map(t => ({
    index: t.index,
    name: t.name,
    type: t.type,
    machine: t.machine,
    macro: t.macro,
  }));
  sendJson(res, 200, { tracks });
}

function handleMacroApiPost(req, res) {
  const parsed = url.parse(req.url, true);
  const action = parsed.query.action || '';

  if (action === 'update_track') {
    readJsonBody(req, (err, body) => {
      if (err) return sendJson(res, 400, { error: 'Invalid JSON' });

      const trackIdx = body.track;
      if (trackIdx === undefined || trackIdx < 0 || trackIdx >= 19) {
        return sendJson(res, 400, { error: 'Invalid track index' });
      }

      const ts = trackState[trackIdx];
      if (body.machine !== undefined) {
        ts.machine = body.machine;
        console.log(`[macro] Track ${trackIdx} machine → ${body.machine}`);
      }
      if (body.macro !== undefined) {
        ts.macro = body.macro;
        console.log(`[macro] Track ${trackIdx} macro → ${body.macro}`);
      }
      if (body.parameters !== undefined && Array.isArray(body.parameters)) {
        ts.parameters = body.parameters;
        console.log(`[macro] Track ${trackIdx} params → [${body.parameters.join(', ')}]`);
      }

      sendJson(res, 200, { ok: true });
    });
    return;
  }

  if (action === 'reload') {
    console.log('[macroapi] Reload requested');
    loadSynthDefinitions();
    sendJson(res, 200, { ok: true });
    return;
  }

  sendJson(res, 400, { error: 'Unknown action: ' + action });
}

// ───────── API: /api/v1/macroapi/definitions ─────────

function handleMacroDefinitionsList(req, res) {
  const defs = loadAllMacroDefinitions();
  sendJson(res, 200, defs);
}

// ───────── API: /api/v1/macroapi/soundpresets ─────────

function handleSoundPresetsList(req, res) {
  const presets = loadAllSoundPresets();
  sendJson(res, 200, presets);
}

// ───────── Static file serving ─────────

function serveStatic(req, res) {
  let reqPath = url.parse(req.url).pathname;
  if (reqPath === '/') reqPath = '/index.html';

  const filePath = path.join(WEBROOT, path.normalize(reqPath));
  if (!filePath.startsWith(WEBROOT)) {
    res.writeHead(403);
    return res.end('Forbidden');
  }

  fs.stat(filePath, (err, stat) => {
    if (err || !stat.isFile()) {
      res.writeHead(404, { 'Content-Type': 'text/plain' });
      return res.end('Not found: ' + reqPath);
    }

    const ext = path.extname(filePath).toLowerCase();
    const mime = MIME[ext] || 'application/octet-stream';

    res.writeHead(200, {
      'Content-Type': mime,
      'Content-Length': stat.size,
      'Cache-Control': 'no-cache',
      'Access-Control-Allow-Origin': '*',
    });

    fs.createReadStream(filePath).pipe(res);
  });
}

// ───────── Utility ─────────

function sendJson(res, code, obj) {
  const body = JSON.stringify(obj);
  res.writeHead(code, {
    'Content-Type': 'application/json; charset=utf-8',
    'Content-Length': Buffer.byteLength(body),
    'Access-Control-Allow-Origin': '*',
    'Cache-Control': 'no-cache',
  });
  res.end(body);
}

function readBody(req, cb) {
  let chunks = [];
  req.on('data', c => chunks.push(c));
  req.on('end', () => cb(null, Buffer.concat(chunks).toString()));
}

function readJsonBody(req, cb) {
  readBody(req, (err, body) => {
    if (err) return cb(err);
    try {
      cb(null, JSON.parse(body));
    } catch (e) {
      cb(e);
    }
  });
}

// ───────── Router ─────────

const server = http.createServer((req, res) => {
  const parsed = url.parse(req.url, true);
  const p = parsed.pathname;

  // Logging
  const ts = new Date().toISOString().slice(11, 19);
  if (!p.includes('favicon')) {
    console.log(`  ${ts} ${req.method} ${p}${parsed.search || ''}`);
  }

  // CORS preflight
  if (req.method === 'OPTIONS') {
    res.writeHead(204, {
      'Access-Control-Allow-Origin': '*',
      'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
      'Access-Control-Allow-Headers': 'Content-Type',
    });
    return res.end();
  }

  // ── API Routes ──
  if (p === '/api/v1/samples' && req.method === 'GET')  return handleSamplesGet(req, res);
  if (p === '/api/v1/samples' && req.method === 'POST') return handleSamplesPost(req, res);
  if (p === '/api/v1/macroapi' && req.method === 'GET')  return handleMacroApiGet(req, res);
  if (p === '/api/v1/macroapi' && req.method === 'POST') return handleMacroApiPost(req, res);
  if (p === '/api/v1/macroapi/definitions' && req.method === 'GET') return handleMacroDefinitionsList(req, res);
  if (p === '/api/v1/macroapi/soundpresets' && req.method === 'GET') return handleSoundPresetsList(req, res);

  // Static files
  serveStatic(req, res);
});

server.listen(PORT, () => {
  console.log(`\n  TBD-16 Persona Prototype Dev Server`);
  console.log(`  ────────────────────────────────────`);
  console.log(`  Static files : ${WEBROOT}`);
  console.log(`  Data dir     : ${DATA_DIR}`);
  console.log(`  Listening    : http://localhost:${PORT}`);
  console.log();
});
