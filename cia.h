/*
 * Copyright 2013 3DSGuy
 * Copyright 2015 173210
 *
 * This file is part of make_cdn_cia.
 *
 * make_cdn_cia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * make_cdn_cia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with make_cdn_cia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>
#include <stdio.h>
#include "chunkio.h"

enum {
	SIGTYPE_RSA4096_SHA1 = 0x10000,
	SIGTYPE_RSA2048_SHA1 = 0x10001,
	SIGTYPE_ECDSA_SHA1 = 0x10002,
	SIGTYPE_RSA4096_SHA256 = 0x10003,
	SIGTYPE_RSA2048_SHA256 = 0x10004,
	SIGTYPE_ECDSA_SHA256 = 0x10005
};

typedef struct
{
	FILE *fp;
	uint64_t titleID;
	uint16_t titleVer;
	chunk_t headerChunk;
	chunk_t xsCert;
	chunk_t caCert;
} TIKCtx;

// Pack stuff to avoid weird behaviour and padding in 64-bit systems
#pragma pack(1)
typedef struct
{
	uint8_t issuer[64];
	uint8_t ecdh[60];
	uint8_t ver;
	uint8_t caCrlVer;
	uint8_t signerCrlVer;
	uint8_t titleKey[16];
	uint8_t pad1;
	uint64_t id;
	uint32_t devID;
	uint64_t titleID;
	uint8_t pad2[2];
	uint16_t tikVer;
	uint8_t pad3[8];
	uint8_t licence;
	uint8_t keyID;
	uint8_t pad4[42];
	uint32_t accountId;
	uint8_t pad5;
	uint8_t audit;
	uint8_t pad6[66];
	uint8_t limits[64];
	uint8_t contentIndex[172];
} TIKHdr;

typedef struct
{
	uint32_t id;
	uint16_t index;
	uint16_t type;
	uint64_t size;
	uint8_t sha256[32];
} TMDContent;

typedef struct
{
	uint16_t indexOffset;
	uint16_t commandCnt;
	uint8_t sha256[0x20];
} TMDContentInfo;

typedef struct
{
	uint8_t issuer[64];
	uint8_t ver;
	uint8_t caCrlVer;
	uint8_t signerCrlVer;
	uint8_t pad0;
	uint64_t sysVer;
	uint64_t titleID;
	uint32_t titleType;
	uint16_t groupID;
	uint32_t saveSize;
	uint32_t privSaveSize;
	uint8_t pad1[4];
	uint8_t twlFlag;
	uint8_t pad2[49];
	uint32_t access;
	uint16_t titleVersion;
	uint16_t contentCnt;
	uint16_t bootContent;
	uint8_t pad3[2];
	uint8_t infoRecordSHA256[32];
	TMDContentInfo contentInfo[64];
} TMDHdr;

typedef struct
{
	FILE *fp;
	uint64_t titleID;
	uint16_t titleVer;
	chunk_t headerChunk;
	chunk_t cpCert;
	chunk_t caCert;
	uint16_t contentCnt;
	TMDContent *content;

	uint16_t *title_index;
} TMDCtx;

typedef struct
{
	uint32_t hdrSize;
	uint16_t type;
	uint16_t ver;
	uint32_t certSize;
	uint32_t tikSize;
	uint32_t tmdSize;
	uint32_t metaSize;
	uint64_t contentSize;
	uint8_t contentIndex[8192];
} CIAHdr;
#pragma pack()

int writeCIA(const TMDCtx *tmd, const TIKCtx *tik, FILE *fp);

int processTIK(TIKCtx *tik_context);
int processTMD(TMDCtx *tmd_context);
