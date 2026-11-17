#!/bin/sh
# SPDX-License-Identifier: GPL-2.0
#
# Nuvoton Binary Image Generator: Bingo
#
# Checks the digest formats (sha1, sha256, sha384, sha512) and FileChecksum,
# file_start_offset with them, reverse and word_swap, the field size check,
# and the mask pass of a field whose content is a file, against sha*sum and od.
#
# Usage: tests/test_digest.sh [path to bingo]   (default: the Linux build)
#
# Copyright (C) 2026 Nuvoton Technologies, All Rights Reserved

set -eu

ROOT=$(cd "$(dirname "$0")/.." && pwd)
BINGO=${1:-$ROOT/deliverables/linux/Release/bingo}
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cd "$WORK"
FAILED=0

check() {
	if [ "$2" = "$3" ]; then
		echo "PASS $1"
	else
		echo "FAIL $1: got $2, expected $3"
		FAILED=1
	fi
}

hex() {
	od -An -v -tx1 -j "$2" -N "$3" "$1" | tr -d ' \n'
}

# A 300-byte input: longer than one SHA-512 block, not a multiple of 4.
awk 'BEGIN { for (i = 0; i < 300; i++) printf "%c", (i * 7 + 3) % 256 }' > input.bin
tail -c +17 input.bin > input_from_16.bin

cat > digest.xml <<'XML'
<?xml version="1.0" encoding="UTF-8"?>
<Bin_Ecc_Map>
	<ImageProperties>
		<BinSize>0x200</BinSize>
		<PadValue>0xFF</PadValue>
	</ImageProperties>
	<BinField>
		<name>SHA1</name>
		<config><ecc>none</ecc><offset>0x000</offset><size>20</size></config>
		<content format='sha1'>input.bin</content>
	</BinField>
	<BinField>
		<name>SHA256</name>
		<config><ecc>none</ecc><offset>0x020</offset><size>32</size></config>
		<content format='sha256'>input.bin</content>
	</BinField>
	<BinField>
		<name>SHA384</name>
		<config><ecc>none</ecc><offset>0x040</offset><size>48</size></config>
		<content format='sha384'>input.bin</content>
	</BinField>
	<BinField>
		<name>SHA512</name>
		<config><ecc>none</ecc><offset>0x080</offset><size>64</size></config>
		<content format='sha512'>input.bin</content>
	</BinField>
	<BinField>
		<name>SHA384_FROM_16</name>
		<config><ecc>none</ecc><offset>0x0C0</offset><size>48</size></config>
		<content format='sha384' file_start_offset='16'>input.bin</content>
	</BinField>
	<BinField>
		<name>SHA384_WORD_SWAP</name>
		<config><ecc>none</ecc><offset>0x100</offset><size>48</size></config>
		<content format='sha384' word_swap='true'>input.bin</content>
	</BinField>
	<BinField>
		<name>SHA256_REVERSE</name>
		<config><ecc>none</ecc><offset>0x140</offset><size>32</size></config>
		<content format='sha256' reverse='true'>input.bin</content>
	</BinField>
	<BinField>
		<name>CHECKSUM</name>
		<config><ecc>none</ecc><offset>0x170</offset><size>4</size></config>
		<content format='FileChecksum'>input.bin</content>
	</BinField>
	<BinField>
		<name>CHECKSUM_FROM_16</name>
		<config><ecc>none</ecc><offset>0x174</offset><size>4</size></config>
		<content format='FileChecksum' file_start_offset='16'>input.bin</content>
	</BinField>
	<BinField>
		<name>BYTES_WORD_SWAP</name>
		<config><ecc>none</ecc><offset>0x160</offset><size>8</size></config>
		<content format='bytes' word_swap='true'>0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08</content>
	</BinField>
</Bin_Ecc_Map>
XML

"$BINGO" digest.xml -o digest.bin > bingo.log 2>&1 || { cat bingo.log; echo "FAIL bingo digest.xml"; exit 1; }

sha384=$(sha384sum input.bin | cut -d' ' -f1)
check sha1 "$(hex digest.bin 0 20)" "$(sha1sum input.bin | cut -d' ' -f1)"
check sha256 "$(hex digest.bin 32 32)" "$(sha256sum input.bin | cut -d' ' -f1)"
check sha384 "$(hex digest.bin 64 48)" "$sha384"
check sha512 "$(hex digest.bin 128 64)" "$(sha512sum input.bin | cut -d' ' -f1)"
check sha384_file_start_offset "$(hex digest.bin 192 48)" "$(sha384sum input_from_16.bin | cut -d' ' -f1)"
check sha384_word_swap "$(hex digest.bin 256 48)" \
	"$(echo "$sha384" | sed 's/\(..\)\(..\)\(..\)\(..\)/\4\3\2\1/g')"
check sha256_reverse "$(hex digest.bin 320 32)" \
	"$(sha256sum input.bin | cut -d' ' -f1 | sed 's/../& /g' | tr ' ' '\n' | grep . | tac | tr -d '\n')"
check bytes_word_swap "$(hex digest.bin 352 8)" "0403020108070605"
check padding "$(hex digest.bin 20 12)" "ffffffffffffffffffffffff"

# Two's complement of the 32-bit byte sum, little endian.
checksum() {
	od -An -v -tu1 "$1" | awk '{ for (i = 1; i <= NF; i++) s += $i }
		END { c = (4294967296 - s % 4294967296) % 4294967296
		      printf "%02x%02x%02x%02x", c % 256, int(c / 256) % 256, int(c / 65536) % 256, int(c / 16777216) }'
}
check checksum "$(hex digest.bin 368 4)" "$(checksum input.bin)"
check checksum_file_start_offset "$(hex digest.bin 372 4)" "$(checksum input_from_16.bin)"

# A digest needs a field of exactly the digest size.
cat > wrong_size.xml <<'XML'
<?xml version="1.0" encoding="UTF-8"?>
<Bin_Ecc_Map>
	<ImageProperties><BinSize>0x40</BinSize></ImageProperties>
	<BinField>
		<name>SHA384_IN_32</name>
		<config><ecc>none</ecc><offset>0</offset><size>32</size></config>
		<content format='sha384'>input.bin</content>
	</BinField>
</Bin_Ecc_Map>
XML
"$BINGO" wrong_size.xml -o wrong_size.bin > wrong_size.log 2>&1 || true
if grep -q "not the 48-byte digest size" wrong_size.log; then
	echo "PASS size_check"
else
	cat wrong_size.log
	echo "FAIL size_check: no digest size error"
	FAILED=1
fi

# Mask pass: a masked file field is selected, an unmasked one is not.
cat > mask.xml <<'XML'
<?xml version="1.0" encoding="UTF-8"?>
<Bin_Ecc_Map>
	<ImageProperties><BinSize>0x80</BinSize></ImageProperties>
	<BinField>
		<name>MASKED_DIGEST</name>
		<config><ecc>none</ecc><offset>0x00</offset><size>48</size></config>
		<content format='sha384'>input.bin</content>
		<mask format='fillField'>0xff</mask>
	</BinField>
	<BinField>
		<name>UNMASKED_DIGEST</name>
		<config><ecc>none</ecc><offset>0x30</offset><size>48</size></config>
		<content format='sha384'>input.bin</content>
	</BinField>
	<BinField>
		<name>UNMASKED_FILE</name>
		<config><ecc>none</ecc><offset>0x60</offset><size>16</size></config>
		<content format='FileContent'>input.bin</content>
	</BinField>
</Bin_Ecc_Map>
XML
"$BINGO" mask.xml -o mask.bin -mask > mask.log 2>&1 || { cat mask.log; echo "FAIL bingo -mask mask.xml"; exit 1; }
ones=$(printf 'ff%.0s' $(seq 48))
zeros=$(printf '00%.0s' $(seq 48))
check mask_masked_digest "$(hex mask.bin 0 48)" "$ones"
check mask_unmasked_digest "$(hex mask.bin 48 48)" "$zeros"
check mask_unmasked_file "$(hex mask.bin 96 16)" "$(printf '00%.0s' $(seq 16))"

exit $FAILED
