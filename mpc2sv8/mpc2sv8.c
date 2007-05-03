/*
  Copyright (c) 2007, The Musepack Development Team
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
#include <stdio.h>
#include <mpc/mpcdec.h>
#include <mpc/minimax.h>

#include "../libmpcdec/decoder.h"
#include "../libmpcdec/internal.h"
#include "../libmpcenc/libmpcenc.h"

#define TMP_BUF_SIZE 128

static void datacpy(mpc_decoder * d, mpc_encoder_t * e)
{
	static const int  offset[] = { 0, 1, 2, 3, 4, 7, 15, 31, 63, 127, 255, 511,
		1023, 2047, 4095, 8191, 16383, 32767 };
	int i, j;

	memcpy(e->SCF_Index_L, d->SCF_Index_L, sizeof(e->SCF_Index_L));
	memcpy(e->SCF_Index_R, d->SCF_Index_R, sizeof(e->SCF_Index_R));
	memcpy(e->Res_L, d->Res_L, sizeof(e->Res_L));
	memcpy(e->Res_R, d->Res_R, sizeof(e->Res_R));
	memcpy(e->MS_Flag, d->MS_Flag, sizeof(e->MS_Flag));

	for( i = 0; i <= d->max_band; i++){
		mpc_int32_t * q_d = d->Q[i].L, * q_e = e->Q[i].L, Res = d->Res_L[i];

		if (Res > 0)
			for( j = 0; j < 36; j++)
				q_e[j] = q_d[j] + offset[Res];

		q_d = d->Q[i].R, q_e = e->Q[i].R, Res = d->Res_R[i];

		if (Res > 0)
			for( j = 0; j < 36; j++)
				q_e[j] = q_d[j] + offset[Res];
	}
}

static void
usage(const char *exename)
{
    printf("Usage: %s <infile.mpc> <outfile.mpc>\n", exename);
}

int
main(int argc, char **argv)
{
    mpc_reader reader;
	mpc_demux* demux;
	mpc_streaminfo si;
	mpc_status err;
	mpc_encoder_t e;
	mpc_uint_t si_size;
	mpc_size_t stream_size;
	size_t r_size;
	FILE * in_file;
	char buf[TMP_BUF_SIZE];

    printf("mpc2sv8 - musepack (mpc) sv7 to sv8 converter\n");
    if(3 != argc) {
        usage(argv[0]);
        return 0;
    }

    err = mpc_reader_init_stdio(&reader, argv[1]);
    if(err < 0) return !MPC_STATUS_OK;

    demux = mpc_demux_init(&reader);
    if(!demux) return !MPC_STATUS_OK;
    mpc_demux_get_info(demux,  &si);

	if (si.stream_version >= 8) {
		fprintf(stderr, "Error : the file \"%s\" is already a sv8 file\n", argv[1]);
		exit(MPC_STATUS_INVALIDSV);
	}

	mpc_encoder_init(&e, si.samples, 6, 1);
	e.outputFile = fopen( argv[2], "rb" );
	if ( e.outputFile != 0 ) {
		fprintf(stderr, "Error : output file \"%s\" already exists\n", argv[2]);
		exit(MPC_STATUS_FILE);
	}
	e.outputFile = fopen( argv[2], "w+b" );
	e.MS_Channelmode = si.ms;

	// copy begining of file
	in_file = fopen(argv[1], "rb");
	if(in_file == 0) return !MPC_STATUS_OK;
	r_size = si.header_position;
	while(r_size) {
		size_t tmp_size = fread(buf, 1, mini(TMP_BUF_SIZE, r_size), in_file);
		if (fwrite(buf, 1, tmp_size, e.outputFile) != tmp_size) {
			fprintf(stderr, "Error writing to target file : \"%s\"\n", argv[2]);
			exit(MPC_STATUS_FILE);
		}
		r_size -= tmp_size;
	}

	// stream conversion
	e.seek_ref = ftell(e.outputFile);
	writeMagic(&e);
	writeStreamInfo( &e, si.max_band, si.ms > 0, si.samples, si.sample_freq,
					  si.channels);
	si_size = writeBlock(&e, "SH", MPC_TRUE, 0);
	writeGainInfo(&e, si.gain_title, si.peak_title, si.gain_album, si.peak_album);
	si_size = writeBlock(&e, "RG", MPC_FALSE, 0);
	writeEncoderInfo(&e, si.profile, si.pns, si.encoder_version / 100,
					  si.encoder_version % 100, 0);
	writeBlock(&e, "EI", MPC_FALSE, 0);
	e.seek_ptr = ftell(e.outputFile);
	writeBits (&e, 0, 16);
	writeBits (&e, 0, 24); // jump 40 bits for seek table pointer
	writeBlock(&e, "SO", MPC_FALSE, 0); // reserve space for seek offset
	while(MPC_TRUE)
	{
		mpc_frame_info frame;

		demux->d->samples_to_skip = MPC_FRAME_LENGTH + MPC_DECODER_SYNTH_DELAY;
		err = mpc_demux_decode(demux, &frame);

		if(frame.bits == -1) break;

		datacpy(demux->d, &e);
		writeBitstream_SV8 ( &e, si.max_band); // write SV8-Bitstream
	}

	if (err != MPC_STATUS_OK)
		fprintf(stderr, "An error occured while decoding, this file may be corrupted\n");

    // write the last incomplete block
	if (e.framesInBlock != 0) {
		if ((e.block_cnt & ((1 << e.seek_pwr) - 1)) == 0) {
			e.seek_table[e.seek_pos] = ftell(e.outputFile);
			e.seek_pos++;
		}
		e.block_cnt++;
		writeBlock(&e, "AP", MPC_FALSE, 0);
	}
	writeSeekTable(&e);
	writeBlock(&e, "ST", MPC_FALSE, 0); // write seek table block
	writeBlock(&e, "SE", MPC_FALSE, 0); // write end of stream block
	if (demux->d->samples != si.samples) {
		fseek(e.outputFile, e.seek_ref + 4, SEEK_SET);
		writeStreamInfo( &e, si.max_band, si.ms > 0, demux->d->samples,
						  si.sample_freq, si.channels);
		writeBlock(&e, "SH", MPC_TRUE, si_size);
		fseek(e.outputFile, 0, SEEK_END);
	}

	// copy end of file

	stream_size = (((mpc_demux_pos(demux) + 7 - 20) >> 3) - si.header_position + 3) & ~3;
	fseek(in_file, si.header_position + stream_size, SEEK_SET);
	while((r_size = fread(buf, 1, TMP_BUF_SIZE, in_file))) {
		if (fwrite(buf, 1, r_size, e.outputFile) != r_size) {
			fprintf(stderr, "Error writing to target file");
			break;
		}
	}

	fclose ( e.outputFile );
	fclose ( in_file );
	mpc_demux_exit(demux);
	mpc_reader_exit_stdio(&reader);
	mpc_encoder_exit(&e);

	return err;
}
