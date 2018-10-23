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

#ifndef TOOL_VERSION_H
#define TOOL_VERSION_H


/*
BingoVersion = N7N6N5N4N3N2N1N0

Where:

·         N5N4 – Bingo code Major Version.

·         N3N2 – Bingo code Minor Version.

·         N1N0 – Bingo code Revision.
*/


#define BingoVersion			0x00000002  //ver 0.0.2

#define VER_MAJ(X)			((X&0x00ff0000)>>16)
#define VER_MIN(X)			((X&0x0000ff00)>>8)
#define VER_REV(X)			((X&0x000000ff)>>0)

#endif //TOOL_VERSION_H
