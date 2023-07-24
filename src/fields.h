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

#ifndef FIELDS_H
#define FIELDS_H

#include "error_correction.h"
#include "bingo_types.h"
#include "pugiXML/pugixml.hpp"
#include <string>
#include <vector>



const std::string ROOT_DESCRIPTOR = "Bin_Ecc_Map";


class Field_Attributes
{
public:
	Field_Attributes(void);
	~Field_Attributes(void);
	UINT32 setAttribute(pugi::xml_attribute &attr);
	UINT32 getAttributesFromNode(pugi::xml_node &node);
	void clearValues();

	enum validNumericAttributes
	{
		attr_format = 0, // describes the format of the input (could be hex, FileSize, FileContent)
		attr_align,		 // describes the alignment of the output
		attr_file_start_offset,
		attr_reverse_bytes,
		NUM_SUPPORTED_ATTRIBUTES
	};
	static const std::string SupportedAttributes[NUM_SUPPORTED_ATTRIBUTES]; //  = {"format", "align", "start_offset"};

	typedef enum formatAttr
	{
		attr_32bit,
		attr_bytes,
		attr_FileSize,
		attr_FileContent,
		NUM_OF_SUPPORTED_FORMAT_ATTR
	}formatAttr;
	static const std::string SupportedFormatAttr[NUM_OF_SUPPORTED_FORMAT_ATTR];
	
	
	UINT32		format_id;
	UINT32		alignment;
	UINT32		fileStartOffset;
	bool		reversed;


};




/*
	Image properties field
*/

class Field_ImageProperties
{
public:
	
	Field_ImageProperties(void);
	~Field_ImageProperties(void);
	static const std::string descriptor;
	
	// XML node handler, according to field structure
	UINT32	handleElememtXML(pugi::xml_node &node);

	enum validConfigs
	{
		configImageSize = 0,
		configPaddingValue = 1,
		NUM_OF_VALID_CONFIGS
	};

	UINT32 setConfiguration(std::string configurationString, std::string valueString);
	
	// field values
	UINT32	size;
	UINT8	paddingValue;


private:
	static const std::string validConfigurationStrings[NUM_OF_VALID_CONFIGS];

};

extern Field_ImageProperties ImageConfig;

/*
	Binary Field Properties
*/
class Field_BinField
{
public:

	Field_BinField(void); // constructor
	~Field_BinField(void);
	static const std::string descriptor;
	UINT32					allocataDataBuffer();
	void			dumpField();
	
	std::string		name;
	ECC_Type		eccType;
	UINT32			offset;
	UINT32			size;
	UINT8			*dataBuffer;
	bool			maskExists;

	

	// Sets the field configuration, according to given attributes
	UINT32					setConfiguration(std::string configurationString, std::string valueString);
	UINT32					setConfiguration(std::string configurationString, std::string valueString, const Field_Attributes &attributes );
	
	// XML node handler, according to field structure
	UINT32					handleElememtXML(pugi::xml_node &node);


	enum validConfigs
	{
		configEcc = 0,
		configOffset,
		configSize,
		NUM_OF_VALID_CONFIGS
	};

private:
	static const std::string validConfigurationStrings[NUM_OF_VALID_CONFIGS];


};




#endif // FIELDS_H
