# Git Workflow / Code ownership

- `main`: nhánh tích hợp, không commit trực tiếp sau bootstrap.
- Giang: `feature/core-backend`; Tùng: `feature/obstacle-gps`; Duy: `feature/sos-fall`; Đạt: `feature/dashboard-environment`.
- Tạo nhánh từ main: `git switch main && git pull && git switch -c feature/xxx`.
- Cập nhật: `git add <đường-dẫn-phần-mình> && git commit -m "feat: ..." && git push -u origin feature/xxx`.
- Mỗi đầu việc có PR, mô tả test + ảnh/log + ảnh hưởng GPIO/contract. Review trước merge.
- Không sửa `main.cpp`, `PinConfig.h`, schema chung nếu không có PR thay đổi được duyệt.
- Không commit `.env`, Wi-Fi password, token Telegram, IP nhạy cảm hoặc dữ liệu GPS cá nhân.
- Sau merge: `git switch main && git pull`; tạo nhánh mới cho đầu việc mới.

**Quyền:** Chủ repo cần invite cả 3 thành viên bằng Settings → Collaborators; quyền write mới được push nhánh. Nếu chưa có quyền, fork + PR.
