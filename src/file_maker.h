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

#ifndef FILE_MAKER_H
#define FILE_MAKER_H
#include <string>
#include <vector>
#include "fields.h"


// FM=File Maker

bool   FM_binFieldSortFunctionHandler(Field_BinField *f1, Field_BinField *f2);

/*
	Validate the following parameters:
	1) fields do not overlap - 
	2) if fileSize is zero, it is calculated or returned, other wise, error is asserted
*/
UINT32 FM_ValidateFieldVector(std::vector<Field_BinField *> &fields, Field_ImageProperties &imageConfig);

/*
	Creates the binary image into a file
*/
UINT32 FM_CreateBinFile(std::vector<Field_BinField *> &fields, Field_ImageProperties &imageConfig, std::string fileName);

#endif // FILE_MAKER_H
