const value = id => document.getElementById(id);
const startPauseButton = value("start-pause");
const stopButton = value("stop");
const stateIcons = {
  start: startPauseButton.querySelector(".icon-start"),
  pause: startPauseButton.querySelector(".icon-pause"),
  resume: startPauseButton.querySelector(".icon-resume"),
};

const session = {
  distanceMeters: 0,
  elapsedMs: 0,
  lastSourceDistanceMeters: null,
  lastTimerUpdateMs: 0,
  refreshTimer: null,
  requestVersion: 0,
};

const formatDuration = seconds => {
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  const remainingSeconds = seconds % 60;
  return `${String(hours).padStart(2, "0")}:${String(minutes).padStart(2, "0")}:${String(remainingSeconds).padStart(2, "0")}`;
};

const formatPace = seconds => {
  if (seconds <= 0) return "--:--";
  const roundedSeconds = Math.round(seconds);
  return `${Math.floor(roundedSeconds / 60)}:${String(roundedSeconds % 60).padStart(2, "0")}`;
};

function renderControls() {
  Object.entries(stateIcons).forEach(([name, icon]) => {
    icon.toggleAttribute("hidden", name !== state.icon);
  });
  startPauseButton.setAttribute("aria-label", state.actionLabel);
  stopButton.disabled = !state.canStop;
}

function renderMetrics(paceSecondsPerKm = 0) {
  value("distance").textContent = (session.distanceMeters / 1000).toFixed(2);
  value("pace").textContent = formatPace(paceSecondsPerKm);
  value("duration").textContent = formatDuration(Math.floor(session.elapsedMs / 1000));
}

function transitionTo(nextState) {
  state = nextState;
  state.enter();
  renderControls();
  renderMetrics();
}

function cancelRefresh() {
  clearTimeout(session.refreshTimer);
  session.refreshTimer = null;
  session.requestVersion++;
}

function updateElapsedTime(nowMs) {
  if (session.lastTimerUpdateMs > 0) {
    session.elapsedMs += nowMs - session.lastTimerUpdateMs;
  }
  session.lastTimerUpdateMs = nowMs;
}

async function fetchTelemetry() {
  const response = await fetch("/api/status", { cache: "no-store" });
  if (!response.ok) throw new Error(response.status);
  return response.json();
}

function scheduleNextRefresh() {
  clearTimeout(session.refreshTimer);
  session.refreshTimer = setTimeout(refreshSession, 1000);
}

async function startSession() {
  cancelRefresh();
  const version = session.requestVersion;

  try {
    const telemetry = await fetchTelemetry();
    if (version !== session.requestVersion) return;

    session.lastSourceDistanceMeters = telemetry.distanceMeters;
    session.lastTimerUpdateMs = performance.now();
    transitionTo(runningState);
    renderMetrics(telemetry.paceSecondsPerKm);
    scheduleNextRefresh();
  } catch (error) {
    renderControls();
  }
}

async function refreshSession() {
  const version = session.requestVersion;

  try {
    const telemetry = await fetchTelemetry();
    if (version !== session.requestVersion) return;

    const nowMs = performance.now();
    updateElapsedTime(nowMs);

    if (session.lastSourceDistanceMeters !== null) {
      session.distanceMeters += Math.max(0, telemetry.distanceMeters - session.lastSourceDistanceMeters);
    }

    session.lastSourceDistanceMeters = telemetry.distanceMeters;
    renderMetrics(telemetry.paceSecondsPerKm);
  } catch (error) {
    if (version !== session.requestVersion) return;
    session.refreshTimer = null;
    return;
  }

  if (version === session.requestVersion) {
    scheduleNextRefresh();
  }
}

const stoppedState = {
  icon: "start",
  actionLabel: "Start",
  canStop: false,
  enter() {
    session.lastSourceDistanceMeters = null;
  },
  startPauseAction() {
    session.distanceMeters = 0;
    session.elapsedMs = 0;
    startSession();
  },
  stopAction() {},
};

const runningState = {
  icon: "pause",
  actionLabel: "Pause",
  canStop: true,
  enter() {},
  startPauseAction() {
    cancelRefresh();
    updateElapsedTime(performance.now());
    transitionTo(pausedState);
  },
  stopAction() {
    cancelRefresh();
    updateElapsedTime(performance.now());
    transitionTo(stoppedState);
  },
};

const pausedState = {
  icon: "resume",
  actionLabel: "Resume",
  canStop: true,
  enter() {
    session.lastSourceDistanceMeters = null;
  },
  startPauseAction: startSession,
  stopAction() {
    cancelRefresh();
    transitionTo(stoppedState);
  },
};

let state = stoppedState;
startPauseButton.addEventListener("click", () => state.startPauseAction());
stopButton.addEventListener("click", () => state.stopAction());

state.enter();
renderControls();
renderMetrics();
