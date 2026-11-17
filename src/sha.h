// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton Binary Image Generator:   Bingo
 *
 * SHA-1 (FIPS 180-4 section 6.1) and SHA-256, SHA-384, SHA-512
 * (FIPS 180-4 sections 6.2, 6.5, 6.4) of a byte buffer.
 *
 * Copyright (C) 2026 Nuvoton Technologies, All Rights Reserved
 */

#ifndef SHA_H
#define SHA_H

#include "bingo_types.h"
#include <vector>

enum SHA_Type
{
	SHA_1,
	SHA_256,
	SHA_384,
	SHA_512
};

// Digest size in bytes: 20, 32, 48 or 64.
UINT32 SHA_DigestSize(SHA_Type type);

// Digest of data, in the byte order of FIPS 180-4 (big-endian words).
std::vector<UINT8> SHA_Digest(SHA_Type type, const std::vector<UINT8> &data);

#endif // SHA_H
