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

// xml_parsing_sample.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <algorithm>    // std::sort
#include <vector>
#include <string>
#include <sstream>
#include "pugiXML/pugiconfig.hpp"
#include "pugiXML/pugixml.hpp"
#include "tool_version.h"
#include "error_correction.h"
#include "errors.h"
#include "fields.h"
#include "utilities.h"
#include "bingo_types.h"
#include "file_maker.h"


#define TERMINATE_APP(STS)		{cout<<endl<<"FAILED"<<endl; exit(STS);}

#define DEBUG_XML_FILE_PATH		"../examples/poleg_fuse_map.xml"
#define DEFAULT_OUTPUT_FILE_PATH  "bin_image.bin"

typedef enum _EXIT_CODE 
{
	//ES=Exit Status
	ES_CLI_PARSING_ERROR	=	0x01,
	ES_XML_PARSING_ERROR	=	0x02,
	ES_STATUS_REPORT_ERROR	=	0x03,
	ES_FILE_GEN_ERR         =	0x04,
	ES_BUILDING_ERROR		=	0x05,
	ES_GENERATING_ERROR		=	0x06
} EXIT_CODE;

using namespace std;


vector <Field_BinField *>	BinFields;
Field_ImageProperties ImageConfig;



UINT32 XML_InputFileParser(pugi::xml_document &doc)
{
	
	UINT32 err = 0;
	string fieldName;
	pugi::xml_node errorNode;
	// make sure the root element is valid
	if (doc.first_child().name() != ROOT_DESCRIPTOR)
	{
		cout << doc.first_child().name() << " should be " << ROOT_DESCRIPTOR << endl;
		return ERR_ILLEGAL_VAL;
	}
	
	// avoiding recursion while assuming we now the structure of the xml_tree
	pugi::xml_node fieldNode;
	
	// go on first level elements (i.e. fields)
	fieldNode = doc.first_child();
	for (pugi::xml_node_iterator it = fieldNode.begin(); it != fieldNode.end(); ++it)
	{
		fieldName = it->name();
		
		if (fieldName == Field_ImageProperties::descriptor)
		{	
			err = ImageConfig.handleElememtXML(*it);	
		}
		else if (fieldName == Field_BinField::descriptor)
		{
			Field_BinField *field = new Field_BinField();
			err = field->handleElememtXML(*it);
			BinFields.push_back(field);
		} 
		else
		{
			err = ERR_ILLEGAL_FIELD;
			ERR_PrintError(ERR_ILLEGAL_FIELD, fieldName);
		}


		if (err)
		{
			errorNode = (*it);
			break;
		}
	}
		// treat each element according to field

	
	if (err)
	{
		
		stringstream errStr;
		errStr << "error at node: " << errorNode.name() << "." << errorNode.first_child().child_value();
		ERR_PrintError(ERR_PARSING, errStr.str());
		return err;
	}
	return STS_OK;
}


int main(int argc, char *argv[])
{
	UINT32 status;
	string	outputFilename = DEFAULT_OUTPUT_FILE_PATH; 
	string  inputXMLFilename = DEBUG_XML_FILE_PATH;

	cout<< endl << "Bingo - Binary Construction and Generation Tool"<<endl;
	cout<<"Bingo version "<<VER_MAJ(BingoVersion)<<"."<<VER_MIN(BingoVersion)<<"."<<VER_REV(BingoVersion)<<endl; 
	
	// command line parser...
	status = CmdLineParser(argc, argv, inputXMLFilename, outputFilename);
	if (status)
	{
		TERMINATE_APP(ES_CLI_PARSING_ERROR);
	}
	
	
	if (verbosLevel)
	{
		cout << "Loading XML File " << inputXMLFilename << "..."<< endl;
	} 
	
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(inputXMLFilename.c_str());
	if (result.status != pugi::status_ok)
	{
		cout << "XML Load result: " << result.description() << endl;
		ERR_PrintError(ERR_PARSING, "XML file could not be loaded");
		TERMINATE_APP(ES_XML_PARSING_ERROR);
	}
	
	if (verbosLevel)
	{
		cout << "XML Load result: " << result.description() << endl;
	
		cout << "Parsing XML (" << inputXMLFilename << ")..."<< endl;
	}

	status = XML_InputFileParser(doc);
	if (status)
	{
		TERMINATE_APP(ES_XML_PARSING_ERROR);
	}



	if (verbosLevel)
	{
		cout << "Validating fields..." << endl;
	}
	
	// sort the binField vector
	std::sort (BinFields.begin(), BinFields.end(), FM_binFieldSortFunctionHandler);

	// validate binary content fields (size, no overrun)
	status = FM_ValidateFieldVector(BinFields, ImageConfig);
	if (status)
	{
		TERMINATE_APP(ES_BUILDING_ERROR);
	}
	

	if (verbosLevel)
	{
		cout << "creating output file " << outputFilename << "..." << endl;
	}
	// create binary file
	status = FM_CreateBinFile(BinFields, ImageConfig, outputFilename);
	if (status)
	{
		TERMINATE_APP(ES_GENERATING_ERROR);
	}
	

	// destruct all binary fields
	while (!BinFields.empty())
	{
		delete BinFields.back();
		BinFields.pop_back();
	}
	
	cout<<endl<<"SUCCESS"<<endl;
	return STS_OK;
}

