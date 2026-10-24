"""Regression checks for the packaging faults found during PS5 deployment."""
from pathlib import Path
import shutil, sys, tempfile, unittest
from validate_package import validate

class PackageChecks(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)/'PPSA99304'
        self.root.mkdir()
        # Only four files are needed to test generic title structure.
        for name in ['eboot.bin','sce_sys/param.json','sce_sys/icon0.png','sce_module/libc.prx']:
            target=self.root/name
            target.parent.mkdir(parents=True,exist_ok=True)
            if name.endswith('.bin'):
                target.write_bytes(b'\x4f\x15\x3d\x1d') # Header check only; native tool checks full SELF.
            elif name.endswith('.prx'):
                target.write_bytes(b'runtime')
            elif name.endswith('icon0.png'):
                shutil.copy2(Path(__file__).parent/'sce_sys/icon0.png',target)
            else:
                # Fixture matches staging metadata without requiring a prior dist build.
                import json
                target.write_text(json.dumps({'titleId':'PPSA99304','contentId':'UP9000-PPSA99304_00-BATTLESTARPS5000',
                    'localizedParameters':{'defaultLanguage':'en-US','en-US':{'titleName':'BattleStar'}}}))
    def check(self): validate(self.root,game_assets=())
    def test_valid(self): self.check()
    def test_missing_icon(self):
        (self.root/'sce_sys/icon0.png').unlink()
        with self.assertRaisesRegex(ValueError,'icon0.png'):self.check()
    def test_elf_renamed_as_eboot(self):
        (self.root/'eboot.bin').write_bytes(b'\x7fELF')
        with self.assertRaisesRegex(ValueError,'SELF'):self.check()
    def test_truncated_png(self):
        icon=self.root/'sce_sys/icon0.png';icon.write_bytes(icon.read_bytes()[:-7])
        with self.assertRaisesRegex(ValueError,'Truncated'):self.check()
    def test_wrong_folder(self):
        dest=self.root.with_name('PPSA99303');self.root.rename(dest);self.root=dest
        with self.assertRaisesRegex(ValueError,'Title ID'):self.check()

if __name__=='__main__':unittest.main(testRunner=unittest.TextTestRunner(stream=sys.stdout,verbosity=2))
