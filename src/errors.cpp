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

#define ERRORS_C

#include <iostream>
#include <string>
#include "errors.h"
#include "bingo_types.h"


using namespace std;

UINT32  ERR_FillErrorDetails(UINT32 err, UINT32 lineNum, std::string &line)
{
	cout<<"Error number: "<<err<<", line number: "<<lineNum<<", line: "<<line<<endl;
	return STS_OK;
}

//Prints the specified message and error specific announcment
UINT32  ERR_PrintError(UINT32 err, const string msg)
{
	string errMsg;

	switch (err)
	{
	case ERR_OPEN_FILE:
		errMsg = "Could not open input file";
		break;
	case ERR_PARSING:
		errMsg = "Parsing error";
		break;
	case ERR_ILLEGAL_VAL:
		errMsg = "Illegal Value";
		break;
	case ERR_AMBIGUITY:
		errMsg = "Ambiguous input";
		break;
	case ERR_OUTPUT_FILE:
		errMsg = "Could not create output file";
		break;
	case ERR_NOT_IMPLEMENTED:
		errMsg = "Command not implemented";
		break;
	case ERR_BAD_IMAGE_SIZE:
		errMsg = "Binary image size does not match calculated size by fields";
		break;
	case ERR_BAD_FIELD_SIZE:
		errMsg = "Binary field image size does not match actual data size";
		break;
	case ERR_SAME_FIELD_TWICE:
		errMsg = "Field encountered twice";
		break;
	case ERR_ILLEGAL_FIELD:
		errMsg = "Unknown XML Field";
		break;
	case ERR_UNKNOWN_ATTR:
		errMsg = "Unknown Field Attribute";
		break;
	case ERR_FILE_NOT_FOUND:
		errMsg = "File not found";
		break;
	case ERR_FILE_ERROR:
		errMsg = "File handling error";
		break;
	case ERR_FIELD_OVERLAP:
		errMsg = "Fields overlap detected";
		break;
	case ERR_IMAGE_TOO_LARGE:
		errMsg = "Image exceeded limit size";
		break;
	case ERR_ECC_ERROR:
		errMsg = "ECC error";
		break;
	case ERR_CMD_LINE_ERR:
		errMsg = "Command line error";
		break;

	default:
		errMsg = "Unrecognized error: ";
	}


	cout<<errMsg<<endl<<msg<<endl;

	return STS_OK;
}
