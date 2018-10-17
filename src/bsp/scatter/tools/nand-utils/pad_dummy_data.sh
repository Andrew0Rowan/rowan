#! /bin/sh

python -B pad_dummy_data.py ../build_imags/MBR_NAND_ECC 0x84000
python -B pad_dummy_data.py ../build_imags/lk_ecc.img 0x84000
python -B pad_dummy_data.py ../build_imags/boot_ecc.img 0x1080000
python -B pad_dummy_data.py ../build_imags/tz_ecc.img 0x42000

cat ../build_imags/MBR_NAND_ECC >> ../build_imags/burner.img
cat ../build_imags/lk_ecc.img >> ../build_imags/burner.img
cat ../build_imags/boot_ecc.img >> ../build_imags/burner.img
cat ../build_imags/tz_ecc.img >> ../build_imags/burner.img
