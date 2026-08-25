#include "webui.h"
#include "config.h"
#include <WebServer.h>

static WebServer  server(80);
static Bulldog*   s_buddy = nullptr;

static Mood parseMood(const String& s) {
    if (s == "wave")    return MOOD_WAVE;
    if (s == "happy")   return MOOD_HAPPY;
    if (s == "coffee")  return MOOD_COFFEE;
    if (s == "smoke")   return MOOD_SMOKE;
    if (s == "sleepy")  return MOOD_SLEEPY;
    if (s == "stretch") return MOOD_STRETCH;
    return MOOD_IDLE;
}

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html>
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Desk Buddy</title>
<style>
 body{font-family:-apple-system,BlinkMacSystemFont,sans-serif;background:#0f1932;color:#eef;margin:0;padding:18px;max-width:480px;margin-left:auto;margin-right:auto}
 h1{font-size:24px;margin:0 0 14px;text-align:center}
 .card{background:#1a2547;border-radius:14px;padding:16px;margin-bottom:14px}
 label{display:block;margin:8px 0 4px;font-size:13px;opacity:.65}
 input,select,button{width:100%;padding:12px;font-size:16px;border-radius:10px;border:none;box-sizing:border-box;font-family:inherit}
 input,select{background:#2b3866;color:#fff;margin-bottom:6px}
 button.primary{background:#ff82aa;color:#111;font-weight:700;cursor:pointer;margin-top:10px}
 .moods{display:grid;grid-template-columns:repeat(3,1fr);gap:8px}
 .moods button{padding:14px 6px;font-size:14px;background:#3d4d84;color:#fff;font-weight:600;border-radius:10px;cursor:pointer}
 .moods button:active{background:#5568b1}
 #status{font-size:13px;opacity:.6;margin-top:12px;text-align:center;min-height:18px}
</style>
</head>
<body>
<h1>&#x1F436; Desk Buddy</h1>

<div class="card">
 <label>Line 1</label>
 <input id="l1" maxlength="18" placeholder="Hey there!">
 <label>Line 2</label>
 <input id="l2" maxlength="22" placeholder="You're doing great">
 <label>Mood</label>
 <select id="mood">
  <option value="wave">Wave</option>
  <option value="happy">Happy</option>
  <option value="coffee">Coffee</option>
  <option value="smoke">Wink</option>
  <option value="sleepy">Sleepy</option>
  <option value="stretch">Stretch</option>
  <option value="idle">Idle</option>
 </select>
 <button class="primary" onclick="send()">Send to buddy</button>
</div>

<div class="card">
 <label>Quick moods</label>
 <div class="moods">
  <button onclick="mood('wave')">Wave</button>
  <button onclick="mood('happy')">Happy</button>
  <button onclick="mood('coffee')">Coffee</button>
  <button onclick="mood('smoke')">Wink</button>
  <button onclick="mood('sleepy')">Sleepy</button>
  <button onclick="mood('stretch')">Stretch</button>
  <button onclick="mood('idle')">Idle</button>
 </div>
</div>

<div id="status"></div>

<script>
const say=m=>{document.getElementById('status').textContent=m;clearTimeout(window._t);window._t=setTimeout(()=>document.getElementById('status').textContent='',2200)};
function send(){
 const l1=document.getElementById('l1').value||'Hey there!';
 const l2=document.getElementById('l2').value||'';
 const m=document.getElementById('mood').value;
 fetch('/show',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},
   body:'l1='+encodeURIComponent(l1)+'&l2='+encodeURIComponent(l2)+'&mood='+m})
  .then(r=>say(r.ok?'Sent!':'Error')).catch(()=>say('Offline'));
}
function mood(m){fetch('/mood?m='+m,{method:'POST'}).then(r=>say(r.ok?'Mood: '+m:'Error'));}
</script>
</body>
</html>
)HTML";

WebUI::WebUI(Bulldog& b) { s_buddy = &b; }

void WebUI::begin() {
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", INDEX_HTML);
    });

    server.on("/show", HTTP_POST, []() {
        String l1 = server.hasArg("l1") ? server.arg("l1") : String("");
        String l2 = server.hasArg("l2") ? server.arg("l2") : String("");
        String m  = server.hasArg("mood") ? server.arg("mood") : String("happy");
        if (l1.length() > 20) l1.remove(20);
        if (l2.length() > 26) l2.remove(26);
        s_buddy->showCustom(l1, l2, parseMood(m), CUSTOM_HOLD_MS);
        server.send(200, "text/plain", "ok");
    });

    server.on("/mood", HTTP_POST, []() {
        String m = server.hasArg("m") ? server.arg("m") : String("idle");
        s_buddy->showCustom(String("Mood"), m, parseMood(m), CUSTOM_HOLD_MS);
        server.send(200, "text/plain", "ok");
    });

    server.onNotFound([]() { server.send(404, "text/plain", "not found"); });
    server.begin();
    Serial.println("WebUI listening on port 80");
}

void WebUI::tick() {
    server.handleClient();
}
