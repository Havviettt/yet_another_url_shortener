async def create_url(service_client, original_url='https://example.com/long'):
    response = await service_client.post(
        '/v1/urls',
        json={'url': original_url},
    )
    assert response.status == 201
    return response.json()


async def test_health(service_client):
    response = await service_client.get('/health')
    assert response.status == 200


async def test_create_redirect_and_metadata(service_client):
    body = await create_url(
        service_client,
        'https://example.com/some/long/path',
    )
    assert body['original_url'] == 'https://example.com/some/long/path'
    assert len(body['short_code']) == 7

    redirect_response = await service_client.get(
        body['short_url'],
        allow_redirects=False,
    )
    assert redirect_response.status == 302
    assert redirect_response.headers['Location'] == body['original_url']

    metadata_response = await service_client.get(
        f"/v1/urls/{body['short_code']}",
    )
    assert metadata_response.status == 200
    metadata = metadata_response.json()
    assert metadata['original_url'] == body['original_url']
    assert metadata['clicks_count'] == 1
    assert metadata['expired'] is False
    assert metadata['expires_at'] is None
    assert 'created_at' in metadata


async def test_delete_url(service_client):
    body = await create_url(service_client)

    delete_response = await service_client.delete(
        f"/v1/urls/{body['short_code']}",
    )
    assert delete_response.status == 204

    get_response = await service_client.get(
        f"/v1/urls/{body['short_code']}",
    )
    assert get_response.status == 404
    assert get_response.json()['error']['code'] == 'url_not_found'


async def test_unknown_short_code_has_structured_error(service_client):
    response = await service_client.get('/not-found', allow_redirects=False)
    assert response.status == 404
    assert response.json()['error'] == {
        'code': 'url_not_found',
        'message': 'short URL not found',
    }
    assert response.json()['request_id']
    assert response.headers['X-Request-ID'] == response.json()['request_id']


async def test_request_id_is_propagated(service_client):
    request_id = 'interview-demo-request'
    response = await service_client.post(
        '/v1/urls',
        headers={'X-Request-ID': request_id},
        json={'url': 'not-a-url'},
    )
    assert response.status == 400
    assert response.headers['X-Request-ID'] == request_id
    assert response.json()['request_id'] == request_id


async def test_rejects_invalid_requests(service_client):
    missing_url = await service_client.post('/v1/urls', json={})
    assert missing_url.status == 400
    assert missing_url.json()['error']['code'] == 'invalid_request'

    invalid_url = await service_client.post(
        '/v1/urls',
        json={'url': 'not-a-url'},
    )
    assert invalid_url.status == 400
    assert invalid_url.json()['error']['code'] == 'invalid_url'

    invalid_expiration = await service_client.post(
        '/v1/urls',
        json={
            'url': 'https://example.com',
            'expires_at': 'tomorrow',
        },
    )
    assert invalid_expiration.status == 400
    assert invalid_expiration.json()['error']['code'] == 'invalid_expiration'

    invalid_calendar_date = await service_client.post(
        '/v1/urls',
        json={
            'url': 'https://example.com',
            'expires_at': '2026-02-31T12:00:00Z',
        },
    )
    assert invalid_calendar_date.status == 400
    assert invalid_calendar_date.json()['error']['code'] == 'invalid_expiration'


async def test_expired_url_returns_gone(service_client):
    create_response = await service_client.post(
        '/v1/urls',
        json={
            'url': 'https://example.com/expired',
            'expires_at': '2020-01-01T00:00:00Z',
        },
    )
    assert create_response.status == 201

    response = await service_client.get(
        create_response.json()['short_url'],
        allow_redirects=False,
    )
    assert response.status == 410
    assert response.json()['error']['code'] == 'url_expired'


async def test_prometheus_metrics(service_client, monitor_client):
    await create_url(service_client, 'https://example.com/metrics')
    response = await monitor_client.get('/metrics')
    assert response.status == 200
    assert 'url_shortener_urls_created' in response.text
