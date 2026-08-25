import pytest

from testsuite.databases.pgsql import discover

pytest_plugins = [
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.postgresql',
]


@pytest.fixture(scope='session')
def pgsql_local(service_source_dir, pgsql_local_create):
    databases = discover.find_schemas(
        'yet_another_url_shortener',
        [service_source_dir / 'postgresql/schemas'],
    )
    return pgsql_local_create(list(databases.values()))
