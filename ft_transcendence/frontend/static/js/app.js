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
    document.getElementById("start-game").addEventListener("click", start);
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
  Tournament.init();
  Chat.init();
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
