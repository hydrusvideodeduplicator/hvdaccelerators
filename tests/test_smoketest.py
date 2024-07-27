from __future__ import annotations

import logging
import unittest
from io import BytesIO
from pathlib import Path

from PIL import Image

from hvdaccelerators import stuff

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
        with open(Path(__file__).parent / "test.png", "rb") as f:
            data = f.read()
            im = Image.open(BytesIO(data))
            im.thumbnail((512, 512))
            im.convert("RGB")
            result = stuff.hash_frame(im.tobytes(), im.width, im.height)
            (pdq_hash, quality) = result
            pdq_hash = str(pdq_hash, encoding="utf-8")
            self.assertEqual(quality, 100)

            # TODO: Why is this different than what vpdq produces?
            # self.assertEqual(len(pdq_hash), 64)
            # self.assertEqual(
            #    pdq_hash,
            #    "c495c53700955a3d86c257c2cddbd3ea5be02547e697a5ead2951a9702b5861f",
            # )
            # print(result)

    def test_hasher(self):
        with open(Path(__file__).parent / "test.png", "rb") as f:
            data = f.read()
            im = Image.open(BytesIO(data))
            im.thumbnail((512, 512))
            im.convert("RGB")

            log.error("Start")
            hasher = stuff.Hasher(60, im.width, im.height)
            for i in range(10):
                hasher.hash_frame(im.tobytes())
            log.error("Done")
            result = hasher.finish()
            self.assertEqual(
                result[1].get_hash(),
                "c495c53700955a3d86c257c2cddbd3ea5be02547e697a5ead2951a9702b5861f",
            )


if __name__ == "__main__":
    unittest.main(module="test_smoketest")
