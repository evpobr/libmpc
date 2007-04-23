/*
  Copyright (c) 2006, The Musepack Development Team
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
#include <math.h>
#include <mpc/mpcdec.h>
#include <mpc/minimax.h>
#include <replaygain/gain_analysis.h>

#include "../libmpcdec/internal.h"
#include "../libmpcdec/huffman.h"
#include "../libmpcdec/mpc_bits_reader.h"

static void usage(const char *exename)
{
	printf("Usage: %s <infile.mpc> [<infile2.mpc> <infile3.mpc> ... ]\n", exename);
}

int main(int argc, char **argv)
{
	MPC_SAMPLE_FORMAT album_max = 0;
	mpc_uint16_t album_gain;
	mpc_uint16_t album_peak;
	mpc_uint16_t * title_gain;
	mpc_uint16_t * title_peak;
	mpc_uint32_t * header_pos;
	int j;

	if(argc < 2) {
		usage(argv[0]);
		return 0;
	}

	title_gain = malloc((sizeof(mpc_uint16_t) * 2 + sizeof(mpc_uint32_t)) * (argc - 1));
	title_peak = title_gain + (argc - 1);
	header_pos = (mpc_uint32_t *) (title_peak + (argc - 1));

	for( j = 1; j < argc; j++){
		MPC_SAMPLE_FORMAT sample_buffer[MPC_DECODER_BUFFER_LENGTH];
		Float_t left_samples[MPC_FRAME_LENGTH * sizeof(Float_t)];
		Float_t right_samples[MPC_FRAME_LENGTH * sizeof(Float_t)];
		MPC_SAMPLE_FORMAT title_max = 0;
		mpc_reader reader;
		mpc_demux* demux;
		mpc_streaminfo si;
		mpc_status err;

		err = mpc_reader_init_stdio(&reader, argv[j]);
		if(err < 0) return !MPC_STATUS_OK;

		demux = mpc_demux_init(&reader);
		if(!demux) return !MPC_STATUS_OK;
		mpc_demux_get_info(demux,  &si);

		if (j == 1) InitGainAnalysis ( si.sample_freq );

		while(1) {
			mpc_frame_info frame;
			int i;

			frame.buffer = sample_buffer;
			mpc_demux_decode(demux, &frame);
			if(frame.bits == -1) break;

			for( i = 0; i < frame.samples; i++){
				left_samples[i] = sample_buffer[2 * i] * (1 << 15);
				right_samples[i] = sample_buffer[2 * i + 1] * (1 << 15);
				title_max = maxf(title_max, sample_buffer[2 * i]);
				title_max = maxf(title_max, sample_buffer[2 * i + 1]);
			}

			AnalyzeSamples(left_samples, right_samples, frame.samples, si.channels);
		}

		title_gain[j-1] = (mpc_uint16_t) (GetTitleGain() * 256);
		title_peak[j-1] = (mpc_uint16_t) (log10(title_max * (1 << 15)) * 20 * 256);
		header_pos[j-1] = si.header_position + 4;

		album_max = maxf(album_max, title_max);

		mpc_demux_exit(demux);
		mpc_reader_exit_stdio(&reader);
	}

	album_gain = (mpc_uint16_t) (GetAlbumGain() * 256);
	album_peak = (mpc_uint16_t) (log10(album_max * (1 << 15)) * 20 * 256);

	for( j = 0; j < argc - 1; j++) {
		unsigned char buffer[64];
		mpc_bits_reader r;
		mpc_block b;
		mpc_uint64_t size;
		FILE * file;

		file = fopen( argv[j + 1], "r+");
		if (file == 0) {
			fprintf(stderr, "Can't open file \"%s\" for writing\n", argv[j + 1]);
			continue;
		}
		fseek(file, header_pos[j] - 4, SEEK_SET);
		fread(buffer, 1, 16, file);
		if (memcmp(buffer, "MPCK", 4) != 0) {
			fprintf(stderr, "Unsupported file format, not a sv8 file : %s\n", argv[j + 1]);
			fclose(file);
			continue;
		}
		r.buff = buffer + 4;
		r.count = 8;

		size = mpc_bits_get_block(&r, &b);

		while( memcmp(b.key, "RG", 2) != 0 ) {
			header_pos[j] += b.size + size;
			fseek(file, header_pos[j], SEEK_SET);
			fread(buffer, 1, 16, file);
			r.buff = buffer;
			r.count = 8;
			size = mpc_bits_get_block(&r, &b);
		}

		header_pos[j] += size;

		buffer[size] = 1; // replaygain version
		buffer[size + 1] = title_gain[j] >> 8;
		buffer[size + 2] = title_gain[j] & 0xFF;
		buffer[size + 3] = title_peak[j] >> 8;
		buffer[size + 4] = title_peak[j] & 0xFF;
		buffer[size + 5] = album_gain >> 8;
		buffer[size + 6] = album_gain & 0xFF;
		buffer[size + 7] = album_peak >> 8;
		buffer[size + 8] = album_peak & 0xFF;

		fseek(file, header_pos[j], SEEK_SET);
		fwrite(buffer + size, 1, b.size, file);
		fclose(file);
	}

	free(title_gain);

    return 0;
}
