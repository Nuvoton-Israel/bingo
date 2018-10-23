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

#ifndef UTILITIES_H
#define UTILITIES_H


#include <string>
#include <sstream>
#include <vector>
#include <fstream>
#include "bingo_types.h"

extern UINT32 verbosLevel;

#define ALIGN(x, alg) (((x) + (alg) - 1) / (alg) * (alg))

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);
UINT32 getFileSize(const char* filename, UINT32 &size);
UINT32 CmdLineParser(int argc, char *argv[], std::string &inputXML, std::string &outBin);
#endif // UTILITIES_H
