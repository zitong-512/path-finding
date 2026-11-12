const grid = document.querySelector('#grid');
const algorithm = document.querySelector('#algorithm');
const algorithmSelect = document.querySelector('#algorithm-select');
const slider = document.querySelector('#step');
const status = document.querySelector('#status');
const play = document.querySelector('#play');
const restart = document.querySelector('#restart');

let run = null;
let cells = [];
let step = 0;
let totalSteps = 0;
let running = false;
let lastJson = null;
let loading = false;
let lastManifest = null;
let manifestLoading = false;

function showRun(data) {
  run = data;
  totalSteps = data.expansionOrder.length ||
    data.discoveredTimes.reduce((max, time) => Math.max(max, time), 0);
  cells = [];
  grid.replaceChildren();
  grid.style.gridTemplateColumns = `repeat(${data.rows[0].length}, var(--cell-size))`;
  algorithm.textContent = data.algorithm;

  for (const row of data.rows) {
    for (const symbol of row) {
      const cell = document.createElement('div');
      cell.className = 'cell';
      cell.dataset.symbol = symbol;
      if (symbol === '#') cell.classList.add('wall');
      if (symbol === 'S') cell.classList.add('start');
      if (symbol === 'G') cell.classList.add('goal');
      if (symbol === 'S' || symbol === 'G') cell.textContent = symbol;
      grid.appendChild(cell);
      cells.push(cell);
    }
  }

  slider.max = totalSteps;
  slider.disabled = false;
  play.disabled = false;
  restart.disabled = false;
  step = 0;
  running = true;
  draw();
}

function draw() {
  if (!run) return;
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
  lastJson = null;
  run = null;
  running = false;
  cells = [];
  grid.replaceChildren();
  slider.disabled = true;
  play.disabled = true;
  restart.disabled = true;
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
  if (loading || !algorithmSelect.value) return;
  loading = true;
  const selected = algorithmSelect.value;
  try {
    const response = await fetch(`run-${selected}.json?reload=${Date.now()}`, { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const source = await response.text();
    if (selected !== algorithmSelect.value) return;
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
    showRun(data);
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
  if (manifestLoading) return;
  manifestLoading = true;
  try {
    const response = await fetch(`algorithms.json?reload=${Date.now()}`, { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const source = await response.text();
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
