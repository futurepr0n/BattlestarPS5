"""Check reproducibility, headroom, click-free boundaries and held-fire overlap."""
import math, unittest
from synthesize_viper import RATE,GAIN,volley,encode,firing_preview

class WeaponAudioChecks(unittest.TestCase):
    @classmethod
    def setUpClass(cls):cls.clips=[volley(i) for i in range(3)]
    def test_format_duration_and_headroom(self):
        for samples in self.clips:
            self.assertEqual(len(samples)%2,0)
            self.assertGreater(len(samples)/2/RATE,.18)
            self.assertLess(len(samples)/2/RATE,.24)
            self.assertTrue(all(math.isfinite(x) for x in samples))
            self.assertAlmostEqual(max(abs(x) for x in samples),.82,places=5)
            self.assertLess(abs(sum(samples)/len(samples)),.002)
    def test_boundaries_and_short_tail(self):
        for samples in self.clips:
            self.assertEqual(samples[:2],[0.,0.])
            self.assertEqual(samples[-2:],[0.,0.])
            tail=samples[-round(RATE*.04)*2:]
            self.assertLess(math.sqrt(sum(x*x for x in tail)/len(tail)),.01)
            peak=max(range(len(samples)),key=lambda i:abs(samples[i]))/2/RATE
            self.assertLess(peak,.045)
    def test_variation_and_reproducibility(self):
        self.assertEqual(encode(self.clips[0]),encode(volley(0)))
        self.assertEqual(len({encode(c) for c in self.clips}),3)
        rms=[math.sqrt(sum(x*x for x in c)/len(c)) for c in self.clips]
        self.assertLess(max(rms)/min(rms),1.3)
    def test_normal_and_rapid_bursts_leave_mix_headroom(self):
        preview=firing_preview(self.clips)
        self.assertTrue(all(math.isfinite(x) for x in preview))
        self.assertLess(max(abs(x) for x in preview),.55)
        self.assertLess(GAIN,.6)

if __name__=='__main__':
    import sys
    unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout,verbosity=2))
