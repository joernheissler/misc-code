#!/usr/bin/python3

"""
Encode a TOTP url string as specified by https://github.com/google/google-authenticator/wiki/Key-Uri-Format
The result can be printed as a QR code for import into mobile apps.

License: CC0
"""

from urllib.parse import urlunsplit, urlencode, quote
from base64 import b32encode
from typing import ByteString, Union


def encode_totp(provider: str, user: str, secret: Union[str, ByteString]) -> str:
    if not isinstance(secret, str):
        secret = b32encode(bytes(secret)).decode().rstrip("=")

    params = {
        "secret": secret,
        "issuer": provider,
    }

    label = quote(provider) + ":" + quote(user)

    return urlunsplit(("otpauth", "totp", "/" + label, urlencode(params, quote_via=quote), ""))


if __name__ == "__main__":
    from sys import argv

    print(encode_totp(*argv[1:]))
