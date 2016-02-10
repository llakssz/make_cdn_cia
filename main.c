/*
 * Copyright (C) 2013 3DSGuy
 * Copyright (C) 2015 173210
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

#include "cia.h"
#include "ctr_endian.h"
#include <errno.h>
#include <unistd.h>

#define VER "1.00"

int main(int argc, char *argv[])
{
	TIKCtx tik;
	TMDCtx tmd;
	FILE *out;

	if(argc != 3) {
		printf("CTR_Toolkit - CIA Generator for CDN Content\n"
			"Version " VER " (C) 2013-2015 3DSGuy, 173210\n\n"
			"Usage: %s <CDN Content Dir> <output CIA file>\n",
			argv[0]);
		return EINVAL;
	}

	out = fopen(argv[2],"wb");
	if (out == NULL) {
		perror("CIA: error");
		return errno;
	}

	if (chdir(argv[1])) {
		perror("CIA: error");
		return errno;
	}

	tik.fp = fopen("cetk","rb");
	if (tik.fp == NULL) {
		perror("CETK: error");
		return errno;
	}
	if (processTIK(&tik)) {
		fclose(tik.fp);
		return errno;
	}

	tmd.fp = fopen("tmd","rb");
	if (tmd.fp == NULL) {
		perror("TMD: error");
		fclose(out);
		fclose(tik.fp);
		return errno;
	}
	if (processTMD(&tmd)) {
		fclose(out);
		fclose(tik.fp);
		fclose(tmd.fp);
		return errno;
	}

	if (tik.titleID != tmd.titleID) {
		printf("warning: CETK and TMD Title IDs do not match\n"
			"       CETK Title ID: 0x%016lluX\n"
			"       TMD Title ID:  0x%016lluX\n",
			be64toh(tik.titleID), be64toh(tmd.titleID));
	}
	
	if (writeCIA(&tmd, &tik, out)) {
		fclose(out);
		fclose(tik.fp);
		fclose(tmd.fp);
		return errno;
	}
	
	return 0;
}
