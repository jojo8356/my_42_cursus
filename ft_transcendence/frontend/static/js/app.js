/* ft_transcendence — vanilla JS single-page application.
 * Mirrors the server-side Pong engine (games/engine.py) for the canvas.
 */
"use strict";

// --------------------------------------------------------------------------- i18n
const I18N = {
  en: {
    auth_title: "Sign in / Register", tab_login: "Login", tab_register: "Register",
    login_btn: "Login", register_btn: "Register", oauth_42: "Login with 42 (OAuth)",
    pong_title: "Pong", vs_ai: "vs AI", start_game: "Start Game",
    customize: "Customize", powerup: "Power-up", instructions: "Controls",
    ai_note: "AI moves at the same paddle speed and refreshes once per second.",
    game_select: "Game", connect4_title: "Connect-4",
    connect4_help: "Click a column (1-7) to drop a piece. First to align 4 wins.",
    tournament_create: "Create / Join Tournament", create_tour: "Create",
    matchmaking: "Matchmaking", join_queue: "Join Matchmaking Queue",
    channels: "Channels", new_channel: "New", chat_title: "Chat", send: "Send",
    profile: "Profile",
  },
  fr: {
    auth_title: "Connexion / Inscription", tab_login: "Connexion", tab_register: "Inscription",
    login_btn: "Connexion", register_btn: "Inscription", oauth_42: "Connexion 42 (OAuth)",
    pong_title: "Pong", vs_ai: "contre IA", start_game: "Lancer",
    customize: "Personnaliser", powerup: "Bonus", instructions: "Contrôles",
    ai_note: "L'IA se déplace à la même vitesse et se rafraîchit une fois par seconde.",
    game_select: "Jeu", connect4_title: "Puissance 4",
    connect4_help: "Cliquez sur une colonne (1-7) pour y poser un pion. Le premier à aligner 4 gagne.",
    tournament_create: "Créer / Rejoindre un tournoi", create_tour: "Créer",
    matchmaking: "Matchmaking", join_queue: "Rejoindre la file",
    channels: "Canaux", new_channel: "Nouveau", chat_title: "Chat", send: "Envoyer",
    profile: "Profil",
  },
  de: {
    auth_title: "Anmelden / Registrieren", tab_login: "Anmelden", tab_register: "Registrieren",
    login_btn: "Anmelden", register_btn: "Registrieren", oauth_42: "Mit 42 anmelden (OAuth)",
    pong_title: "Pong", vs_ai: "gegen KI", start_game: "Starten",
    customize: "Anpassen", powerup: "Power-up", instructions: "Steuerung",
    ai_note: "Die KI bewegt sich mit derselben Paddle-Geschwindigkeit und 刷新t einmal pro Sekunde.",
    game_select: "Spiel", connect4_title: "Connect-4",
    connect4_help: "Klicke auf eine Spalte (1-7), um einen Stein fallen zu lassen. Wer 4er setzt, gewinnt.",
    tournament_create: "Turnier erstellen / beitreten", create_tour: "Erstellen",
    matchmaking: "Matchmaking", join_queue: "Queue beitreten",
    channels: "Kanäle", new_channel: "Neu", chat_title: "Chat", send: "Senden",
    profile: "Profil",
  },
};

let currentLang = localStorage.getItem("lang") || "en";
function applyI18n() {
  const dict = I18N[currentLang] || I18N.en;
  document.querySelectorAll("[data-i18n]").forEach((el) => {
    const key = el.getAttribute("data-i18n");
    if (dict[key]) el.textContent = dict[key];
  });
  document.documentElement.lang = currentLang;
}

// --------------------------------------------------------------------------- API client
const API = (function () {
  let access = localStorage.getItem("access");
  let refresh = localStorage.getItem("refresh");

  function setTokens(tokens) {
    if (tokens.access) access = tokens.access;
    if (tokens.refresh) refresh = tokens.refresh;
    localStorage.setItem("access", access || "");
    localStorage.setItem("refresh", refresh || "");
  }
  function clearTokens() {
    access = refresh = null;
    localStorage.removeItem("access");
    localStorage.removeItem("refresh");
  }
  function authHeader() {
    return access ? { Authorization: "Bearer " + access } : {};
  }

  async function request(method, path, body) {
    const opts = {
      method,
      headers: { "Content-Type": "application/json", ...authHeader() },
    };
    if (body !== undefined) opts.body = JSON.stringify(body);
    const res = await fetch(path, opts);
    if (res.status === 401 && refresh) {
      const tok = await fetch("/api/auth/refresh/", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ refresh }),
      });
      if (tok.ok) {
        const data = await tok.json();
        setTokens({ access: data.access });
        return request(method, path, body);
      }
      clearTokens();
      Router.go("/auth");
    }
    const text = await res.text();
    const data = text ? JSON.parse(text) : {};
    if (!res.ok) throw Object.assign(new Error(JSON.stringify(data)), { data, status: res.status });
    return data;
  }

  return {
    setTokens, clearTokens, authHeader,
    get: (p) => request("GET", p),
    post: (p, b) => request("POST", p, b),
    put: (p, b) => request("PUT", p, b),
    del: (p) => request("DELETE", p),
    hasAuth: () => !!access,
  };
})();

// --------------------------------------------------------------------------- router (hash-based SPA, Back/Forward)
const Router = (function () {
  const routes = ["auth", "game", "tournament", "chat", "profile"];
  function parse() {
    const hash = location.hash.replace(/^#\//, "") || "auth";
    const [name] = hash.split("?");
    return name;
  }
  function show(name) {
    routes.forEach((r) => {
      const el = document.getElementById("view-" + r);
      if (el) el.classList.toggle("d-none", r !== name);
    });
    // hide auth-specific nav
    document.getElementById("logout-btn").classList.toggle("d-none", !API.hasAuth());
  }
  function go(name) { location.hash = "/" + name; }
  function render() {
    const name = parse();
    show(name);
    if (name === "tournament") Tournament.load();
    if (name === "chat") Chat.connect();
    if (name === "profile") Profile.load();
    if (name === "auth" && API.hasAuth()) go("game");
  }
  window.addEventListener("hashchange", render);
  return { go, render, parse };
})();

// --------------------------------------------------------------------------- auth view
const Auth = (function () {
  let mode = "login";
  function init() {
    document.querySelectorAll("[data-auth]").forEach((a) => {
      a.addEventListener("click", (e) => {
        e.preventDefault();
        mode = a.dataset.auth;
        document.querySelectorAll("[data-auth]").forEach((x) => x.classList.remove("active"));
        a.classList.add("active");
        document.getElementById("auth-login-fields").classList.toggle("d-none", mode !== "login");
        document.getElementById("auth-register-fields").classList.toggle("d-none", mode !== "register");
      });
    });
    document.getElementById("login-submit").addEventListener("click", login);
    document.getElementById("reg-submit").addEventListener("click", register);
    document.getElementById("oauth-btn").addEventListener("click", oauthLogin);
  }
  async function login() {
    const email = document.getElementById("login-email").value;
    const password = document.getElementById("login-password").value;
    try {
      const data = await API.post("/api/auth/login/", { email, password });
      API.setTokens(data.tokens);
      Router.go("game");
    } catch (e) { showError(e); }
  }
  async function register() {
    const email = document.getElementById("reg-email").value;
    const username = document.getElementById("reg-username").value;
    const password = document.getElementById("reg-password").value;
    try {
      const data = await API.post("/api/auth/register/", { email, username, password });
      API.setTokens(data.tokens);
      Router.go("game");
    } catch (e) { showError(e); }
  }
  async function oauthLogin() {
    try {
      const data = await API.get("/api/auth/oauth/authorize/");
      // In production this redirects to data.authorization_url. For the demo
      // we simulate the callback with the mock code.
      window.location.href = data.authorization_url;
    } catch (e) { showError(e); }
  }
  function showError(e) {
    const el = document.getElementById("auth-error");
    el.textContent = (e.data && e.data.error) || e.message || "Error";
  }
  return { init };
})();

// --------------------------------------------------------------------------- Pong game (canvas, mirrors engine.py)
const Pong = (function () {
  const W = 800, H = 600, PADDLE_W = 12, PADDLE_H = 100, R = 8;
  const PADDLE_SPEED = 6, BALL_SPEED = 6;
  const canvas = () => document.getElementById("pong-canvas");
  let game = null, raf = null, keys = {}, powerupBtn = null;

  function newGame(ai) {
    game = {
      left: { x: 40, y: H / 2 - PADDLE_H / 2 },
      right: { x: W - 40 - PADDLE_W, y: H / 2 - PADDLE_H / 2 },
      ball: { x: W / 2, y: H / 2 }, vx: ai ? -BALL_SPEED : BALL_SPEED, vy: 0,
      leftScore: 0, rightScore: 0, ai: ai, running: false,
      leftPower: [], rightPower: [],
    };
  }

  function start() {
    if (game) stop();
    newGame(document.getElementById("vs-ai").checked);
    game.running = true;
    game.ball.x = W / 2; game.ball.y = H / 2;
    game.vx = game.ai ? -BALL_SPEED : BALL_SPEED; game.vy = 0;
    if (!raf) raf = requestAnimationFrame(loop);
  }
  function stop() { game && (game.running = false); if (raf) cancelAnimationFrame(raf); raf = null; }

  function loop() {
    if (game && game.running) {
      handleInput();
      step();
      draw();
    }
    raf = requestAnimationFrame(loop);
  }

  function handleInput() {
    if (!game) return;
    if (keys["w"] || keys["W"]) game.left.y -= PADDLE_SPEED;
    if (keys["s"] || keys["S"]) game.left.y += PADDLE_SPEED;
    // AI: refresh once per second, simulate keyboard input
    if (game.ai) {
      if (!game._aiT) game._aiT = 0;
      game._aiT += 1;
      if (game._aiT >= 60) { // ~1s at 60fps
        aiMove();
        game._aiT = 0;
      }
    }
    clampPaddle(game.left);
    clampPaddle(game.right);
  }

  function aiMove() {
    // predict ball y, move toward it at fixed speed (mirrors ai.py)
    let py = game.ball.y;
    let vy = game.vy;
    if (vy === 0) { /* stay */ }
    else {
      const dir = vy > 0 ? 1 : -1;
      const dist = BALL_SPEED * 1; // 1 second
      const low = R, high = H - R, span = high - low;
      let pos = py, rem = dist;
      while (rem > 0) {
        if (dir > 0) {
          const to = high - pos;
          if (rem <= to) { pos += rem; rem = 0; }
          else { rem -= to; pos = high; dir = -1; }
        } else {
          const to = pos - low;
          if (rem <= to) { pos -= rem; rem = 0; }
          else { rem -= to; pos = low; dir = 1; }
        }
      }
      py = pos;
    }
    const center = game.right.y + PADDLE_H / 2;
    if (py < center - 2) game.right.y -= PADDLE_SPEED;
    else if (py > center + 2) game.right.y += PADDLE_SPEED;
  }

  function clampPaddle(p) {
    p.y = Math.max(0, Math.min(H - PADDLE_H, p.y));
  }

  function step() {
    const speed = BALL_SPEED;
    game.ball.x += game.vx;
    game.ball.y += game.vy;
    if (game.ball.y - R <= 0) { game.ball.y = R; game.vy = Math.abs(game.vy); }
    if (game.ball.y + R >= H) { game.ball.y = H - R; game.vy = -Math.abs(game.vy); }
    // paddle collisions
    if (ballHits(game.left, game.ball)) { game.vx = Math.abs(game.vx); game.vy = spin(game.ball.y, game.left); }
    if (ballHits(game.right, game.ball)) { game.vx = -Math.abs(game.vx); game.vy = spin(game.ball.y, game.right); }
    // score
    if (game.ball.x < 0) { game.rightScore++; resetBall(1); }
    else if (game.ball.x > W) { game.leftScore++; resetBall(-1); }
  }
  function ballHits(paddle, ball) {
    return ball.x - R < paddle.x + PADDLE_W && ball.x + R > paddle.x &&
           ball.y > paddle.y && ball.y < paddle.y + PADDLE_H;
  }
  function spin(by, paddle) {
    const hit = Math.max(-0.5, Math.min(0.5, (by - paddle.y) / PADDLE_H - 0.5));
    return hit * BALL_SPEED * 2;
  }
  function resetBall(dir) {
    game.ball.x = W / 2; game.ball.y = H / 2;
    game.vx = BALL_SPEED * dir; game.vy = 0;
  }

  function draw() {
    const c = canvas(); if (!c) return;
    const ctx = c.getContext("2d");
    ctx.clearRect(0, 0, W, H);
    ctx.fillStyle = "#e6e6e6";
    // center line
    ctx.fillRect(W / 2 - 1, 0, 2, H);
    // paddles
    ctx.fillRect(game.left.x, game.left.y, PADDLE_W, PADDLE_H);
    ctx.fillRect(game.right.x, game.right.y, PADDLE_W, PADDLE_H);
    // ball
    ctx.beginPath(); ctx.arc(game.ball.x, game.ball.y, R, 0, Math.PI * 2); ctx.fill();
    // score
    ctx.font = "32px sans-serif"; ctx.fillText(game.leftScore, 60, 60);
    ctx.fillText(game.rightScore, W - 90, 60);
  }

  function init() {
    document.addEventListener("keydown", (e) => { keys[e.key] = true; });
    document.addEventListener("keyup", (e) => { keys[e.key] = false; });
    const cb = document.getElementById("customize-btn");
    cb.addEventListener("click", async () => {
      const data = await API.get("/api/games/customizations/");
      alert("Customizations: " + data.customizations.map((c) => c.name).join(", "));
    });
    powerupBtn = document.getElementById("powerup-btn");
    powerupBtn.addEventListener("click", () => {
      if (!game) return;
      if (game.leftPower.length < 2) game.leftPower.push("speed");
    });
  }
  return { init, start, stop };
})();

// --------------------------------------------------------------------------- Connect-4 (module 7: second game)
const Connect4 = (function () {
  const ROWS = 6, COLS = 7, WIN = 4;
  const EMPTY = 0, P1 = 1, P2 = 2;
  const canvas = () => document.getElementById("connect4-canvas");
  const CELL = 80, PAD = 20;
  let game = null, raf = null, current = "connect4";

  function newGame(ai) {
    game = {
      board: Array.from({ length: ROWS }, () => Array(COLS).fill(0)),
      turn: P1,
      winner: 0,
      ai: !!ai,
      aiSide: ai ? P2 : null,
      over: false,
    };
  }

  function start() {
    if (game) stop();
    newGame(document.getElementById("vs-ai").checked);
    game.over = false;
    if (!raf) raf = requestAnimationFrame(loop);
    draw();
  }
  function stop() { if (raf) cancelAnimationFrame(raf); raf = null; }
  function loop() { draw(); raf = requestAnimationFrame(loop); }

  function isValid(col) {
    return col >= 0 && col < COLS && game.board[0][col] === EMPTY;
  }
  function drop(col) {
    if (game.over || !isValid(col)) return false;
    let r = 0;
    while (r < ROWS && game.board[r][col] !== EMPTY) r++;
    game.board[r][col] = game.turn;
    if (checkWin(r, col, game.turn)) game.winner = game.turn;
    else if (game.board[0].every((v) => v !== EMPTY)) game.winner = 3;
    else game.turn = game.turn === P1 ? P2 : P1;
    game.over = game.winner !== 0;
    return true;
  }
  function checkWin(row, col, player) {
    const dirs = [[0, 1], [1, 0], [1, 1], [1, -1]];
    for (const [dr, dc] of dirs) {
      let count = 1;
      let r = row + dr, c = col + dc;
      while (r >= 0 && r < ROWS && c >= 0 && c < COLS && game.board[r][c] === player) { count++; r += dr; c += dc; }
      r = row - dr; c = col - dc;
      while (r >= 0 && r < ROWS && c >= 0 && c < COLS && game.board[r][c] === player) { count++; r -= dr; c -= dc; }
      if (count >= WIN) return true;
    }
    return false;
  }

  // Heuristic AI (mirrors connect4_ai.py): centre control + threats, no A*.
  function aiMove() {
    if (!game || game.over) return;
    const available = [];
    for (let c = 0; c < COLS; c++) if (isValid(c)) available.push(c);
    if (!available.length) return;
    let best = -Infinity, bestCol = available[0];
    for (const col of available) {
      const score = minimax(col, 3, true);
      if (score > best) { best = score; bestCol = col; }
    }
    drop(bestCol);
    draw();
  }
  function cloneBoard() { return game.board.map((row) => row.slice()); }
  function evalBoard(b, me) {
    let score = 0;
    const opp = me === P1 ? P2 : P1;
    for (let r = 0; r < ROWS; r++) for (let c = 0; c < COLS; c++) {
      if (b[r][c] === me) score += 1 + (1 - Math.abs(c - 3) / 3) * 0.5;
      else if (b[r][c] === opp) score -= 1 + (1 - Math.abs(c - 3) / 3) * 0.5;
    }
    // lines of 2 and 3
    for (let r = 0; r < ROWS; r++) for (let c = 0; c < COLS; c++) {
      for (const [dr, dc] of [[0, 1], [1, 0], [1, 1], [1, -1]]) {
        let cnt = 0, rr = r, cc = c;
        while (rr >= 0 && rr < ROWS && cc >= 0 && cc < COLS && b[rr][cc] === me) { cnt++; rr += dr; cc += dc; }
        if (cnt === 2) score += 2;
        else if (cnt === 3) score += 12;
        else if (cnt >= 4) score += 1000;
      }
    }
    return score;
  }
  function minimax(col, depth, maximizing) {
    // simulate drop
    const b = cloneBoard();
    let r = 0;
    while (r < ROWS && b[r][col] !== EMPTY) r++;
    b[r][col] = game.turn;
    const me = game.aiSide;
    // quick win check on this board
    let winner = 0;
    if (checkWinOn(b, r, col, game.turn)) winner = game.turn;
    else if (b[0].every((v) => v !== EMPTY)) winner = 3;
    if (winner !== 0 || depth === 0) {
      return winner === me ? 1000 - (4 - depth) : winner === (me === P1 ? P2 : P1) ? -1000 + (4 - depth) : 0;
    }
    const nextTurn = game.turn === P1 ? P2 : P1;
    const avail = [];
    for (let c = 0; c < COLS; c++) if (b[0][c] === EMPTY) avail.push(c);
    if (!avail.length) return 0;
    if (maximizing) {
      let best = -Infinity;
      for (const c of avail) {
        const nb = cloneBoard2(b);
        let rr = 0;
        while (rr < ROWS && nb[rr][c] !== EMPTY) rr++;
        nb[rr][c] = nextTurn;
        const sub = minimaxOn(nb, nextTurn, me, depth - 1, false);
        if (sub > best) best = sub;
      }
      return best;
    } else {
      let best = Infinity;
      for (const c of avail) {
        const nb = cloneBoard2(b);
        let rr = 0;
        while (rr < ROWS && nb[rr][c] !== EMPTY) rr++;
        nb[rr][c] = nextTurn;
        const sub = minimaxOn(nb, nextTurn, me, depth - 1, true);
        if (sub < best) best = sub;
      }
      return best;
    }
  }
  function cloneBoard2(b) { return b.map((row) => row.slice()); }
  function checkWinOn(b, row, col, player) {
    const dirs = [[0, 1], [1, 0], [1, 1], [1, -1]];
    for (const [dr, dc] of dirs) {
      let count = 1;
      let r = row + dr, c = col + dc;
      while (r >= 0 && r < ROWS && c >= 0 && c < COLS && b[r][c] === player) { count++; r += dr; c += dc; }
      r = row - dr; c = col - dc;
      while (r >= 0 && r < ROWS && c >= 0 && c < COLS && b[r][c] === player) { count++; r -= dr; c -= dc; }
      if (count >= WIN) return true;
    }
    return false;
  }
  function minimaxOn(b, turn, me, depth, maximizing) {
    // recursive eval on a cloned board
    let winner = 0;
    for (let r = 0; r < ROWS && winner === 0; r++)
      for (let c = 0; c < COLS && winner === 0; c++)
        if (b[r][c] !== EMPTY && checkWinOn(b, r, c, b[r][c])) winner = b[r][c];
    if (winner !== 0) return winner === me ? 1000 : -1000;
    if (depth === 0) return evalBoard(b, me);
    const avail = [];
    for (let c = 0; c < COLS; c++) if (b[0][c] === EMPTY) avail.push(c);
    if (!avail.length) return 0;
    const nextTurn = turn === P1 ? P2 : P1;
    if (maximizing) {
      let best = -Infinity;
      for (const c of avail) {
        const nb = cloneBoard2(b);
        let rr = 0;
        while (rr < ROWS && nb[rr][c] !== EMPTY) rr++;
        nb[rr][c] = turn;
        const sub = minimaxOn(nb, nextTurn, me, depth - 1, false);
        if (sub > best) best = sub;
      }
      return best;
    } else {
      let best = Infinity;
      for (const c of avail) {
        const nb = cloneBoard2(b);
        let rr = 0;
        while (rr < ROWS && nb[rr][c] !== EMPTY) rr++;
        nb[rr][c] = turn;
        const sub = minimaxOn(nb, nextTurn, me, depth - 1, true);
        if (sub < best) best = sub;
      }
      return best;
    }
  }

  function draw() {
    const c = canvas(); if (!c || !game) return;
    const ctx = c.getContext("2d");
    ctx.clearRect(0, 0, c.width, c.height);
    const W = COLS * CELL, H = ROWS * CELL;
    const ox = (c.width - W) / 2, oy = (c.height - H) / 2;
    // grid
    for (let r = 0; r <= ROWS; r++) {
      ctx.beginPath(); ctx.moveTo(ox, oy + r * CELL); ctx.lineTo(ox + W, oy + r * CELL); ctx.stroke();
    }
    for (let col = 0; col <= COLS; col++) {
      ctx.beginPath(); ctx.moveTo(ox + col * CELL, oy); ctx.lineTo(ox + col * CELL, oy + H); ctx.stroke();
    }
    // pieces (row 0 top)
    for (let r = 0; r < ROWS; r++) for (let c = 0; c < COLS; c++) {
      const v = game.board[r][c];
      if (v === EMPTY) continue;
      const cx = ox + c * CELL + CELL / 2;
      const cy = oy + r * CELL + CELL / 2;
      ctx.beginPath(); ctx.arc(cx, cy, CELL / 2 - 4, 0, Math.PI * 2);
      ctx.fillStyle = v === P1 ? "#e63946" : "#ffd166";
      ctx.fill();
    }
    // turn indicator
    ctx.fillStyle = "#333";
    ctx.font = "14px sans-serif";
    ctx.fillText(game.over ? "Game over" : ("Player " + game.turn + " to move"), ox, oy - 8);
  }

  function init() {
    document.querySelectorAll(".connect4-col").forEach((btn) => {
      btn.addEventListener("click", () => {
        if (!game || game.over) return;
        if (drop(Number(btn.dataset.col))) {
          draw();
          if (game.ai && game.turn === game.aiSide && !game.over) aiMove();
        }
      });
    });
  }
  function show(showIt) {
    const cv = canvas();
    cv.classList.toggle("d-none", !showIt);
    document.getElementById("connect4-controls").classList.toggle("d-none", !showIt);
    document.getElementById("connect4-controls-help").classList.toggle("d-none", !showIt);
    document.getElementById("pong-controls").classList.toggle("d-none", showIt);
    if (showIt) { start(); } else { stop(); }
  }
  return { init, start, stop, show, draw };
})();

// --------------------------------------------------------------------------- tournament
const Tournament = (function () {
  async function load() {
    try {
      const data = await API.get("/api/tournaments/");
      const el = document.getElementById("tour-list");
      el.innerHTML = "";
      if (!data.tournaments.length) { el.innerHTML = "<p class='text-muted'>No tournaments</p>"; return; }
      data.tournaments.forEach((t) => {
        const row = document.createElement("div");
        row.className = "d-flex justify-content-between align-items-center border rounded p-2 my-1";
        row.innerHTML = `<span>${t.name} <span class="badge">${t.status}</span></span>
          <button class="btn btn-sm btn-outline-primary join-tour" data-id="${t.id}">Join</button>`;
        el.appendChild(row);
      });
      el.querySelectorAll(".join-tour").forEach((b) =>
        b.addEventListener("click", () => join(b.dataset.id)));
    } catch (e) { console.error(e); }
  }
  async function create() {
    const name = document.getElementById("tour-name").value || "Tournament";
    await API.post("/api/tournaments/", { name });
    load();
  }
  async function join(id) {
    const alias = prompt("Your alias:") || "player";
    await API.post("/api/tournaments/" + id + "/", { alias });
    load();
  }
  async function joinQueue() {
    const data = await API.post("/api/tournaments/matchmaking/", {});
    const info = document.getElementById("queue-info");
    if (data.matched) info.textContent = "Match found: you vs user " + data.match[1];
    else info.textContent = "Waiting in queue... (" + data.queue_size + " waiting)";
  }
  function init() {
    document.getElementById("create-tour").addEventListener("click", create);
    document.getElementById("join-queue").addEventListener("click", joinQueue);
  }
  return { init, load };
})();

// --------------------------------------------------------------------------- chat (websocket)
const Chat = (function () {
  let ws = null, currentChannel = null;
  function connect() {
    if (!API.hasAuth()) return;
    if (ws) { ws.close(); }
    const proto = location.protocol === "https:" ? "wss" : "ws";
    ws = new WebSocket(`${proto}://${location.host}/ws/chat/`);
    ws.onmessage = (e) => {
      const msg = JSON.parse(e.data);
      if (msg.type === "dm") appendMsg(msg.from, msg.content);
      else if (msg.type === "channel") appendMsg(msg.from, msg.content);
    };
    renderChannels();
  }
  async function renderChannels() {
    try {
      const data = await API.get("/api/chat/channels/");
      const el = document.getElementById("channel-list");
      el.innerHTML = "";
      data.channels.concat(data.public).forEach((c) => {
        const a = document.createElement("a");
        a.className = "d-block p-2 border-bottom text-decoration-none text-light";
        a.href = "#";
        a.textContent = c.name;
        a.addEventListener("click", (e) => { e.preventDefault(); selectChannel(c); });
        el.appendChild(a);
      });
    } catch (e) { console.error(e); }
  }
  function selectChannel(c) {
    currentChannel = c;
    document.getElementById("chat-title").textContent = "# " + c.name;
    document.getElementById("chat-messages").innerHTML = "";
  }
  function appendMsg(who, content) {
    const el = document.getElementById("chat-messages");
    const div = document.createElement("div");
    div.className = "chat-msg";
    div.innerHTML = `<span class="who">${who}:</span> ${content}`;
    el.appendChild(div);
    el.scrollTop = el.scrollHeight;
  }
  function send() {
    const input = document.getElementById("chat-input");
    const text = input.value.trim();
    if (!text || !ws) return;
    if (currentChannel) {
      ws.send(JSON.stringify({ type: "channel", channel: currentChannel.id, content: text }));
    } else {
      const peer = prompt("Send DM to user id:");
      if (peer) ws.send(JSON.stringify({ type: "dm", to: Number(peer), content: text }));
    }
    input.value = "";
  }
  function init() {
    document.getElementById("chat-send").addEventListener("click", send);
    document.getElementById("chat-input").addEventListener("keydown", (e) => {
      if (e.key === "Enter") send();
    });
    document.getElementById("new-channel").addEventListener("click", async () => {
      const name = prompt("Channel name:");
      if (name) { await API.post("/api/chat/channels/", { name }); renderChannels(); }
    });
  }
  return { init, connect };
})();

// --------------------------------------------------------------------------- profile
const Profile = (function () {
  async function load() {
    try {
      const data = await API.get("/api/auth/profile/");
      const u = data.user;
      const matches = await API.get("/api/auth/matches/");
      const body = document.getElementById("profile-body");
      body.innerHTML = `
        <div class="row">
          <div class="col-md-4">
            <img class="rounded" src="${u.avatar || 'https://via.placeholder.com/80'}" width="80" height="80" />
            <h4>${u.username}</h4>
            <p class="text-muted">${u.email}</p>
            <p>Wins: <b>${u.wins}</b> · Losses: <b>${u.losses}</b></p>
            <p>2FA: <b>${u.two_factor_enabled ? 'on' : 'off'}</b> · Lang: <b>${u.language}</b></p>
            <button class="btn btn-sm btn-outline-danger" id="anon-btn">Anonymize (GDPR)</button>
            <button class="btn btn-sm btn-outline-warning" id="del-btn">Delete account (GDPR)</button>
          </div>
          <div class="col-md-8">
            <h5>Match History</h5>
            <ul class="list-group">
              ${matches.matches.map((m) => `
                <li class="list-group-item">vs ${m.opponent} —
                  <b>${m.result}</b> (${m.user_score}-${m.opponent_score})</li>`).join("") || "<li>No matches yet</li>"}
            </ul>
          </div>
        </div>`;
      document.getElementById("anon-btn").addEventListener("click", async () => {
        await API.post("/api/auth/gdpr/anonymize/", {}); load();
      });
      document.getElementById("del-btn").addEventListener("click", async () => {
        if (confirm("Delete account permanently?")) {
          await API.post("/api/auth/gdpr/delete/", {});
          API.clearTokens(); Router.go("auth");
        }
      });
    } catch (e) { console.error(e); }
  }
  return { load };
})();

// --------------------------------------------------------------------------- bootstrap
function boot() {
  applyI18n();
  Auth.init();
  Pong.init();
  Connect4.init();
  Tournament.init();
  Chat.init();

  // Game selector: switch between Pong and Connect-4 (module 7).
  const gameSelect = document.getElementById("game-select");
  function switchGame(name) {
    if (name === "connect4") {
      Pong.stop();
      document.getElementById("pong-canvas").classList.add("d-none");
      Connect4.show(true);
    } else {
      Connect4.stop();
      document.getElementById("connect4-canvas").classList.add("d-none");
      document.getElementById("pong-canvas").classList.remove("d-none");
      Pong.show ? Pong.show(false) : null;
    }
  }
  gameSelect.addEventListener("change", (e) => switchGame(e.target.value));
  document.getElementById("start-game").addEventListener("click", () => {
    const name = gameSelect.value;
    if (name === "connect4") Connect4.start();
    else Pong.start();
  });

  document.getElementById("lang-switcher").value = currentLang;
  document.getElementById("lang-switcher").addEventListener("change", (e) => {
    currentLang = e.target.value;
    localStorage.setItem("lang", currentLang);
    applyI18n();
  });
  document.getElementById("logout-btn").addEventListener("click", () => {
    API.clearTokens(); Router.go("auth");
  });
  Router.render();
}
document.addEventListener("DOMContentLoaded", boot);
