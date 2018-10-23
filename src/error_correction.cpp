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
		status = 0;
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
	else
	{
		status = ERR_NOT_IMPLEMENTED;
		ERR_PrintError(status, "requested ECC scheme is not implemented\n");
	}
	return status;
}


