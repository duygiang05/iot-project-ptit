# Local infrastructure — Giang

`compose.yaml`: Mosquitto `127.0.0.1:1883`, PostgreSQL `127.0.0.1:5433` (port container 5432), DB `smartcane`, user `smartcane`, password qua `POSTGRES_PASSWORD`.

Tại project root:
```bash
cp .env.example .env
# Sửa .env, đặt POSTGRES_PASSWORD mạnh
docker compose --env-file .env -f infra/compose.yaml up -d
docker compose --env-file .env -f infra/compose.yaml ps
```

**Lưu ý:** Mosquitto `allow_anonymous true` chỉ dùng trên localhost dev. Không mở port Internet. Wokwi trình duyệt không truy cập `localhost` của máy; cần broker test có thể truy cập an toàn và không để lộ credential.
