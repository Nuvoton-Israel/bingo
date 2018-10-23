@echo off
color 7

set BINGO=..\deliverables\windows\bingo.exe

%BINGO% BootBlockHeader.xml  -o output_binaries\BootBlockHeader.bin || goto FAILED
signIt.exe   -i output_binaries\BootBlockHeader.bin --in_start_offset 0x140 --keyform raw --private_key_file input_binaries\rsa_private_key_0.bin --modulu_file input_binaries\rsa_public_key_0.bin --signature_offset 0x8 --ebmed_signature || goto FAILED
%BINGO% ubootHeader.xml      -o output_binaries\ubootHeader.bin || goto FAILED
signIt.exe   -i output_binaries\ubootHeader.bin   --in_start_offset 0x140 --keyform raw --private_key_file input_binaries\rsa_private_key_0.bin --modulu_file input_binaries\rsa_public_key_0.bin --signature_offset 0x8 --ebmed_signature || goto FAILED
%BINGO% mergeBootHeaders.xml -o output_binaries\mergedBootHeaders.bin || goto FAILED
%BINGO% poleg_spi_image.xml   -o output_binaries\poleg_spi_image.bin || goto FAILED

%BINGO% poleg_key_map.xml     -o output_binaries\poleg_key_map.bin || goto FAILED
%BINGO% poleg_fuse_map.xml    -o output_binaries\poleg_fuse_map.bin || goto FAILED


rem *********************************
rem Post result + Gather deliverables
rem *********************************
:PASSED
color A
SET result=Pass
@echo ============================== PASS ===============================
@echo OK building binaries
@echo ====================================================================
GOTO END

:FAILED
color C
@echo ============================== ERROR ===============================
@echo Error building binaries
@echo ====================================================================
SET result=Fail

:END
ECHO %result% > result.txt
