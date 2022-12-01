@REM ----------------------------------------------------------------
@REM ah.exe Option
@REM  -b or --board   : BoardName   (16 Bytes)
@REM  -m or --model   : ModelName   (16 Bytes)
@REM  -c or --cinfo   : BuildDate/CRC16/CRC32/CRC64   (16 Bytes - Current DeskTop Date and Time) - MD5/SHA1/MD6/SHA256
@REM  -v or --version : VersionName (16 Bytes)
@REM
@REM  * 각각 정해진 Bytes수 (16Byte*4) = 64 Byte 까지만 적용된다.
@REM --------------------------------------------------------------------------------------------------------
@REM
@REM --------------------------------------------------------------------------------------------------------
@REM --- Attach Header to binary ----
@REM
@REM ah.exe -b JF200S-RevA  --model STM32F302K8U6 --cinfo MD5      --input STM32.bin --output STM32.out  -v 1.8.3
ah.exe --board JF200S-RevA  --model STM32F302K8U6 --cinfo sha1      --input ah.bin  --output aho.sha1   -v 1.80.32
@REM ah.exe -b M200S-RevA   --model STM32F302K8U6 --cinfo sha224   --input ah.bin  --output aho.R30   -v 1.8.3
ah.exe --board M200S-RevA   --model STM32F302K8U6 --cinfo date   --input ah.bin  --output aho.date  -v 1.8.3
@REM ah.exe -b M200S-RevA   --model STM32F302K8U6 --cinfo CRC16  --input ah.bin  --output aho.R40  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo crc64  --input ah.bin  --output aho.c64  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo crc64isc  --input ah.bin  --output aho.isc  -v 1.8.3
@REM ah.exe -b M200S-RevA   --model STM32F302K8U6 --cinfo adler32  --input ah.bin  --output aho.R60  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo SHAKE128  --input ah.bin  --output aho.sh128up  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo shake128  --input ah.bin  --output aho.sh128lo  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo MD2  --input ah.bin  --output aho.md2  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo MD4  --input ah.bin  --output aho.md4  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo MD5  --input ah.bin  --output aho.md5  -v 1.8.3
ah.exe --board JF200S-RevA   --model STM32F302K8U6 --cinfo blake224  --input ah.bin  --output aho.blake224  -v 1.8.3
@REM
@REM --------------------------------------------------------------------------------------------------------
@REM
@REM
@REM --------------------------------------------------------------------------------------------------------
@REM ah.exe --detach 64  -i aho.R60 -o ah2.exe  
@REM --------------------------------------------------------------------------------------------------------
@REM
@REM
@REM
@REM=== Convert Intel Hex to Bin ======
@REM ah.exe -i micom_jf8.hex  -o MCU_JF8.bin  --intel
ah.exe -i hex/micom_SL.hex   -o MCU_SL_max.bin   -L --length a0000
ah.exe -i hex/micom_SL.hex   -o MCU_SL_640kB.bin   -L --length 640kB
@REM ah.exe -i koino.hex      -o MCU_koino.bin   -L --verbose 1
ah.exe -i hex/micom_917.hex   -o MCU_917ff_max.bin   --intel --length a0000  --verbose 1 
ah.exe -i hex/micom_917.hex   -o MCU_917ab_max.bin   --intel --length a0000  --padbyte ab 
@REM=== Convert motorolA Hex to Bin ======
ah.exe -i hex/mahind.hex     -o MCU_mahind.bin   --motorola   
ah.exe -i hex/romp.hex       -o MCU_romp_ab_max.bin  --motorola --length 10000 --padbyte ab   
ah.exe -i hex/romp.hex       -o MCU_romp_max.bin     --motorola --length 10000 --verbose 1
ah.exe -i hex/romp.hex       -o MCU_romp_cc_max_ALL.bin   --motorola --length 10000  --padbyte 0xcc --allpadarea
@REM
@REM
@REM== ah.exe -i micom_mnav_um_ExAmp-navi_9_2_2.hex      -o um_avn_ext_L7_60th.bin_INN  --intel --length a0000  --padbyte FF 
@REM== ah.exe -i micom_mnav_um_ExAmp-navi-AUO_9_2_2.hex  -o um_avn_ext_L7_60th.bin_AUO  --intel --length a0000  --padbyte ff 
@REM== ah.exe -i micom_mnav_um-navi_9_2_2.hex            -o um_avn_int_L7_60th.bin_INN  --intel --length a0000  --padbyte ff 
@REM== ah.exe -i micom_mnav_um-navi-AUO_9_2_2.hex        -o um_avn_int_L7_60th.bin_AUO  --intel --length a0000  --padbyte FF 
@REM
@REM 
@REM
@REM 
ah.exe -i hex/booter_standalone.rec  --output booter_standalone_len.bin --motorola --length cac00 --zeroforced 
ah.exe -i hex/booter_standalone.rec  --output booter_standalone_all.bin --motorola --length cac00 --zeroforced --padbyte ee 
ah.exe -i hex/booter_standalone.rec  --output booter_standalone_ee.bin --motorola --length cac00 --zeroforced --padbyte ee --allpadarea 

ah.exe -i hex/Aardvark_2_14_03.s19   --output Aardvark_2_14_03_ZERO.bin  --motorola --zeroforced 
ah.exe -i hex/Aardvark_2_14_03.s19   --output Aardvark_2_14_03_length.bin  --motorola --zeroforced  --length 30000
ah.exe -i hex/Aardvark_2_14_03.s19   --output Aardvark_2_14_03_192kb.bin  --motorola --zeroforced  --length 192kB

ah.exe -i hex/LAN3320KT_NaviBox_13E-PA19A.hex         -o LAN3320KT_NaviBox_13E.bin  --motorola 
ah.exe -i hex/PIC18LF46K22.hex    -o PIC18LF46K22.bin --intel 
ah.exe -i hex/PIC18LF46K22.hex    -o PIC18LF46K22_ZERO.bin --intel --zeroforced
ah.exe -i hex/PIC24FJ256GA106.X.production.hex -o PIC24FJ256GA106.bin --intel 
ah.exe -i hex/PLL32MHZ.hex  -o PLL32MHZ.bin  --motorola 
ah.exe -i hex/STM32.hex  -o STM32.bin --intel 
ah.exe -i hex/STM32.hex  -o STM32_ZERO.bin --intel --zeroforced
ah.exe -i hex/Aardvark_2_04_DR_LIB.s19 -o Aardvark_2_04_DR_LIB.bin   --motorola  --zeroforced
ah.exe -i hex/Aardvark_2_04_DR_LIB.s19 -o Aardvark_2_04_DR_LIB_145KB.bin   --motorola  --zeroforced --length 145kB --padbyte dd --allpadarea

@REM 
@REM 
@REM 
@REM -- SHA1 or MD5 hash ---
ah.exe --input $.bin$  --output  SHA1Sum.txt  --checksum md5   --verbose datesize
ah.exe --input $.zip  --append SHA1Sum.txt  --checksum md6   --verbose datesize
ah.exe --input $.zip  --append SHA1Sum.txt  --checksum SHA1  --verbose datesize
ah.exe --input @.zip  --append SHA1Sum.txt  --checksum SHA224  --verbose datesize
ah.exe --input $.bin  --append SHA1Sum.txt  --checksum SHA256  --verbose datesize
ah.exe --input @.bin  --append SHA1Sum.txt  --checksum SHA3-224  --verbose datesize
ah.exe --input @.bin  --append SHA1Sum.txt  --checksum SHA3-256  --verbose datesize
ah.exe --input @.bin  --append SHA1Sum.txt  --checksum SHA3-384  --verbose datesize
ah.exe --input @.bin  --append SHA1Sum.txt  --checksum SHA3-512  --verbose datesize
ah.exe --input @.bin  --append SHA1Sum.txt  --checksum shake128  --verbose datesize
ah.exe --input @.bin  --append SHA1Sum.txt  --checksum shake256  --verbose datesize
@REM
@REM 
@REM 
@REM --------------------------------------------------------------------------------------------------------
@REM -- merge files (Boot image & App image)
ah.exe --join 2a00 -i STM_mcu_Boot.bin  STM_mcu_302K8_App.bin  --output  MCU_TotalRun_ff.bin 
ah.exe --join 0x2a00 -i STM_mcu_Boot.bin  STM_mcu_302K8_App.bin  --output  MCU_TotalRun_AB.bin --padbyte ab
@REM
@REM --------------------------------------------------------------------------------------------------------
@REM 
@REM

@REM --- files encapsulation --------------------------------------------------------------------------------
ah.exe --model LGE-BLTNCAM-2.0 --version BLTNCAM-2.0 --merge ca72_bl3.rom  home-directory.ext4  tc-boot-tcc8053-dvrs.img  tcc8053-linux-dvrs-lpd4x322_sv1.0-tcc8053-dvrs.dtb  telechips-dvrs-image-tcc8053-dvrs.ext4 --output bltncam.lge
@REM --- files extract --------------------------------------------------------------------------------
ah.exe --extract bltncam.lge

PAUSE
