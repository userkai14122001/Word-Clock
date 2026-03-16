#include "web_pages.h"

String pageIndex() {
    return
        "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/>"
        "<meta http-equiv='Pragma' content='no-cache'/>"
        "<meta http-equiv='Expires' content='0'/>"
        "<title>WordClock Setup</title>"
        "<style>"
        ".tabs{display:flex;gap:12px;margin-bottom:20px;}"
        ".tabs a{padding:8px 14px;background:#ddd;border-radius:6px;"
        "text-decoration:none;color:#000;font-weight:bold;}"
        ".tabs a:hover{background:#bbb;}"
        "label{display:block;margin-top:12px;font-weight:bold;}"
        "input,select{width:100%;padding:8px;margin-top:4px;font-size:16px;}"
        "</style>"
        "</head><body>"

        "<div class='tabs'>"
        "<a href='/wifi'>WIFI</a>"
        "<a href='/mqtt'>MQTT</a>"
        "<a href='/reboot'>REBOOT</a>"
        "</div>"

        "<h1>WordClock Setup</h1>"
        "<p>Bitte wählen Sie einen der oberen bereiche aus.</p>"

        "</body></html>";
}

String pageWifi() {
    return
        "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/>"
        "<meta http-equiv='Pragma' content='no-cache'/>"
        "<meta http-equiv='Expires' content='0'/>"
        "<title>WLAN Setup</title>"
        "<style>"
        ".tabs{display:flex;gap:12px;margin-bottom:20px;}"
        ".tabs a{padding:8px 14px;background:#ddd;border-radius:6px;"
        "text-decoration:none;color:#000;font-weight:bold;}"
        ".tabs a:hover{background:#bbb;}"
        "label{display:block;margin-top:12px;font-weight:bold;}"
        "input,select{width:100%;padding:8px;margin-top:4px;font-size:16px;}"
        "</style>"
        "</head><body>"

        "<div class='tabs'>"
        "<a href='/wifi'>WIFI</a>"
        "<a href='/mqtt'>MQTT</a>"
        "<a href='/reboot'>REBOOT</a>"
        "</div>"

        "<h1>WLAN konfigurieren</h1>"

        "<form action='/save' method='POST'>"

        "<label>SSID</label>"
        "<select name='ssid'><option>Bitte warten</option></select>"

        "<label>Passwort</label>"
        "<input id='pw' name='wifipass' type='password'>"
        "<button type='button' onclick=\"var x=document.getElementById('pw'); x.type=(x.type==='password'?'text':'password');\">Passwort anzeigen</button>"

        "<br><br><input type='submit' value='Speichern'>"
        "</form>"

        "<script>"
        "window.addEventListener('DOMContentLoaded',()=>{"
        " console.log('JS gestartet');"
        " fetch('/scan').then(r=>r.json()).then(list=>{"
        "   console.log('SCAN RESULT:', list);"
        "   let sel=document.querySelector('select[name=ssid]');"
        "   sel.innerHTML='';"
        "   list.forEach(ssid=>{"
        "     let o=document.createElement('option');"
        "     o.textContent=ssid;"
        "     sel.appendChild(o);"
        "   });"
        " }).catch(err=>console.error('SCAN ERROR:',err));"
        "});"
        "</script>"

        "</body></html>";
}


String pageMqtt() {
    return
        "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<meta http-equiv='Cache-Control' content='no-cache, no-store, must-revalidate'/>"
        "<meta http-equiv='Pragma' content='no-cache'/>"
        "<meta http-equiv='Expires' content='0'/>"
        "<title>MQTT Setup</title>"
        "<style>"
        ".tabs{display:flex;gap:12px;margin-bottom:20px;}"
        ".tabs a{padding:8px 14px;background:#ddd;border-radius:6px;"
        "text-decoration:none;color:#000;font-weight:bold;}"
        ".tabs a:hover{background:#bbb;}"
        "label{display:block;margin-top:12px;font-weight:bold;}"
        "input{width:100%;padding:8px;margin-top:4px;font-size:16px;}"
        "</style>"
        "</head><body>"

        "<div class='tabs'>"
        "<a href='/wifi'>WIFI</a>"
        "<a href='/mqtt'>MQTT</a>"
        "<a href='/reboot'>REBOOT</a>"
        "</div>"

        "<h1>MQTT konfigurieren</h1>"

        "<form action='/save' method='POST'>"
        "<label>Broker IP</label><input name='mqtt_host'>"
        "<label>User</label><input name='mqtt_user'>"
        "<label>Passwort</label><input name='mqtt_pass' type='password'>"
        "<br><br><input type='submit' value='Speichern'>"
        "</form>"

        "</body></html>";
}

String pageReboot() {
    return
        "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Reboot</title>"
        "<style>"
        ".tabs{display:flex;gap:12px;margin-bottom:20px;}"
        ".tabs a{padding:8px 14px;background:#ddd;border-radius:6px;"
        "text-decoration:none;color:#000;font-weight:bold;}"
        ".tabs a:hover{background:#bbb;}"
        "</style>"
        "</head><body>"

        "<div class='tabs'>"
        "<a href='/wifi'>WIFI</a>"
        "<a href='/mqtt'>MQTT</a>"
        "<a href='/reboot'>REBOOT</a>"
        "</div>"

        "<h1>WordClock Neustart</h1>"
        "<p>Hier kannst du die Uhr neu starten.</p>"

        "<form action='/do_reboot' method='POST'>"
        "<input type='submit' value='Jetzt neu starten'>"
        "</form>"

        "</body></html>";
}

String pageSaved() {
    return
        "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'>"
        "<title>Gespeichert</title>"
        "</head><body>"
        "<h1>Einstellungen gespeichert!</h1>"
        "<p>Die Einstellungen wurden gespeichert.</p>"
        "<p>Ein Neustart erfolgt nur über den Reboot-Tab.</p>"
        "</body></html>";
}
