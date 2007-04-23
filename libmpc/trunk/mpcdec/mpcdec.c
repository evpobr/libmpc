/*
  Copyright (c) 2005, The Musepack Development Team
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
#include <assert.h>
#include <time.h>
#include <mpc/mpcdec.h>
#include <libwaveformat.h>

#ifdef WIN32
#include <crtdbg.h>
#endif

t_wav_uint32 mpc_wav_output_write(void* p_user_data, void const* p_buffer, t_wav_uint32 p_bytes)
{
    FILE* p_handle = (FILE*) p_user_data;
    return (t_wav_uint32) fwrite(p_buffer, 1, p_bytes, p_handle);
}

t_wav_uint32 mpc_wav_output_seek(void* p_user_data, t_wav_uint32 p_position)
{
    FILE* p_handle = (FILE*) p_user_data;
    return (t_wav_uint32) !fseek(p_handle, p_position, SEEK_SET);
}

static void
usage(const char *exename)
{
    printf("Usage: %s <infile.mpc> [<outfile.wav>]\n", exename);
}

int
main(int argc, char **argv)
{
    mpc_reader reader;
	mpc_demux* demux;
	mpc_streaminfo si;
	mpc_status err;
    MPC_SAMPLE_FORMAT sample_buffer[MPC_DECODER_BUFFER_LENGTH];
    clock_t begin, end, sum; int total_samples; t_wav_output_file wav_output;
    mpc_bool_t is_wav_output;

    printf("mpcdec - musepack (mpc) decoder sample application\n");
    if(3 < argc && argc < 2)
    {
        usage(argv[0]);
        return 0;
    }

    err = mpc_reader_init_stdio(&reader, argv[1]);
    if(err < 0) return !MPC_STATUS_OK;

    demux = mpc_demux_init(&reader);
    if(!demux) return !MPC_STATUS_OK;
    mpc_demux_get_info(demux,  &si);

    is_wav_output = argc > 2;
    if(is_wav_output)
    {
        t_wav_output_file_callback wavo_fc;
        memset(&wav_output, 0, sizeof wav_output);
        wavo_fc.m_seek      = mpc_wav_output_seek;
        wavo_fc.m_write     = mpc_wav_output_write;
        wavo_fc.m_user_data = fopen(argv[2], "wb");
        if(!wavo_fc.m_user_data) return !MPC_STATUS_OK;
        err = waveformat_output_open(&wav_output, wavo_fc, si.channels, 16, 0, si.sample_freq, (t_wav_uint32) si.samples * 2);
        if(!err) return !MPC_STATUS_OK;
    }

    sum = total_samples = 0;
    while(MPC_TRUE)
    {
        mpc_frame_info frame;

        frame.buffer = sample_buffer;
        begin        = clock();
        err = mpc_demux_decode(demux, &frame);
        end          = clock();
        if(frame.bits == -1) break;

        total_samples += frame.samples;
        sum           += end - begin;

        if(is_wav_output)
            if(waveformat_output_process_float32(&wav_output, sample_buffer, frame.samples*2) < 0)
                break;
    }

	if (err != MPC_STATUS_OK)
		printf("An error occured while decoding\n");

    printf("%u samples ", total_samples);
	if (sum <= 0) sum = 1;
	total_samples = (mpc_uint32_t) ((mpc_uint64_t) total_samples * CLOCKS_PER_SEC * 100 / ((mpc_uint64_t)si.sample_freq * sum));
    printf("decoded in %u ms (%u.%02ux)\n",
		   (unsigned int) (sum * 1000 / CLOCKS_PER_SEC),
           total_samples / 100,
           total_samples % 100
           );

    mpc_demux_exit(demux);
    mpc_reader_exit_stdio(&reader);
    if(is_wav_output)
    {
        waveformat_output_close(&wav_output);
        fclose(wav_output.m_callback.m_user_data);
    }

#ifdef WIN32
    assert(_CrtCheckMemory());
    _CrtDumpMemoryLeaks();
#endif
    return err;
}
