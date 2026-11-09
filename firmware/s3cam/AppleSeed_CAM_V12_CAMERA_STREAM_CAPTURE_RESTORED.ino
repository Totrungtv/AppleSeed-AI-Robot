#include <Arduino.h>
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include "SD_MMC.h"
#include "FS.h"
#include "img_converters.h"

// APPLE SEED CAMERA V11 - CAMERA + SD ONLY
// Board: ESP32-S3-CAM N16R8 + OV2640
// SD_MMC 1-bit: CLK=39, CMD=38, D0=40
// Camera pin map retained from the previously working V8/V9/V10 firmware.

static const char* AP_SSID = "APPLE_SEED_CAM";
static const char* AP_PASS = "CHANGE_THIS_AP_PASSWORD";

WebServer server(80);
WiFiServer streamServer(81);
WiFiClient streamClient;

#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     15
#define SIOD_GPIO_NUM      4
#define SIOC_GPIO_NUM      5
#define Y9_GPIO_NUM       16
#define Y8_GPIO_NUM       17
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       12
#define Y5_GPIO_NUM       10
#define Y4_GPIO_NUM        8
#define Y3_GPIO_NUM        9
#define Y2_GPIO_NUM       11
#define VSYNC_GPIO_NUM     6
#define HREF_GPIO_NUM      7
#define PCLK_GPIO_NUM     13

#define SD_CLK_GPIO       39
#define SD_CMD_GPIO       38
#define SD_D0_GPIO        40

bool cameraReady = false;
bool cameraUsesJpeg = false;
bool sdReady = false;
String sdMessage = "SD CHUA KIEM TRA";
volatile bool captureBusy = false;

const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="vi"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Apple Seed Camera</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#071522;color:#eafaff;font-family:Arial;text-align:center}
header{padding:14px;background:#102c42;color:#5dffff;font-size:22px;font-weight:bold}main{padding:12px}
.view{position:relative;width:100%;max-width:1000px;margin:auto;background:#000;overflow:hidden;border:1px solid #286078;border-radius:12px}
#cam{display:block;width:100%;height:auto;min-height:180px;object-fit:contain}
.grid{position:absolute;inset:0;pointer-events:none;background:linear-gradient(to right,transparent 33.15%,rgba(255,255,255,.45) 33.33%,transparent 33.5%,transparent 66.48%,rgba(255,255,255,.45) 66.66%,transparent 66.84%),linear-gradient(to bottom,transparent 33.15%,rgba(255,255,255,.45) 33.33%,transparent 33.5%,transparent 66.48%,rgba(255,255,255,.45) 66.66%,transparent 66.84%)}
button{padding:13px 20px;margin:8px;border:1px solid #4de4e7;border-radius:12px;background:#12334b;color:white;font-size:16px;cursor:pointer;touch-action:manipulation}
button:disabled{opacity:.55;cursor:wait}button.shutter{width:82px;height:82px;padding:0;border:6px solid #dff;border-radius:50%;background:#fff;color:#102c42;font-weight:bold;box-shadow:0 0 0 4px rgba(77,228,231,.18),0 5px 0 #789;transition:transform .08s,box-shadow .08s}
button.shutter:active{transform:translateY(5px) scale(.94);box-shadow:0 0 0 3px rgba(77,228,231,.28)}
.badge{padding:8px 12px;display:inline-block;border-radius:8px;background:#102c42;margin:4px}#result{display:none;max-width:min(100%,1000px);height:auto;margin:12px auto;border-radius:10px}
#state,#photo{min-height:22px}
</style></head><body><header>APPLE SEED · CAMERA</header><main>
<p id="state">Đang khởi động camera...</p><div class="badge" id="sd">Đang kiểm tra thẻ nhớ...</div>
<div class="view"><img id="cam" alt="Camera live"><div class="grid"></div></div>
<br><button id="reconnectBtn" type="button">KẾT NỐI LẠI</button>
<button id="shutter" class="shutter" type="button" aria-label="Chụp ảnh">●</button>
<button id="closePhotoBtn" type="button">ĐÓNG ẢNH</button>
<p id="photo"></p><img id="result" alt="Ảnh vừa chụp">
<p>Video trực tiếp VGA · Ảnh chụp UXGA (1600×1200 nếu OV2640 hỗ trợ) · Lưu vào microSD</p>
</main><script>
const cam=document.getElementById('cam');
const stateEl=document.getElementById('state');
const photoEl=document.getElementById('photo');
const resultEl=document.getElementById('result');
let photoUrl=null;
function startStream(){
  stateEl.textContent='Đang kết nối camera...';
  cam.onload=()=>{stateEl.textContent='ĐANG NHẬN HÌNH CAMERA';};
  cam.onerror=()=>{stateEl.textContent='Mất luồng hình. Nhấn KẾT NỐI LẠI.';};
  cam.src='';
  setTimeout(()=>{cam.src='http://'+location.hostname+':81/stream?t='+Date.now();},180);
}
async function refreshSD(){
  const badge=document.getElementById('sd');
  try{
    const r=await fetch('/status?t='+Date.now(),{cache:'no-store'});
    if(!r.ok) throw new Error('HTTP '+r.status);
    const s=await r.json();
    if(s.sd){badge.textContent='SD READY · '+s.cardType+' · '+s.totalMB+' MB · ĐỌC/GHI OK';badge.style.background='#14533d';}
    else{badge.textContent=s.sdMessage||'SD KHÔNG SẴN SÀNG';badge.style.background='#713b24';}
  }catch(e){badge.textContent='KHÔNG ĐỌC ĐƯỢC TRẠNG THÁI SD';badge.style.background='#713b24';}
}
document.getElementById('reconnectBtn').addEventListener('click',async()=>{
  const b=document.getElementById('reconnectBtn');b.disabled=true;stateEl.textContent='Đang khởi động lại luồng camera...';
  try{const r=await fetch('/stream/reconnect?t='+Date.now(),{cache:'no-store'});if(!r.ok)throw new Error('HTTP '+r.status);await new Promise(resolve=>setTimeout(resolve,150));startStream();}
  catch(e){stateEl.textContent='Không kết nối lại được: '+e.message;}
  finally{setTimeout(()=>{b.disabled=false;},500);}
});
document.getElementById('closePhotoBtn').addEventListener('click',()=>{
  resultEl.style.display='none';resultEl.removeAttribute('src');
  if(photoUrl){URL.revokeObjectURL(photoUrl);photoUrl=null;}
  photoEl.textContent='Đã đóng ảnh xem trước.';
});
document.getElementById('shutter').addEventListener('click',async()=>{
  const btn=document.getElementById('shutter');
  if(btn.disabled)return;
  btn.disabled=true;photoEl.textContent='Đang chụp ảnh độ phân giải cao...';
  try{
    const r=await fetch('/capture?full=1&t='+Date.now(),{cache:'no-store'});
    if(!r.ok)throw new Error(await r.text());
    const blob=await r.blob();
    if(photoUrl)URL.revokeObjectURL(photoUrl);
    photoUrl=URL.createObjectURL(blob);resultEl.src=photoUrl;resultEl.style.display='block';
    const a=document.createElement('a');a.href=photoUrl;a.download='AppleSeed_UXGA.jpg';document.body.appendChild(a);a.click();a.remove();
    const saved=r.headers.get('X-SD-Saved')==='1';
    const path=r.headers.get('X-SD-Path')||'';
    photoEl.textContent=saved?'ĐÃ CHỤP VÀ LƯU SD: '+path:'ĐÃ CHỤP NHƯNG CHƯA LƯU ĐƯỢC VÀO SD';
    refreshSD();
  }catch(e){photoEl.textContent='LỖI CHỤP ẢNH: '+e.message;}
  finally{btn.disabled=false;}
});
startStream();refreshSD();setInterval(refreshSD,4000);
</script></body></html>)HTML";

bool initCamera(){
  camera_config_t c = {};
  c.ledc_channel = LEDC_CHANNEL_0;
  c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM; c.pin_d1 = Y3_GPIO_NUM;
  c.pin_d2 = Y4_GPIO_NUM; c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM; c.pin_d5 = Y7_GPIO_NUM;
  c.pin_d6 = Y8_GPIO_NUM; c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM; c.pin_pclk = PCLK_GPIO_NUM;
  c.pin_vsync = VSYNC_GPIO_NUM; c.pin_href = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM; c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM; c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 20000000;
  c.frame_size = FRAMESIZE_VGA;
  c.fb_count = psramFound() ? 2 : 1;
  c.grab_mode = CAMERA_GRAB_LATEST;
  c.fb_location = psramFound() ? CAMERA_FB_IN_PSRAM : CAMERA_FB_IN_DRAM;

  // Keep the proven V10 behavior: try hardware JPEG, then retain RGB565 fallback.
  c.pixel_format = PIXFORMAT_JPEG;
  esp_err_t err = esp_camera_init(&c);
  if (err == ESP_OK) {
    cameraUsesJpeg = true;
    sensor_t *sensor = esp_camera_sensor_get();
    if (sensor) {
      sensor->set_framesize(sensor, FRAMESIZE_VGA);
      sensor->set_quality(sensor, 12);
      sensor->set_brightness(sensor, 0);
      sensor->set_contrast(sensor, 1);
      Serial.printf("OV2640 PID=0x%04X; hardware JPEG active\n", sensor->id.PID);
    }
    return true;
  }

  Serial.printf("JPEG init failed 0x%x; trying RGB565 fallback\n", (unsigned)err);
  esp_camera_deinit();
  c.pixel_format = PIXFORMAT_RGB565;
  c.fb_count = 1;
  c.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  err = esp_camera_init(&c);
  if (err != ESP_OK) {
    Serial.printf("RGB565 init failed 0x%x\n", (unsigned)err);
    return false;
  }
  cameraUsesJpeg = false;
  sensor_t *sensor = esp_camera_sensor_get();
  if (sensor) {
    sensor->set_framesize(sensor, FRAMESIZE_VGA);
    Serial.printf("OV2640 PID=0x%04X; RGB565 fallback active\n", sensor->id.PID);
  }
  return true;
}

bool initSDCard(){
  sdReady=false;sdMessage="DANG KHOI TAO SD";
  if(!SD_MMC.setPins(SD_CLK_GPIO,SD_CMD_GPIO,SD_D0_GPIO)){sdMessage="LOI CAU HINH CHAN SD";return false;}
  if(!SD_MMC.begin("/sdcard",true,false,20000)){sdMessage="KHONG MOUNT DUOC SD";return false;}
  uint8_t t=SD_MMC.cardType();
  if(t==CARD_NONE){sdMessage="KHONG NHAN THE SD";SD_MMC.end();return false;}
  if(!SD_MMC.exists("/AppleSeed")&&!SD_MMC.mkdir("/AppleSeed")){sdMessage="TAO THU MUC THAT BAI";return false;}
  const char *p="/AppleSeed/.sd_test.tmp";const char data[]="APPLESEED_SD_OK";
  File w=SD_MMC.open(p,FILE_WRITE);if(!w){sdMessage="SD KHONG GHI DUOC";return false;}
  size_t n=w.write((const uint8_t*)data,sizeof(data)-1);w.flush();w.close();
  if(n!=sizeof(data)-1){sdMessage="SD GHI DU LIEU LOI";return false;}
  File r=SD_MMC.open(p,FILE_READ);if(!r){sdMessage="SD KHONG DOC DUOC";return false;}
  char back[sizeof(data)]={0};size_t nr=r.readBytes(back,sizeof(data)-1);r.close();SD_MMC.remove(p);
  if(nr!=sizeof(data)-1||memcmp(back,data,sizeof(data)-1)!=0){sdMessage="SD DOC LAI KHONG KHOP";return false;}
  sdReady=true;sdMessage="SD SAN SANG - DOC/GHI OK";return true;
}

String nextPhotoPath(){char p[48];for(uint32_t i=1;i<1000000;i++){snprintf(p,sizeof(p),"/AppleSeed/IMG_%06lu.jpg",(unsigned long)i);if(!SD_MMC.exists(p))return String(p);}return String();}

void handleRoot(){server.sendHeader("Cache-Control","no-store");server.send_P(200,"text/html; charset=utf-8",INDEX_HTML);}
void handleStatus(){
  uint8_t t=sdReady?SD_MMC.cardType():CARD_NONE;bool ok=sdReady&&t!=CARD_NONE;
  uint32_t mb=ok?(uint32_t)(SD_MMC.cardSize()/(1024ULL*1024ULL)):0;
  String type="NONE";if(t==CARD_MMC)type="MMC";else if(t==CARD_SD)type="SD";else if(t==CARD_SDHC)type="SDHC";
  String j="{\"camera\":"+String(cameraReady?"true":"false")+",\"sd\":"+String(ok?"true":"false")+",\"sdMessage\":\""+sdMessage+"\",\"cardType\":\""+type+"\",\"totalMB\":"+String(mb)+"}";
  server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",j);
}
void handleCapture(){
  if (!cameraReady) { server.send(503, "text/plain", "Camera not ready"); return; }
  if (captureBusy) { server.send(503, "text/plain", "Camera is busy; try again"); return; }
  captureBusy = true; // Pause MJPEG while the sensor changes resolution.

  sensor_t *sensor = esp_camera_sensor_get();
  if (sensor && psramFound()) sensor->set_framesize(sensor, FRAMESIZE_UXGA);
  delay(300);

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    if (sensor) sensor->set_framesize(sensor, FRAMESIZE_VGA);
    captureBusy = false;
    server.send(503, "text/plain", "Camera capture failed: framebuffer NULL");
    return;
  }

  uint8_t *jpgBuf = nullptr;
  size_t jpgLen = 0;
  bool allocatedJpeg = false;
  if (fb->format == PIXFORMAT_JPEG) {
    jpgLen = fb->len;
    jpgBuf = (uint8_t*)ps_malloc(jpgLen);
    if (!jpgBuf) jpgBuf = (uint8_t*)malloc(jpgLen);
    if (jpgBuf) { memcpy(jpgBuf, fb->buf, jpgLen); allocatedJpeg = true; }
  } else {
    allocatedJpeg = frame2jpg(fb, 92, &jpgBuf, &jpgLen);
  }
  esp_camera_fb_return(fb);
  if (sensor) sensor->set_framesize(sensor, FRAMESIZE_VGA);

  if (!allocatedJpeg || !jpgBuf || jpgLen == 0) {
    if (jpgBuf) free(jpgBuf);
    captureBusy = false;
    server.send(500, "text/plain", "JPEG conversion/copy failed");
    return;
  }

  String path;
  bool saved = false;
  if (sdReady) {
    path = nextPhotoPath();
    if (path.length()) {
      File f = SD_MMC.open(path, FILE_WRITE);
      if (f) { size_t n = f.write(jpgBuf, jpgLen); f.flush(); f.close(); saved = (n == jpgLen); }
    }
  }

  WiFiClient client = server.client();
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Content-Disposition", "inline; filename=AppleSeed.jpg");
  server.sendHeader("X-SD-Saved", saved ? "1" : "0");
  server.sendHeader("X-SD-Path", saved ? path : "NOT_SAVED");
  server.setContentLength(jpgLen);
  server.send(200, "image/jpeg", "");
  client.write(jpgBuf, jpgLen);
  Serial.printf("PHOTO %u bytes; SD=%s\n", (unsigned)jpgLen, saved ? path.c_str() : "NOT SAVED");
  free(jpgBuf);
  captureBusy = false;
}

void handleStreamReconnect(){if(streamClient)streamClient.stop();server.sendHeader("Cache-Control","no-store");server.send(200,"text/plain","stream reset");}

void handleStream(){
  if (!streamClient || !streamClient.connected()) {
    WiFiClient incoming = streamServer.available();
    if (!incoming) return;
    streamClient.stop();
    streamClient = incoming;
    streamClient.setNoDelay(true);
    streamClient.setTimeout(100);
    uint32_t startWait = millis();
    while (streamClient.connected() && !streamClient.available() && millis() - startWait < 500) delay(1);
    while (streamClient.available()) {
      String line = streamClient.readStringUntil('\n');
      if (line == "\r" || line.length() <= 1) break;
    }
    streamClient.print("HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nCache-Control: no-cache\r\nConnection: keep-alive\r\nContent-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n");
    return;
  }
  if (!cameraReady || captureBusy) return;

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) { delay(25); return; }
  uint8_t *jpgBuf = nullptr;
  size_t jpgLen = 0;
  bool allocatedJpeg = false;
  if (fb->format == PIXFORMAT_JPEG) {
    jpgBuf = fb->buf;
    jpgLen = fb->len;
  } else {
    allocatedJpeg = frame2jpg(fb, 82, &jpgBuf, &jpgLen);
  }
  if (!jpgBuf || jpgLen == 0 || (fb->format != PIXFORMAT_JPEG && !allocatedJpeg)) {
    esp_camera_fb_return(fb);
    if (allocatedJpeg && jpgBuf) free(jpgBuf);
    streamClient.stop();
    return;
  }
  streamClient.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", (unsigned)jpgLen);
  size_t sent = streamClient.write(jpgBuf, jpgLen);
  streamClient.print("\r\n");
  esp_camera_fb_return(fb);
  if (allocatedJpeg && jpgBuf) free(jpgBuf);
  if (sent == 0 || !streamClient.connected()) streamClient.stop();
  delay(35);
}

void setup(){
  Serial.begin(115200);delay(700);Serial.println("\nAPPLE SEED CAMERA V12 - CAMERA STREAM + CAPTURE RESTORED");
  Serial.printf("PSRAM: %s\n",psramFound()?"FOUND":"NOT FOUND");
  cameraReady=initCamera();Serial.printf("Camera: %s; mode=%s\n",cameraReady?"READY":"FAILED",cameraUsesJpeg?"JPEG":"RGB565->JPEG software");
  sdReady=initSDCard();Serial.printf("SD: %s - %s\n",sdReady?"READY":"FAILED",sdMessage.c_str());
  WiFi.mode(WIFI_AP);WiFi.softAP(AP_SSID,AP_PASS);
  Serial.printf("Wi-Fi: %s / %s\n",AP_SSID,AP_PASS);Serial.print("Open http://");Serial.println(WiFi.softAPIP());
  server.on("/",HTTP_GET,handleRoot);server.on("/status",HTTP_GET,handleStatus);
  server.on("/capture",HTTP_GET,handleCapture);server.on("/stream/reconnect",HTTP_GET,handleStreamReconnect);
  server.begin();streamServer.begin();Serial.println("HTTP 80 + MJPEG 81 ready");
}
void loop(){server.handleClient();handleStream();server.handleClient();delay(1);}
