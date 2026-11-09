# ESP32-S3 camera firmware

Board target duoc ghi trong source: ESP32-S3-CAM N16R8 + OV2640.

Snapshot ung vien: `archive/AppleSeed_CAM_V12_CAMERA_STREAM_CAPTURE_RESTORED.ino`.

Endpoints:
- `http://192.168.4.1/` — giao dien camera
- `http://192.168.4.1:81/stream` — MJPEG stream
- `/status` — trang thai camera/SD
- `/capture?full=1` — chup anh, thu luu vao SD
- `/stream/reconnect` — reset stream client

SD_MMC 1-bit trong source: CLK GPIO39, CMD GPIO38, D0 GPIO40.
Camera pin map: XCLK15, SIOD4, SIOC5, Y2..Y9 = 11,9,8,10,12,18,17,16; VSYNC6, HREF7, PCLK13.

Day la ghi chu doc source, khong phai xac nhan build hay test phan cung.
