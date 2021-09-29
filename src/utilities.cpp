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
#include "utilities.h"
#include "errors.h"

UINT32 verbosLevel = 0;
extern int isMaskRequested;
/*
	Utilities
*/

// [ String split to vector routines:
using namespace std;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}
/*
//]
UINT32 GetIntegerFromString(std::string str, UINT32 &val)
{
	try
	{
		val = std::stol(str,0,0); // auto-detect base
	}
	catch (std::exception& e)
	{
		std::cerr << "std exception: " << e.what() << '\n';
		return ERR_ILLEGAL_VAL;
	}
	
	return STS_OK;
}

UINT32 GetIntegerFromString(std::string str, UINT16 &val)
{
	try
	{
		val = (UINT16) std::stol(str,0,0); // auto-detect base
	}
	catch (std::exception& e)
	{
		std::cerr << "std exception: " << e.what() << '\n';
		return ERR_ILLEGAL_VAL;
	}

	return STS_OK;
}

UINT32 GetIntegerFromString(std::string str, UINT8 &val)
{
	try
	{
		val = (UINT8) std::stol(str,0,0); // auto-detect base
	}
	catch (std::exception& e)
	{
		std::cerr << "std exception: " << e.what() << '\n';
		return ERR_ILLEGAL_VAL;
	}

	return STS_OK;
}
*/


UINT32 getFileSize(const char* filename, UINT32 &size)
{
	UINT32 err = 0;
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	if (in.is_open() == false)
	{
		err = ERR_FILE_NOT_FOUND;
		ERR_PrintError(err, filename);
		size = 0;
		return err;
	}
	size = (UINT32) in.tellg(); 
	in.close();
	return STS_OK;
}

void CmdLine_printUsage(string programName)
{
	cout << "usage: " << endl;
	cout << "\t" << programName << " <xml_config_file> [-o <binary_output_file>]" << endl;
	cout << "\t" << programName << " -i <xml_config_file> [-o <binary_output_file>]" << endl;
}

UINT32 CmdLineParser(int argc, char *argv[], string &inputXML, string &outBin)
{
	bool foundFile = false;
	if (argc < 2)
	{
		CmdLine_printUsage(argv[0]);
		return ERR_CMD_LINE_ERR;
	}

	for (int i=1; i < argc ; ++i)
	{
		string arg = argv[i];
		
		// this is a flag
		if (arg[0] == '-')
		{
			if (arg == "-mask")
			{
				isMaskRequested = 1;

			}
			else if (arg == "-i") // handle input file
			{
				inputXML = argv[i+1];
				++i;
			}
			else if (arg == "-o") // handle output file
			{
				outBin = argv[i+1];
				++i;
			}
			else if (arg[1] == 'v') // handle verbosity level
			{
				for (UINT8 i = 1; i < arg.size(); ++i)
				{
					if (arg[i] == 'v')
					{
						++verbosLevel;
					}
				}
			}
			else if (arg == "--DEBUG")
			{
				// used the debug values for the input and output
				return STS_OK;
			}
			else
			{
				cout << "unknown flag " << arg <<endl;
				CmdLine_printUsage(argv[0]);
				return ERR_CMD_LINE_ERR;
			}
		}
		else // this is the name of the XML input file
		{
			if (!foundFile)
			{
				inputXML = arg;
				foundFile = true;
			}
			else
			{
				cout << "more than one XML input files: " << inputXML << "," << arg <<endl;
				CmdLine_printUsage(argv[0]);
				return ERR_CMD_LINE_ERR;
			}
		}

	}
	cout << "Input XML path: " << inputXML << "\t Output Bin path: " << outBin << endl;
	return STS_OK;
}