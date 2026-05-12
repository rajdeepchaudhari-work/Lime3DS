#include "input_common/web_controller/controller_html.h"

namespace InputCommon::WebController {

const char* CONTROLLER_HTML = R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
<title>Lime3DS Controller</title>
<style>
*{box-sizing:border-box;margin:0;padding:0;-webkit-tap-highlight-color:transparent;touch-action:none;user-select:none;-webkit-user-select:none;}
html,body{width:100%;height:100%;background:#111;overflow:hidden;font-family:-apple-system,sans-serif;}
#controller{position:relative;width:100vw;height:100dvh;}

/* ── Groups: transform-origin top-left so scale doesn't shift position ── */
.group{
  position:absolute;
  transform-origin:0 0;
  will-change:transform;
}

/* ── Edit mode visuals ─────────────────────────────────────── */
.group.edit-mode{outline:2px dashed #555;outline-offset:10px;border-radius:8px;cursor:grab;}
.group.edit-mode:active{cursor:grabbing;}
.group.edit-mode::before{
  content:'✥';position:absolute;top:-26px;left:50%;transform:translateX(-50%);
  color:#666;font-size:18px;pointer-events:none;
}

/* ── Shoulder buttons ──────────────────────────────────────── */
.shoulder{
  width:108px;height:46px;
  background:#252525;color:#777;
  font-size:18px;font-weight:700;
  display:flex;align-items:center;justify-content:center;
  transition:background .08s;
}
.shoulder.pressed{background:#3a3a3a;color:#bbb;}
#btn-l{border-radius:18px 6px 12px 26px;}
#btn-r{border-radius:6px 18px 26px 12px;}

/* ── D-pad cross ───────────────────────────────────────────── */
#dpad{display:grid;grid-template-columns:46px 46px 46px;grid-template-rows:46px 46px 46px;gap:3px;}
.dc{background:transparent;pointer-events:none;}
.dd{
  background:#252525;border-radius:5px;
  display:flex;align-items:center;justify-content:center;
  color:#666;font-size:15px;
  transition:background .08s;
}
.dd.pressed{background:#3a3a3a;color:#bbb;}
.dm{background:#252525;border-radius:3px;}

/* ── Circle pad ────────────────────────────────────────────── */
#circle-outer{
  width:162px;height:162px;border-radius:50%;
  background:radial-gradient(circle at 42% 36%,#2c2c2c,#191919);
  border:3px solid #2e2e2e;
  position:relative;
  box-shadow:0 6px 28px #00000099,inset 0 1px 3px #3a3a3a44;
}
#circle-thumb{
  width:82px;height:82px;border-radius:50%;
  background:radial-gradient(circle at 38% 32%,#484848,#242424);
  border:2px solid #3a3a3a;
  position:absolute;top:50%;left:50%;
  transform:translate(-50%,-50%);
  pointer-events:none;
  box-shadow:0 4px 12px #00000077,inset 0 1px 2px #55555533;
  /* No transition on transform — must track finger exactly */
}
#circle-outer.active #circle-thumb{background:radial-gradient(circle at 38% 32%,#565656,#2c2c2c);}

/* ── ABXY face buttons ─────────────────────────────────────── */
.face{
  position:absolute;width:56px;height:56px;border-radius:50%;
  background:#252525;color:#777;
  font-size:15px;font-weight:700;
  display:flex;align-items:center;justify-content:center;
  box-shadow:0 4px 14px #00000055;
  transition:background .08s;
}
.face.pressed{background:#3a3a3a;color:#ccc;}

/* ── System buttons ────────────────────────────────────────── */
.sys{
  width:58px;height:58px;border-radius:50%;
  background:#1e1e1e;color:#555;
  font-size:10px;font-weight:700;
  display:flex;align-items:center;justify-content:center;text-align:center;line-height:1.3;
  box-shadow:0 3px 10px #00000055;
  transition:background .08s;
}
.sys.pressed{background:#2e2e2e;color:#aaa;}

/* ── Edit button ───────────────────────────────────────────── */
#edit-btn{
  position:fixed;top:10px;left:50%;transform:translateX(-50%);
  padding:8px 28px;border-radius:20px;
  background:#1a1a1a;border:1.5px solid #333;
  color:#484848;font-size:12px;font-weight:700;letter-spacing:.6px;
  cursor:pointer;z-index:9999;
  transition:background .15s,color .15s,border-color .15s;
}
#edit-btn.active{background:#222;border-color:#666;color:#bbb;box-shadow:0 0 12px #33333366;}

#edit-banner{
  position:fixed;bottom:10px;left:50%;transform:translateX(-50%);
  color:#333;font-size:11px;pointer-events:none;opacity:0;
  transition:opacity .25s;white-space:nowrap;
}
#edit-banner.visible{opacity:1;}

/* ── Landscape gate ────────────────────────────────────────── */
#rotate-msg{
  display:none;
  position:fixed;inset:0;background:#111;z-index:99999;
  flex-direction:column;align-items:center;justify-content:center;gap:18px;
}
#rotate-msg svg{width:64px;height:64px;opacity:.35;}
#rotate-msg p{color:#444;font-size:15px;letter-spacing:.3px;}
@media (orientation:portrait){
  #rotate-msg{display:flex;}
  #controller,#edit-btn,#edit-banner{visibility:hidden;}
}
</style>
</head>
<body>

<!-- Rotate notice (portrait mode) -->
<div id="rotate-msg">
  <svg viewBox="0 0 24 24" fill="#fff">
    <path d="M16.48 2.52c3.27 1.55 5.61 4.72 5.97 8.48h1.5C23.44 4.84 18.29 0 12 0l-.66.03 3.81 3.81 1.33-1.32zm-6.25-.77c-.59-.59-1.54-.59-2.12 0L1.75 8.11c-.59.59-.59 1.54 0 2.12l12.02 12.02c.59.59 1.54.59 2.12 0l6.36-6.36c.59-.59.59-1.54 0-2.12L10.23 1.75zm4.6 19.44L2.81 9.17l6.36-6.36 12.02 12.02-6.36 6.36zm-7.36.29C4.2 19.93 1.86 16.76 1.5 13H0c.56 6.16 5.71 11 12 11l.66-.03-3.81-3.81-1.38 1.32z"/>
  </svg>
  <p>Rotate to landscape to use the controller</p>
</div>

<div id="controller">

  <!-- L shoulder -->
  <div class="group" id="grp-l" style="left:2vw;top:6vh;">
    <div id="btn-l" class="shoulder" data-btn="l">L</div>
  </div>

  <!-- R shoulder -->
  <div class="group" id="grp-r" style="right:2vw;top:6vh;">
    <div id="btn-r" class="shoulder" data-btn="r">R</div>
  </div>

  <!-- D-pad -->
  <div class="group" id="grp-dpad" style="left:3vw;top:36vh;">
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

  <!-- Circle pad -->
  <div class="group" id="grp-circle" style="left:19vw;top:50vh;">
    <div id="circle-outer">
      <div id="circle-thumb"></div>
    </div>
  </div>

  <!-- ABXY -->
  <div class="group" id="grp-abxy" style="right:3vw;top:36vh;">
    <div style="position:relative;width:150px;height:150px;">
      <div class="face" data-btn="x" style="top:0;left:47px;">X</div>
      <div class="face" data-btn="y" style="top:47px;left:0;">Y</div>
      <div class="face" data-btn="a" style="top:47px;left:94px;">A</div>
      <div class="face" data-btn="b" style="top:94px;left:47px;">B</div>
    </div>
  </div>

  <!-- Select / Start -->
  <div class="group" id="grp-ss" style="left:calc(50% - 67px);top:72vh;">
    <div style="display:flex;gap:18px;">
      <div class="sys" data-btn="select">SEL</div>
      <div class="sys" data-btn="start">START</div>
    </div>
  </div>

</div>

<button id="edit-btn">EDIT</button>
<div id="edit-banner">drag to move &bull; pinch to resize</div>

<script>
(function(){
'use strict';

// ── Orientation lock ───────────────────────────────────────────
try{screen.orientation.lock('landscape').catch(()=>{});}catch(e){}

// ── Edit mode ──────────────────────────────────────────────────
let editMode=false;
const editBtn=document.getElementById('edit-btn');
const editBanner=document.getElementById('edit-banner');
const allGroups=Array.from(document.querySelectorAll('.group'));

editBtn.addEventListener('pointerdown',e=>{e.stopPropagation();});
editBtn.addEventListener('click',()=>{
  editMode=!editMode;
  editBtn.textContent=editMode?'DONE':'EDIT';
  editBtn.classList.toggle('active',editMode);
  editBanner.classList.toggle('visible',editMode);
  allGroups.forEach(g=>g.classList.toggle('edit-mode',editMode));
  // Release all pressed buttons when entering edit mode
  if(editMode) document.querySelectorAll('.pressed').forEach(el=>{
    el.classList.remove('pressed');
    if(el.dataset.btn) send({button:el.dataset.btn,pressed:false});
  });
  if(editMode&&circleActive!==null) releaseCircle();
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

// ── Button handling ────────────────────────────────────────────
// Note: stopPropagation is intentionally NOT called here so that
// in edit mode the event bubbles up to the group drag handler.
document.querySelectorAll('[data-btn]').forEach(el=>{
  el.addEventListener('pointerdown',e=>{
    if(editMode)return;          // let event bubble to group for dragging
    el.setPointerCapture(e.pointerId);
    el.classList.add('pressed');
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
  const vis=radius*0.36;
  thumb.style.transform=`translate(calc(-50% + ${dx*vis}px),calc(-50% + ${-dy*vis}px))`;
  sendAnalog(parseFloat(dx.toFixed(4)),parseFloat(dy.toFixed(4)));
}
function releaseCircle(){
  circleActive=null;
  outer.classList.remove('active');
  thumb.style.transform='translate(-50%,-50%)';
  sendAnalog(0,0);
}
// Note: no stopPropagation — bubbles to group for drag in edit mode
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

// ── Group drag & pinch — works for ALL groups in edit mode ─────
// Because transform-origin is 0 0, visual top-left === CSS left/top.
// This means getBoundingClientRect().left/top matches grp.style.left/top
// at any scale, so no position jumps occur when switching between
// drag and scale interactions.
const scaleMap={};

function setupGroup(grp){
  const key='wc3_'+grp.id;
  try{
    const s=JSON.parse(localStorage.getItem(key)||'{}');
    if(s.left!=null){grp.style.left=s.left;grp.style.right='';}
    else if(s.right!=null){grp.style.right=s.right;grp.style.left='';}
    if(s.top!=null)grp.style.top=s.top;
    if(s.scale!=null){
      scaleMap[grp.id]=s.scale;
      grp.style.transform=`scale(${s.scale})`;
    }
  }catch(err){}

  const ptrs={};
  let dragAnchor=null,elAnchor=null,pinchStart=null,pinchBaseScale=1;

  function ptrCount(){return Object.keys(ptrs).length;}

  function resolveToLeft(){
    const rect=grp.getBoundingClientRect();
    grp.style.left=rect.left+'px';
    grp.style.right='';
    grp.style.top=rect.top+'px';
  }

  function save(){
    localStorage.setItem(key,JSON.stringify({
      left:grp.style.left,right:grp.style.right,
      top:grp.style.top,scale:scaleMap[grp.id]||1
    }));
  }

  grp.addEventListener('pointerdown',e=>{
    if(!editMode)return;
    // Capture ALL touches in edit mode — buttons, joystick, everything.
    // Button/circle handlers already bail early when editMode===true,
    // so they don't call setPointerCapture; this handler takes ownership.
    grp.setPointerCapture(e.pointerId);
    ptrs[e.pointerId]={x:e.clientX,y:e.clientY};

    if(ptrCount()===1){
      resolveToLeft();
      elAnchor={x:parseFloat(grp.style.left),y:parseFloat(grp.style.top)};
      dragAnchor={x:e.clientX,y:e.clientY};
      pinchStart=null;
    } else if(ptrCount()===2){
      resolveToLeft();
      elAnchor={x:parseFloat(grp.style.left),y:parseFloat(grp.style.top)};
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
    } else if(ptrCount()===2&&pinchStart!=null){
      const pts=Object.values(ptrs);
      const d=Math.hypot(pts[0].x-pts[1].x,pts[0].y-pts[1].y);
      const sc=Math.min(2.2,Math.max(0.35,pinchBaseScale*(d/pinchStart)));
      scaleMap[grp.id]=sc;
      grp.style.transform=`scale(${sc})`;
    }
  });

  const up=e=>{
    if(!ptrs[e.pointerId])return;
    delete ptrs[e.pointerId];
    if(ptrCount()===0){
      save();
      dragAnchor=null;pinchStart=null;
    } else if(ptrCount()===1){
      // Second finger lifted — switch back to drag with remaining finger
      const rem=Object.values(ptrs)[0];
      resolveToLeft();
      elAnchor={x:parseFloat(grp.style.left),y:parseFloat(grp.style.top)};
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
