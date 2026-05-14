#include "input_common/web_controller/controller_html.h"

namespace InputCommon::WebController {

const char* CONTROLLER_HTML = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<meta name="theme-color" content="#111">
<link rel="manifest" href="/manifest.json">
<title>Lime3DS Controller</title>
<style>
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent;touch-action:none;user-select:none;-webkit-user-select:none;}
html{width:100%;height:100%;background:#111;overflow:hidden;}
body{width:100%;height:100vh;height:100svh;background:#111;overflow:hidden;font-family:-apple-system,sans-serif;}

/* ── Three-row scaffold ────────────────────────────────────── */
#ui{display:flex;flex-direction:column;width:100%;height:100%;position:relative;}

/* Shoulder row — L left, spacer, R right */
#row-top{
  display:flex;flex-direction:row;align-items:flex-start;
  padding:5px 6px 0;flex:0 0 auto;
}
#row-top-spacer{flex:1;}

/* Main row — D-pad+Circle | Screen | ABXY */
#row-main{
  display:flex;flex-direction:row;align-items:center;
  flex:1 1 0;min-height:0;
}
#col-left,#col-right{
  display:flex;flex-direction:column;
  align-items:center;justify-content:center;
  gap:12px;padding:0 4px;
  flex:0 0 27vw;min-width:0;
}
#col-center{
  flex:1 1 0;min-width:0;
  display:flex;flex-direction:column;
  align-items:center;justify-content:center;
  padding:0 2px;
}

/* SELECT/START row */
#row-bottom{
  display:flex;flex-direction:row;
  align-items:center;justify-content:center;
  padding:0 8px 6px;flex:0 0 auto;
}

/* ── Groups ─────────────────────────────────────────────────── */
.group{position:relative;transform-origin:0 0;will-change:transform;}
.group.edit-mode{outline:2px dashed #484848;outline-offset:6px;border-radius:8px;cursor:grab;}
.group.edit-mode:active{cursor:grabbing;}

/* ── Shoulder buttons ─────────────────────────────────────── */
.shoulder{
  width:88px;height:40px;
  background:#252525;color:#777;
  font-size:16px;font-weight:700;
  display:flex;align-items:center;justify-content:center;
  transition:background .08s;
}
.shoulder.pressed{background:#3a3a3a;color:#ccc;}
#btn-l{border-radius:16px 5px 8px 20px;}
#btn-r{border-radius:5px 16px 20px 8px;}

/* ── D-pad ──────────────────────────────────────────────────── */
#dpad{display:grid;grid-template-columns:44px 44px 44px;grid-template-rows:44px 44px 44px;gap:3px;}
.dc{background:transparent;pointer-events:none;}
.dd{background:#252525;border-radius:5px;display:flex;align-items:center;justify-content:center;color:#666;font-size:15px;transition:background .08s;}
.dd.pressed{background:#3a3a3a;color:#ccc;}
.dm{background:#252525;border-radius:3px;}

/* ── Circle pad ─────────────────────────────────────────────── */
#circle-outer{
  width:118px;height:118px;border-radius:50%;
  background:radial-gradient(circle at 42% 36%,#2c2c2c,#191919);
  border:3px solid #2e2e2e;position:relative;
  box-shadow:0 4px 18px #00000088,inset 0 1px 3px #3a3a3a44;
}
#circle-thumb{
  width:56px;height:56px;border-radius:50%;
  background:radial-gradient(circle at 38% 32%,#484848,#242424);
  border:2px solid #3a3a3a;
  position:absolute;top:50%;left:50%;transform:translate(-50%,-50%);
  pointer-events:none;box-shadow:0 3px 10px #00000077;
}
#circle-outer.active #circle-thumb{background:radial-gradient(circle at 38% 32%,#565656,#2c2c2c);}

/* ── ABXY face buttons ──────────────────────────────────────── */
.face{
  position:absolute;width:50px;height:50px;border-radius:50%;
  background:#252525;color:#777;
  font-size:13px;font-weight:700;
  display:flex;align-items:center;justify-content:center;
  box-shadow:0 3px 10px #00000055;transition:background .08s;
}
.face.pressed{background:#3a3a3a;color:#ccc;}

/* ── System buttons (SELECT/START) ─────────────────────────── */
.sys{
  width:52px;height:52px;border-radius:50%;
  background:#1e1e1e;color:#555;
  font-size:9px;font-weight:700;line-height:1.3;text-align:center;
  display:flex;align-items:center;justify-content:center;
  box-shadow:0 3px 8px #00000055;transition:background .08s;
}
.sys.pressed{background:#2e2e2e;color:#aaa;}

/* ── Bottom (touch) screen ──────────────────────────────────── */
#screen-wrap{
  position:relative;width:100%;max-width:320px;
  background:#0a0a12;border-radius:8px;
  border:2px solid #2a2a2a;overflow:hidden;
  box-shadow:0 4px 20px #00000099;
}
#bottom-screen{
  display:block;width:100%;height:auto;aspect-ratio:4/3;
  image-rendering:pixelated;image-rendering:crisp-edges;
  background:#0a0a12;
}
#screen-touch{position:absolute;inset:0;touch-action:none;}

/* ── Controls bar (EDIT / RESET) ────────────────────────────── */
#controls-bar{
  position:fixed;top:6px;left:50%;transform:translateX(-50%);
  display:flex;gap:8px;z-index:9999;
}
.ctrl-btn{
  padding:5px 18px;border-radius:16px;
  background:#161616;border:1.5px solid #2a2a2a;
  color:#383838;font-size:10px;font-weight:700;letter-spacing:.5px;
  cursor:pointer;transition:all .15s;touch-action:manipulation;
  -webkit-tap-highlight-color:transparent;
}
#edit-btn.active{background:#1e1e1e;border-color:#555;color:#aaa;}
#reset-btn{opacity:0;pointer-events:none;transition:opacity .2s;}
#reset-btn.visible{opacity:1;pointer-events:auto;}

#edit-banner{
  position:fixed;bottom:6px;left:50%;transform:translateX(-50%);
  color:#2a2a2a;font-size:10px;pointer-events:none;opacity:0;
  transition:opacity .25s;white-space:nowrap;
}
#edit-banner.visible{opacity:1;}

/* ── Portrait gate ─────────────────────────────────────────── */
#rotate-msg{
  display:none;position:fixed;inset:0;background:#111;z-index:99999;
  flex-direction:column;align-items:center;justify-content:center;gap:16px;
}
#rotate-msg svg{width:56px;height:56px;opacity:.3;}
#rotate-msg p{color:#444;font-size:14px;}
@media(orientation:portrait){
  #rotate-msg{display:flex;}
  #ui,#controls-bar,#edit-banner{visibility:hidden;}
}
</style>
</head>
<body>

<div id="rotate-msg">
  <svg viewBox="0 0 24 24" fill="#fff">
    <path d="M16.48 2.52c3.27 1.55 5.61 4.72 5.97 8.48h1.5C23.44 4.84 18.29 0 12 0l-.66.03 3.81 3.81 1.33-1.32zm-6.25-.77c-.59-.59-1.54-.59-2.12 0L1.75 8.11c-.59.59-.59 1.54 0 2.12l12.02 12.02c.59.59 1.54.59 2.12 0l6.36-6.36c.59-.59.59-1.54 0-2.12L10.23 1.75zm4.6 19.44L2.81 9.17l6.36-6.36 12.02 12.02-6.36 6.36zm-7.36.29C4.2 19.93 1.86 16.76 1.5 13H0c.56 6.16 5.71 11 12 11l.66-.03-3.81-3.81-1.38 1.32z"/>
  </svg>
  <p>Rotate to landscape</p>
</div>

<div id="ui">

  <!-- ══ Row top: L shoulder · spacer · R shoulder ══ -->
  <div id="row-top">
    <div class="group" id="grp-l">
      <div id="btn-l" class="shoulder" data-btn="l">L</div>
    </div>
    <div id="row-top-spacer"></div>
    <div class="group" id="grp-r">
      <div id="btn-r" class="shoulder" data-btn="r">R</div>
    </div>
  </div>

  <!-- ══ Row main: D-pad + Circle | Screen | ABXY ══ -->
  <div id="row-main">

    <div id="col-left">
      <div class="group" id="grp-dpad">
        <div id="dpad">
          <div class="dc"></div>
          <div class="dd" data-btn="up">&#9650;</div>
          <div class="dc"></div>
          <div class="dd" data-btn="left">&#9664;</div>
          <div class="dm"></div>
          <div class="dd" data-btn="right">&#9654;</div>
          <div class="dc"></div>
          <div class="dd" data-btn="down">&#9660;</div>
          <div class="dc"></div>
        </div>
      </div>
      <div class="group" id="grp-circle">
        <div id="circle-outer">
          <div id="circle-thumb"></div>
        </div>
      </div>
    </div>

    <div id="col-center">
      <div class="group" id="grp-screen">
        <div id="screen-wrap">
          <canvas id="bottom-screen" width="320" height="240"></canvas>
          <div id="screen-touch"></div>
        </div>
      </div>
    </div>

    <div id="col-right">
      <div class="group" id="grp-abxy">
        <div style="position:relative;width:140px;height:140px;">
          <div class="face" data-btn="x" style="top:0;left:45px;">X</div>
          <div class="face" data-btn="y" style="top:45px;left:0;">Y</div>
          <div class="face" data-btn="a" style="top:45px;left:90px;">A</div>
          <div class="face" data-btn="b" style="top:90px;left:45px;">B</div>
        </div>
      </div>
    </div>

  </div>

  <!-- ══ Row bottom: SELECT/START ══ -->
  <div id="row-bottom">
    <div class="group" id="grp-ss">
      <div style="display:flex;gap:16px;">
        <div class="sys" data-btn="select">SEL</div>
        <div class="sys" data-btn="start">START</div>
      </div>
    </div>
  </div>

</div><!-- #ui -->

<div id="controls-bar">
  <button id="edit-btn" class="ctrl-btn">EDIT</button>
  <button id="reset-btn" class="ctrl-btn">RESET</button>
</div>
<div id="edit-banner">drag to move &bull; pinch to resize</div>

<script>
(function(){
'use strict';

try{screen.orientation.lock('landscape').catch(()=>{});}catch(e){}

// ── Edit mode ──────────────────────────────────────────────────
let editMode=false;
const editBtn=document.getElementById('edit-btn');
const resetBtn=document.getElementById('reset-btn');
const editBanner=document.getElementById('edit-banner');
const allGroups=Array.from(document.querySelectorAll('.group'));

editBtn.addEventListener('pointerdown',e=>e.stopPropagation());
resetBtn.addEventListener('pointerdown',e=>e.stopPropagation());

editBtn.addEventListener('click',()=>{
  editMode=!editMode;
  editBtn.textContent=editMode?'DONE':'EDIT';
  editBtn.classList.toggle('active',editMode);
  editBanner.classList.toggle('visible',editMode);
  resetBtn.classList.toggle('visible',editMode);
  allGroups.forEach(g=>g.classList.toggle('edit-mode',editMode));
  if(editMode){
    document.querySelectorAll('.pressed').forEach(el=>{
      el.classList.remove('pressed');
      if(el.dataset.btn)send({button:el.dataset.btn,pressed:false});
    });
    if(circleActive!==null)releaseCircle();
    sendTouch(0.5,0.5,false);
  }
});

resetBtn.addEventListener('click',()=>{
  allGroups.forEach(g=>{
    localStorage.removeItem('wc5_'+g.id);
    g.style.position='';
    g.style.left='';
    g.style.top='';
    g.style.right='';
    g.style.transform='';
    scaleMap[g.id]=1;
  });
  localStorage.removeItem('wc5_vw');
  // Exit edit mode after reset
  editMode=false;
  editBtn.textContent='EDIT';
  editBtn.classList.remove('active');
  editBanner.classList.remove('visible');
  resetBtn.classList.remove('visible');
  allGroups.forEach(g=>g.classList.remove('edit-mode'));
});

// ── Input sending ──────────────────────────────────────────────
function send(payload){
  fetch('/input',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify(payload),keepalive:true}).catch(()=>{});
}

let pendingAnalog=null,rafPending=false;
function sendAnalog(x,y){
  pendingAnalog={x,y};
  if(!rafPending){
    rafPending=true;
    requestAnimationFrame(()=>{
      if(pendingAnalog)send({analog:pendingAnalog});
      pendingAnalog=null;rafPending=false;
    });
  }
}

function sendTouch(x,y,pressed){
  send({touch:{x,y,pressed}});
}

function haptic(ms){try{navigator.vibrate&&navigator.vibrate(ms);}catch(e){}}

// ── Button handling ────────────────────────────────────────────
document.querySelectorAll('[data-btn]').forEach(el=>{
  el.addEventListener('pointerdown',e=>{
    if(editMode)return;
    el.setPointerCapture(e.pointerId);
    el.classList.add('pressed');
    haptic(18);
    send({button:el.dataset.btn,pressed:true});
  });
  const release=()=>{
    if(!el.classList.contains('pressed'))return;
    el.classList.remove('pressed');
    send({button:el.dataset.btn,pressed:false});
  };
  el.addEventListener('pointerup',release);
  el.addEventListener('pointercancel',release);
});

// ── Circle pad ─────────────────────────────────────────────────
const outer=document.getElementById('circle-outer');
const thumb=document.getElementById('circle-thumb');
let circleActive=null;

function updateCircle(px,py){
  const r=outer.getBoundingClientRect();
  const cx=r.left+r.width/2,cy=r.top+r.height/2,radius=r.width/2;
  let dx=(px-cx)/radius,dy=-(py-cy)/radius;
  const len=Math.hypot(dx,dy);
  if(len>1){dx/=len;dy/=len;}
  const vis=radius*0.35;
  thumb.style.transform=`translate(calc(-50% + ${dx*vis}px),calc(-50% + ${-dy*vis}px))`;
  sendAnalog(parseFloat(dx.toFixed(4)),parseFloat(dy.toFixed(4)));
}
function releaseCircle(){
  circleActive=null;
  outer.classList.remove('active');
  thumb.style.transform='translate(-50%,-50%)';
  sendAnalog(0,0);
}
outer.addEventListener('pointerdown',e=>{
  if(editMode)return;
  outer.setPointerCapture(e.pointerId);
  circleActive=e.pointerId;
  outer.classList.add('active');
  updateCircle(e.clientX,e.clientY);
});
outer.addEventListener('pointermove',e=>{
  if(e.pointerId!==circleActive)return;
  updateCircle(e.clientX,e.clientY);
});
outer.addEventListener('pointerup',releaseCircle);
outer.addEventListener('pointercancel',releaseCircle);

// ── Screen canvas polling (~20fps, works on all browsers) ──────
(function(){
  const canvas=document.getElementById('bottom-screen');
  const ctx=canvas.getContext('2d');
  let pending=false;
  function next(){
    if(pending)return;
    pending=true;
    const img=new Image();
    img.onload=()=>{
      ctx.drawImage(img,0,0,canvas.width,canvas.height);
      pending=false;
      setTimeout(next,100);
    };
    img.onerror=()=>{ pending=false; setTimeout(next,500); };
    img.src='/screen?t='+Date.now();
  }
  next();
})();

// ── Screen touch forwarding ────────────────────────────────────
const screenTouch=document.getElementById('screen-touch');
let lastTouchMs=0;
function screenTouchCoords(e){
  const rect=screenTouch.getBoundingClientRect();
  return[Math.max(0,Math.min(1,(e.clientX-rect.left)/rect.width)),
         Math.max(0,Math.min(1,(e.clientY-rect.top)/rect.height))];
}
screenTouch.addEventListener('pointerdown',e=>{
  if(editMode)return;
  screenTouch.setPointerCapture(e.pointerId);
  const[x,y]=screenTouchCoords(e);
  haptic(12);
  lastTouchMs=Date.now();
  sendTouch(x,y,true);
});
screenTouch.addEventListener('pointermove',e=>{
  if(editMode||!e.buttons)return;
  const now=Date.now();
  if(now-lastTouchMs<33)return; // ~30fps max to reduce Wi-Fi load
  lastTouchMs=now;
  const[x,y]=screenTouchCoords(e);
  sendTouch(x,y,true);
});
const releaseScreenTouch=()=>sendTouch(0.5,0.5,false);
screenTouch.addEventListener('pointerup',releaseScreenTouch);
screenTouch.addEventListener('pointercancel',releaseScreenTouch);

// ── Group drag & pinch (edit mode only) ───────────────────────
// wc5_ prefix — positions are invalidated when viewport width changes
// so stale positions from a different device/orientation don't apply.
const LSKEY='wc5_';
const scaleMap={};

const savedVW=parseInt(localStorage.getItem(LSKEY+'vw')||'0');
const validPositions=savedVW>0&&Math.abs(savedVW-window.innerWidth)<=30;

function setupGroup(grp){
  const key=LSKEY+grp.id;
  try{
    const s=JSON.parse(localStorage.getItem(key)||'{}');
    if(validPositions&&s.left!=null){
      grp.style.position='absolute';
      grp.style.left=s.left;grp.style.top=s.top;grp.style.right='';
    }
    if(s.scale!=null){scaleMap[grp.id]=s.scale;grp.style.transform=`scale(${s.scale})`;}
  }catch(err){}

  const ptrs={};
  let dragAnchor=null,elAnchor=null,pinchStart=null,pinchBaseScale=1;

  function ptrCount(){return Object.keys(ptrs).length;}

  function resolveToAbsolute(){
    const uiRect=document.getElementById('ui').getBoundingClientRect();
    const rect=grp.getBoundingClientRect();
    grp.style.position='absolute';
    grp.style.left=(rect.left-uiRect.left)+'px';
    grp.style.top=(rect.top-uiRect.top)+'px';
    grp.style.right='';
    // No parent mutation needed: #ui has position:relative, so all absolute
    // groups are contained by it regardless of their nesting depth.
  }

  function save(){
    localStorage.setItem(LSKEY+'vw',String(window.innerWidth));
    localStorage.setItem(key,JSON.stringify({
      left:grp.style.left,top:grp.style.top,scale:scaleMap[grp.id]||1
    }));
  }

  grp.addEventListener('pointerdown',e=>{
    if(!editMode)return;
    grp.setPointerCapture(e.pointerId);
    ptrs[e.pointerId]={x:e.clientX,y:e.clientY};

    if(ptrCount()===1){
      resolveToAbsolute();
      elAnchor={x:parseFloat(grp.style.left)||0,y:parseFloat(grp.style.top)||0};
      dragAnchor={x:e.clientX,y:e.clientY};
      pinchStart=null;
    }else if(ptrCount()===2){
      resolveToAbsolute();
      elAnchor={x:parseFloat(grp.style.left)||0,y:parseFloat(grp.style.top)||0};
      dragAnchor=null;
      const pts=Object.values(ptrs);
      pinchStart=Math.hypot(pts[0].x-pts[1].x,pts[0].y-pts[1].y);
      pinchBaseScale=scaleMap[grp.id]||1;
    }
  });

  grp.addEventListener('pointermove',e=>{
    if(!ptrs[e.pointerId])return;
    ptrs[e.pointerId]={x:e.clientX,y:e.clientY};

    if(ptrCount()===1&&dragAnchor&&elAnchor){
      grp.style.left=(elAnchor.x+(e.clientX-dragAnchor.x))+'px';
      grp.style.top=(elAnchor.y+(e.clientY-dragAnchor.y))+'px';
    }else if(ptrCount()===2&&pinchStart!=null){
      const pts=Object.values(ptrs);
      const d=Math.hypot(pts[0].x-pts[1].x,pts[0].y-pts[1].y);
      const sc=Math.min(2.2,Math.max(0.3,pinchBaseScale*(d/pinchStart)));
      scaleMap[grp.id]=sc;
      grp.style.transform=`scale(${sc})`;
    }
  });

  const up=e=>{
    if(!ptrs[e.pointerId])return;
    delete ptrs[e.pointerId];
    const n=ptrCount();
    if(n===0){save();dragAnchor=null;pinchStart=null;}
    else if(n===1){
      const rem=Object.values(ptrs)[0];
      elAnchor={x:parseFloat(grp.style.left)||0,y:parseFloat(grp.style.top)||0};
      dragAnchor={x:rem.x,y:rem.y};
      pinchStart=null;
    }
  };
  grp.addEventListener('pointerup',up);
  grp.addEventListener('pointercancel',up);
  grp.addEventListener('lostpointercapture',e=>{delete ptrs[e.pointerId];});
}

allGroups.forEach(setupGroup);

document.addEventListener('contextmenu',e=>e.preventDefault());
document.addEventListener('dblclick',e=>e.preventDefault());
})();
</script>
</body>
</html>)HTML";

} // namespace InputCommon::WebController
