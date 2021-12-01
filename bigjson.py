"""
Iteratively encode json in python, i.e. giant json structures can be created
with constant memory usage.

License: CC0
"""

import json
from collections.abc import Iterable, Iterator
from typing import TypeVar, Any


T = TypeVar("T")
U = TypeVar("U")
V = TypeVar("V")

class EmptyIter(Iterable[T]):
    def __init__(self, it: Iterable[T]):
        self._it = iter(it)
        self._iterated = False
        try:
            self._first = next(self._it)
            self._empty = False
        except StopIteration:
            self._empty = True

    def __len__(self) -> int:
        if self._iterated:
            raise Exception("Already iterated")
        return 0 if self._empty else 1

    def __bool__(self) -> bool:
        if self._iterated:
            raise Exception("Already iterated")
        return not self._empty

    def __iter__(self) -> Iterator[T]:
        if self._iterated:
            raise Exception("Already iterated")
        self._iterated = True
        if self._empty:
            return
        yield self._first
        yield from self._it



class IterList(EmptyIter[T], list[T]):
    pass

class IterDict(EmptyIter[tuple[U, V]], dict[Any, Any]):
    def items(self) -> Iterator[tuple[U, V]]:
        yield from self


def gen(it: Iterable[int]) -> Iterator[dict[str, int]]:
    for i in it:
        yield {
            "a": i**1,
            "b": i**2,
            "c": i**3,
        }

def gend(it: Iterable[int]) -> Iterator[tuple[int, Iterable[Iterator[dict[str, int]]]]]:
    for i in it:
        yield i, IterList(gen(range(i * i)))


#obj = IterList(gen(range(1000000)))
#obj = list(gen(range(1000000)))

obj = IterDict(gend(range(1000)))

for chunk in json.JSONEncoder().iterencode(obj):
    print(chunk, end="")
print()
