// SPDX-License-Identifier: GPL-2.0
/*
 * Nuvoton Binary Image Generator:   Bingo
 *
 * SHA-1 (FIPS 180-4 section 6.1) and SHA-256, SHA-384, SHA-512
 * (FIPS 180-4 sections 6.2, 6.5, 6.4) of a byte buffer.
 *
 * Copyright (C) 2026 Nuvoton Technologies, All Rights Reserved
 */

#include "sha.h"

using namespace std;

typedef unsigned long long UINT64_T;

static UINT32 rotr32(UINT32 x, int n) { return (x >> n) | (x << (32 - n)); }
static UINT32 rotl32(UINT32 x, int n) { return (x << n) | (x >> (32 - n)); }
static UINT64_T rotr64(UINT64_T x, int n) { return (x >> n) | (x << (64 - n)); }

/*
	Message padding (FIPS 180-4 section 5.1): 0x80, zeros, then the message
	length in bits, big-endian, in the last lengthSize bytes of the last block.
*/
static vector<UINT8> pad(const vector<UINT8> &data, UINT32 blockSize, UINT32 lengthSize)
{
	vector<UINT8> msg(data);
	UINT64_T bits = (UINT64_T)data.size() * 8;
	msg.push_back(0x80);
	while (msg.size() % blockSize != blockSize - lengthSize)
	{
		msg.push_back(0);
	}
	// the length fits 64 bits; a 128-bit length field has zero upper bytes
	for (UINT32 i = 0; i < lengthSize; ++i)
	{
		UINT32 shift = 8 * (lengthSize - 1 - i);
		msg.push_back(shift < 64 ? (UINT8)(bits >> shift) : 0);
	}
	return msg;
}

static vector<UINT8> sha1(const vector<UINT8> &data)
{
	UINT32 h[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
	vector<UINT8> msg = pad(data, 64, 8);
	for (size_t block = 0; block < msg.size(); block += 64)
	{
		UINT32 w[80];
		for (int t = 0; t < 16; ++t)
		{
			w[t] = (UINT32)msg[block + 4*t] << 24 | (UINT32)msg[block + 4*t + 1] << 16 |
				(UINT32)msg[block + 4*t + 2] << 8 | msg[block + 4*t + 3];
		}
		for (int t = 16; t < 80; ++t)
		{
			w[t] = rotl32(w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16], 1);
		}
		UINT32 a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
		for (int t = 0; t < 80; ++t)
		{
			UINT32 f, k;
			if (t < 20)      { f = (b & c) | (~b & d);          k = 0x5A827999; }
			else if (t < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1; }
			else if (t < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
			else             { f = b ^ c ^ d;                   k = 0xCA62C1D6; }
			UINT32 temp = rotl32(a, 5) + f + e + k + w[t];
			e = d; d = c; c = rotl32(b, 30); b = a; a = temp;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
	}
	vector<UINT8> digest;
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 3; j >= 0; --j)
		{
			digest.push_back((UINT8)(h[i] >> (8*j)));
		}
	}
	return digest;
}

static const UINT32 K256[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static vector<UINT8> sha256(const vector<UINT8> &data)
{
	UINT32 h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
	vector<UINT8> msg = pad(data, 64, 8);
	for (size_t block = 0; block < msg.size(); block += 64)
	{
		UINT32 w[64];
		for (int t = 0; t < 16; ++t)
		{
			w[t] = (UINT32)msg[block + 4*t] << 24 | (UINT32)msg[block + 4*t + 1] << 16 |
				(UINT32)msg[block + 4*t + 2] << 8 | msg[block + 4*t + 3];
		}
		for (int t = 16; t < 64; ++t)
		{
			UINT32 s0 = rotr32(w[t-15], 7) ^ rotr32(w[t-15], 18) ^ (w[t-15] >> 3);
			UINT32 s1 = rotr32(w[t-2], 17) ^ rotr32(w[t-2], 19) ^ (w[t-2] >> 10);
			w[t] = w[t-16] + s0 + w[t-7] + s1;
		}
		UINT32 a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
		for (int t = 0; t < 64; ++t)
		{
			UINT32 t1 = hh + (rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25)) + ((e & f) ^ (~e & g)) + K256[t] + w[t];
			UINT32 t2 = (rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
			hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
	}
	vector<UINT8> digest;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 3; j >= 0; --j)
		{
			digest.push_back((UINT8)(h[i] >> (8*j)));
		}
	}
	return digest;
}

static const UINT64_T K512[80] = {
	0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL, 0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL, 0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL, 0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL, 0x06ca6351e003826fULL, 0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL, 0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL,
	0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL, 0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL, 0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 0xc67178f2e372532bULL,
	0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL, 0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL,
	0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

// SHA-512 and SHA-384 differ only in the initial hash value and the digest length.
static vector<UINT8> sha512(const vector<UINT8> &data, const UINT64_T init[8], UINT32 digestWords)
{
	UINT64_T h[8];
	for (int i = 0; i < 8; ++i)
	{
		h[i] = init[i];
	}
	vector<UINT8> msg = pad(data, 128, 16);
	for (size_t block = 0; block < msg.size(); block += 128)
	{
		UINT64_T w[80];
		for (int t = 0; t < 16; ++t)
		{
			w[t] = 0;
			for (int j = 0; j < 8; ++j)
			{
				w[t] = (w[t] << 8) | msg[block + 8*t + j];
			}
		}
		for (int t = 16; t < 80; ++t)
		{
			UINT64_T s0 = rotr64(w[t-15], 1) ^ rotr64(w[t-15], 8) ^ (w[t-15] >> 7);
			UINT64_T s1 = rotr64(w[t-2], 19) ^ rotr64(w[t-2], 61) ^ (w[t-2] >> 6);
			w[t] = w[t-16] + s0 + w[t-7] + s1;
		}
		UINT64_T a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
		for (int t = 0; t < 80; ++t)
		{
			UINT64_T t1 = hh + (rotr64(e, 14) ^ rotr64(e, 18) ^ rotr64(e, 41)) + ((e & f) ^ (~e & g)) + K512[t] + w[t];
			UINT64_T t2 = (rotr64(a, 28) ^ rotr64(a, 34) ^ rotr64(a, 39)) + ((a & b) ^ (a & c) ^ (b & c));
			hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
		}
		h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
	}
	vector<UINT8> digest;
	for (UINT32 i = 0; i < digestWords; ++i)
	{
		for (int j = 7; j >= 0; --j)
		{
			digest.push_back((UINT8)(h[i] >> (8*j)));
		}
	}
	return digest;
}

static const UINT64_T H384[8] = {
	0xcbbb9d5dc1059ed8ULL, 0x629a292a367cd507ULL, 0x9159015a3070dd17ULL, 0x152fecd8f70e5939ULL,
	0x67332667ffc00b31ULL, 0x8eb44a8768581511ULL, 0xdb0c2e0d64f98fa7ULL, 0x47b5481dbefa4fa4ULL
};

static const UINT64_T H512[8] = {
	0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL, 0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
	0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL, 0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

UINT32 SHA_DigestSize(SHA_Type type)
{
	switch (type)
	{
	case SHA_1:   return 20;
	case SHA_256: return 32;
	case SHA_384: return 48;
	default:      return 64;
	}
}

vector<UINT8> SHA_Digest(SHA_Type type, const vector<UINT8> &data)
{
	switch (type)
	{
	case SHA_1:   return sha1(data);
	case SHA_256: return sha256(data);
	case SHA_384: return sha512(data, H384, 6);
	default:      return sha512(data, H512, 8);
	}
}
