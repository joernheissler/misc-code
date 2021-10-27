#!/usr/bin/env python3

from datetime import datetime
import hmac
from hashlib import sha256
from urllib.parse import urlencode, quote

region = 'eu-central-1'
bucket = 'wulftest2017'
path = 'the_cloud.png'
host = 's3.{}.amazonaws.com'.format(region)
url = 'https://{}/{}/{}'.format(host, bucket, path)
access_key_id = 'AKIAIFIPHCAK5IGPWRCA'
access_key_secret = 'xXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxXxX'
expire = 300
algorithm = 'AWS4-HMAC-SHA256'

request = 'aws4_request'
service = 's3'

def hmac_sha256(k, v):
    return hmac.new(k, v.encode(), 'sha256').digest()

def urlenc(params):
    return urlencode(sorted(params.items()), safe='_.-~', encoding='utf-8', quote_via=quote)

now = datetime.utcnow()
date = now.strftime('%Y%m%d')
isodate = now.strftime('%Y%m%dT%H%M%SZ')


signing_key = hmac_sha256(('AWS4' + access_key_secret).encode(), date)
signing_key = hmac_sha256(signing_key, region)
signing_key = hmac_sha256(signing_key, service)
signing_key = hmac_sha256(signing_key, request)

scope = '/'.join([date, region, service, request])

params = {
    'X-Amz-Algorithm': algorithm,
    'X-Amz-Credential': access_key_id + '/' + scope,
    'X-Amz-Date': isodate,
    'X-Amz-Expires': str(expire),
    'X-Amz-SignedHeaders': 'host',
}

canonical_request = '\n'.join([
    'GET',
    '/{}/{}'.format(bucket, path),
    urlenc(params),
    'host:' + host,
    '',
    'host',
    'UNSIGNED-PAYLOAD',
])

string_to_sign = '\n'.join([
    algorithm,
    isodate,
    scope,
    sha256(canonical_request.encode()).hexdigest(),
])

signature = hmac_sha256(signing_key, string_to_sign).hex()
params['X-Amz-Signature'] = signature

print('{}?{}'.format(url, urlenc(params)))
