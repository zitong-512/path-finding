const grid = document.querySelector('#grid');
const algorithm = document.querySelector('#algorithm');
const algorithmSelect = document.querySelector('#algorithm-select');
const slider = document.querySelector('#step');
const status = document.querySelector('#status');
const play = document.querySelector('#play');
const restart = document.querySelector('#restart');
const runMap = document.querySelector('#run-map');
const discardEdits = document.querySelector('#discard-edits');
const editorStatus = document.querySelector('#editor-status');
const saveStatus = document.querySelector('#save-status');
const editorHint = 'Click a cell to toggle a wall, or select S or G and click its new location.';

let run = null;
let cells = [];
let step = 0;
let totalSteps = 0;
let running = false;
let lastJson = null;
let loading = false;
let lastManifest = null;
let manifestLoading = false;
let draftRows = [];
let dirty = false;
let submitting = false;
let mapRevision = 0;
let selectedEndpoint = null;
let saveVersion = 0;
let saveTask = Promise.resolve();

function saveDraft() {
  const rows = draftRows.slice();
  const version = ++saveVersion;
  saveStatus.textContent = 'Saving map...';
  // Serialize writes so a slower, older request cannot replace a newer edit.
  saveTask = saveTask.catch(() => {}).then(async () => {
    if (version !== saveVersion) return;
    const response = await fetch('/api/map', {
      method: 'POST', headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ rows }),
    });
    if (!response.ok) throw new Error('Could not save map. Check that serve.py is running.');
    if (version === saveVersion) saveStatus.textContent = 'Map saved.';
  });
  saveTask.catch(error => {
    if (version === saveVersion) saveStatus.textContent = error.message;
  });
  return saveTask;
}

function setSymbol(index, symbol) {
  const width = draftRows[0].length;
  const y = Math.floor(index / width);
  const x = index % width;
  draftRows[y] = draftRows[y].slice(0, x) + symbol + draftRows[y].slice(x + 1);
}

function renderCells(rows) {
  cells = [];
  grid.replaceChildren();
  grid.style.gridTemplateColumns = `repeat(${rows[0].length}, var(--cell-size))`;
  rows.forEach((row, y) => {
    [...row].forEach((symbol, x) => {
      const cell = document.createElement('button');
      cell.type = 'button';
      cell.className = 'cell';
      cell.dataset.symbol = symbol;
      cell.dataset.index = cells.length;
      cell.setAttribute('aria-label', `Row ${y + 1}, column ${x + 1}: ${
        symbol === '#' ? 'wall' : symbol === 'S' ? 'start' : symbol === 'G' ? 'goal' : 'open'}`);
      if (symbol === '#') cell.classList.add('wall');
      if (symbol === 'S') cell.classList.add('start');
      if (symbol === 'G') cell.classList.add('goal');
      if (symbol === selectedEndpoint) cell.classList.add('selected-endpoint');
      cell.setAttribute('aria-pressed', String(symbol === selectedEndpoint));
      if (symbol === 'S' || symbol === 'G') cell.textContent = symbol;
      cell.disabled = submitting;
      grid.appendChild(cell);
      cells.push(cell);
    });
  });
}

grid.onclick = event => {
  const cell = event.target.closest('.cell');
  if (!cell || submitting || !run || cell.disabled) return;
  const index = Number(cell.dataset.index);
  const symbol = cell.dataset.symbol;
  if (symbol === 'S' || symbol === 'G') {
    selectedEndpoint = selectedEndpoint === symbol ? null : symbol;
    running = false;
    draw();
    cells.forEach(item => {
      item.classList.remove('selected-endpoint');
      const selected = item.dataset.symbol === selectedEndpoint;
      if (selected) item.classList.add('selected-endpoint');
      item.setAttribute('aria-pressed', String(selected));
    });
    editorStatus.textContent = selectedEndpoint
      ? `Select a new cell for ${selectedEndpoint}. Click ${selectedEndpoint} again to cancel.` : editorHint;
    return;
  }
  if (selectedEndpoint) {
    const previous = cells.findIndex(item => item.dataset.symbol === selectedEndpoint);
    setSymbol(previous, '.');
    setSymbol(index, selectedEndpoint);
    selectedEndpoint = null;
  } else {
    setSymbol(index, symbol === '#' ? '.' : '#');
  }
  dirty = true;
  ++mapRevision;
  running = false;
  slider.disabled = play.disabled = restart.disabled = true;
  algorithmSelect.disabled = true;
  runMap.disabled = discardEdits.disabled = false;
  renderCells(draftRows);
  cells[index].focus();
  editorStatus.textContent = 'Map changed. Run algorithms to update the results.';
  status.textContent = 'Animation paused while editing.';
  play.textContent = 'Play';
  saveDraft();
};

discardEdits.onclick = () => {
  if (!run || submitting) return;
  dirty = false;
  selectedEndpoint = null;
  ++mapRevision;
  showRun(run);
  saveDraft();
};

runMap.onclick = async () => {
  if (!run || submitting) return;
  submitting = true;
  selectedEndpoint = null;
  ++mapRevision;
  running = false;
  algorithmSelect.disabled = true;
  slider.disabled = play.disabled = restart.disabled = true;
  runMap.disabled = discardEdits.disabled = true;
  cells.forEach(cell => { cell.disabled = true; });
  editorStatus.textContent = 'Running algorithms...';
  try {
    await saveDraft();
    const response = await fetch('/api/run', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ rows: draftRows }),
    });
    if (response.status === 404 || response.status === 501 || response.status === 405) {
      throw new Error('Start python serve.py (macOS/Linux: python3 serve.py) to run edited maps.');
    }
    const result = await response.json();
    if (!response.ok) throw new Error(result.error || `HTTP ${response.status}`);
    const data = result.runs[algorithmSelect.value];
    if (!data) throw new Error('The selected algorithm did not return a result.');
    dirty = false;
    submitting = false;
    lastJson = null;
    showRun(data);
    editorStatus.textContent = 'Results updated. Click a cell to edit again.';
  } catch (error) {
    editorStatus.textContent = error.message;
    status.textContent = 'Could not update results. Your map edits are kept.';
  } finally {
    submitting = false;
    runMap.disabled = false;
    discardEdits.disabled = !dirty;
    algorithmSelect.disabled = dirty;
    cells.forEach(cell => {
      cell.disabled = false;
    });
    if (!dirty) slider.disabled = play.disabled = restart.disabled = false;
  }
};

function showRun(data) {
  run = data;
  totalSteps = data.expansionOrder.length ||
    data.discoveredTimes.reduce((max, time) => Math.max(max, time), 0);
  draftRows = data.rows.slice();
  renderCells(draftRows);
  algorithm.textContent = data.algorithm;
  runMap.disabled = false;
  discardEdits.disabled = true;
  algorithmSelect.disabled = false;
  editorStatus.textContent = editorHint;

  slider.max = totalSteps;
  slider.disabled = false;
  play.disabled = false;
  restart.disabled = false;
  step = 0;
  running = true;
  draw();
}

function draw() {
  if (!run || dirty || submitting) return;
  if (run.expansionOrder.length) {
    const counts = new Array(cells.length).fill(0);
    for (let i = 0; i < step; ++i) counts[run.expansionOrder[i]]++;
    cells.forEach((cell, index) => {
      cell.classList.remove('visited', 'current', 'path');
      if (counts[index]) cell.classList.add('visited');
      if (cell.dataset.symbol === '.') cell.textContent = counts[index] || '';
      cell.title = counts[index] ? `Expanded ${counts[index]} time(s)` : '';
    });
    if (step > 0 && (step < totalSteps || run.truncated)) {
      cells[run.expansionOrder[step - 1]].classList.add('current');
    }
    const revisits = step - counts.filter(count => count > 0).length;
    status.textContent = step === totalSteps
      ? (run.truncated
        ? `Stopped at ${step} expansions (limit) · ${revisits} revisits`
        : run.path.length
          ? `Path: ${run.path.length - 1} steps · ${step} expansions · ${revisits} revisits`
          : run.goalExpanded
            ? `Goal reached, but prev links cycle · ${step} expansions · ${revisits} revisits`
          : `No path · ${step} expansions · ${revisits} revisits`)
      : `Expanded ${step} of ${totalSteps} · ${revisits} revisits`;
  } else {
    cells.forEach((cell, index) => {
      cell.classList.remove('visited', 'current', 'path');
      const time = run.discoveredTimes[index];
      if (time > 0 && time <= step) cell.classList.add('visited');
      if (time === step && step < totalSteps) cell.classList.add('current');
    });
    status.textContent = step === totalSteps
      ? (run.path.length
        ? `Path: ${run.path.length - 1} steps · discovered ${step} cells`
        : `No path · discovered ${step} cells`)
      : `Discovered ${step} of ${totalSteps} cells`;
  }
  if (step === totalSteps) {
    run.path.forEach(index => cells[index].classList.add('path'));
  }
  slider.value = step;
  play.textContent = running ? 'Pause' : 'Play';
}

play.onclick = () => {
  if (step === totalSteps) step = 0;
  running = !running;
  draw();
};
restart.onclick = () => { step = 0; running = true; draw(); };
slider.oninput = () => { step = Number(slider.value); running = false; draw(); };
function clearRun() {
  selectedEndpoint = null;
  lastJson = null;
  run = null;
  running = false;
  cells = [];
  grid.replaceChildren();
  slider.disabled = true;
  play.disabled = true;
  restart.disabled = true;
  runMap.disabled = discardEdits.disabled = true;
  const selectedOption = algorithmSelect.selectedOptions[0];
  algorithm.textContent = selectedOption ? selectedOption.textContent : 'Grid pathfinding';
  status.textContent = 'Loading animation...';
}
algorithmSelect.onchange = () => {
  clearRun();
  refreshRun();
};

setInterval(() => {
  if (!running || !run || step >= totalSteps) return;
  ++step;
  if (step === totalSteps) running = false;
  draw();
}, 90);

async function refreshRun() {
  if (loading || dirty || selectedEndpoint || submitting || !algorithmSelect.value) return;
  loading = true;
  const selected = algorithmSelect.value;
  const revision = mapRevision;
  try {
    const response = await fetch(`run-${selected}.json?reload=${Date.now()}`, { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const source = await response.text();
    if (dirty || selectedEndpoint || submitting || revision !== mapRevision || selected !== algorithmSelect.value) return;
    if (source === lastJson) return;
    const data = JSON.parse(source);
    if (!Array.isArray(data.rows) || !data.rows.length ||
        !Array.isArray(data.discoveredTimes) || !Array.isArray(data.path) ||
        !Array.isArray(data.expansionOrder) || typeof data.truncated !== 'boolean' ||
        typeof data.goalExpanded !== 'boolean' ||
        data.discoveredTimes.length !== data.rows.reduce((count, row) => count + row.length, 0) ||
        data.expansionOrder.some(index => !Number.isInteger(index) ||
          index < 0 || index >= data.discoveredTimes.length)) {
      throw new Error('Invalid run.json');
    }
    if (JSON.stringify(data) !== JSON.stringify(run)) showRun(data);
    lastJson = source;
  } catch (error) {
    // A build may still be writing the file. The next poll will retry.
    if (!run && selected === algorithmSelect.value) {
      status.textContent = `Could not load run-${selected}.json: ${error.message}. Press Ctrl+Shift+B to build.`;
    }
  } finally {
    loading = false;
    if (selected !== algorithmSelect.value) refreshRun();
  }
}

async function refreshManifest() {
  if (manifestLoading || dirty || selectedEndpoint || submitting) return;
  manifestLoading = true;
  try {
    const response = await fetch(`algorithms.json?reload=${Date.now()}`, { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const source = await response.text();
    if (dirty || selectedEndpoint || submitting) return;
    if (source === lastManifest) return;
    const data = JSON.parse(source);
    if (!Array.isArray(data.algorithms) || !data.algorithms.length ||
        data.algorithms.some(item => !item || !/^[a-z_][a-z0-9_]*$/.test(item.id) ||
          typeof item.name !== 'string' || !item.name.length)) {
      throw new Error('Invalid algorithms.json');
    }

    const previous = algorithmSelect.value;
    algorithmSelect.replaceChildren();
    for (const item of data.algorithms) {
      const option = document.createElement('option');
      option.value = item.id;
      option.textContent = item.name;
      algorithmSelect.appendChild(option);
    }
    algorithmSelect.disabled = false;
    const defaultAlgorithm = data.algorithms.find(item => item.id === 'breadthfirstsearch');
    algorithmSelect.value = data.algorithms.some(item => item.id === previous)
      ? previous : (defaultAlgorithm || data.algorithms[0]).id;
    lastManifest = source;
    if (algorithmSelect.value !== previous) {
      clearRun();
      refreshRun();
    }
  } catch (error) {
    if (algorithmSelect.disabled) {
      status.textContent = `Could not load algorithms.json: ${error.message}. Press Ctrl+Shift+B to build.`;
    }
  } finally {
    manifestLoading = false;
  }
}

refreshManifest();
setInterval(refreshManifest, 800);
setInterval(refreshRun, 800);
