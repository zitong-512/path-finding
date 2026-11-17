// Run with: node --test tests/editor.test.cjs
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { test } = require('node:test');

class Element {
  constructor() {
    this.children = [];
    this.dataset = {};
    this.style = {};
    this.attributes = {};
    this.disabled = false;
    this.value = '';
    this.textContent = '';
    const classes = new Set();
    this.classList = {
      add: (...names) => names.forEach(name => classes.add(name)),
      remove: (...names) => names.forEach(name => classes.delete(name)),
      contains: name => classes.has(name),
    };
  }
  replaceChildren() { this.children = []; }
  appendChild(child) { this.children.push(child); }
  setAttribute(name, value) { this.attributes[name] = value; }
  closest() { return this; }
  focus() { this.focused = true; }
  get selectedOptions() { return [{ textContent: 'BFS' }]; }
}

function editor() {
  const elements = new Map();
  const context = vm.createContext({
    document: {
      querySelector: name => {
        if (!elements.has(name)) elements.set(name, new Element());
        return elements.get(name);
      },
      createElement: () => new Element(),
    },
    fetch: () => new Promise(() => {}),
    setInterval: () => 0,
    console,
  });
  const evaluate = source => vm.runInContext(source, context);
  evaluate(fs.readFileSync(path.join(__dirname, '../web/app.js'), 'utf8'));
  context.initialRun = {
    algorithm: 'BFS', rows: ['S.G'], discoveredTimes: [1, 2, 3],
    expansionOrder: [], path: [0, 1, 2], truncated: false, goalExpanded: false,
  };
  evaluate("algorithmSelect.value = 'breadthfirstsearch'; showRun(initialRun);");
  return { context, evaluate, elements };
}

test('toggle either way, protect endpoints, and discard without changing saved rows', () => {
  const { evaluate, elements } = editor();
  evaluate('grid.onclick({ target: cells[0] }); grid.onclick({ target: cells[2] });');
  assert.equal(evaluate('dirty'), false);
  evaluate('grid.onclick({ target: cells[1] });');
  assert.equal(evaluate('draftRows[0]'), 'S#G');
  assert.equal(evaluate('run.rows[0]'), 'S.G');
  assert.equal(evaluate('running'), false);
  assert.equal(elements.get('#play').disabled, true);
  assert.equal(elements.get('#algorithm-select').disabled, true);
  evaluate('grid.onclick({ target: cells[1] });');
  assert.equal(evaluate('draftRows[0]'), 'S.G');
  evaluate('grid.onclick({ target: cells[1] }); discardEdits.onclick();');
  assert.equal(evaluate('draftRows[0]'), 'S.G');
  assert.equal(evaluate('dirty'), false);
  assert.equal(elements.get('#play').disabled, false);
});

test('a poll started before editing cannot overwrite the draft', async () => {
  const { context, evaluate } = editor();
  context.fetch = () => new Promise(resolve => { context.finishPoll = resolve; });
  const poll = evaluate('refreshRun()');
  evaluate('grid.onclick({ target: cells[1] });');
  context.finishPoll({ ok: true, text: async () => JSON.stringify(context.initialRun) });
  await poll;
  assert.equal(evaluate('draftRows[0]'), 'S#G');
  assert.equal(evaluate('dirty'), true);
});

test('successful rerun submits edits and polling identical results preserves playback', async () => {
  const { context, evaluate, elements } = editor();
  evaluate('grid.onclick({ target: cells[1] });');
  context.updatedRun = { ...context.initialRun, rows: ['S#G'], discoveredTimes: [1, 0, 0], path: [] };
  let request;
  context.fetch = async (url, options) => {
    request = { url, options };
    return { ok: true, status: 200, json: async () => ({ runs: { breadthfirstsearch: context.updatedRun } }) };
  };
  await evaluate('runMap.onclick()');
  assert.equal(request.url, '/api/run');
  assert.deepEqual(JSON.parse(request.options.body), { rows: ['S#G'] });
  assert.equal(evaluate('run.rows[0]'), 'S#G');
  assert.equal(evaluate('dirty'), false);
  assert.equal(elements.get('#algorithm-select').disabled, false);
  evaluate('step = 1; running = false;');
  context.fetch = async () => ({ ok: true, text: async () => JSON.stringify(context.updatedRun) });
  await evaluate('refreshRun()');
  assert.equal(evaluate('step'), 1);
  assert.equal(evaluate('running'), false);
});

test('failed reruns retain editable draft and keep stale animation disabled', async () => {
  const { context, evaluate, elements } = editor();
  evaluate('grid.onclick({ target: cells[1] });');
  context.fetch = async () => ({ ok: false, status: 504, json: async () => ({ error: 'Algorithm timed out' }) });
  await evaluate('runMap.onclick()');
  assert.equal(evaluate('draftRows[0]'), 'S#G');
  assert.equal(evaluate('run.rows[0]'), 'S.G');
  assert.equal(evaluate('dirty'), true);
  assert.equal(elements.get('#run-map').disabled, false);
  assert.equal(elements.get('#play').disabled, true);
  assert.equal(elements.get('#editor-status').textContent, 'Algorithm timed out');
  assert.equal(evaluate('cells[1].disabled'), false);
});

test('static preview explains how to enable reruns and retains the draft', async () => {
  const { context, evaluate, elements } = editor();
  evaluate('grid.onclick({ target: cells[1] });');
  context.fetch = async () => ({ ok: false, status: 501 });
  await evaluate('runMap.onclick()');
  assert.match(elements.get('#editor-status').textContent, /python serve.py/);
  assert.equal(evaluate('draftRows[0]'), 'S#G');
});
