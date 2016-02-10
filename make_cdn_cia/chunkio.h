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

#include <stdbool.h>
#include <stdio.h>

typedef struct
{
	FILE * file;
	long int offset;
	size_t size;
} chunk_t;

bool alignFilePointer(FILE * file, size_t chunkSize);
bool chunkMarkStart(chunk_t * chunk, FILE * file);
bool chunkMarkEnd(chunk_t * chunk, FILE * file);
bool chunkAppendToFile(const chunk_t * chunk, FILE * output);
