from __future__ import annotations

import logging
import unittest
from typing import TYPE_CHECKING

from hvdaccelerators import stuff

if TYPE_CHECKING:
    pass

log = logging.getLogger(__name__)
log.setLevel(logging.WARNING)
logging.basicConfig()


class TestSmokeTest(unittest.TestCase):
    def setUp(self):
        pass

    def test_smoke_test(self):
        result = stuff.add(1, 2)
        self.assertEqual(result, 3)

        result = stuff.hamming_distance(
            "1111111111111111111111111111111111111111111111111111111111111110",
            "0000000000000000000000000000000000000000000000000000000000000000",
        )
        self.assertEqual(result, 63)


if __name__ == "__main__":
    unittest.main(module="test_smoketest")
