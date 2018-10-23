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
 
#include <iostream>       
#include <sstream>
#include <cstring> //for memset
#include "errors.h"
#include "utilities.h"
#include "fields.h"

using namespace std;


template <class UINT_T> 
UINT32 GetIntegerFromString(string str, UINT_T &val)
{
	try
	{
		val = (UINT_T) std::stoll(str,0,0); // auto-detect base
	}
	catch (std::exception& e)
	{
		std::cerr << "std exception: " << e.what() << '\n';
		return ERR_ILLEGAL_VAL;
	}

	return STS_OK;
}

/*
	Generic numeric string parser, for fixed UINT output
*/
template <class UINT_T> 
UINT32 HandleNumericValueString(string str, UINT_T &val, const Field_Attributes &attributes)
{
	
	UINT32 err;
	val = 0;
	// if the input is given in hex, relate to the string as raw data
	if (attributes.format_id == Field_Attributes::attr_bytes)
	{	
		// split to bytes (incase string has spaces)
		vector<string> str_vals = split(str, ' ');
			
		// if size does not coincide with val size
		if (sizeof(val) < str_vals.size())
		{
			// assert warning/error
			ERR_PrintError(ERR_ILLEGAL_VAL, "Field value size larger than expected, taking only first bytes");
			
			//return(ERR_ILLEGAL_VAL);
		}
		else
		{
			// little endian, lowest byte located at the first address
			int i = 0;
			for (vector<string>::iterator it = str_vals.begin(); it != str_vals.end(); ++it)
			{
				UINT_T tempVal;
				err = GetIntegerFromString(*it, tempVal);
				if (err)
				{
					return err;
				}
				val |= (tempVal << i*8);
				i++;
				// in case the size of the data larger than the input type, take only first bytes
				if (i >= sizeof(val))
				{
					break;
				}
			}
		}
	}
	else if (attributes.format_id == Field_Attributes::attr_32bit)
	{
		err = GetIntegerFromString(str, val);
		if (err)
		{
			return err;
		}
	}	
	else if (attributes.format_id == Field_Attributes::attr_FileSize)
	{
		// in this case str contains a path to a file, and val should be the size of it
		UINT32 fileSize;
		UINT32 maxSize;
	
		maxSize = (UINT_T) val - 1;
		err = getFileSize(str.c_str(), fileSize);
		if (err)
		{
			return err;
		}
		if (fileSize > maxSize) // overflow occured
		{
			string errStr = "File Size too large for the size of the variable, taking lower bytes";
			ERR_PrintError(ERR_ILLEGAL_VAL, errStr);
			//return ERR_ILLEGAL_VAL;
			val = (UINT_T) fileSize;
		}
		else
		{
			val = (UINT_T) fileSize;
		}
	}
	else if (attributes.format_id == Field_Attributes::attr_FileContent)
	{
		// in this case str contains a path to a file, and val should be the content of it (only those who gets in the var type)
		ifstream infile;
		// try to open the file
		infile.open(str.c_str(), ios::binary);
		if (!infile.is_open())
		{
			string errStr = "Filename: " + str;
			ERR_PrintError(ERR_FILE_NOT_FOUND, errStr);
			return ERR_FILE_NOT_FOUND;
		}
		// go to the specified offset
		if (attributes.fileStartOffset)
		{
			infile.seekg(attributes.fileStartOffset);

			// if the offset does not exist, assert an error
			if(!infile.good())
			{
				string errString = "offset not found in file";
				ERR_PrintError(ERR_FILE_ERROR, errString);
				return ERR_FILE_ERROR;
			}
		}
				

		// little endian, lowest byte located at the first address
		UINT32 tempVal;
		for (int i = 0; i < sizeof(val) ; ++i)
		{
			tempVal = (UINT8) infile.get();
			if (infile.good())
			{
				val |= (UINT_T) (tempVal << i*8);
			}
			else
			{
				string errString = "reached end of file prematurely";
				ERR_PrintError(ERR_FILE_ERROR, errString);
				return ERR_FILE_ERROR;
			}

		}
		
		// close file
		infile.close();
	}

	// perform alignment on val data according to attributes
	if (attributes.alignment)
	{
		val = ALIGN(val, (UINT_T) attributes.alignment);
	}

	return STS_OK;

}

/*
	dedicated numeric string parser, for buffers output
*/
UINT32 HandleNumericValueString(std::string str, UINT8 * &buff, UINT32 buffSize, const Field_Attributes &attributes, UINT8 padValue=0)
{
	UINT32 err;
	
	if (buffSize == 0)
	{
		err = ERR_ILLEGAL_VAL;
		string errStr = "Element size can not be  0";
		ERR_PrintError(err, errStr);
		return err;
	}
	buff = new UINT8[buffSize];
	memset(buff, padValue, buffSize);

	// if the input is given in hex, relate to the string as raw data
	if (attributes.format_id == Field_Attributes::attr_bytes)
	{	
		// split to bytes (in case string has spaces)
		vector<string> str_vals = split(str, ' ');

		// if size does not coincide with val size
		if (buffSize < str_vals.size())
		{
			// assert warning/error
			ERR_PrintError(ERR_ILLEGAL_VAL, "Field value size larger than expected, taking only first bytes");
			
			// stop the parser
			return(ERR_ILLEGAL_VAL);
		}
		else
		{
			// little endian, lowest byte located at the first address
			UINT32 i = 0;
			for (vector<string>::iterator it = str_vals.begin(); it != str_vals.end(); ++it)
			{
				UINT8 tempVal;
				err = GetIntegerFromString(*it, tempVal);
				if (err)
				{
					return err;
				}
				buff[i] = tempVal;
				i++;
				// in case the size of the data larger than the input type, take only first bytes
				if (i >= buffSize)
				{
					break;
				}
			}
		}
	}
	else if (attributes.format_id == Field_Attributes::attr_32bit)
	{
		UINT32 tempVal;
		err = GetIntegerFromString(str, tempVal);
		if (err)
		{
			return err;
		}
		// make sure tempVal is not bigger that the buffer
		if (buffSize < 4)
		{
			if ((tempVal>>(8*buffSize)) != 0)
			{
				err = ERR_ILLEGAL_VAL;
				ostringstream errStrStream;
				errStrStream << "given value: " << hex << tempVal << " larger than field size: " << buffSize;
				ERR_PrintError(err, errStrStream.str());
				return err;
			}
			memcpy(buff, &tempVal, buffSize);
		}
		else
		{
			memcpy(buff, &tempVal, sizeof(tempVal));
		}
		
	}	
	else if (attributes.format_id == Field_Attributes::attr_FileSize)
	{
		// in this case str contains a path to a file, and val should be the size of it
		UINT32 fileSize;
		UINT32 maxSize;

		if (buffSize > 4)
		{
			err = ERR_ILLEGAL_VAL;
			string errStr = "when using the FileSize attribute, maximum field size should not exceed 4 bytes";
			ERR_PrintError(ERR_ILLEGAL_VAL, errStr);
		}
		else if (buffSize == 4)
		{
			maxSize = 0xFFFFFFFF;
		}
		else
		{
			maxSize = (UINT32) (1<<(buffSize*8)) - 1;
		}

		err = getFileSize(str.c_str(), fileSize);
		if (err)
		{
			return err;
		}
		if (fileSize > maxSize) // overflow occured
		{
			string errStr = "File Size too large for the size buffer, taking lower bytes";
			ERR_PrintError(ERR_ILLEGAL_VAL, errStr);
			//return ERR_ILLEGAL_VAL;
			memcpy(buff, &fileSize, buffSize);
		}
		else
		{
			memcpy(buff, &fileSize, buffSize);
		}
	}
	else if (attributes.format_id == Field_Attributes::attr_FileContent)
	{
		// in this case str contains a path to a file, and val should be the content of it (only those who gets in the var type)
		ifstream infile;
		// try to open the file
		infile.open(str.c_str(), ios::binary);
		if (!infile.is_open())
		{
			string errStr = "Filename: " + str;
			ERR_PrintError(ERR_FILE_NOT_FOUND, errStr);
			return ERR_FILE_NOT_FOUND;
		}
		// go to the specified offset
		if (attributes.fileStartOffset)
		{
			infile.seekg(attributes.fileStartOffset);

			// if the offset does not exist, assert an error
			if(!infile.good())
			{
				string errString = "offset not found in file";
				ERR_PrintError(ERR_FILE_ERROR, errString);
				return ERR_FILE_ERROR;
			}
		}


		// little endian, lowest byte located at the first address
		UINT8 tempVal;
		for (UINT32 i = 0; i < buffSize ; ++i)
		{
			tempVal = (UINT8) infile.get();
			if (infile.good())
			{
				buff[i] = tempVal;
			}
			else
			{
				string errString = "reached end of file prematurely";
				ERR_PrintError(ERR_FILE_ERROR, errString);
				return ERR_FILE_ERROR;
			}

		}

		// close file
		infile.close();
	}

	// reverse buffer in case 
	if (attributes.reversed)
	{
		for (UINT32 i = 0; i < (UINT32)buffSize/2; ++i) 
		{
			UINT8 temp = buff[buffSize-i-1];
			buff[buffSize-i-1] = buff[i];
			buff[i] = temp;
		}
	}
	// perform alignment on val data according to attributes
	if (attributes.alignment)
	{
		if (buffSize > 4)
		{
			err = ERR_ILLEGAL_VAL;
			string errStr = "can not align values which are larger than 4 bytes";
			ERR_PrintError(err, errStr);
			return err;
		}
		
		UINT32 tempVal = 0;
		memcpy(&tempVal, buff, buffSize);
		tempVal = ALIGN(tempVal,attributes.alignment);

		memcpy(buff, &tempVal, buffSize);
		
	}

	return STS_OK;
}


const std::string Field_BinField::descriptor = "BinField";
const std::string Field_BinField::validConfigurationStrings[NUM_OF_VALID_CONFIGS] = {"ecc", "offset", "size"};


/*
The Binary Field
*/

Field_BinField::Field_BinField()
{
	this->dataBuffer = nullptr;
	this->name = "";
	this->eccType = ECC_noECC;
	this->offset = 0;
	this->size = 0;
}

Field_BinField::~Field_BinField()
{
	delete[] dataBuffer;
}

UINT32 Field_BinField::setConfiguration( std::string configurationString, std::string valueString, const Field_Attributes &attributes )
{
	UINT32 err;
	for (UINT32 selectedConfig = 0; selectedConfig < NUM_OF_VALID_CONFIGS ; selectedConfig++)
	{
		if (configurationString == validConfigurationStrings[selectedConfig])
		{
			if (selectedConfig == configEcc)
			{
				// Configure ECC method, no attributes supported

				// make sure the configuration did not appear twice
				if (this->eccType != ECC_noECC)
				{
					ERR_PrintError(ERR_SAME_FIELD_TWICE, configurationString);
					return ERR_SAME_FIELD_TWICE;
				}

				if (valueString == "none")
				{
					this->eccType = ECC_noECC;
				}
				else if (valueString == "majority")
				{
					this->eccType = ECC_majorityRule;
				}
				else if (valueString == "nibble")
				{
					this->eccType = ECC_nibbleParity;
				}
				else if (valueString == "10_bits_majority")
				{
					this->eccType = ECC_10BitsMajorityRule;
				}
				else
				{
					ERR_PrintError(ERR_NOT_IMPLEMENTED, valueString);
					return ERR_NOT_IMPLEMENTED;
				}
			}
			else if (selectedConfig == configOffset)
			{
				// configure start offset of the field inside the binary image
				// make sure field was not encountered twice
				if (this->offset != 0)
				{
					ERR_PrintError(ERR_SAME_FIELD_TWICE, configurationString);
					return ERR_SAME_FIELD_TWICE;
				}

				err = HandleNumericValueString(valueString, this->offset, attributes); 
				if (err)
				{
					ERR_PrintError(err, valueString);
					return err; 
				}

			}
			else if (selectedConfig == configSize)
			{
				if (this->size != 0)
				{
					ERR_PrintError(ERR_SAME_FIELD_TWICE, configurationString);
					return ERR_SAME_FIELD_TWICE;
				}

				err = HandleNumericValueString(valueString, this->size, attributes); 
				if (err)
				{
					ERR_PrintError(err, valueString);
					return err; 
				}
			}
			
			return STS_OK;
		}
	}

	// error - no valid configuration 
	ERR_PrintError(ERR_ILLEGAL_FIELD, configurationString);
	return ERR_ILLEGAL_FIELD;
}

UINT32 Field_BinField::setConfiguration( std::string configurationString, std::string valueString )
{
	Field_Attributes dummyAttr;
	return setConfiguration(configurationString, valueString, dummyAttr);
}




UINT32 Field_BinField::handleElememtXML( pugi::xml_node &node )
{

	UINT32 err;
	Field_Attributes attributes;
	// The BinField field is two levels deep
	for (pugi::xml_node_iterator node_it = node.begin(); node_it != node.end(); ++node_it)
	{
		
		string subField = node_it->name();
		if (subField == "config")
		{
			pugi::xml_node configNode = (*node_it);
			for (pugi::xml_node_iterator L2_it = configNode.begin(); L2_it != configNode.end(); ++L2_it)
			{
				string configurationString = L2_it->name();
				string valueString = L2_it->child_value();
				
				attributes.getAttributesFromNode(*L2_it);
				err = setConfiguration(configurationString, valueString, attributes);
				if (err)
				{
					std::cout << "error encountered at " << this->name << "." << subField << "." << configurationString << "=" << valueString<<endl;
					return err;
				}
			}
		}
		else if (subField == "name")
		{
			this->name = node_it->child_value();
		}
		else if (subField == "content")
		{

			err = attributes.getAttributesFromNode(*node_it);
			if (err)
			{
				std::cout << "error encountered at " << this->name << "." << subField<<endl;
				return err;
			}

			string configurationString = node_it->name();
			string valueString = node_it->child_value();
			//if the value string is not empty, handle it
			if (valueString != "")
			{
 				err = HandleNumericValueString(valueString, dataBuffer, size, attributes, ImageConfig.paddingValue);
				if (err)
				{
					std::cout << "error encountered at " << this->name << "." << subField << "." << configurationString << "=" << valueString<<endl;;	
					return err;
				}
			}
			else // value string is empty, fill buffer with padding value
			{
				dataBuffer = new UINT8[size];
				memset(dataBuffer, ImageConfig.paddingValue, size);
			}

			
		}
		else
		{
			err = ERR_ILLEGAL_FIELD;
			stringstream errStr;
			errStr << "error encountered at " << this->name << "." << subField;
			return err;
			ERR_PrintError(err, errStr.str());
			return err;
		}
	}
	return STS_OK;
}

void Field_BinField::dumpField()
{
	cout << "Name: " << this->name << endl;
	cout << " eccType:" << this->eccType;
	cout << " size:" << this->size;
	cout << " data:" << endl;

	if (this->dataBuffer != nullptr)
	{
		for (UINT32 i = 0; i < this->size; ++i)
		{
			if (i %16 == 0)
			{
				cout << endl;
			}
			printf("0x%02X ",dataBuffer[i]);
	
		}
		cout << endl;
	}
}

const std::string Field_ImageProperties::descriptor = "ImageProperties";
const std::string Field_ImageProperties::validConfigurationStrings[NUM_OF_VALID_CONFIGS] = {"BinSize", "PadValue"};

/*
Binary Image Properties field

*/
Field_ImageProperties::Field_ImageProperties(void)
{
	this->size = 0;
	this->paddingValue = 0;
}
Field_ImageProperties::~Field_ImageProperties(void)
{
}


UINT32 Field_ImageProperties::setConfiguration( std::string configurationString, std::string valueString )
{
	UINT32 err;
	// make sure that size is initialized and was not written already
	for (UINT32 selectedConfig = 0; selectedConfig < NUM_OF_VALID_CONFIGS ; selectedConfig++)
	{
		if (configurationString == validConfigurationStrings[selectedConfig])
		{
			if (selectedConfig == configImageSize)
			{

				if (this->size != 0)
				{
					ERR_PrintError(ERR_SAME_FIELD_TWICE, configurationString);
					return ERR_SAME_FIELD_TWICE;
				}

				err = GetIntegerFromString(valueString, this->size); 
				if (err)
				{
					ERR_PrintError(err, valueString);
					return err; 
				}
			}
			else if(selectedConfig == configPaddingValue)
			{
				if (this->paddingValue != 0)
				{
					ERR_PrintError(ERR_SAME_FIELD_TWICE, configurationString);
					return ERR_SAME_FIELD_TWICE;
				}

				err = GetIntegerFromString(valueString, this->paddingValue); 
				if (err)
				{
					ERR_PrintError(err, valueString);
					return err; 
				}
			}

			return STS_OK;
		}
	}

	// error - no valid configuration 
	ERR_PrintError(ERR_ILLEGAL_FIELD, configurationString);
	return ERR_ILLEGAL_FIELD;
}

UINT32 Field_ImageProperties::handleElememtXML( pugi::xml_node &node )
{
	UINT32 err;
	// The Image properties field is one level deep
	for (pugi::xml_node_iterator node_it = node.begin(); node_it != node.end(); ++node_it)
	{
		string configurationString = node_it->name();
		string valueString = node_it->child_value();
		err = setConfiguration(configurationString, valueString);
		if (err)
		{
			return err;
		}
	}
	return STS_OK;
}


Field_Attributes::Field_Attributes(void)
{
	format_id = attr_32bit;
	alignment = 0;
	fileStartOffset = 0;
	reversed = false;
}


Field_Attributes::~Field_Attributes(void)
{
}

void Field_Attributes::clearValues()
{
	format_id = attr_32bit;
	alignment = 0;
	fileStartOffset = 0;
	reversed = false;
}

UINT32 Field_Attributes::setAttribute( pugi::xml_attribute &attr )
{
	string attrName ;
	string attrValue;
	for (int attrIdx = 0; attrIdx < NUM_SUPPORTED_ATTRIBUTES; ++attrIdx)
	{
		attrName = attr.name();
		attrValue = attr.value();

		if (SupportedAttributes[attrIdx] == attrName)
		{
			if (attrIdx == attr_format)
			{
				if (attrValue == SupportedFormatAttr[attr_32bit])
				{
					this->format_id = attr_32bit;
				}
				else if (attrValue == SupportedFormatAttr[attr_bytes])
				{
					this->format_id = attr_bytes;
				}
				else if (attrValue == SupportedFormatAttr[attr_FileSize])
				{
					this->format_id = attr_FileSize;
				}
				else if (attrValue == SupportedFormatAttr[attr_FileContent])
				{
					this->format_id = attr_FileContent;
				}
				else
				{
					string errStr = attrName+"="+attrValue;
					ERR_PrintError(ERR_UNKNOWN_ATTR, errStr);
					return ERR_UNKNOWN_ATTR;
				}

				return STS_OK;
			} 
			else if (attrIdx == attr_align)
			{
				UINT32 err = GetIntegerFromString(attrValue, this->alignment);
				if (err)
				{
					string errStr = attrName+"="+attrValue;
					ERR_PrintError(err, errStr);
					return err;
				}

				return STS_OK;
			}
			else if (attrIdx == attr_file_start_offset)
			{
				UINT32 err = GetIntegerFromString(attrValue, this->fileStartOffset);
				if (err)
				{
					string errStr = attrName+"="+attrValue;
					ERR_PrintError(err, errStr);
					return err;
				}
				return STS_OK;
			}
			else if (attrIdx == attr_reverse_bytes)
			{
				if (attrValue == "true")
				{
					this->reversed = true;
				}
				else if (attrValue == "false")
				{
					this->reversed = false;
				}
				else
				{
					UINT32 err = ERR_UNKNOWN_ATTR;
					string errStr = attrName+"="+attrValue;
					ERR_PrintError(err, errStr);
					return err;
				}
				return STS_OK;
			}
		} 

	}

	string errStr = attrName+"="+attrValue;
	ERR_PrintError(ERR_UNKNOWN_ATTR, errStr);
	return ERR_UNKNOWN_ATTR;
}

UINT32 Field_Attributes::getAttributesFromNode( pugi::xml_node &node )
{
	UINT32 err;
	this->clearValues();
	if (node.first_attribute().empty() == false)
	{
		for (pugi::xml_attribute_iterator ait = node.attributes_begin(); ait != node.attributes_end(); ++ait)
		{
			err = this->setAttribute(*ait);
			if (err)
			{
				return err;
			}
		}
	}

	return STS_OK;
}

const string Field_Attributes::SupportedAttributes[NUM_SUPPORTED_ATTRIBUTES] = {"format", "align", "file_start_offset", "reverse"};
const string Field_Attributes::SupportedFormatAttr[NUM_OF_SUPPORTED_FORMAT_ATTR] = {"32bit" ,"bytes", "FileSize", "FileContent"};
