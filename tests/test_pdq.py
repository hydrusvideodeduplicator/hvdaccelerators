from __future__ import annotations

import logging
import unittest
import binascii

from hvdaccelerators import vpdq

log = logging.getLogger(__name__)
log.setLevel(logging.WARNING)
logging.basicConfig()

PDQ_HASH = "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff163"


def hex_to_bin(hex_str):
    # Convert hex string to binary string, padded to full length
    scale = 16  # Hexadecimal
    num_of_bits = len(hex_str) * 4
    return bin(int(hex_str, scale))[2:].zfill(num_of_bits)


def hamming_distance_hex(hex1, hex2):
    if len(hex1) != len(hex2):
        raise ValueError("Hex strings must be of equal length")

    bin1 = hex_to_bin(hex1)
    bin2 = hex_to_bin(hex2)

    # Count differing bits
    return sum(b1 != b2 for b1, b2 in zip(bin1, bin2))


class TestPdq(unittest.TestCase):
    def test_hexString(self):
        hex_str = PDQ_HASH
        pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
        self.assertEqual(pdq_hash.toHexString(), hex_str)

    def test_badHexString(self):
        bad_hex_strings = [
            "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff16",  # too short by 1 char
            "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff1",  # too short by 2 chars
            "",  # empty
        ]
        for hex_str in bad_hex_strings:
            with self.assertRaises(ValueError):
                vpdq.PdqHash256.fromHexString(hex_str)

        too_long_hex_str = [
            "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff1631",  # too long by 1 char
            "09c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff1631",  # too long by 2 chars
        ]
        for hex_str in too_long_hex_str:
            # If too long, exception should not be raised. This is checked automatically by pytest.
            pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
            self.assertEqual(str(pdq_hash), hex_str[:64])

    def test___str__(self):
        hex_str = PDQ_HASH
        pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
        self.assertEqual(str(pdq_hash), hex_str)

    def test___repr__(self):
        hex_str = PDQ_HASH
        pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
        self.assertEqual(repr(pdq_hash), hex_str)

    def test_hamming_distance(self):
        expected = hamming_distance_hex(PDQ_HASH, "7981a3ca8eb538e3e3f2862dcc9a71a6a389ec32a2619bc6683cf663c9c45f83")
        actual = vpdq.hammingDistanceStrings(
            PDQ_HASH, "7981a3ca8eb538e3e3f2862dcc9a71a6a389ec32a2619bc6683cf663c9c45f83"
        )
        self.assertEqual(actual, expected)

    def test_string_conversion_round_trip(self):
        vpdqHexString = (
            PDQ_HASH + PDQ_HASH + "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff163" + PDQ_HASH
        )
        vpdqHash = vpdq.VpdqHash.from_string(vpdqHexString)
        self.assertEqual(str(vpdqHash), vpdqHexString)

    def test_to_bytes(self):
        vpdqHexString = "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff163"
        vpdqHash = vpdq.VpdqHash.from_string(vpdqHexString)
        self.assertEqual(type(vpdqHash.bytes), bytes)
        self.assertEqual(vpdqHexString, binascii.hexlify(vpdqHash.bytes).decode())

    def test_from_bytes(self):
        # Single frame
        vpdqHexString = "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff163"
        vpdqHash = vpdq.VpdqHash.from_bytes(binascii.unhexlify(vpdqHexString))
        self.assertEqual(vpdqHash, vpdq.VpdqHash.from_string(vpdqHexString))

        # Multiple frames
        vpdqHexString = (
            PDQ_HASH + PDQ_HASH + "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff163" + PDQ_HASH
        )
        vpdqHash = vpdq.VpdqHash.from_bytes(binascii.unhexlify(vpdqHexString))
        self.assertEqual(str(vpdqHash), vpdqHexString)


if __name__ == "__main__":
    unittest.main(module="test_pdq")
