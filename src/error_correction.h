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

#ifndef ERROR_CORRECTION_H
#define ERROR_CORRECTION_H

#include "bingo_types.h"

const UINT32 ECC_SIZE_FOR_10BIT_MAJORITY = 2;
typedef enum ECC_Type
{
	ECC_noECC = 0,
	ECC_nibbleParity,
	ECC_majorityRule,
	ECC_10BitsMajorityRule
}ECC_Type;

inline UINT32 ECC_sizeMultipliyer(ECC_Type ecc)
{
	if (ecc == ECC_noECC)
	{
		return 1;
	} 
	else if (ecc == ECC_nibbleParity)
	{
		return 2;
	}
	else if (ecc == ECC_majorityRule)
	{
		return 3;
	} 
	else if (ecc == ECC_10BitsMajorityRule)
	{
		return 2;
	}
	// not suppose to get here
	return 0;
}


UINT32 ECC_performECC(ECC_Type type, UINT8 *dataIn, UINT8 *dataOut, UINT32 size);
#endif // ERROR_CORRECTION_H
