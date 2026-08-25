CREATE SCHEMA IF NOT EXISTS url_shortener;

CREATE TABLE IF NOT EXISTS url_shortener.urls (
    id BIGSERIAL PRIMARY KEY,
    short_code VARCHAR(12) NOT NULL UNIQUE,
    original_url TEXT NOT NULL,
    created_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    expires_at TIMESTAMPTZ,
    clicks_count BIGINT NOT NULL DEFAULT 0
);
