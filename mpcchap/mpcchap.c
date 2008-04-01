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

#include <sys/stat.h>

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
	printf(
	        "Usage: %s <infile.mpc> <chapterfile.ini>\n"
	        "   if chapterfile.ini exists, chapter tags in infile.mpc will be\n"
	        "   replaced by those from chapterfile.ini, else chapters will be\n"
	        "   dumped to chapterfile.ini\n"
	        "   chapterfile.ini is something like :\n"
	        "   	[chapter_start_sample]\n"
	        "   	SomeKey=Some Value\n"
	        "   	SomeOtherKey=Some Other Value\n"
	        "   	[other_chapter_start]\n"
	        "   	YouKnowWhatKey=I think you start to understand ...\n"
	        , exename);
}

mpc_status add_chaps(char * mpc_file, char * chap_file, mpc_demux * demux, mpc_streaminfo * si)
{
	struct stat stbuf;
	FILE * in_file;
	int chap_pos, end_pos, chap_size, i;

	chap_pos = (demux->chap_pos >> 3) + si->header_position;
	end_pos = mpc_demux_pos(demux) >> 3;
	chap_size = end_pos - chap_pos;

	stat(mpc_file, &stbuf);
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
		if (chap_pos > si->samples - si->beg_silence)
			fprintf(stderr, "warning : chapter %i starts @ %lli after the end of the stream (%lli)\n", i + 1, chap_pos, si->samples - si->beg_silence);
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

	return MPC_STATUS_OK;
}

mpc_status dump_chaps(mpc_demux * demux, char * chap_file, int chap_nb)
{
	int i;
	unsigned int tag_size;
	FILE * out_file;
	char * tag;

	if (chap_nb <= 0)
		return MPC_STATUS_OK;

	out_file = fopen(chap_file, "wb");
	if (out_file == 0)
		return !MPC_STATUS_OK;

	for (i = 0; i < chap_nb; i++) {
		int item_count, j;
		fprintf(out_file, "[%lli]\n", mpc_demux_chap(demux, i, &tag, &tag_size));
		item_count = tag[8] | (tag[9] << 8) | (tag[10] << 16) | (tag[11] << 24);
		tag += 24;
		for( j = 0; j < item_count; j++){
			int key_len = strlen(tag + 8);
			int value_len = tag[0] | (tag[1] << 8) | (tag[2] << 16) | (tag[3] << 24);
			fprintf(out_file, "%s=%.*s\n", tag + 8, value_len, tag + 9 + key_len);
			tag += 9 + key_len + value_len;
		}
		fprintf(out_file, "\n");
	}

	fclose(out_file);

	return MPC_STATUS_OK;
}

int main(int argc, char **argv)
{
	mpc_reader reader;
	mpc_demux* demux;
	mpc_streaminfo si;
	char * mpc_file, * chap_file;
	mpc_status err;
	FILE * test_file;

	if (argc != 3)
		usage(argv[0]);

	mpc_file = argv[1];
	chap_file = argv[2];

	err = mpc_reader_init_stdio(&reader, mpc_file);
	if(err < 0) return !MPC_STATUS_OK;

	demux = mpc_demux_init(&reader);
	if(!demux) return !MPC_STATUS_OK;
	mpc_demux_get_info(demux,  &si);

	if (si.stream_version < 8) {
		fprintf(stderr, "this file cannot be edited, please convert it first to sv8 using mpc2sv8\n");
		exit(!MPC_STATUS_OK);
	}

	int chap_nb = mpc_demux_chap_nb(demux);

	test_file = fopen(chap_file, "rb" );
	if (test_file == 0) {
		err = dump_chaps(demux, chap_file, chap_nb);
	} else {
		fclose(test_file);
		err = add_chaps(mpc_file, chap_file, demux, &si);
	}

	mpc_demux_exit(demux);
	mpc_reader_exit_stdio(&reader);
	return err;
}
