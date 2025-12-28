# Mũ Bảo Hiểm Đa Năng Tăng Tính Năng An Toàn Cho Người Tham Gia Giao Thông

Mũ bảo hiểm thông minh tích hợp các tính năng cảnh báo va chạm và mô phỏng chức năng bảo vệ tự động trên MATLAB.

## MỤC LỤC

- [Giới Thiệu](#giới-thiệu)
- [Thông Số Kỹ Thuật](#thông-số-kỹ-thuật)
- [Danh Sách Linh Kiện](#danh-sách-linh-kiện)
- [Sơ Đồ Nguyên Lý và PCB](#sơ-đồ-nguyên-lý-và-pcb)
- [Hướng Dẫn Lắp Ráp](#hướng-dẫn-lắp-ráp)
- [Ảnh/Video Demo](#ảnhvideo-demo)

## Giới Thiệu

### Dự án làm gì?
Dự án phát triển một **mũ bảo hiểm thông minh** tích hợp các tính năng an toàn nâng cao cho người tham gia giao thông, đặc biệt là người điều khiển xe máy và các phương tiện hai bánh.

Các tính năng chính:
- Cảnh báo va chạm dựa trên cảm biến gia tốc và con quay hồi chuyển.
- Cảnh báo không cài quay mũ, phát hiện nồng độ cồn.
- Phát hiện tai nạn gửi vị trí , link google map, thời gian về điện thoại người thân bằng module sim768c.
- Mô phỏng chức năng bảo vệ tự động trên phần mềm **MATLAB/Simulink**.

### Người dùng chính
- Người tham gia giao thông điều khiển **xe máy**, **xe gắn máy**, **xe đạp điện**, **xe scooter** và các phương tiện hai bánh khác.

### Mục tiêu thiết kế

**Thực tiễn**  
Tạo ra một sản phẩm thực tế, có tính ứng dụng cao nhằm nâng cao mức độ an toàn cho người tham gia giao thông, giảm thiểu nguy cơ tai nạn và hỗ trợ phát hiện kịp thời khi xảy ra sự cố.

**Giáo dục**  
- Tìm hiểu sâu về vi điều khiển **ESP32** (Wi-Fi, Bluetooth, xử lý cảm biến).
- Nghiên cứu hệ thống nhúng (embedded systems).
- Thực hành thiết kế phần cứng (sơ đồ nguyên lý, PCB).
- Mô phỏng và phân tích hệ thống điều khiển trên **MATLAB/Simulink**.

## Thông Số Kỹ Thuật

| Thông số                  | Chi tiết                                      |
|---------------------------|-----------------------------------------------|
| Vi điều khiển             | ESP32-WROOM-32 (dual-core, Wi-Fi, Bluetooth) |
| Cảm biến                  | MPU6050 (6-axis IMU: gia tốc + con quay hồi chuyển) |
| Cảnh báo                  | Buzzer              |
| Nguồn điện                | Pin Li-Po 3.7V 1000–2000mAh                  |
| Thời gian hoạt động       | ~8–12 giờ (tùy dung lượng pin)               |     |          |
| Mô phỏng                  | MATLAB/Simulink (collision detection & airbag simulation) |

## Danh Sách Linh Kiện

| STT | Linh kiện                          | Số lượng | Ghi chú / Mô tả                                                                 |
|-----|------------------------------------|----------|---------------------------------------------------------------------------------|
| 1   | ESP32-WROOM-32 Development Board   | 1        | Vi điều khiển chính (dual-core, Wi-Fi + Bluetooth)                             |
| 2   | MPU6050 (6-axis IMU)               | 1        | Cảm biến gia tốc + con quay hồi chuyển (GY-521 module)                         |
| 3   | MQ-3 Alcohol Sensor                | 1        | Cảm biến nồng độ cồn trong hơi thở (dùng để phát hiện say rượu khi lái xe)     |
| 4   | VL53L0X / VL53L1X Laser ToF        | 1        | Cảm biến khoảng cách laser (Time-of-Flight) để phát hiện vật cản phía trước    |
| 5   | NEO-6M GPS Module                  | 1        | Module định vị GPS (cung cấp tọa độ, tốc độ, thời gian)                        |
| 6   | SIM768C                            | 1        | Module GSM/GPRS (gửi tin nhắn SMS hoặc gọi điện khi phát hiện tai nạn)         |
| 7   | Buzzer (active hoặc passive)       | 1        | Cảnh báo âm thanh khi phát hiện va chạm hoặc nồng độ cồn cao                   |
| 8   | Pin       3.7V                     | 1        | Dung lượng đề xuất 1000–3000mAh (tùy thời gian sử dụng)                        |
| 9   | Module sạc pin TP4056              | 1        | Module sạc và bảo vệ                                                           |
| 10  | PCB tùy chỉnh hoặc protoboard      | 1        | Để hàn và lắp ráp linh kiện                                                    |
| 11  | Mũ bảo hiểm tiêu chuẩn             | 1        | Để gắn toàn bộ hệ thống linh kiện vào                                          |
## Sơ Đồ Nguyên Lý và PCB

- **Sơ đồ nguyên lý**: 
<img width="1053" height="757" alt="image" src="https://github.com/user-attachments/assets/6284aa70-c2bd-4bb5-a90c-cf739e9bdcfc" />

- **Thiết kế PCB**: 
<img width="326" height="351" alt="image" src="https://github.com/user-attachments/assets/a9850be5-a9a9-49db-bbf1-ee929f7564b9" />

## Hướng Dẫn Lắp Ráp
<img width="1307" height="743" alt="image" src="https://github.com/user-attachments/assets/39263158-0d53-4b38-a097-1731da5fc089" />

1.	Chuẩn bị hộp nhựa ABS , sau đó gắn PCB vào đáy hộp nhựa + pin 18650 trên nắp hộp bằng keo nến chuyên dụng.
2.	Cố định MPU6050 bằng keo nến ngay đỉnh đầu.
3.	Gắn MQ-3 vào khoang phía trước cằm.
4.	Gắn VL53L0X vào quai trái bằng keo nến và một tấm chắn vào quai bên phải để cảm biến có thể đo khoảng cách.
5.	Gắn module GPS kèm anten ra sát vỏ ngoài.
6.	Gắn module và anten GSM sát vỏ hôp.
7.	Khoan lỗ nhỏ trên hộp nhựa và gắn bruzzer. 
8.	Khoan lỗ để gắn công tắc.
9.	Đậy nặp và vặn ốc cố định.
10.	 Kiểm tra cuối cùng: đội thử, chạy xe 10 km đầu tiên
<img width="341" height="560" alt="image" src="https://github.com/user-attachments/assets/7a7eba01-9934-4e70-bc73-40c1d6fbfc56" />

## Ảnh demo
<img width="482" height="301" alt="image" src="https://github.com/user-attachments/assets/9fcf7717-4390-427b-8c6c-df42bbd3de16" />
<img width="362" height="643" alt="image" src="https://github.com/user-attachments/assets/611a82f9-3003-4f7c-b5be-3bfe750c91f2" />

