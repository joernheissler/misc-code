#!/usr/bin/python3

"""
Function to convert a QR code into UTF-8, inspired by "qrencode -t UTF8".

Requires the qrcodegen package:
    * https://github.com/nayuki/QR-Code-generator
    * pip install qrcodegen
    * apt install python3-qrcodegen

License: CC0
"""

from typing import Generator, Sequence
from qrcodegen import QrCode


def encode_qr_utf8(
    code: QrCode,
    top: int = 2,
    left: int = 2,
    right: int = 2,
    bottom: int = 2,
    invert: bool = False,
    top_blank: bool = False,
) -> Generator[str, None, None]:
    """
    Format a QR code into an iterable of strings.

    Args:
        code: QR code to encode
        top: top margin
        left: left margin
        right: right margin
        bottom: buttom margin
        invert: invert the output
        top_blank: add a blank row at top

    Returns:
        Generator of formatted lines
    """
    BLOCKS = " ▄▀█" if invert else "█▀▄ "

    size = code.get_size()

    def rows() -> Generator[Sequence[bool], None, None]:
        if top_blank:
            yield [not invert] * (size + (left + right))

        for __ in range(top):
            yield [False] * (size + (left + right))

        for y in range(size):
            yield [False] * left + [code.get_module(x, y) for x in range(size)] + [False] * right

        for __ in range(bottom):
            yield [False] * (size + (left + right))

        if (top + size + bottom) % 2 != top_blank:
            yield [not invert] * (size + (left + right))

    for r0, r1 in zip(*([rows()] * 2)):
        yield "".join(BLOCKS[c0 * 2 + c1] for c0, c1 in zip(r0, r1))


if __name__ == "__main__":
    from sys import argv

    qr = QrCode.encode_text(" ".join(argv[1:]), QrCode.Ecc.LOW)

    print()
    for line in encode_qr_utf8(qr):
        print("  " + line)
    print()
