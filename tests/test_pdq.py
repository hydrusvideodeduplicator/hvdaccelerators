from __future__ import annotations

import logging
import unittest

from hvdaccelerators import vpdq

log = logging.getLogger(__name__)
log.setLevel(logging.WARNING)
logging.basicConfig()

PDQ_HASH = "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff163"


class TestPdq(unittest.TestCase):
    def test_hexString(self):
        hex_str = PDQ_HASH
        pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
        self.assertEqual(pdq_hash.toHexString(), hex_str)

    def test_badHexString(self):
        bad_hex_strings = [
            "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff1631",  # too long by 1 char
            "09c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff1631",  # too long by 2 chars
            "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff16",  # too short by 1 char
            "9c151c3af838278e3ef57c180c7d031c07aefd12f2ccc1e18f2a1e1c7d0ff16",  # too short by 2 chars
            "",  # empty
        ]
        for hex_str in bad_hex_strings:
            with self.assertRaises(ValueError):
                vpdq.PdqHash256.fromHexString(hex_str)

    def test___str__(self):
        hex_str = PDQ_HASH
        pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
        self.assertEqual(str(pdq_hash), hex_str)

    def test___repr__(self):
        hex_str = PDQ_HASH
        pdq_hash = vpdq.PdqHash256.fromHexString(hex_str)
        self.assertEqual(repr(pdq_hash), hex_str)


if __name__ == "__main__":
    unittest.main(module="test_pdq")
