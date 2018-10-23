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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstring>
#include "error_correction.h"
#include "errors.h"
#include "file_maker.h"

using namespace std;

bool FM_binFieldSortFunctionHandler( Field_BinField *f1, Field_BinField *f2 )
{
	return ((f1->offset) < (f2->offset));
}

UINT32 FM_ValidateFieldVector( std::vector<Field_BinField *> &fields, Field_ImageProperties &imageConfig )
{
	UINT32 err;

	// make sure there is no field overlap
	UINT32 prevOffset = 0;
	UINT32 prevSize = 0;
	
	for (vector<Field_BinField *>::iterator it = fields.begin(); it != fields.end(); ++it)
	{
		// make sure no overlapping
		if ((prevOffset + prevSize) > (*it)->offset)
		{
			err = ERR_FIELD_OVERLAP;
			string errStr = "between fields " + (*it)->name + " and " + (*(it-1))->name;
			ERR_PrintError(err, errStr);
			return err;
		}

		prevOffset = (*it)->offset;
		prevSize = (*it)->size * ECC_sizeMultipliyer((*it)->eccType);

		// if the ECC is 10 bit majority make sure size is 2 bytes
		if (((*it)->eccType == ECC_10BitsMajorityRule) && ((*it)->size != ECC_SIZE_FOR_10BIT_MAJORITY) )
		{
			err = ERR_ILLEGAL_VAL;
			string errStr = (*it)->name + ": Field size for 10 bits majority ECC size is not valid";
			ERR_PrintError(err, errStr);
			return err;
		}
	}



	UINT32 calculateSize = prevOffset + prevSize;

	if (calculateSize < prevOffset)
	{
		// result overflowed
		err = ERR_ILLEGAL_VAL;
		string errStr = "calculated file size overflows (32bit)";
		ERR_PrintError(err, errStr);
		return err;
	}

	if (imageConfig.size == 0)
	{
		// calculate needed buffer/file size
		imageConfig.size = calculateSize;
	}
	else
	{
		// make sure that size does not reach limit
		if (calculateSize > imageConfig.size)
		{
			err = ERR_IMAGE_TOO_LARGE;
			stringstream errStr;
			errStr << "Limit according to XML: " << imageConfig.size <<  ", actual:"  << calculateSize;
			ERR_PrintError(err, errStr.str());
			return err;
		}
	}

	return STS_OK;
}

//************************************
// Function:  FM_CreateBinFile - creates the binary image, and writes it to the file
//								 this is done field by field.
// Returns:   UINT32
// Parameter: std::vector<Field_BinField * > & fields
// Parameter: Field_ImageProperties & imageConfig
// Parameter: string fileName
// Precondition: 
//		1) Fields are sorted by location in the array, with no overlaps. 
//		2) imageConfige.size is valid (i.e image is not smaller than all fields)
//		* notice: these preconditions are tested by FM_ValidateFieldVector
//************************************
UINT32 FM_CreateBinFile( std::vector<Field_BinField *> &fields, Field_ImageProperties &imageConfig, string fileName)
{
	UINT32 err = 0;
	// temporary buffer that will hold data to be passed to file stream
	UINT8 *tempBuff;

	// indicates current offset in image
	UINT32 currentOffset = 0;

	// open the output file for writing
	ofstream outFile (fileName.c_str(), ofstream::binary);
	if (!outFile.is_open())
	{
		err = ERR_FILE_ERROR;
		string errStr = "Error creating or opening file " + fileName;
		ERR_PrintError(err, errStr);
		return err;
	}
	
	// run through the fields, fill a temporary buffer encoded data, and then fill write data to the file
	for (vector<Field_BinField *>::iterator it = fields.begin(); it != fields.end(); ++it)
	{
		// if there is a space between the previous offset and the current one, fill it with padding
		if ((*it)->offset > currentOffset)
		{
			UINT32 paddingSize = (*it)->offset - currentOffset;
			tempBuff = new UINT8[paddingSize];
			memset(tempBuff, imageConfig.paddingValue, paddingSize);

			outFile.write((char *)tempBuff, paddingSize);
			if (!outFile.good())
			{
				err = ERR_FILE_ERROR;
				string errStr = "Error writing to file " + fileName;
				ERR_PrintError(err, errStr);
				break;
			}
			
			currentOffset += paddingSize;
			delete[] tempBuff;
		}

		if ((*it)->eccType == ECC_noECC)
		{
			//in this case the data stays intact, so copy the buffer directly from the field object
			// (this could save some time and memory when the field data content is large
			// and no ECC scheme is applied)
			outFile.write((char *)(*it)->dataBuffer, (*it)->size);
			if (!outFile.good())
			{
				err = ERR_FILE_ERROR;
				string errStr = "Error writing to file '" + fileName + "'. field name:" + (*it)->name;
				ERR_PrintError(err, errStr);
				break;
			}

			currentOffset += (*it)->size;
		} 
		else
		{
			// calculate post-encoding size
			UINT32 tempBuffSize = (*it)->size * ECC_sizeMultipliyer((*it)->eccType);

			// allocate buffer
			tempBuff = new UINT8[tempBuffSize];

			// fill buffer with padding data
			memset(tempBuff, imageConfig.paddingValue, tempBuffSize);
			
			// perform ECC
			err = ECC_performECC((*it)->eccType, (*it)->dataBuffer, tempBuff, tempBuffSize);
			if (err)
			{
				break;
			}

			// write buffer to file
			outFile.write((char *)tempBuff, tempBuffSize);
			if (!outFile.good())
			{
				err = ERR_FILE_ERROR;
				string errStr = "Error writing to file " + fileName;
				ERR_PrintError(err, errStr);
				break;
			}
			// release buffer and prepare for next vector
			currentOffset += tempBuffSize;
			delete[] tempBuff;
		}
	}

	// if everything so far was OK, and did not reach the end of the image, fill the rest of it with padding
	if ((err == STS_OK) && (currentOffset < imageConfig.size))
	{
		UINT32 tempBuffSize = imageConfig.size - currentOffset;
		tempBuff = new UINT8[tempBuffSize];
		memset(tempBuff, imageConfig.paddingValue, tempBuffSize);
		// write buffer to file
		outFile.write((char *)tempBuff, tempBuffSize);
		if (!outFile.good())
		{
			err = ERR_FILE_ERROR;
			string errStr = "Error writing to file " + fileName;
			ERR_PrintError(err, errStr);
		}
		delete[] tempBuff;

	}
	outFile.close();
	return err;

}

