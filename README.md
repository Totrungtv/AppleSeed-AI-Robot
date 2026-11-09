# Apple Seed AI Robot

Du an robot AI Apple Seed — tach rieng firmware theo tung bo phan de de bao tri, kiem thu va nang cap.

> **Trang thai:** Kho ma nguon da khoi tao. Chua phai ban firmware all-in-one va chua duoc xac nhan build tren phan cung thuc te.

## Nguyen tac phat trien

1. **GIU NGUYEN** cac chuc nang dang hoat dong.
2. **KE THUA** ma nguon cu truoc khi sua; khong ghi de ban dang dung.
3. **MO RONG** theo module, moi thay doi co changelog va huong dan rollback.
4. Khong tu y chot GPIO khi chua doi chieu dung revision board va so do day.
5. Moi firmware release phai co build log va checklist test phan cung.

## Phan cung da ghi nhan

- Arduino UNO R3 + L298N, robot 4 banh.
- ESP32-S3-CAM N16R8 + OV2640, microSD.
- Bo dieu khien ESP32 TFT 320x480, cam ung GT911.
- INMP441 microphone, PCM5102A DAC, PAM8610 amplifier, loa.
- Pin Li-ion 2S, BMS 2S va mach ha ap LM2596.

Chi tiet va cac diem can xac minh: `docs/HARDWARE_BASELINE.md`.

## Cau truc muc tieu

- `firmware/uno/` — dieu khien dong co, cam bien, telemetry.
- `firmware/s3cam/` — camera, web UI, microSD.
- `firmware/controller/` — man hinh TFT, dieu khien robot.
- `components/audio/` — microphone, I2S, DAC va am thanh.
- `components/xiaozhi/` — tich hop voice assistant XiaoZhi.
- `docs/` — so do, pin map, build va kiem thu.

Cac thu muc firmware se duoc them sau khi doi chieu va dua ma nguon hien co vao kho. Khong tao code gia lap de danh dau la da tich hop.

## Quy trinh dong gop

- Tao nhanh rieng cho thay doi.
- Giu ban firmware goc va ghi ro file nao la ban dang dung.
- Kiem tra diff truoc khi commit.
- Ghi ro: da kiem tra tinh dung cu phap, da build, hay da test tren board.
