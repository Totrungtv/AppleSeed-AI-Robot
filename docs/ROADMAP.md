# Roadmap va tieu chi chap nhan

## Giai doan 0 — Bao toan ma nguon

- [ ] Dua firmware camera dang hoat dong vao repo, giu nguyen ban goc.
- [ ] Dua firmware UNO dang hoat dong vao repo.
- [ ] Dua firmware TFT dang hoat dong vao repo.
- [ ] Ghi ro ten file, ngay, board, thu vien va cach nap.
- [ ] Khong sua code trong giai doan sao luu.

## Giai doan 1 — On dinh camera + SD

- [ ] Video stream hien thi lien tuc.
- [ ] Chup anh do phan giai cao va luu microSD.
- [ ] Nut ket noi lai thuc su khoi dong lai stream.
- [ ] Nut dong preview hoat dong.
- [ ] Trang thai SD cap nhat tu endpoint status.
- [ ] Test lien tuc va ghi lai FPS/do tre/dung luong anh.

## Giai doan 2 — Giao tiep UNO

- [ ] Chot UART va muc logic.
- [ ] Lenh tien/lui/re/trai/phai/stop.
- [ ] Telemetry pin, cam bien mua, khoang cach.
- [ ] Stop uu tien va fail-safe khi mat ket noi.
- [ ] Test robot tren gia truoc khi chay tren san.

## Giai doan 3 — Audio

- [ ] Xac minh pin I2S va xung dot ngoai vi.
- [ ] Thu INMP441 doc am thanh.
- [ ] Thu PCM5102A + PAM8610 + loa voi am luong thap.
- [ ] Kiem tra full-duplex truoc khi bat mic va loa dong thoi.

## Giai doan 4 — XiaoZhi va AI

- [ ] Chon dung board target va phien ban ESP-IDF theo upstream.
- [ ] Build firmware XiaoZhi goc thanh cong truoc.
- [ ] Tich hop camera/SD/robot UART theo component, khong thay the audio stack tuy tien.
- [ ] Kiem tra ket noi Internet, dich vu AI va bao mat thong tin.
- [ ] Test tat ca tinh nang cu sau moi lan hop nhat.

## Quy tac phat hanh

Moi release phai ghi:
- Commit va firmware goc lam nen.
- Thay doi pin map (neu co).
- Ket qua build va board/SDK.
- Ket qua test thuc te va cac loi con ton tai.
- Cach rollback ve ban truoc.
