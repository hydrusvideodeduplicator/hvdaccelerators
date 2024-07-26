from __future__ import annotations

import logging
import unittest
from pathlib import Path
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

    def test_hashing(self):
        from io import BytesIO

        from PIL import Image

        with open(Path(__file__).parent / "test.jpg", "rb") as f:
            data = f.read()
            im = Image.open(BytesIO(data))
            im.thumbnail((512, 512))
            im.convert("RGB")
            result = stuff.hash_frame(im.tobytes(), im.width, im.height)
            (pdq_hash, quality) = result
            pdq_hash = str(pdq_hash, encoding="utf-8")
            self.assertEqual(quality, 100)
            self.assertEqual(
                pdq_hash,
                "60f71d288ad588fd5722df82565df507a2fd0f2aa180a0ff5f8282d5a877750a",
            )
            print(result)


if __name__ == "__main__":
    unittest.main(module="test_smoketest")
