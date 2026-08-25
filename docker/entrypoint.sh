#!/bin/sh
set -eu

psql "$DB_CONNECTION" -v ON_ERROR_STOP=1 \
  -f /app/postgresql/schemas/db_1.sql

exec /app/yet_another_url_shortener \
  --config /app/configs/static_config.yaml \
  --config_vars /app/configs/config_vars.yaml
