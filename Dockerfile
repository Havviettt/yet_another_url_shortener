FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg:v3.0 AS builder

WORKDIR /src
COPY . .
RUN cmake --preset release \
    && cmake --build build-release -j 2

FROM ghcr.io/userver-framework/ubuntu-22.04-userver-pg:v3.0 AS runtime

RUN useradd --create-home --uid 10001 --shell /usr/sbin/nologin app
WORKDIR /app

COPY --from=builder /src/build-release/yet_another_url_shortener /app/yet_another_url_shortener
COPY configs /app/configs
COPY postgresql /app/postgresql
COPY docker/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh && chown -R app:app /app

USER app
EXPOSE 8080 8081
ENTRYPOINT ["/app/entrypoint.sh"]
