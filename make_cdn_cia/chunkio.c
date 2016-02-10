/*
 * Copyright 2015 Marcos Vives Del Sol <socram8888@gmail.com>
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

#include "chunkio.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

#ifndef likely
#define likely(x) (x)
#endif
#ifndef unlikely
#define unlikely(x) (x)
#endif

static uint8_t * STATIC_BUFFER = NULL;
static const size_t STATIC_BUFFER_SIZE = 1024 * 1024;

void initBuffer()
{
	if (likely(STATIC_BUFFER != NULL))
		return;

	STATIC_BUFFER = malloc(STATIC_BUFFER_SIZE);
	if (likely(STATIC_BUFFER != NULL))
		return;

	perror("Unable to malloc work buffer");
	exit(1);
}

bool fastClone(FILE * in, FILE * out, size_t remaining)
{
	initBuffer();

	while (remaining > STATIC_BUFFER_SIZE)
	{
		if (fread(STATIC_BUFFER, STATIC_BUFFER_SIZE, 1, in) != 1)
			return false;

		if (fwrite(STATIC_BUFFER, STATIC_BUFFER_SIZE, 1, out) != 1)
			return false;

		remaining -= STATIC_BUFFER_SIZE;
	}

	if (remaining != 0)
	{
		if (fread(STATIC_BUFFER, remaining, 1, in) != 1)
			return false;

		if (fwrite(STATIC_BUFFER, remaining, 1, out) != 1)
			return false;
	}

	return true;
}

bool alignFilePointer(FILE * f, size_t chunkSize)
{
	long int mask = chunkSize - 1;

	// For bit logic to work, chunk size *must be a power of 2*
	assert((chunkSize & mask) == 0);

	long int pos = ftell(f);
	long int usedBytes = pos & (chunkSize - 1);

	if (usedBytes)
	{
		if (fseek(f, chunkSize - usedBytes, SEEK_CUR))
			return false;
	}

	return true;
}

bool chunkMarkStart(chunk_t * chunk, FILE * file)
{
	long int offset = ftell(file);
	if (unlikely(offset == -1L))
		return false;

	chunk->file = file;
	chunk->offset = offset;
	chunk->size = 0;

	return true;
}

bool chunkMarkEnd(chunk_t * chunk, FILE * file)
{
	long int offset = ftell(file);
	if (unlikely(offset == -1L))
		return false;

	chunk->size = offset - chunk->offset;
	return true;
}

bool chunkAppendToFile(const chunk_t * chunk, FILE * output)
{
	int ret = fseek(chunk->file, chunk->offset, SEEK_SET);
	if (unlikely(ret != 0))
		return false;

	return fastClone(chunk->file, output, chunk->size);
}
