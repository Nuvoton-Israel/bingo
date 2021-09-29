// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton NPCM7xx Binary Image Generator:   Bingo
 *
 * This tool is a general purpose header builder
 * It is used to create a header descibed in an external
 * xml file.
 * To add changes to the header: update the external xml only.
 * Bingo can also be used to build an binary image from multiple sources
 * of data: binary files, arrays and const data.
 *
 * Copyright (C) 2018 Nuvoton Technologies, All Rights Reserved
 */

#include <string>
#include <cstring>
#include "error_correction.h"
#include "errors.h"
#include "bingo_types.h"

using namespace std;

// private functions for the ECC scope
UINT32 ECC_encodeMajorityRule(UINT8 *dataIn, UINT8 *dataOut, UINT32 size);
UINT32 ECC_encodeNibbleParity(UINT8 *dataIn, UINT8 *dataOut, UINT32 size);
UINT32 ECC_encode_SECDED_Parity(UINT8 *dataIn, UINT8 *dataOut, UINT32 size);
UINT32 ECC_encodeMajorityRule_10Bit(UINT8 *dataIn, UINT8 *dataOut, UINT32 size);


UINT32 ECC_encodeMajorityRule( UINT8 *dataIn, UINT8 *dataOut, UINT32 size )
{
	UINT32 err;
	UINT32 dataInSize = size / 3;
	// make sure given encoded buffer size is proper
	if (size % 3)
	{
		err = ERR_ECC_ERROR;
		string errStr = "Size not proper for Majority Rule ECC";
		ERR_PrintError(err, errStr);
		return err;
	}

	// copy two copies of dataIn into dataOut
	memcpy(&dataOut[0], dataIn, dataInSize);
	memcpy(&dataOut[dataInSize], dataIn, dataInSize);
	memcpy(&dataOut[dataInSize * 2], dataIn, dataInSize);

	// fill the rest of the buffer with zeros, in order to avoid false decoding
	return STS_OK;
}

UINT32 ECC_encodeMajorityRule_10Bit( UINT8 *dataIn, UINT8 *dataOut, UINT32 size )
{
	UINT32 err;
	// check that size is OK for 10 bits majority rule
	if (size != ECC_SIZE_FOR_10BIT_MAJORITY*2) // only four bytes are valid
	{
		err = ERR_ECC_ERROR;
		string errStr = "Size not proper for 10 bits Majority Rule ECC";
		ERR_PrintError(err, errStr);
		return err;
	}

	UINT16 inputRaw;  // contains a copy of the input data for easier handling
	UINT32 outputRaw = 0; // contains a copy of the output data for easier handling
	memcpy(&inputRaw, dataIn, sizeof(inputRaw));

	if (inputRaw & 0xFC00)
	{
		err = ERR_ECC_ERROR;
		string errStr = "10 bit majority field contains high bits at location above 10 bits";
		ERR_PrintError(err, errStr);
		return err;
	}
	inputRaw &= 0x3FF; // use only lower 10 bits

	// Encoding starts now.

	// define bit fields for the encoded output in order to use the SET_VAR_FIELD macro:
#define	LOWER_10_F  0 , 10
#define MIDDLE_10_F 10 , 10
#define UPPER_10_F  20 , 10

	SET_VAR_FIELD(outputRaw, LOWER_10_F, inputRaw);
	SET_VAR_FIELD(outputRaw, MIDDLE_10_F, inputRaw);
	SET_VAR_FIELD(outputRaw, UPPER_10_F, inputRaw);

	// end of encoding, copy data to output buffer
	memcpy(dataOut, &outputRaw, size);

	return STS_OK;
}

UINT32 ECC_encodeMaskNibbleParity(UINT8 *dataIn, UINT8 *dataOut, UINT32 size)
{	
	UINT32	i;
	for (i = 0; i < size; ++i)
	{
		dataOut[i] = (dataIn[0]);
	}
	return STS_OK;
}


UINT32 ECC_encodeNibbleParity( UINT8 *dataIn, UINT8 *dataOut, UINT32 size )
{
	UINT8	nibble, encData;
	UINT32	encIndex;

	UINT8 B0, B1, B2, B3;
	//Define the Bit Field macros in order to use the SET_VAR_FIELD macro:
#define BITF0   0, 1
#define BITF1   1, 1
#define BITF2   2, 1
#define BITF3   3, 1
#define BITF4   4, 1
#define BITF5   5, 1
#define BITF6   6, 1
#define BITF7   7, 1
	// define bit fields for nibbles
#define	LOWER_NIBBLE_F	0 , 4
#define HIGHER_NIBBLE_F 4 , 4

	for (encIndex = 0; encIndex < size; ++encIndex)
	{
		nibble = ((encIndex&1) == 0) ? (READ_VAR_FIELD(dataIn[encIndex/2], LOWER_NIBBLE_F)) : (READ_VAR_FIELD(dataIn[encIndex/2], HIGHER_NIBBLE_F));

		B0 = READ_VAR_BIT(nibble, 0);
		B1 = READ_VAR_BIT(nibble, 1);
		B2 = READ_VAR_BIT(nibble, 2);
		B3 = READ_VAR_BIT(nibble, 3);

		/* lower nibble of encoded data is equal to actual nibble */
		encData = nibble;

		/* higher nibble of encoded data is xored value of other bits */
		SET_VAR_FIELD(encData, BITF4, (B0^B1));
		SET_VAR_FIELD(encData, BITF5, (B2^B3));
		SET_VAR_FIELD(encData, BITF6, (B0^B2));
		SET_VAR_FIELD(encData, BITF7, (B1^B3));


		dataOut[encIndex] = encData;

	}
	return STS_OK;
}

//************************************
// Function:        FUSE_get_CRC   -       Calc CRC
// Parameter:       UINT8 * datain -       pointer to encoded data buffer (buffer size should be 2 x dataout)
// Parameter:       UINT32  size   -       size of encoded data
// Returns:         CRC
// Description:
//                  Calc the CRC according to hamming. CRC lower bit is the R1 (location 1), etc
//                  MSb is the parity (nidded for double error detection.  )
//************************************
static UINT8 FUSE_get_CRC(UINT8 *datain, UINT32 size)
{
	int i;
	UINT8 CRC; // hamming code, 7 bits
	UINT8 R1;
	UINT8 R2;
	UINT8 R4;
	UINT8 R8;
	UINT8 R16;
	UINT8 R32;
	UINT8 R64;
	UINT8 parity = 0;
	UINT8 *arr = datain;

#define BIT_A(n)  (READ_VAR_BIT(datain[(n-1)>>3], ((n-1) % 8)))

	R1 = BIT_A(1) ^ BIT_A(2) ^ BIT_A(4) ^ BIT_A(5) ^ BIT_A(7) ^ BIT_A(9) ^ BIT_A(11) ^
		BIT_A(12) ^ BIT_A(14) ^ BIT_A(16) ^ BIT_A(18) ^ BIT_A(20) ^
		BIT_A(22) ^ BIT_A(24) ^ BIT_A(26) ^ BIT_A(27) ^ BIT_A(29) ^ BIT_A(31) ^
		BIT_A(33) ^ BIT_A(35) ^ BIT_A(37) ^ BIT_A(39) ^ BIT_A(41) ^ BIT_A(43) ^
		BIT_A(45) ^ BIT_A(47) ^ BIT_A(49) ^ BIT_A(51) ^ BIT_A(53) ^ BIT_A(55) ^
		BIT_A(57) ^ BIT_A(58) ^ BIT_A(60) ^ BIT_A(62) ^ BIT_A(64);


	R2 = BIT_A(1) ^ BIT_A(3) ^ BIT_A(4) ^ BIT_A(6) ^ BIT_A(7) ^ BIT_A(10) ^ BIT_A(11) ^ BIT_A(13) ^ BIT_A(14) ^
		BIT_A(17) ^ BIT_A(18) ^ BIT_A(21) ^ BIT_A(22) ^ BIT_A(25) ^
		BIT_A(26) ^ BIT_A(28) ^ BIT_A(29) ^ BIT_A(32) ^ BIT_A(33) ^ BIT_A(36) ^ BIT_A(37) ^ BIT_A(40) ^
		BIT_A(41) ^ BIT_A(44) ^ BIT_A(45) ^ BIT_A(48) ^ BIT_A(49) ^ BIT_A(52) ^
		BIT_A(53) ^ BIT_A(56) ^ BIT_A(57) ^ BIT_A(59) ^ BIT_A(60) ^ BIT_A(63) ^ BIT_A(64);

	R4 = BIT_A(2) ^ BIT_A(3) ^ BIT_A(4) ^
		BIT_A(8) ^ BIT_A(9) ^ BIT_A(10) ^ BIT_A(11) ^
		BIT_A(15) ^ BIT_A(16) ^ BIT_A(17) ^ BIT_A(18) ^
		BIT_A(23) ^ BIT_A(24) ^ BIT_A(25) ^ BIT_A(26) ^
		BIT_A(30) ^ BIT_A(31) ^ BIT_A(32) ^ BIT_A(33) ^
		BIT_A(38) ^ BIT_A(39) ^ BIT_A(40) ^ BIT_A(41) ^
		BIT_A(46) ^ BIT_A(47) ^ BIT_A(48) ^ BIT_A(49) ^
		BIT_A(54) ^ BIT_A(55) ^ BIT_A(56) ^ BIT_A(57) ^
		BIT_A(61) ^ BIT_A(62) ^ BIT_A(63) ^ BIT_A(64);

	R8 = BIT_A(5) ^ BIT_A(6) ^ BIT_A(7) ^ BIT_A(8) ^ BIT_A(9) ^ BIT_A(10) ^ BIT_A(11) ^
		BIT_A(19) ^ BIT_A(20) ^ BIT_A(21) ^ BIT_A(22) ^ BIT_A(23) ^ BIT_A(24) ^ BIT_A(25) ^ BIT_A(26) ^
		BIT_A(34) ^ BIT_A(35) ^ BIT_A(36) ^ BIT_A(37) ^ BIT_A(38) ^ BIT_A(39) ^ BIT_A(40) ^ BIT_A(41) ^
		BIT_A(50) ^ BIT_A(51) ^ BIT_A(52) ^ BIT_A(53) ^ BIT_A(54) ^ BIT_A(55) ^ BIT_A(56) ^ BIT_A(57);

	R16 = BIT_A(12) ^ BIT_A(13) ^ BIT_A(14) ^ BIT_A(15) ^ BIT_A(16) ^ BIT_A(17) ^ BIT_A(18) ^ BIT_A(19) ^ BIT_A(20) ^ BIT_A(21) ^ BIT_A(22) ^ BIT_A(23) ^ BIT_A(24) ^ BIT_A(25) ^ BIT_A(26) ^
		BIT_A(42) ^ BIT_A(43) ^ BIT_A(44) ^ BIT_A(45) ^ BIT_A(46) ^ BIT_A(47) ^ BIT_A(48) ^ BIT_A(49) ^ BIT_A(50) ^ BIT_A(51) ^ BIT_A(52) ^ BIT_A(53) ^ BIT_A(54) ^ BIT_A(55) ^ BIT_A(56) ^ BIT_A(57);

	R32 = BIT_A(27) ^ BIT_A(28) ^ BIT_A(29) ^ BIT_A(30) ^ BIT_A(31) ^ BIT_A(32) ^ BIT_A(33) ^ BIT_A(34) ^ BIT_A(35) ^
		BIT_A(36) ^ BIT_A(37) ^ BIT_A(38) ^ BIT_A(39) ^
		BIT_A(40) ^ BIT_A(41) ^ BIT_A(42) ^ BIT_A(43) ^ BIT_A(44) ^ BIT_A(45) ^ BIT_A(46) ^ BIT_A(47) ^ BIT_A(48) ^
		BIT_A(49) ^ BIT_A(50) ^ BIT_A(51) ^ BIT_A(52) ^ BIT_A(53) ^
		BIT_A(54) ^ BIT_A(55) ^ BIT_A(56) ^ BIT_A(57);

	R64 = BIT_A(58) ^ BIT_A(59) ^ BIT_A(60) ^ BIT_A(61) ^ BIT_A(62) ^ BIT_A(63) ^ BIT_A(64);

	parity = 0;
	for (i = 1; i <= 64; i++)
	{
		parity ^= BIT_A(i);
	}


#undef BIT_A

	R1 = R1 & 0x01;
	R2 = R2 & 0x01;
	R4 = R4 & 0x01;
	R8 = R8 & 0x01;
	R16 = R16 & 0x01;
	R32 = R32 & 0x01;
	R64 = R64 & 0x01;

	CRC = (R1 << 0) + (R2 << 1) + (R4 << 2) + (R8 << 3) + (R16 << 4) + (R32 << 5) + (R64 << 6) + (parity << 7);

	return CRC;
}

//************************************
// Function:  ECC_encode_SECDED_Parity - execute ECC SECDED (hamming code). Add one byte of CRC for each 8 bytes.
//            Note: all CRCs are gathered at the end of the array.
// Returns:   UINT32 status according to errors.h
// Parameter: UINT8 * dataIn - input data buffer
// Parameter: UINT8 * dataOut - output data buffer
// Parameter: UINT32 size - size of the encoded data buffer
//************************************
UINT32 ECC_encode_SECDED_Parity(UINT8 *dataIn, UINT8 *dataOut, UINT32 size)
{
	UINT32 i = 0;
	UINT8 CRC; // hamming code, 7 bits + parity
	UINT8 *arr = dataIn;
	UINT32 cnt = 0;

	UINT32 encoded_size = DIV_CEILING(8 * size, 9);

	memcpy(dataOut, dataIn, encoded_size);

	for (cnt = 0; cnt < encoded_size; cnt += 8)
	{

		// each 64 bits (8 bytes) get a CRC byte at the end of the array
		arr = dataIn + cnt;
		CRC = FUSE_get_CRC(arr, MIN(encoded_size - cnt, 8));
		dataOut[encoded_size + i++] = CRC;
	}

	return STS_OK;
}

//************************************
// Function:  ECC_performECC - dispatcher for the differnt ECC schemes
// Returns:   UINT32 status according to errors.h
// Parameter: ECC_Type type
// Parameter: UINT8 * dataIn - input data buffer
// Parameter: UINT8 * dataOut - output data buffer
// Parameter: UINT32 size - size of the encoded data buffer
//************************************
UINT32 ECC_performECC(ECC_Type type, UINT8 *dataIn, UINT8 *dataOut, UINT32 size)
{
	UINT32 status;
	if (type == ECC_noECC)
	{
		memcpy(dataOut, dataIn, size);
		status = STS_OK;
	}
	else if (type == ECC_Mask_nibbleParity)
	{
		status = ECC_encodeMaskNibbleParity(dataIn, dataOut, size);
		status = STS_OK;
	}
	else if (type == ECC_nibbleParity)
	{
		status = ECC_encodeNibbleParity(dataIn, dataOut, size);
	}
	else if (type == ECC_majorityRule)
	{
		status = ECC_encodeMajorityRule(dataIn, dataOut, size);
	}
	else if (type == ECC_10BitsMajorityRule)
	{
		status = ECC_encodeMajorityRule_10Bit(dataIn, dataOut, size);
	}
	else if (type == ECC_SECDED)
	{
		status = ECC_encode_SECDED_Parity(dataIn, dataOut, size);
	}
	else
	{
		status = ERR_NOT_IMPLEMENTED;
		ERR_PrintError(status, "requested ECC scheme is not implemented\n");
	}
	return status;
}


