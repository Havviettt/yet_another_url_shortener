# Yet Another URL Shortener

Учебный URL shortener на C++20, userver и PostgreSQL. Проект показывает полный
путь HTTP-запроса от handler до базы данных, тестирование и базовую
наблюдаемость через Prometheus и Grafana.

## API

| Метод | Путь | Назначение |
|---|---|---|
| `GET` | `/health` | Проверка работоспособности |
| `POST` | `/v1/urls` | Создание короткой ссылки |
| `GET` | `/v1/urls/{short_code}` | Метаданные и число переходов |
| `DELETE` | `/v1/urls/{short_code}` | Удаление ссылки |
| `GET` | `/{short_code}` | Редирект на исходный URL |
| `GET` | `/metrics` на порту `8081` | Метрики Prometheus |

Пример создания:

```bash
curl -i -X POST http://localhost:8080/v1/urls \
  -H 'Content-Type: application/json' \
  -H 'X-Request-ID: local-example' \
  -d '{"url":"https://example.com/very/long/path","expires_at":"2030-01-01T00:00:00Z"}'
```

`expires_at` необязателен и принимается в UTC-формате RFC 3339. Ошибки имеют
одинаковую форму:

```json
{
  "error": {
    "code": "url_not_found",
    "message": "short URL not found"
  },
  "request_id": "..."
}
```

## Разработка

1. Установить Docker Desktop и расширение VS Code Dev Containers.
2. Открыть проект в VS Code и выполнить `Dev Containers: Reopen in Container`.
3. В терминале контейнера запустить сервис:

```bash
make run
```

Unit-тесты и функциональный testsuite с отдельной PostgreSQL:

```bash
make test
```

## Production-профиль

Весь локальный стек запускается одной командой:

```bash
docker compose --profile production up --build -d
```

После запуска доступны:

- API: <http://localhost:8080>;
- метрики userver и приложения: <http://localhost:8081/metrics>;
- Prometheus: <http://localhost:9090>;
- Grafana: <http://localhost:3000>, логин и пароль `admin` / `admin`.

Datasource и dashboard `URL Shortener Overview` создаются в Grafana
автоматически. Эти учётные данные предназначены только для локальной разработки.

Остановка стека:

```bash
docker compose --profile production down
```

## Архитектура

```text
HTTP request
    ↓
handler (HTTP/JSON/status/request ID)
    ↓
service (валидация и бизнес-правила)
    ↓
repository (SQL)
    ↓
PostgreSQL
```

- `src/handlers/` — HTTP endpoints;
- `src/services/` — сценарии создания, получения, редиректа и удаления;
- `src/repositories/` — запросы к PostgreSQL;
- `src/domain/` — модель короткой ссылки;
- `src/http/` — общий формат ошибок и `X-Request-ID`;
- `src/observability/` — бизнес-счётчики;
- `configs/` — конфигурация компонентов userver;
- `postgresql/schemas/` — схема базы данных;
- `monitoring/` — Prometheus и provisioning Grafana;
- `tests/` — unit- и функциональные тесты;
- `.devcontainer/` — воспроизводимое окружение разработки;
- `Dockerfile` и `docker-compose.yml` — production-образ и локальный стек.

Логи пишутся в JSON. К событиям создания, редиректа и удаления добавляются
`request_id` и `short_code`, чтобы один запрос можно было проследить по логам.
