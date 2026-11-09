# UNO firmware

Firmware dieu khien dong co va cam bien Arduino UNO R3.

Snapshot duoc tim thay: `archive/AppleSeed_AI_Robot_V1_5_UART_FAILSAFE.ino`.

Pin map theo source:
- L298N IN1..IN4: D5,D6,D7,D8
- Servo: D3
- HC-SR04: TRIG D9, ECHO D10
- Rain sensor: A0
- Battery divider: A1
- UART: Serial 9600 (UNO RX/TX)

Fail-safe trong source dung timeout 700 ms khi dang chay manual. Can test mat ket noi va STOP truoc khi cho robot chay tren san.

Khong nap firmware nay neu chua doi chieu pin map va firmware controller dang dung.
