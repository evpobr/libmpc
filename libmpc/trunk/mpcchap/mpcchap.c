/*
  Copyright (c) 2008, The Musepack Development Team
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above
  copyright notice, this list of conditions and the following
  disclaimer in the documentation and/or other materials provided
  with the distribution.

  * Neither the name of the The Musepack Development Team nor the
  names of its contributors may be used to endorse or promote
  products derived from this software without specific prior
  written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <mpc/mpcdec.h>
#include "../libmpcdec/internal.h"
#include "../libmpcenc/libmpcenc.h"
#include "iniparser.h"

// #include <sys/types.h>
#include <sys/stat.h>
// #include <unistd.h> 

// tags.c
void    Init_Tags        ( void );
int     FinalizeTags     ( FILE* fp, unsigned int Version, unsigned int flags );
int     addtag           ( const char* key, size_t keylen, const char* value, size_t valuelen, int converttoutf8, int flags );
#define TAG_NO_HEADER 1
#define TAG_NO_FOOTER 2
#define TAG_NO_PREAMBLE 4
#define TAG_VERSION 2000

#ifdef _MSC_VER
#define atoll _atoi64
#endif

static void usage(const char *exename)
{
	printf("Usage: %s <infile.mpc> <chapterfile.ini>\n", exename);
}

int main(int argc, char **argv)
{
	mpc_reader reader;
	mpc_demux* demux;
	mpc_streaminfo si;
	char * mpc_file, * chap_file;
	FILE * in_file;
	mpc_status err;
	int i;

	if (argc != 3)
		usage(argv[0]);
	
	mpc_file = argv[1];
	chap_file = argv[2];

	err = mpc_reader_init_stdio(&reader, argv[1]);
	if(err < 0) return !MPC_STATUS_OK;

	demux = mpc_demux_init(&reader);
	if(!demux) return !MPC_STATUS_OK;
	mpc_demux_get_info(demux,  &si);

	if (si.stream_version < 8) {
		fprintf(stderr, "this file cannot be edited, please convert it first to sv8 using mpc2sv8\n");
		exit(!MPC_STATUS_OK);
	}

	int chap_nb = mpc_demux_chap_nb(demux);
	for (i = 0; i < chap_nb; i++) {
		printf("chap %i : %i\n", i+1, mpc_demux_chap(demux, i, 0, 0));
	}
	int chap_pos = (demux->chap_pos >> 3) + si.header_position;
	int end_pos = mpc_demux_pos(demux) >> 3;
	int chap_size = end_pos - chap_pos;
	printf("chap-size : %i, end_pos : %x\n", chap_size, end_pos);
	struct stat stbuf;
	stat(argv[1], &stbuf);
	char * tmp_buff = malloc(stbuf.st_size - chap_pos - chap_size);
	in_file = fopen( mpc_file, "r+b" );
	fseek(in_file, chap_pos + chap_size, SEEK_SET);
	fread(tmp_buff, 1, stbuf.st_size - chap_pos - chap_size, in_file);
	fseek(in_file, chap_pos, SEEK_SET);
	
	dictionary * dict = iniparser_load(chap_file);

	int nchap = iniparser_getnsec(dict);
	for (i = 0; i < nchap; i++) {
		Init_Tags();
		int j, nitem, tag_len = 0;
		char * chap_sec = iniparser_getsecname(dict, i);
		mpc_int64_t chap_pos = atoll(chap_sec);
		nitem = iniparser_getnkey(dict, i);
		for (j = 0; j < nitem; j++) {
			char * item_key, * item_value;
			int key_len, item_len;
			item_key = iniparser_getkeyname(dict, i, j, & item_value);
			key_len = strlen(item_key);
			item_len = strlen(item_value);
			addtag(item_key, key_len, item_value, item_len, 0, 0);
			tag_len += key_len + item_len;
		}
		tag_len += 24 + nitem * 9;
		char block_size[12] = "CT";
		char sample_offset[10];
		int offset_size = encodeSize(chap_pos, sample_offset, MPC_FALSE);
		tag_len = encodeSize(tag_len + offset_size + 2, block_size + 2, MPC_TRUE);
		fwrite(block_size, 1, tag_len + 2, in_file);
		fwrite(sample_offset, 1, offset_size, in_file);
		FinalizeTags(in_file, TAG_VERSION, TAG_NO_FOOTER | TAG_NO_PREAMBLE);
	}
	
	fwrite(tmp_buff, 1, stbuf.st_size - chap_pos - chap_size, in_file);
	ftruncate(fileno(in_file), ftell(in_file));
	fclose(in_file);
	free(tmp_buff);
	iniparser_freedict(dict);
	return 0;
}
