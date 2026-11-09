# Hardware baseline — Apple Seed AI Robot

Tai lieu nay tong hop thong tin da duoc ghi nhan trong qua trinh lap robot. Day khong phai so do day hoan chinh. Hay doi chieu truc tiep board va anh dau day truoc khi cap nguon.

## 1. Bo dieu khien dong co UNO R3

| Chuc nang | Chan UNO da ghi nhan |
|---|---:|
| L298N IN1 | D5 |
| L298N IN2 | D6 |
| L298N IN3 | D7 |
| L298N IN4 | D8 |
| HC-SR04 TRIG | D9 |
| HC-SR04 ECHO | D10 |
| Servo | D3 |
| Cam bien mua | A0 |
| Do dien ap pin qua mach chia ap | A1 |
| LCD I2C / RTC I2C | SDA A4, SCL A5 |

Luu y:
- Chan ENA/ENB cua L298N can duoc xac nhan theo cach cam jumper/dau day hien tai.
- GND cua UNO, L298N va cac module dung chung nguon phai noi chung mass.
- Khong dua dien ap pin 2S truc tiep vao chan analog UNO; phai co mach chia ap va tinh he so.
- Khong cap dong co tu chan 5V UNO.

## 2. ESP32-S3-CAM N16R8 + OV2640

Camera pin map tung duoc su dung trong firmware cu:
- XCLK GPIO15
- SIOD GPIO4, SIOC GPIO5
- Y2..Y9: GPIO11, 9, 8, 10, 12, 18, 17, 16
- VSYNC GPIO6, HREF GPIO7, PCLK GPIO13
- PWDN=-1, RESET=-1

microSD 1-bit tung duoc cau hinh:
- CLK GPIO39
- CMD GPIO38
- DATA0 GPIO40

**Can xac minh lai voi board revision va firmware dang chay.** Khong su dung GPIO35/36/37 cho ngoai vi neu board dung OPI PSRAM. GPIO19/20 co the lien quan USB native. Khong gan pin I2S/UART moi khi chua kiem tra header thuc te.

## 3. Bo dieu khien TFT

- ESP32 TFT 320x480, touch GT911.
- I2C touch da ghi nhan: SDA GPIO21, SCL GPIO22, INT GPIO32, RST GPIO33.
- Firmware cu co cac ban V2.x/V3.x; phai chon dung ban dang chay truoc khi sua.

## 4. Audio (chua chot pin)

- INMP441 microphone I2S.
- PCM5102A DAC I2S.
- PAM8610 amplifier analog input.
- Loa 4 ohm 3 W.

Chua chot GPIO audio. Cac mau pin XiaoZhi tham khao co the xung dot voi microSD va camera. Can kiem tra board config va kha nang I2S full-duplex truoc khi lap.

An toan:
- PAM8610 co ngo ra loa dang BTL; khong noi cuc am loa xuong GND.
- Kiem tra dien ap cap cho amplifier va cong suat loa; bat dau voi am luong thap.
- Khong cap 5V vao chan 3V3 cua ESP32.

## 5. Nguon

- Pin 2S Li-ion: 7.4 V danh dinh, 8.4 V khi day.
- BMS 2S va LM2596 buck.
- Can xac dinh dong tai toi da, dien ap dau ra buck va cach tach nguon motor/logic.
- GND chung khi can giao tiep tin hieu; tranh de dong motor chay qua duong mass logic nho.

## 6. Cac diem can xac nhan truoc khi hop nhat

1. Anh ro hai mat ESP32-S3-CAM va ten GPIO in tren header.
2. File firmware camera dang chay tot nhat va firmware UNO/TFT dang dung.
3. Pin UART giua S3-CAM va UNO, muc logic 3.3 V.
4. So do nguon thuc te va dien ap do duoc.
5. Pin I2S thuc te va xung dot voi SD/camera/USB.
