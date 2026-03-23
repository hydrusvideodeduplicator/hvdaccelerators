"""
hvdaccelerators contains C++ implementations of Hydrus Video Deduplicator algorithms to improve performance.
"""
from __future__ import annotations
import typing
__all__: list[str] = ['PdqHash256', 'VideoHasher', 'VpdqHash', 'hammingDistanceStrings', 'hamming_distance', 'hash_frame', 'matchHash', 'matchHashBytes', 'vpdqFeature']
class PdqHash256:
    HASH256_HEX_NUM_NYBBLES: typing.ClassVar[int] = 64
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def fromHexString(self: str) -> PdqHash256:
        ...
    def hammingDistanceLE(self, arg0: PdqHash256, arg1: typing.SupportsFloat | typing.SupportsIndex) -> bool:
        ...
    def toHexString(self) -> str:
        ...
class VideoHasher:
    @typing.overload
    def __init__(self, arg0: typing.SupportsFloat | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex, arg2: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: typing.SupportsFloat | typing.SupportsIndex, arg1: typing.SupportsInt | typing.SupportsIndex, arg2: typing.SupportsInt | typing.SupportsIndex, arg3: typing.SupportsInt | typing.SupportsIndex) -> None:
        ...
    def finish(self) -> VpdqHash:
        ...
    def hash_frame(self, arg0: bytes) -> None:
        ...
class VpdqHash:
    __hash__: typing.ClassVar[None] = None
    bytesPerPdqHash: typing.ClassVar[int] = 32
    bytes: bytes
    def __eq__(self, arg0: VpdqHash) -> bool:
        ...
    def __init__(self) -> None:
        ...
    def __len__(self) -> int:
        ...
    def __ne__(self, arg0: VpdqHash) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def empty(self) -> bool:
        ...
    def from_bytes(self: str) -> VpdqHash:
        ...
    def from_string(self: str) -> VpdqHash:
        ...
    @property
    def pdqHashes(self) -> str:
        ...
class vpdqFeature:
    def __init__(self) -> None:
        ...
    def __repr__(self) -> str:
        ...
    def __str__(self) -> str:
        ...
    def from_str(self: str) -> vpdqFeature:
        ...
    def get_hash(self) -> str:
        ...
    def to_string(self) -> str:
        ...
    @property
    def pdqHash(self) -> PdqHash256:
        ...
def hammingDistanceStrings(arg0: str, arg1: str) -> int:
    """
    Calculate the hamming distance between two PDQ hashes.
    """
def hamming_distance(arg0: str, arg1: str) -> int:
    """
    Calculate the hamming distance between two PDQ hashes.
    """
def hash_frame(arg0: bytes, arg1: typing.SupportsInt | typing.SupportsIndex, arg2: typing.SupportsInt | typing.SupportsIndex) -> tuple[bytes, int]:
    """
    hash a frame
    """
def matchHash(arg0: VpdqHash, arg1: VpdqHash, arg2: typing.SupportsInt | typing.SupportsIndex) -> int:
    """
    Calculate the similarity between two VPDQ hashes.
    """
def matchHashBytes(arg0: bytes, arg1: bytes, arg2: typing.SupportsInt | typing.SupportsIndex) -> int:
    """
    Calculate the similarity between two VPDQ hashes (raw bytes).
    """
