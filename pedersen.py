#!/usr/bin/python3

# generated with openssl dhparam -2 4096
# This took a long time.

dhparam = '''
-----BEGIN DH PARAMETERS-----
MIICCAKCAgEA8FsghpTusENDzVdREgAR1YGECBgguUTTR+U+6L4ZKFHg8U+Jw2Hm
BtmYRRSPKVr+JqGf0uZR410K8UWmPjCypBgtQaRhWy3sa/cbJOw5wA5aHrA8OQB4
vakjFzBxFewfbpw8rf1wV3acvAGzDlQVdE815WPYj0Sic6WaXpe7+6xMgEErWxb4
ByXm0CxhFoxZaBRu6aUyL2vEAhPtQLKbZeGFS0S2wYL0XWfg5dN2xhY5KAMcz7sZ
I38pQ/DNryLILS8kKGrJwwTG3ugqUAf3kA4zitUXYQQU8c3Ky8H4xNgtaR1VPUYR
eqTdZKi5M8sdLdr+W+jwRKwap4scvom7rirehE8gDf+hDrDIkKoWZfqsjGVpyW9W
8h4GywL1wPhaLCWbsrQzBSLMW0YF6JjrRacc2Sm35yHJzpI5J+smDKNNTN45fH+S
5JqgACf7dHRB0m448J9w6n91YT7e5v7aU9BPgLyGAdJM4ADu0EoPIcBCogBANyxu
K/FyQPYUhnC1/QEwvemd3qc/8OpBPfSHju+bMIZ9RAeyWlSPSjqBCdV05/yCpi11
JdDYHULKFchzfF0JPg3rlraSdlzdbvH4f6H/Lw91Z2pV3YKQzibz6Z2UMfdaEs31
v/d/h7er8PnueXo1CxCa0508lFuq501o3ouRZD24zlYbF6ph7hMa6IMCAQI=
-----END DH PARAMETERS-----
'''

from base64 import b64decode
import math
import re

# extract the value from above dhparam.
q = int.from_bytes(
        b64decode(
            re.sub(r'^.* PARAMETERS-----(.*)-----END DH.*$', r'\1', dhparam, flags=re.S)
        )[8:-3],
        'big'
)

g = pow(int.from_bytes(f'{math.pi:.100f}'.encode(), 'big'), 42, q)
h = pow(int.from_bytes(f'{math.e:.100f}'.encode(), 'big'), 42, q)

def pederhash(ab: bytes) -> bytes:
    a = int.from_bytes(ab[:len(ab) // 2], 'big')
    b = int.from_bytes(ab[len(ab) // 2:], 'big')

    if max(a, b) >= q // 2:
        raise ValueError

    return pow(g, a, q) * pow(h, b, q) % q


print(pederhash(b'Hello, World!').to_bytes((q.bit_length() + 7) // 8, 'big').hex())
