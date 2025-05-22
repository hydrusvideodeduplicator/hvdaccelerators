from __future__ import annotations

import logging
import unittest
from io import BytesIO
from pathlib import Path

from hvdaccelerators import vpdq
from PIL import Image

log = logging.getLogger(__name__)
log.setLevel(logging.WARNING)
logging.basicConfig()


class TestSmokeTest(unittest.TestCase):
    def setUp(self):
        pass

    def test_smoke_test(self):
        result = vpdq.hamming_distance(
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
            result = vpdq.hash_frame(im.tobytes(), im.width, im.height)
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
        # Run multiple times to try and catch any concurrency issues.
        for _ in range(10):
            with open(Path(__file__).parent / "test.png", "rb") as f:
                data = f.read()
                im = Image.open(BytesIO(data))
                im.thumbnail((512, 512))
                im.convert("RGB")

                log.error("Start")
                thread_count = 0  # auto
                hasher = vpdq.VideoHasher(60, im.width, im.height, thread_count)
                # Hash a bunch of frames, simulating a video.
                for _ in range(100):
                    hasher.hash_frame(im.tobytes())
                log.error("Done")
                result = hasher.finish()
                for frame in result:
                    self.assertEqual(
                        frame.get_hash(),
                        "c495c53700955a3d86c257c2cddbd3ea5be02547e697a5ead2951a9702b5861f",
                    )


if __name__ == "__main__":
    unittest.main(module="test_smoketest")
