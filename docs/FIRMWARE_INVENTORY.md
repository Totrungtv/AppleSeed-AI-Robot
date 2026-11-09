# Firmware inventory

Cac ban ma nguon hien co trong workspace va duoc chon lam ung vien de dua vao repo. Bang nay KHONG khang dinh ban nao la ban dang chay tren robot.

| Module | File snapshot hien co | Ghi chu |
|---|---|---|
| Camera + SD | AppleSeed_CAM_V12_CAMERA_STREAM_CAPTURE_RESTORED.ino | Co MJPEG port 81, HTTP port 80, endpoint /status, /capture va /stream/reconnect. Giữ fallback RGB565->JPEG. |
| UNO | AppleSeed_AI_Robot_V1_5_UART_FAILSAFE.ino | D5-D8 motor, D3 servo, D9/D10 HC-SR04, A0 rain, A1 battery. Manual failsafe 700 ms theo code. |
| TFT controller | AppleSeed_ESP32_V3_3_HYBRID_WIFI_MANAGER_FIX2.ino | Touch GT911 GPIO21/22/32/33; UART2 RX16/TX17; USB Serial debug 115200, UNO UART 9600. |

## Trang thai

- Cac ten file tren duoc tim thay trong workspace tai thoi diem tao inventory.
- Chua dua binary/firmware len board tu repo.
- Chua co ket qua Arduino/ESP-IDF build tu moi truong nay.
- Chua xac nhan day noi thuc te trung khop voi cac pin map trong source.
- Can giu nguyen cac file snapshot, khong ghi de bang ban moi.

## Luu y can xu ly truoc khi cong khai ma nguon

- TFT snapshot co AP password mac dinh trong source; neu muon cong khai repo, can chuyen sang placeholder/Preferences truoc khi dua file nay len.
- Camera snapshot cung co AP password mac dinh. Can xac nhan day la mat khau demo hay mat khau dang dung.
- Kiem tra tat ca token/credentials trong cac firmware cu truoc khi push.
