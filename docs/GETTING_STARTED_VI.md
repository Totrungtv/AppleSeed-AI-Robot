# Huong dan su dung repository Apple Seed AI Robot

## 1. Mo repository
Truy cap: https://github.com/Totrungtv/AppleSeed-AI-Robot

Thu muc tai lieu:
- docs/HARDWARE_BASELINE.md: thong tin phan cung va pin map da ghi nhan.
- docs/FIRMWARE_INVENTORY.md: danh sach snapshot firmware tim thay trong workspace.
- docs/ROADMAP.md: thu tu tich hop va checklist.

## 2. Tai ma nguon ve may tinh
Tren GitHub, mo file can tai, bam nut Raw hoac nut Download raw file. Luu file vao thu muc rieng tren may tinh. Khong nap firmware chi dua vao ten file; doc dung board target va pin map truoc.

## 3. Mo bang Arduino IDE
1. Cai Arduino IDE.
2. Mo file .ino tu dung thu muc.
3. Chon dung board va cong COM.
4. Bam Verify (dau tich) truoc; chi bam Upload khi Verify thanh cong va da doi chieu day noi.
5. Mo Serial Monitor dung baud rate ghi trong README cua module.

## 4. Quy tac an toan khi cap nhat firmware
- Khong ghi de file goc dang chay.
- Tao ban copy moi truoc khi sua.
- Moi lan chi thay doi mot module.
- Thu tren ban robot ke/banh nang khoi mat dat truoc.
- Kiem tra nut STOP va hanh vi khi mat Wi-Fi/UART.
- Neu co loi, nap lai ban snapshot cu.

## 5. Trang thai upload
Cac file README va tai lieu huong dan da co trong repository. Cac file .ino snapshot can duoc dua len sau khi kiem tra va loai bo thong tin Wi-Fi/credential khoi ban Public. Khong coi firmware la da build hoac da test neu chua co log tu Arduino IDE/ESP-IDF va test tren board that.

## 6. Bao mat
Repository dang Public. Khong commit mat khau Wi-Fi nha, token thiet bi, API key, Supabase service-role key hoac thong tin ca nhan. Neu mat khau AP tung duoc nap vao board va da lo ra tren repo, hay doi mat khau AP trong firmware moi va nap lai khi san sang.
