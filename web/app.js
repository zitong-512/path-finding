const grid = document.querySelector('#grid');
const algorithm = document.querySelector('#algorithm');
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

function showRun(data) {
  run = data;
  totalSteps = data.discoveredTimes.reduce((max, time) => Math.max(max, time), 0);
  cells = [];
  grid.replaceChildren();
  grid.style.gridTemplateColumns = `repeat(${data.rows[0].length}, var(--cell-size))`;
  algorithm.textContent = data.algorithm;

  for (const row of data.rows) {
    for (const symbol of row) {
      const cell = document.createElement('div');
      cell.className = 'cell';
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
  cells.forEach((cell, index) => {
    cell.classList.remove('visited', 'current', 'path');
    const time = run.discoveredTimes[index];
    if (time > 0 && time <= step) cell.classList.add('visited');
    if (time === step && step < totalSteps) cell.classList.add('current');
  });
  if (step === totalSteps) {
    run.path.forEach(index => cells[index].classList.add('path'));
  }
  slider.value = step;
  status.textContent = step === totalSteps
    ? (run.path.length
      ? `Path: ${run.path.length - 1} steps · discovered ${step} cells`
      : `No path · discovered ${step} cells`)
    : `Discovered ${step} of ${totalSteps} cells`;
  play.textContent = running ? 'Pause' : 'Play';
}

play.onclick = () => {
  if (step === totalSteps) step = 0;
  running = !running;
  draw();
};
restart.onclick = () => { step = 0; running = true; draw(); };
slider.oninput = () => { step = Number(slider.value); running = false; draw(); };

setInterval(() => {
  if (!running || !run || step >= totalSteps) return;
  ++step;
  if (step === totalSteps) running = false;
  draw();
}, 90);

async function refreshRun() {
  if (loading) return;
  loading = true;
  try {
    const response = await fetch(`run.json?reload=${Date.now()}`, { cache: 'no-store' });
    if (!response.ok) throw new Error(`HTTP ${response.status}`);
    const source = await response.text();
    if (source === lastJson) return;
    const data = JSON.parse(source);
    if (!Array.isArray(data.rows) || !data.rows.length ||
        !Array.isArray(data.discoveredTimes) || !Array.isArray(data.path) ||
        data.discoveredTimes.length !== data.rows.reduce((count, row) => count + row.length, 0)) {
      throw new Error('Invalid run.json');
    }
    showRun(data);
    lastJson = source;
  } catch (error) {
    // A build may still be writing run.json. The next poll will retry.
    if (!run) status.textContent = 'Run the C++ app to generate web/run.json.';
  } finally {
    loading = false;
  }
}

refreshRun();
setInterval(refreshRun, 800);
