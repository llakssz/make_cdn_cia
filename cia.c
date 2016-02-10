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
 *
 * You should have received a copy of the GNU General Public License
 * along with make_cdn_cia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cia.h"
#include "ctr_endian.h"
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static size_t getSigSize(uint32_t sigType)
{
	switch (be32toh(sigType)) {
		case SIGTYPE_RSA4096_SHA1:
		case SIGTYPE_RSA4096_SHA256:
			return 512;

		case SIGTYPE_RSA2048_SHA1:
		case SIGTYPE_RSA2048_SHA256:
			return 256;

		case SIGTYPE_ECDSA_SHA1:
		case SIGTYPE_ECDSA_SHA256:
			return 60;

		default:
			errno = EILSEQ;
			return 0;
	}
}

static size_t getCertSize(uint32_t sigType)
{
	switch (be32toh(sigType)) {
		case SIGTYPE_RSA4096_SHA1:
		case SIGTYPE_RSA4096_SHA256:
			return 1020;

		case SIGTYPE_RSA2048_SHA1:
		case SIGTYPE_RSA2048_SHA256:
			return 764;

		case SIGTYPE_ECDSA_SHA1:
		case SIGTYPE_ECDSA_SHA256:
			return 140;

		default:
			errno = EILSEQ;
			return 0;
	}
}

static int certSaveAndSkip(FILE * f, chunk_t * cert)
{
	uint32_t typeId;
	size_t certSize;

	chunkMarkStart(cert, f);

	if (fread(&typeId, sizeof(uint32_t), 1, f) != 1)
	{
		perror("certSaveAndSkip: unable to read type ID");
		return -1;
	}

	certSize = getCertSize(typeId);
	if (!certSize)
	{
		fprintf(stderr, "certSaveAndSkip: unknown cert type ID: %08X\n", typeId);
		return -1;
	}

	if (fseek(f, certSize, SEEK_CUR))
	{
		perror("certSaveAndSkip: unable to skip certificate data");
		return -1;
	}

	chunkMarkEnd(cert, f);

	return 0;
}

static int skipSignature(FILE * f)
{
	uint32_t sigType;
	size_t size;

	// Read signature type
	if (fread(&sigType, sizeof(sigType), 1, f) != 1)
	{
		perror("skipSignature: error reading signature type");
		return -1;
	}

	// Calculate signature size, and skip it
	size = getSigSize(sigType);
	if (fseek(f, size, SEEK_CUR))
	{
		perror("skipSignature: error seeking");
		return -1;
	}

	if (!alignFilePointer(f, 64))
	{
		perror("skipSignature: error aligning file pointer");
		return -1;
	}

	return 0;
}

static int buildCIAHdr(CIAHdr *cia, const TIKCtx *tik, const TMDCtx *tmd)
{
	uint16_t index, i;

	assert(cia != NULL);
	assert(tik != NULL);
	assert(tmd != NULL);

	cia->hdrSize = htole32(sizeof(*cia));
	cia->type = htole16(0);
	cia->ver = htole16(0);
	cia->certSize = htole32(tik->caCert.size + tik->xsCert.size + tmd->cpCert.size);
	cia->tikSize = htole32(tik->headerChunk.size);
	cia->tmdSize = htole32(tmd->headerChunk.size);
	cia->metaSize = htole32(0);
	cia->contentSize = 0;
	for (i = 0; i < tmd->contentCnt; i++)
		cia->contentSize += be64toh(tmd->content[i].size);
	cia->contentSize = htole64(cia->contentSize);

	memset(cia->contentIndex, 0, sizeof(cia->contentIndex));
	for (i = 0; i < tmd->contentCnt; i++) {
		index = be16toh(tmd->content[i].index);
		cia->contentIndex[index >> 3] |= 0x80 >> (index & 7);
	}

	return 0;
}

int writeCIA(const TMDCtx *tmd, const TIKCtx *tik, FILE *fp)
{
	CIAHdr cia;
	FILE *content;
	char buf[1220];
	uint16_t i;
	size_t left;

	buildCIAHdr(&cia, tik, tmd);
	if (fseek(fp, 0, SEEK_SET)) {
		perror("CIA: error");
		return -1;
	}
	if (fwrite(&cia, sizeof(cia), 1, fp) <= 0) {
		perror("CIA: error");
		return -1;
	}

	alignFilePointer(fp, 64);

	if (!chunkAppendToFile(&tik->caCert, fp))
	{
		perror("CIA: could not add CA cert to cia");
		return -1;
	}

	if (!chunkAppendToFile(&tik->xsCert, fp))
	{
		perror("CIA: could not add XS cert to cia");
		return -1;
	}

	if (!chunkAppendToFile(&tmd->cpCert, fp))
	{
		perror("CIA: could not add CP cert to cia");
		return -1;
	}

	alignFilePointer(fp, 64);

	if (!chunkAppendToFile(&tik->headerChunk, fp))
	{
		perror("CIA: could not add ticket data to cia");
		return -1;
	}

	alignFilePointer(fp, 64);

	if (!chunkAppendToFile(&tmd->headerChunk, fp))
	{
		perror("CIA: could not add tmd data to cia");
		return -1;
	}

	alignFilePointer(fp, 64);

	for (i = 0; i < tmd->contentCnt; i++) {
		sprintf(buf, "%08x", be32toh(tmd->content[i].id));

		content = fopen(buf, "rb");
		if (content == NULL) {
#ifdef _WIN32
			sprintf(buf, "0x%08X: error", be32toh(tmd->content[i].id));
			perror(buf);
			return -1;
#else
			for (i = 0; i < 16; i++)
				if (islower(((unsigned char *)buf)[i]))
					((unsigned char *)buf)[i] = toupper(((unsigned char *)buf)[i]);

			content = fopen(buf, "rb");
			if (content == NULL) {
				sprintf(buf, "0x%08X: error", be32toh(tmd->content[i].id));
				perror(buf);
				return -1;
			}
#endif
		}
		for (left = be64toh(tmd->content[i].size); left > sizeof(buf); left -= sizeof(buf)) {
			if (fread(buf, sizeof(buf), 1, content) <= 0) {
				sprintf(buf, "0x%08X: error", be32toh(tmd->content[i].id));
				perror(buf);
				return -1;
			}
			if (fwrite(buf, sizeof(buf), 1, fp) <= 0) {
				perror("CIA: error");
				return -1;
			}
		}
		if (fread(buf, left, 1, content) <= 0) {
			sprintf(buf, "0x%08X: error", be32toh(tmd->content[i].id));
			perror(buf);
			return -1;
		}
		if (fwrite(buf, left, 1, fp) <= 0) {
			perror("CIA: error");
			return -1;
		}
		fclose(content);
	}

	if (fclose(fp)) {
		perror("CIA: error");
		return -1;
	}

	return 0;
}

int processTIK(TIKCtx *tik)
{
	TIKHdr hdr;

	// Check for programming errors
	assert(tik != NULL);

	// Rewind file
	if (fseek(tik->fp, 0, SEEK_SET))
	{
		perror("CETK: error rewinding file");
		return -1;
	}

	chunkMarkStart(&tik->headerChunk, tik->fp);

	// Read signature type
	if (skipSignature(tik->fp))
	{
		perror("CETK: error skipping leading signature");
		return -1;
	}

	// Now read ticket header to extract title ID
	if (fread(&hdr, sizeof(hdr), 1, tik->fp) <= 0)
	{
		perror("CETK: error");
		return -1;
	}
	tik->titleID = hdr.titleID;

	chunkMarkEnd(&tik->headerChunk, tik->fp);

	// Read "XS" cert
	if (certSaveAndSkip(tik->fp, &tik->xsCert))
	{
		fprintf(stderr, "CETK: Unable to extract XS certificate\n");
		return -1;
	}

	// Read CA cert
	if (certSaveAndSkip(tik->fp, &tik->caCert))
	{
		fprintf(stderr, "CETK: Unable to extract CA certificate\n");
		return -1;
	}

	return 0;
}

int processTMD(TMDCtx *tmd)
{
	TMDHdr hdr;

	// Check for programming errors
	assert(tmd != NULL);

	// Rewind file
	if (fseek(tmd->fp, 0, SEEK_SET))
	{
		perror("TMD: error rewinding file");
		return -1;
	}

	chunkMarkStart(&tmd->headerChunk, tmd->fp);

	// Read signature type
	if (skipSignature(tmd->fp))
	{
		perror("TMD: error skipping leading signature");
		return -1;
	}

	if (fread(&hdr, sizeof(hdr), 1, tmd->fp) != 1)
	{
		perror("TMD: error reading header");
		return -1;
	}
	tmd->titleID = hdr.titleID;

	tmd->contentCnt = be16toh(hdr.contentCnt);
	tmd->content = malloc(sizeof(TMDContent) * tmd->contentCnt);
	if (fread(tmd->content, sizeof(TMDContent), tmd->contentCnt, tmd->fp)
		< tmd->contentCnt) {
		perror("content: error");
		return -1;
	}

	chunkMarkEnd(&tmd->headerChunk, tmd->fp);

	if (certSaveAndSkip(tmd->fp, &tmd->cpCert))
	{
		fprintf(stderr, "TMD: Unable to extract CP certificate\n");
		return -1;
	}

	if (certSaveAndSkip(tmd->fp, &tmd->caCert))
	{
		fprintf(stderr, "TMD: Unable to extract CA certificate\n");
		return -1;
	}

	return 0;
}
