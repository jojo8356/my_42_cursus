"""Self-contained HTTP/WebSocket GUI client (stdlib only).

The browser talks only to this HTTP server over a relative ``/ws`` endpoint. The
GUI server opens the TCP TAP connection server-side, so the browser never reaches
``localhost`` and the page needs no CDN, package manager, or external network.
"""

from __future__ import annotations

import base64
import hashlib
import json
import socket
import threading
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from typing import Any, Optional, Tuple

import protocol
from logging_json import JsonLogger, NullLogger

DEFAULT_GUI_HOST = "0.0.0.0"
DEFAULT_GUI_PORT = 8080
DEFAULT_TAP_HOST = "127.0.0.1"
DEFAULT_TAP_PORT = 4242
WS_GUID = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
MAX_WS_MESSAGE = 65536


HTML = r'''<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#0d1216"><title>Arrakis · TAP</title>
<style>
:root{color-scheme:dark;--bg:#0b1115;--panel:#121b20;--panel2:#172329;--line:#25363c;--sand:#e6bd73;--muted:#91a3a4;--text:#e7eeee;--green:#8bbf8b;--red:#e87970;--blue:#7fc0c6;--mono:ui-monospace,SFMono-Regular,Menlo,monospace;--sans:Inter,ui-sans-serif,system-ui,-apple-system,"Segoe UI",sans-serif}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at 35% -20%,#243a36 0,transparent 42%),var(--bg);color:var(--text);font:14px var(--sans);min-height:100vh}button,input{font:inherit}button{cursor:pointer;color:inherit}header{height:68px;border-bottom:1px solid var(--line);display:flex;align-items:center;justify-content:space-between;padding:0 clamp(16px,4vw,56px);background:#0b1115dd;position:sticky;top:0;z-index:3;backdrop-filter:blur(16px)}.brand{display:flex;align-items:center;gap:12px}.mark{width:34px;height:34px;border:1px solid #856b3e;border-radius:50%;display:grid;place-items:center;color:var(--sand);font:700 17px Georgia;background:linear-gradient(145deg,#302519,#111816)}.brand b{font-size:14px;letter-spacing:.18em;text-transform:uppercase}.brand small{display:block;color:var(--muted);font:10px var(--mono);margin-top:4px;letter-spacing:.12em}.top-right{display:flex;gap:18px;align-items:center}.wire{font:11px var(--mono);color:var(--muted);display:flex;align-items:center;gap:8px}.dot{width:7px;height:7px;border-radius:50%;background:#647275}.dot.on{background:var(--green);box-shadow:0 0 12px #8bbf8b99}.online{font:11px var(--mono);color:var(--sand);border-left:1px solid var(--line);padding-left:18px}.wrap{max-width:1440px;margin:auto;padding:28px clamp(16px,4vw,56px) 42px}.heading{display:flex;justify-content:space-between;gap:16px;align-items:end;margin-bottom:20px}.eyebrow{font:10px var(--mono);letter-spacing:.2em;text-transform:uppercase;color:var(--sand)}h1{font:500 clamp(25px,4vw,38px) Georgia,serif;margin:7px 0 0;letter-spacing:.01em}.heading p{color:var(--muted);font-size:12px;margin:7px 0 0}.layout{display:grid;grid-template-columns:minmax(230px,285px) minmax(350px,1fr) minmax(270px,320px);gap:14px;align-items:stretch}.col{display:flex;flex-direction:column;gap:14px;min-width:0}.card{background:linear-gradient(155deg,#142026,#10171b);border:1px solid var(--line);border-radius:13px;overflow:hidden;box-shadow:0 14px 44px #0002}.cardhead{display:flex;justify-content:space-between;align-items:center;padding:14px 16px;border-bottom:1px solid var(--line)}.cardhead h2{font:10px var(--mono);letter-spacing:.17em;text-transform:uppercase;color:#b8c4c1;margin:0}.cardbody{padding:16px}.player-card{display:flex;align-items:center;gap:12px}.avatar{width:42px;height:42px;border-radius:50%;border:1px solid #675735;background:#25271d;color:var(--sand);display:grid;place-items:center;font:21px Georgia}.player-name{font:17px Georgia}.player-state{font:10px var(--mono);color:var(--muted);margin-top:5px}.tag{font:9px var(--mono);letter-spacing:.1em;text-transform:uppercase;color:var(--green);border:1px solid #416044;border-radius:99px;padding:4px 7px}.barrow{display:flex;justify-content:space-between;color:var(--muted);font:10px var(--mono);margin:17px 0 7px}.bar{height:6px;background:#273338;border-radius:99px;overflow:hidden}.bar span{height:100%;display:block;width:100%;background:linear-gradient(90deg,#b76d4b,#d7a05c);border-radius:99px;transition:width .3s}.bar.xp span{background:linear-gradient(90deg,#538a85,#94c8ac)}.mini-stats{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:13px}.statbox{border:1px solid var(--line);background:#0c1317;border-radius:8px;padding:9px 10px}.statbox small{display:block;font:9px var(--mono);color:var(--muted);letter-spacing:.12em;text-transform:uppercase}.statbox b{display:block;font:15px Georgia;margin-top:4px;color:#e0d3b6}.connect-row{display:flex;gap:7px;margin-top:14px}.connect-row input,.composer input,.cmdbar input{background:#0b1215;border:1px solid var(--line);color:var(--text);outline:none;border-radius:7px;padding:10px 11px;min-width:0}.connect-row input:focus,.composer input:focus,.cmdbar input:focus{border-color:#a18451}.connect-row input{flex:1;width:80px}.primary{background:var(--sand);color:#181812;border:0;border-radius:7px;padding:0 13px;font-weight:700;font-size:11px}.primary:hover{background:#f0cf8d}.primary:disabled{opacity:.5;cursor:not-allowed}.small-note{font:10px var(--mono);color:var(--muted);line-height:1.55;margin-top:10px}.room-card{min-height:310px}.room-hero{padding:25px 22px 19px;background:radial-gradient(ellipse at 75% 0%,#795f3433,transparent 50%);border-bottom:1px solid var(--line)}.room-code{font:10px var(--mono);color:var(--sand);letter-spacing:.1em}.room-name{font:clamp(25px,3vw,34px) Georgia,serif;margin:8px 0}.room-desc{color:#aab7b3;font:13px/1.65 Georgia,serif;max-width:650px;margin:0}.room-content{padding:17px 22px;display:grid;grid-template-columns:1fr 1fr;gap:18px}.section-label{font:9px var(--mono);letter-spacing:.16em;color:var(--muted);text-transform:uppercase;margin-bottom:9px}.pills{display:flex;gap:6px;flex-wrap:wrap}.pill{font:10px var(--mono);color:#d0d8d4;background:#1b292d;border:1px solid #2d4143;border-radius:5px;padding:5px 7px}.empty{font:11px var(--mono);color:#637478}.exit-grid{display:flex;gap:7px;flex-wrap:wrap}.exit{border:1px solid #52613f;background:#1d281f;color:#c8d4b3;border-radius:6px;padding:7px 10px;font:10px var(--mono);text-transform:uppercase}.exit:hover{border-color:var(--sand);color:var(--sand);transform:translateY(-1px)}.sleep{border-color:#45586a;background:#1b232d;color:#a8c4d1}.map-line{font:10px var(--mono);color:#718487;line-height:1.7;margin-top:9px}.chat-card{display:flex;flex-direction:column;min-height:330px;flex:1}.chat-log{height:245px;overflow:auto;padding:14px 15px;display:flex;flex-direction:column;gap:10px}.chat-line{font-size:11px;line-height:1.55;word-break:break-word;color:#ccd5d2}.chat-line .time{font:9px var(--mono);color:#66787a;margin-right:7px}.chat-line.event{color:#95a9a1}.chat-line.err{color:var(--red)}.chat-line.ok{color:#d5c18f}.composer{display:flex;gap:7px;padding:11px;border-top:1px solid var(--line);margin-top:auto}.composer input{flex:1;font-size:11px}.composer button{border:1px solid var(--line);background:#1c292d;border-radius:7px;padding:0 12px;color:var(--blue);font:10px var(--mono)}.composer button:hover{border-color:var(--blue)}.quick{display:flex;gap:6px;flex-wrap:wrap;padding:0 12px 12px}.quick button{border:1px solid var(--line);background:#151f23;color:#a9b8b3;border-radius:5px;padding:5px 8px;font:9px var(--mono)}.quick button:hover{color:var(--sand);border-color:#74623f}.list{display:flex;flex-direction:column;gap:7px}.list-item{display:flex;justify-content:space-between;gap:8px;align-items:center;border:1px solid var(--line);border-radius:7px;background:#0e1519;padding:9px 10px;text-align:left}.list-item:hover{border-color:#675735}.list-item b{font:11px var(--mono);color:#d5ded8}.list-item small{font:9px var(--mono);color:var(--muted)}.quest{border-left:2px solid #a18451;padding:2px 0 2px 10px;margin:0 0 12px}.quest:last-child{margin-bottom:0}.quest b{font:11px var(--mono);color:#d6c49c}.quest p{font:11px/1.5 var(--sans);color:var(--muted);margin:4px 0 0}.quest progress{width:100%;height:4px;margin-top:7px;accent-color:var(--sand)}.cmdbar{display:flex;gap:8px;margin-top:14px}.cmdbar input{flex:1;font:11px var(--mono)}.cmdbar button{border:1px solid #7c6944;background:#1e241c;border-radius:7px;padding:0 13px;color:var(--sand);font:10px var(--mono)}.footer{display:flex;justify-content:space-between;gap:10px;color:#56676a;font:9px var(--mono);margin-top:14px}.hidden{display:none!important}
@media(max-width:1100px){.layout{grid-template-columns:minmax(210px,270px) minmax(320px,1fr)}.right{grid-column:1/-1;display:grid;grid-template-columns:1fr 1fr;align-items:stretch}.right .chat-card{grid-column:1/-1;min-height:280px}.chat-log{height:190px}}
@media(max-width:690px){header{height:58px}.brand small{display:none}.wrap{padding-top:20px}.heading{align-items:start;flex-direction:column}.layout{grid-template-columns:1fr}.left,.right{grid-column:auto;display:flex}.room-content{grid-template-columns:1fr}.room-hero{padding:20px}.chat-card{min-height:290px}.chat-log{height:180px}.online{padding-left:9px}.top-right{gap:9px}}
</style>
</head>
<body>
<header><div class="brand"><div class="mark">A</div><div><b>The Answer Protocol</b><small>ARRAKIS · MULTIPLAYER FRONTIER</small></div></div><div class="top-right"><div class="wire"><i id="wireDot" class="dot"></i><span id="wireLabel">CONNECTING</span></div><div class="online">ONLINE <b id="online">—</b></div></div></header>
<main class="wrap"><div class="heading"><div><div class="eyebrow">Field console / 42TAP</div><h1>Across the deep desert.</h1><p>A shared world. Every command is a line on the wire.</p></div><div class="eyebrow" id="worldLabel">SHIELD WALL · ARRAKEEN</div></div>
<div class="layout">
<section class="col left">
<div class="card"><div class="cardhead"><h2>Operator</h2><span class="tag" id="playerStatus">OFFLINE</span></div><div class="cardbody"><div class="player-card"><div class="avatar" id="avatar">?</div><div><div class="player-name" id="playerName">Not connected</div><div class="player-state" id="playerMeta">Establish a TAP session</div></div></div><div class="barrow"><span>VITALITY</span><b id="hpLabel">— / —</b></div><div class="bar"><span id="hpBar"></span></div><div class="barrow"><span>FIELD READINESS</span><b id="readiness">—</b></div><div class="bar xp"><span id="xpBar" style="width:0"></span></div><div class="mini-stats"><div class="statbox"><small>Solari</small><b id="gold">0</b></div><div class="statbox"><small>Squad</small><b id="groupName">None</b></div></div><div id="connectPanel"><div class="connect-row"><input id="username" maxlength="20" placeholder="Choose a callsign" autocomplete="off"><button class="primary" id="connectBtn" disabled>ENTER</button></div><div class="small-note">Use 1–20 letters, digits, underscore or hyphen. The server will greet before authentication.</div></div></div></div>
<div class="card"><div class="cardhead"><h2>Carried</h2><span class="eyebrow" id="inventoryCount">0 items</span></div><div class="cardbody"><div class="list" id="inventory"><div class="empty">Inventory not loaded.</div></div></div></div>
<div class="card"><div class="cardhead"><h2>Active objectives</h2><span class="eyebrow">QUEST LOG</span></div><div class="cardbody" id="quests"><div class="empty">No active quests.</div></div></div>
</section>
<section class="col center">
<div class="card room-card"><div class="cardhead"><h2>Current location</h2><span class="eyebrow" id="roomId">WORLD / —</span></div><div class="room-hero"><div class="room-code" id="roomCode">COORDINATES UNAVAILABLE</div><div class="room-name" id="roomName">Arrakis</div><p class="room-desc" id="roomDesc">Connect to the frontier to see what lies beyond the shield wall.</p></div><div class="room-content"><div><div class="section-label">Present / players</div><div class="pills" id="players"><span class="empty">—</span></div></div><div><div class="section-label">Present / characters</div><div class="pills" id="npcs"><span class="empty">—</span></div></div><div><div class="section-label">On the ground</div><div class="pills" id="items"><span class="empty">—</span></div></div><div><div class="section-label">Exits / move</div><div class="exit-grid" id="exits"><span class="empty">—</span></div><div class="map-line" id="mapLine">Map awaits a room scan.</div></div></div></div>
<div class="card chat-card"><div class="cardhead"><h2>Live wire</h2><span class="eyebrow" id="chatScope">ROOM / GLOBAL FEED</span></div><div class="chat-log" id="feed" aria-live="polite"></div><div class="quick"><button data-cmd="LOOK">LOOK</button><button data-cmd="STATUS">STATUS</button><button data-cmd="WHO">WHO</button><button data-cmd="INVENTORY">INVENTORY</button><button data-cmd="QUESTS">QUESTS</button><button data-cmd="HELP">HELP</button></div><form class="composer" id="chatForm"><input id="chatInput" maxlength="700" placeholder="Room chat · say something…" autocomplete="off"><button>TRANSMIT</button></form></div>
<form class="cmdbar" id="commandForm"><input id="commandInput" maxlength="1024" placeholder="Raw command · e.g. TALK guard, TAKE thumper"><button>EXECUTE</button></form>
</section>
<section class="col right">
<div class="card"><div class="cardhead"><h2>Nearby intelligence</h2><span class="eyebrow">SCAN</span></div><div class="cardbody"><div class="list" id="nearby"><div class="empty">A room scan will reveal local targets.</div></div></div></div>
<div class="card"><div class="cardhead"><h2>Session</h2><span class="eyebrow">RFC 42TAP / 1</span></div><div class="cardbody"><div class="small-note" style="margin:0">One command per LF-terminated line. Events arrive asynchronously; replies and world updates share this live stream.</div><div class="list" style="margin-top:12px"><button class="list-item" data-cmd="CHAT GLOBAL Arrakis is listening."><b>Global broadcast</b><small>CHAT GLOBAL</small></button><button class="list-item" data-cmd="SLEEP"><b>Rest here</b><small>SAFE ROOM ONLY</small></button><button class="list-item" data-cmd="QUIT"><b>Disconnect</b><small>QUIT</small></button></div></div></div>
</section></div><div class="footer"><span>ARRAKIS IS A SHARED WORLD · EVENTS ARE LIVE</span><span>STDlib bridge · HTTP + WebSocket · TCP</span></div></main>
<script>
(()=>{
'use strict';
const $=id=>document.getElementById(id);let ws=null,connected=false,player='',pending=[],room=null,stats=null,inventory=[];
function addLine(text,kind=''){const node=document.createElement('div');node.className='chat-line '+kind;const time=document.createElement('span');time.className='time';time.textContent=new Date().toLocaleTimeString([],{hour:'2-digit',minute:'2-digit',second:'2-digit'});node.append(time,document.createTextNode(text));$('feed').append(node);$('feed').scrollTop=$('feed').scrollHeight;while($('feed').children.length>180)$('feed').firstChild.remove()}
function setWire(on,label){$('wireDot').classList.toggle('on',on);$('wireLabel').textContent=label}
function send(raw){if(!ws||ws.readyState!==WebSocket.OPEN){addLine('The TCP bridge is offline.','err');return false}raw=String(raw).trim();if(!raw)return false;ws.send(raw);pending.push(raw);if(pending.length>80)pending.shift();addLine('› '+raw,'ok');return true}
function requestRefresh(){if(!connected)return;send('LOOK');send('STATUS');send('INVENTORY');send('QUESTS')}
function renderRoom(data){room=data;$('roomName').textContent=data.name||'Unknown location';$('roomDesc').textContent=data.description||'';$('roomCode').textContent=(data.name||'LOCATION').toUpperCase();$('roomId').textContent=data.id||'WORLD / —';$('players').replaceChildren();(data.players||[]).forEach(n=>pill($('players'),n));if(!(data.players||[]).length)pill($('players'),'No one here','empty');$('npcs').replaceChildren();(data.npcs||[]).forEach(n=>pill($('npcs'),n.replace(/^npc\./,'')));if(!(data.npcs||[]).length)pill($('npcs'),'No characters','empty');$('items').replaceChildren();(data.items||[]).forEach(n=>pill($('items'),n.replace(/^item\./,'')));if(!(data.items||[]).length)pill($('items'),'Nothing on the ground','empty');const exits=$('exits');exits.replaceChildren();const directions=Object.keys(data.exits||{});directions.forEach(dir=>{const b=document.createElement('button');b.className='exit';b.textContent='↗ '+dir;b.title='Move '+dir;b.onclick=()=>send('MOVE '+dir);exits.append(b)});if(data.can_sleep){const b=document.createElement('button');b.className='exit sleep';b.textContent='☾ Rest';b.onclick=()=>send('SLEEP');exits.append(b)}if(!directions.length)pill(exits,'No exits','empty');$('mapLine').textContent=directions.length?'Connected paths: '+directions.map(d=>d+' → '+data.exits[d].replace(/^loc\./,'')).join('  ·  '):'No paths visible.';const nearby=$('nearby');nearby.replaceChildren();(data.npcs||[]).forEach(id=>addTarget(nearby,id,'TALK '+id));(data.items||[]).forEach(id=>addTarget(nearby,id,'TAKE '+id));if(!nearby.children.length)nearby.innerHTML='<div class="empty">Nothing in scanning range.</div>'}
function pill(parent,text,extra=''){const e=document.createElement('span');e.className='pill '+extra;e.textContent=text;parent.append(e)}
function addTarget(parent,label,cmd){const b=document.createElement('button');b.className='list-item';const title=document.createElement('b');title.textContent=label;const action=document.createElement('small');action.textContent=cmd.split(' ')[0];b.append(title,action);b.onclick=()=>send(cmd);parent.append(b)}
function renderStatus(d){const hp=Number(d.hp||0),max=Number(d.max_hp||100),ratio=max?Math.max(0,Math.min(100,hp/max*100)):0;$('hpLabel').textContent=hp+' / '+max;$('hpBar').style.width=ratio+'%';$('readiness').textContent=d.status||'alive';$('xpBar').style.width=ratio+'%';$('gold').textContent=String(d.gold??0);$('playerStatus').textContent=(d.status||'ALIVE').toUpperCase();$('playerStatus').style.color=d.status==='dead'?'var(--red)':'var(--green)';$('playerName').textContent=player||'Operator';$('avatar').textContent=(player||'?').slice(0,1).toUpperCase();$('playerMeta').textContent='HP '+hp+' / '+max+' · '+(d.status||'alive')}
function renderInventory(items){inventory=Array.isArray(items)?items:[];$('inventoryCount').textContent=inventory.length+' item'+(inventory.length===1?'':'s');const root=$('inventory');root.replaceChildren();if(!inventory.length){root.innerHTML='<div class="empty">No carried items.</div>';return}inventory.forEach(id=>{const b=document.createElement('button');b.className='list-item';const name=document.createElement('b');name.textContent=id.replace(/^item\./,'');const action=document.createElement('small');action.textContent='EXAMINE';b.append(name,action);b.onclick=()=>send('EXAMINE '+id);root.append(b)})}
function renderQuests(items){const root=$('quests');root.replaceChildren();const active=(Array.isArray(items)?items:[]).filter(q=>q.status==='active');if(!active.length){root.innerHTML='<div class="empty">No active quests.</div>';return}active.forEach(q=>{const box=document.createElement('div');box.className='quest';const b=document.createElement('b');b.textContent=(q.quest_id||'QUEST').replace(/^quest\./,'').replaceAll('_',' ').toUpperCase();const p=document.createElement('p');p.textContent='Progress '+(q.progress||'0/1');box.append(b,p);root.append(box)})}
function handle(line){const msg=line.startsWith('EVT ')||line.startsWith('(')?null:line.startsWith('OK')||line.startsWith('ERR')?pending.shift():null;const kind=line.startsWith('ERR ')?'err':line.startsWith('EVT ')||line.startsWith('(')?'event':line.startsWith('OK')?'ok':'';addLine(line,kind);if(line.startsWith('EVT STATS ')){const m=line.match(/players=(\d+)/);if(m)$('online').textContent=m[1];return}if(line.startsWith('EVT ROOM PRESENCE ')||line.startsWith('EVT ROOM ITEM ')||line.startsWith('EVT ROOM NPC ')||line.startsWith('EVT ROOM COMBAT ')){if(connected)setTimeout(()=>send('LOOK'),40);return}if(line.startsWith('EVT GROUP ')){if(line.includes('JOIN')||line.includes('LEAVE')||line.includes('DISBAND')||line.includes('KICK'))send('GROUP INFO');return}if(line.startsWith('('))return;if(line.startsWith('OK hello')){setWire(true,'TAP WIRE OPEN');return}if(line.startsWith('ERR ')){if(msg&&msg.split(' ')[0].toUpperCase()==='CONNECT')connected=false;return}if(!line.startsWith('OK'))return;const payload=line.slice(2).trim();if(!payload)return;if(msg&&msg.split(/\s+/,1)[0].toUpperCase()==='CONNECT'){connected=true;$('connectPanel').classList.add('hidden');requestRefresh();return}let parsed;try{parsed=JSON.parse(payload)}catch(_){if(msg){const verb=msg.split(/\s+/,1)[0].toUpperCase();if(verb==='MOVE'){setTimeout(requestRefresh,50)}if(verb==='QUIT'){connected=false;$('connectPanel').classList.remove('hidden');setWire(false,'DISCONNECTED')}}return}if(parsed&&typeof parsed==='object'&&!Array.isArray(parsed)){if(parsed.id&&parsed.exits)renderRoom(parsed);else if(Object.prototype.hasOwnProperty.call(parsed,'max_hp'))renderStatus(parsed);else if(parsed.hp&&parsed.max_hp)renderStatus(parsed);else if(parsed.npc&&parsed.dialogue)addLine(parsed.npc+': '+parsed.dialogue,'event');else if(parsed.quest_id&&parsed.status==='completed'){addLine('Quest complete · reward '+(parsed.reward||'none'),'ok');setTimeout(()=>send('QUESTS'),40)}else if(parsed.fled&&parsed.room){setTimeout(requestRefresh,40)}else if(parsed.players&&parsed.leader){$('groupName').textContent=parsed.id||'Squad'}}else if(Array.isArray(parsed)&&msg){const verb=msg.split(/\s+/,1)[0].toUpperCase();if(verb==='INVENTORY')renderInventory(parsed);if(verb==='QUESTS')renderQuests(parsed)}if(msg){const verb=msg.split(/\s+/,1)[0].toUpperCase();if(['TAKE','DROP','SLEEP','ATTACK','DEFEND','FLEE'].includes(verb))setTimeout(()=>{send('STATUS');send('INVENTORY');send('QUESTS')},50)}}
function openWire(){const proto=location.protocol==='https:'?'wss:':'ws:';ws=new WebSocket(proto+'//'+location.host+'/ws');setWire(false,'CONNECTING');ws.onopen=()=>{setWire(true,'WEBSOCKET OPEN');$('connectBtn').disabled=false;addLine('GUI bridge open. Awaiting TAP greeting.','event')};ws.onmessage=e=>handle(String(e.data));ws.onerror=()=>{setWire(false,'WIRE ERROR');addLine('WebSocket connection failed. Check the GUI server.','err')};ws.onclose=()=>{setWire(false,'DISCONNECTED');connected=false;$('connectPanel').classList.remove('hidden');$('connectBtn').disabled=true;addLine('Session closed. Refresh to reconnect.','event')}}
$('connectBtn').onclick=()=>{const n=$('username').value.trim();if(!/^[A-Za-z0-9_-]{1,20}$/.test(n)){addLine('Callsign must be 1–20 letters, digits, underscore or hyphen.','err');return}player=n;send('CONNECT '+n)};$('username').addEventListener('keydown',e=>{if(e.key==='Enter')$('connectBtn').click()});$('chatForm').onsubmit=e=>{e.preventDefault();const text=$('chatInput').value.trim();if(text){send('CHAT ROOM '+text);$('chatInput').value=''}};$('commandForm').onsubmit=e=>{e.preventDefault();const raw=$('commandInput').value;$('commandInput').value='';send(raw)};document.querySelectorAll('[data-cmd]').forEach(b=>b.addEventListener('click',()=>send(b.dataset.cmd)));openWire();
})();
</script></body></html>'''


def websocket_frame(payload: bytes, opcode: int = 1) -> bytes:
    """Encode a final, unmasked server-to-client WebSocket frame."""
    first = 0x80 | (opcode & 0x0F)
    length = len(payload)
    if length < 126:
        return bytes((first, length)) + payload
    if length <= 0xFFFF:
        return bytes((first, 126)) + length.to_bytes(2, "big") + payload
    return bytes((first, 127)) + length.to_bytes(8, "big") + payload


def _write_ws_frame(stream, lock, stop, data: bytes, opcode: int = 1) -> bool:
    """Write one bounded WebSocket frame unless its connection has stopped."""
    if len(data) > MAX_WS_MESSAGE:
        data = data[:MAX_WS_MESSAGE]
    with lock:
        if stop.is_set():
            return False
        stream.write(websocket_frame(data, opcode))
        stream.flush()
    return True


def _try_write_ws_frame(stream, lock, stop, data: bytes, opcode: int = 1) -> bool:
    """Best-effort write used for connection-failure and close responses."""
    try:
        return _write_ws_frame(stream, lock, stop, data, opcode)
    except OSError:
        return False


def _close_payload(code: int, reason: str = "") -> bytes:
    """Build an RFC 6455 close payload with at most 123 UTF-8 reason bytes."""
    encoded = reason.encode("utf-8")[:123]
    while True:
        try:
            encoded.decode("utf-8")
            break
        except UnicodeDecodeError:
            encoded = encoded[:-1]
    return code.to_bytes(2, "big") + encoded


def _valid_close_code(code: int) -> bool:
    """Whether ``code`` is a status code a peer may place on the wire."""
    return (1000 <= code < 5000 and code not in (1004, 1005, 1006, 1015)
            and not 1016 <= code < 3000)


def _read_exact(stream, size: int) -> bytes:
    """Read exactly ``size`` bytes from a buffered HTTP input stream."""
    parts = bytearray()
    while len(parts) < size:
        chunk = stream.read(size - len(parts))
        if not chunk:
            return b""
        parts.extend(chunk)
    return bytes(parts)


class GuiHTTPServer(ThreadingHTTPServer):
    """HTTP server that carries the TAP TCP connection behind each WebSocket."""

    daemon_threads = True
    allow_reuse_address = True

    def __init__(self, address: Tuple[str, int], tap_host: str, tap_port: int,
                 logger: Any) -> None:
        super().__init__(address, GuiRequestHandler)
        self.tap_host = tap_host
        self.tap_port = tap_port
        self.logger = logger


class GuiRequestHandler(BaseHTTPRequestHandler):
    """Serve the single-page UI and implement RFC 6455 text frames."""

    server: GuiHTTPServer
    protocol_version = "HTTP/1.1"

    def log_message(self, format: str, *args: Any) -> None:
        self.server.logger.log("INFO", "gui http", type="gui_http",
                               ip=self.client_address[0], request=format % args)

    def do_GET(self) -> None:  # noqa: N802 - stdlib HTTP handler API
        if self.path in ("/", "/index.html"):
            body = HTML.encode("utf-8")
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Security-Policy",
                             "default-src 'self'; style-src 'unsafe-inline'; "
                             "script-src 'unsafe-inline'; connect-src 'self' ws: wss:")
            self.end_headers()
            self.wfile.write(body)
            return
        if self.path == "/health":
            body = json.dumps({"status": "ok", "service": "tap-gui"}).encode("ascii")
            self.send_response(200)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
            return
        if self.path == "/ws":
            self._upgrade_websocket()
            return
        self.send_error(404, "not found")

    def _upgrade_websocket(self) -> None:
        self.close_connection = True
        key = self.headers.get("Sec-WebSocket-Key", "")
        version = self.headers.get("Sec-WebSocket-Version", "")
        upgrade = self.headers.get("Upgrade", "").lower()
        connection_tokens = {
            token.strip().lower() for token in self.headers.get("Connection", "").split(",")
        }
        try:
            decoded_key = base64.b64decode(key, validate=True)
        except (ValueError, TypeError):
            decoded_key = b""
        if (upgrade != "websocket" or "upgrade" not in connection_tokens or version != "13"
                or len(decoded_key) != 16):
            self.send_error(400, "invalid WebSocket handshake")
            return

        accept = base64.b64encode(hashlib.sha1((key + WS_GUID).encode("ascii")).digest())
        self.send_response_only(101, "Switching Protocols")
        self.send_header("Upgrade", "websocket")
        self.send_header("Connection", "Upgrade")
        self.send_header("Sec-WebSocket-Accept", accept.decode("ascii"))
        self.end_headers()
        self.wfile.flush()
        self._websocket_session()

    def _websocket_session(self) -> None:
        logger = self.server.logger
        stop = threading.Event()
        write_lock = threading.Lock()
        backend: Optional[socket.socket] = None

        def ws_send_frame(data: bytes, opcode: int = 1) -> None:
            _write_ws_frame(self.wfile, write_lock, stop, data, opcode)

        def ws_send(text: str, opcode: int = 1) -> None:
            ws_send_frame(text.encode("utf-8"), opcode)

        def ws_close(code: int, reason: str) -> None:
            ws_send_frame(_close_payload(code, reason), opcode=8)

        try:
            backend = socket.create_connection(
                (self.server.tap_host, self.server.tap_port), timeout=5.0
            )
            backend.settimeout(0.2)
        except OSError as exc:
            logger.log("ERROR", "GUI cannot reach TAP server", type="gui_bridge",
                       ip=self.client_address[0], error=str(exc))
            _try_write_ws_frame(
                self.wfile, write_lock, stop,
                protocol.err("CONNECTION_FAILED").encode("utf-8"),
            )
            return
        assert backend is not None

        def relay_backend() -> None:
            pending = bytearray()
            assert backend is not None
            while not stop.is_set():
                try:
                    chunk = backend.recv(4096)
                except socket.timeout:
                    continue
                except OSError:
                    break
                if not chunk:
                    break
                pending.extend(chunk)
                while b"\n" in pending:
                    raw, _, remainder = pending.partition(b"\n")
                    pending[:] = remainder
                    try:
                        ws_send(raw.decode("utf-8"))
                    except (UnicodeDecodeError, OSError, ValueError):
                        stop.set()
                        return
                if len(pending) > protocol.MAX_LINE_BYTES * 4:
                    pending.clear()
            stop.set()

        reader = threading.Thread(target=relay_backend, name="tap-gui-tcp-reader", daemon=True)
        reader.start()
        fragments = bytearray()
        fragmented_opcode = 0
        try:
            while not stop.is_set():
                header = _read_exact(self.rfile, 2)
                if not header:
                    break
                first, second = header
                if first & 0x70:  # extensions were not negotiated
                    ws_close(1002, "unsupported WebSocket extension")
                    break
                final = bool(first & 0x80)
                opcode = first & 0x0F
                masked = bool(second & 0x80)
                length = second & 0x7F
                if not masked:
                    ws_close(1002, "client frames must be masked")
                    break
                if length == 126:
                    data = _read_exact(self.rfile, 2)
                    if not data:
                        break
                    length = int.from_bytes(data, "big")
                    if length < 126:
                        ws_close(1002, "non-canonical frame length")
                        break
                elif length == 127:
                    data = _read_exact(self.rfile, 8)
                    if not data:
                        break
                    length = int.from_bytes(data, "big")
                    if length >> 63:
                        ws_close(1002, "invalid frame length")
                        break
                    if length <= 0xFFFF:
                        ws_close(1002, "non-canonical frame length")
                        break
                if length > MAX_WS_MESSAGE:
                    ws_close(1009, "message too large")
                    break
                mask = _read_exact(self.rfile, 4)
                payload = _read_exact(self.rfile, length)
                if len(mask) != 4 or len(payload) != length:
                    break
                payload = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
                if opcode >= 8 and (not final or length > 125):
                    ws_close(1002, "invalid control frame")
                    break

                if opcode == 8:  # close
                    if len(payload) == 1:
                        ws_close(1002, "invalid close payload")
                    elif len(payload) >= 2:
                        code = int.from_bytes(payload[:2], "big")
                        if not _valid_close_code(code):
                            ws_close(1002, "invalid close status")
                        else:
                            try:
                                payload[2:].decode("utf-8")
                            except UnicodeDecodeError:
                                ws_close(1007, "invalid close reason")
                            else:
                                _try_write_ws_frame(self.wfile, write_lock, stop, payload,
                                                    opcode=8)
                    else:
                        _try_write_ws_frame(self.wfile, write_lock, stop, b"", opcode=8)
                    break
                if opcode == 9:  # ping; control payload is opaque bytes
                    ws_send_frame(payload, opcode=10)
                    continue
                if opcode == 10:  # pong
                    continue
                if opcode == 0:  # continuation
                    if not fragmented_opcode:
                        ws_close(1002, "unexpected continuation")
                        break
                    fragments.extend(payload)
                    if len(fragments) > MAX_WS_MESSAGE:
                        ws_close(1009, "message too large")
                        break
                    if final:
                        opcode = fragmented_opcode
                        fragmented_opcode = 0
                        payload = bytes(fragments)
                        fragments.clear()
                    else:
                        continue
                elif opcode in (1, 2):
                    if fragmented_opcode:
                        ws_close(1002, "nested fragmented message")
                        break
                    if not final:
                        fragmented_opcode = opcode
                        fragments[:] = payload
                        continue
                else:
                    ws_close(1002, "unsupported opcode")
                    break

                if opcode != 1:
                    ws_close(1003, "text frames only")
                    break
                try:
                    command = payload.decode("utf-8")
                except UnicodeDecodeError:
                    ws_close(1007, "invalid UTF-8 text")
                    break
                if "\n" in command or "\r" in command:
                    ws_send(protocol.err("BAD_REQUEST"))
                    continue
                if len(command.encode("utf-8")) > protocol.MAX_LINE_BYTES:
                    ws_send(protocol.err("LINE_TOO_LONG"))
                    continue
                if not command.strip():
                    continue
                try:
                    assert backend is not None
                    backend.sendall(protocol.frame(command))
                except OSError:
                    ws_send(protocol.err("SEND_FAILED"))
                    break
        except (OSError, ValueError, TimeoutError) as exc:
            logger.log("INFO", "GUI WebSocket closed", type="gui_bridge",
                       ip=self.client_address[0], reason=str(exc))
        finally:
            stop.set()
            try:
                backend.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            try:
                backend.close()
            except OSError:
                pass
            if reader.is_alive():
                reader.join(timeout=0.5)
            logger.log("INFO", "GUI session ended", type="gui_bridge",
                       ip=self.client_address[0])


def create_gui_server(host: str = DEFAULT_GUI_HOST, port: int = DEFAULT_GUI_PORT,
                      tap_host: str = DEFAULT_TAP_HOST, tap_port: int = DEFAULT_TAP_PORT,
                      logger: Optional[Any] = None) -> GuiHTTPServer:
    """Create (but do not start) the HTTP/WebSocket GUI bridge."""
    json_logger = logger if logger is not None else NullLogger()
    return GuiHTTPServer((host, port), tap_host, tap_port, json_logger)


def serve_gui(host: str = DEFAULT_GUI_HOST, port: int = DEFAULT_GUI_PORT,
              tap_host: str = DEFAULT_TAP_HOST, tap_port: int = DEFAULT_TAP_PORT,
              logger: Optional[Any] = None) -> None:
    """Serve the single-page client until SIGINT/SIGTERM."""
    json_logger = logger if logger is not None else JsonLogger()
    httpd = create_gui_server(host, port, tap_host, tap_port, json_logger)
    actual_port = int(httpd.server_address[1])
    print(f"TAP GUI listening on http://{host}:{actual_port} (TAP {tap_host}:{tap_port})",
          flush=True)
    try:
        httpd.serve_forever(poll_interval=0.2)
    except KeyboardInterrupt:
        pass
    finally:
        httpd.shutdown()
        httpd.server_close()
