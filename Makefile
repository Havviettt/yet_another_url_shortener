PROJECT_NAME := yet_another_url_shortener
NPROCS ?= $(shell nproc)

.PHONY: configure build test run migrate format

configure:
	cmake --preset debug

build: configure
	cmake --build build-debug -j $(NPROCS)

test: configure
	cmake --build build-debug -j $(NPROCS)
	cd build-debug && ctest --output-on-failure

migrate:
	psql "$$DB_CONNECTION" -v ON_ERROR_STOP=1 -f postgresql/schemas/db_1.sql

run: build migrate
	./build-debug/$(PROJECT_NAME) \
		--config configs/static_config.yaml \
		--config_vars configs/config_vars.yaml

format:
	find src tests -type f \( -name '*.cpp' -o -name '*.hpp' \) -print0 | xargs -0 clang-format -i
