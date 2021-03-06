/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1996-2004 Carnegie Mellon University.  All rights 
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */
#include <stdio.h>
#include <stdlib.h>
#if !defined(WIN32) || defined(GNUWINCE)
#include <unistd.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#if !defined(O_BINARY)
#define O_BINARY 0
#endif
#endif
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#ifdef _WIN32
#pragma warning (disable: 4996 4018)
#endif

#if defined(WIN32) && !defined(GNUWINCE)
#include <io.h>
#include <errno.h>
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "fe.h"
#include "strfuncs.h"
#include "cmd_ln.h"
#include "err.h"
#include "ckd_alloc.h"
#include "byteorder.h"

#include "wave2feat.h"


globals_t *fe_parse_options(int argc, char **argv);
int32 fe_convert_files(globals_t * P);
int32 fe_build_filenames(globals_t * P, char *fileroot, char **infilename,
                         char **outfilename);
int32 fe_openfiles(globals_t * P, fe_t * FE, char *infile, int32 * fp_in,
                   int32 * nsamps, int32 * nframes, int32 * nblocks,
                   char *outfile, int32 * fp_out);
int32 fe_readblock_spch(globals_t * P, int32 fp, int32 nsamps,
                        int16 * buf);
int32 fe_writeblock_feat(globals_t * P, fe_t * FE, int32 fp, int32 nframes,
                         mfcc_t ** feat);
int32 fe_closefiles(int32 fp_in, int32 fp_out);
int32 fe_convert_with_dct(globals_t * P, fe_t * FE, char *infile, char *outfile);

/*       
         7-Feb-00 M. Seltzer - wrapper created for new front end -
         does blockstyle processing if necessary. If input stream is
         greater than DEFAULT_BLOCKSIZE samples (currently 200000)
         then it will read and write in DEFAULT_BLOCKSIZE chunks. 
         
         Had to change fe_process_utt(). Now the 2d feature array
         is allocated internally to that function rather than
         externally in the wrapper. 
         
         Added usage display with -help switch for help

         14-Feb-00 M. Seltzer - added NIST header parsing for 
         big endian/little endian parsing. kind of a hack.

         changed -wav switch to -nist to avoid future confusion with
         MS wav files
         
         added -mach_endian switch to specify machine's byte format
*/

int32
convert(int32 argc, char **argv)
{
    globals_t *P;

	//err_set_logfile("convert.log");
    P = fe_parse_options(argc, argv);
	if (P== NULL) 
		return NULL;
    if (fe_convert_files(P) != FE_SUCCESS) {
        E_FATAL("error converting files...exiting\n");
    }
    free(P);
    return (0);
}


int32
fe_convert_files(globals_t * P)
{

    fe_t *FE;
    char *infile, *outfile, fileroot[MAXCHARS];
    FILE *ctlfile;
    int16 *spdata = NULL;
    int32 splen =
        0, total_samps, frames_proc, nframes, nblocks, last_frame;
    int32 fp_in, fp_out, last_blocksize = 0, curr_block, total_frames;
    mfcc_t **cep = NULL, **last_frame_cep;
    int32 return_value;
    int32 warn_zero_energy = 0;
    int32 process_utt_return_value;

    if ((FE = fe_init_auto_r(P->config)) == NULL) {
        E_ERROR("memory alloc failed...exiting\n");
        return (FE_MEM_ALLOC_ERROR);
    }

    if (P->is_batch) {
        int32 nskip = P->nskip;
        int32 runlen = P->runlen;

        if ((ctlfile = fopen(P->ctlfile, "r")) == NULL) {
            E_ERROR("Unable to open control file %s\n", P->ctlfile);
            fe_free(FE);
            return (FE_CONTROL_FILE_ERROR);
        }
        while (fscanf(ctlfile, "%s", fileroot) != EOF) {
            if (nskip > 0) {
                --nskip;
                continue;
            }
            if (runlen > 0) {
                --runlen;
            }
            else if (runlen == 0) {
                break;
            }

            fe_build_filenames(P, fileroot, &infile, &outfile);

            if (P->verbose)
                E_INFO("%s\n", infile);

            if (P->convert) {
                /* Special case for doing various DCTs */
                return_value = fe_convert_with_dct(P, FE, infile, outfile);
                ckd_free(infile);
                ckd_free(outfile);
                infile = outfile = NULL;
                if (return_value != FE_SUCCESS) {
                    fe_free(FE);
                    return return_value;
                }
                continue;
            }
            return_value =
                fe_openfiles(P, FE, infile, &fp_in,
                             &total_samps, &nframes, &nblocks,
                             outfile, &fp_out);
            ckd_free(infile);
            ckd_free(outfile);
            infile = outfile = NULL;
            if (return_value != FE_SUCCESS) {
                fe_free(FE);
                return (return_value);
            }

            warn_zero_energy = 0;

            if (nblocks * P->blocksize >= total_samps)
                last_blocksize =
                    total_samps - (nblocks - 1) * P->blocksize;

            if (!fe_start_utt(FE)) {
                curr_block = 1;
                total_frames = frames_proc = 0;
                /*execute this loop only if there is more than 1 block to
                   be processed */
                while (curr_block < nblocks) {
                    splen = P->blocksize;
                    if ((spdata =
                         (int16 *) calloc(splen, sizeof(int16))) == NULL) {
                        E_ERROR
                            ("Unable to allocate memory block of %d shorts for input speech\n",
                             splen);
                        fe_free(FE);
                        return (FE_MEM_ALLOC_ERROR);
                    }
                    if (fe_readblock_spch
                        (P, fp_in, splen, spdata) != splen) {
                        E_ERROR("error reading speech data\n");
                        fe_free(FE);
                        return (FE_INPUT_FILE_READ_ERROR);
                    }
                    process_utt_return_value =
                        fe_process_utt(FE, spdata,
                                       splen, &cep, &frames_proc);
                    if (process_utt_return_value != FE_SUCCESS) {
                        if (FE_ZERO_ENERGY_ERROR ==
                            process_utt_return_value) {
                            warn_zero_energy = 1;
                        }
                        else {
                            fe_free(FE);
                            return (process_utt_return_value);
                        }
                    }
                    if (frames_proc > 0)
                        fe_writeblock_feat(P, FE,
                                           fp_out, frames_proc, cep);
                    if (cep != NULL) {
                        ckd_free_2d((void **) cep);
                        cep = NULL;
                    }
                    curr_block++;
                    total_frames += frames_proc;
                    free(spdata);
                    spdata = NULL;
                }
                /* process last (or only) block */
                free(spdata);
                spdata = NULL;
                splen = last_blocksize;

                if ((spdata =
                     (int16 *) calloc(splen, sizeof(int16))) == NULL) {
                    E_ERROR
                        ("Unable to allocate memory block of %d shorts for input speech\n",
                         splen);
                    fe_free(FE);
                    return (FE_MEM_ALLOC_ERROR);
                }

                if (fe_readblock_spch(P, fp_in, splen, spdata) != splen) {
                    E_ERROR("error reading speech data\n");
                    fe_free(FE);
                    return (FE_INPUT_FILE_READ_ERROR);
                }

                process_utt_return_value =
                    fe_process_utt(FE, spdata, splen, &cep, &frames_proc);
                if (process_utt_return_value != FE_SUCCESS) {
                    if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
                        warn_zero_energy = 1;
                    }
                    else {
                        fe_free(FE);
                        return (process_utt_return_value);
                    }
                }
                if (frames_proc > 0)
                    fe_writeblock_feat(P, FE, fp_out, frames_proc, cep);
                if (cep != NULL) {
                    ckd_free_2d((void **) cep);
                    cep = NULL;
                }
                curr_block++;
                last_frame_cep =
                    (mfcc_t **) ckd_calloc_2d(1,
                                              fe_get_output_size(FE),
                                              sizeof(float32));
                process_utt_return_value =
                    fe_end_utt(FE, last_frame_cep[0], &last_frame);
                if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
                    warn_zero_energy = 1;
                }
                else {
                    assert(process_utt_return_value == FE_SUCCESS);
                }
                if (last_frame > 0) {
                    fe_writeblock_feat(P, FE, fp_out,
                                       last_frame, last_frame_cep);
                    frames_proc++;
                }
                total_frames += frames_proc;

                fe_closefiles(fp_in, fp_out);
                free(spdata);
                spdata = NULL;
                if (last_frame_cep != NULL) {
                    ckd_free_2d((void **)
                                last_frame_cep);
                    last_frame_cep = NULL;
                }
                if (warn_zero_energy) {
                    E_WARN
                        ("File %s has some frames with zero energy. Consider using dither\n",
                         infile);
                }
            }
            else {
                E_ERROR("fe_start_utt() failed\n");
                return (FE_START_ERROR);
            }
        }
    }
    else if (P->is_single) {

        fe_build_filenames(P, fileroot, &infile, &outfile);
        if (P->verbose)
            printf("%s\n", infile);

        /* Special case for doing various DCTs. */
        if (P->convert != WAV2FEAT) {
            int rv;

            rv = fe_convert_with_dct(P, FE, infile, outfile);
            ckd_free(infile);
            ckd_free(outfile);
            infile = outfile = NULL;
            fe_free(FE);
            return rv;
        }

        return_value =
            fe_openfiles(P, FE, infile, &fp_in, &total_samps,
                         &nframes, &nblocks, outfile, &fp_out);
        ckd_free(infile);
        ckd_free(outfile);
        infile = outfile = NULL;
        if (return_value != FE_SUCCESS) {
            fe_free(FE);
            return (return_value);
        }

        warn_zero_energy = 0;

        if (nblocks * P->blocksize >= total_samps)
            last_blocksize = total_samps - (nblocks - 1) * P->blocksize;

        if (!fe_start_utt(FE)) {
            curr_block = 1;
            total_frames = frames_proc = 0;
            /*execute this loop only if there are more than 1 block to
               be processed */
            while (curr_block < nblocks) {
                splen = P->blocksize;
                if ((spdata =
                     (int16 *) calloc(splen, sizeof(int16))) == NULL) {
                    E_ERROR
                        ("Unable to allocate memory block of %d shorts for input speech\n",
                         splen);
                    fe_free(FE);
                    return (FE_MEM_ALLOC_ERROR);
                }
                if (fe_readblock_spch(P, fp_in, splen, spdata) != splen) {
                    E_ERROR("Error reading speech data\n");
                    fe_free(FE);
                    return (FE_INPUT_FILE_READ_ERROR);
                }
                process_utt_return_value =
                    fe_process_utt(FE, spdata, splen, &cep, &frames_proc);
                if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
                    warn_zero_energy = 1;
                }
                else {
                    assert(process_utt_return_value == FE_SUCCESS);
                }
                if (frames_proc > 0)
                    fe_writeblock_feat(P, FE, fp_out, frames_proc, cep);
                if (cep != NULL) {
                    ckd_free_2d((void **) cep);
                    cep = NULL;
                }
                curr_block++;
                total_frames += frames_proc;
                if (spdata != NULL) {
                    free(spdata);
                    spdata = NULL;
                }
            }
            /* process last (or only) block */
            if (spdata != NULL) {
                free(spdata);
                spdata = NULL;
            }
            splen = last_blocksize;
            if ((spdata = (int16 *) calloc(splen, sizeof(int16))) == NULL) {
                E_ERROR
                    ("Unable to allocate memory block of %d shorts for input speech\n",
                     splen);
                fe_free(FE);
                return (FE_MEM_ALLOC_ERROR);
            }
            if (fe_readblock_spch(P, fp_in, splen, spdata) != splen) {
                E_ERROR("Error reading speech data\n");
                fe_free(FE);
                return (FE_INPUT_FILE_READ_ERROR);
            }
            process_utt_return_value =
                fe_process_utt(FE, spdata, splen, &cep, &frames_proc);
            free(spdata);
            spdata = NULL;
            if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
                warn_zero_energy = 1;
            }
            else {
                assert(process_utt_return_value == FE_SUCCESS);
            }
            if (frames_proc > 0)
                fe_writeblock_feat(P, FE, fp_out, frames_proc, cep);
            if (cep != NULL) {
                ckd_free_2d((void **) cep);
                cep = NULL;
            }

            curr_block++;
            last_frame_cep =
                (mfcc_t **) ckd_calloc_2d(1,
                                          fe_get_output_size(FE),
                                          sizeof(float32));
            process_utt_return_value =
                fe_end_utt(FE, last_frame_cep[0], &last_frame);
            if (FE_ZERO_ENERGY_ERROR == process_utt_return_value) {
                warn_zero_energy = 1;
            }
            else {
                assert(process_utt_return_value == FE_SUCCESS);
            }
            if (last_frame > 0) {
                fe_writeblock_feat(P, FE, fp_out,
                                   last_frame, last_frame_cep);
                frames_proc++;
            }
            total_frames += frames_proc;

            fe_closefiles(fp_in, fp_out);
            if (last_frame_cep != NULL) {
                ckd_free_2d((void **) last_frame_cep);
                last_frame_cep = NULL;
            }
        }
        else {
            E_ERROR("fe_start_utt() failed\n");
            fe_free(FE);
            return (FE_START_ERROR);
        }

        if (warn_zero_energy) {
            E_WARN
                ("File %s has some frames with zero energy. Consider using dither\n",
                 infile);
        }
    }
    else {
        E_ERROR("Unknown mode - single or batch?\n");
        fe_free(FE);
        return (FE_UNKNOWN_SINGLE_OR_BATCH);
    }

    fe_free(FE);
    return (FE_SUCCESS);
}

void
fe_validate_parameters(globals_t * P)
{

    if ((P->is_batch) && (P->is_single)) {
        E_FATAL("You cannot define an input file and a control file\n");
    }

    if (P->wavfile == NULL && P->wavdir == NULL) {
        E_FATAL("No input file or file directory given\n");
    }

    if (P->cepfile == NULL && P->cepdir == NULL) {
        E_FATAL("No cepstra file or file directory given\n");
    }

    if (P->ctlfile == NULL && P->cepfile == NULL && P->wavfile == NULL) {
        E_FATAL("No control file given\n");
    }

    if (P->nchans > 1) {
        E_INFO("Files have %d channels of data\n", P->nchans);
        E_INFO("Will extract features for channel %d\n", P->whichchan);
    }

    if (P->whichchan > P->nchans) {
        E_FATAL("You cannot select channel %d out of %d\n",
                P->whichchan, P->nchans);
    }

    if ((cmd_ln_float32_r(P->config, "-upperf") * 2)
        > cmd_ln_float32_r(P->config, "-samprate")) {
        E_WARN("Upper frequency higher than Nyquist frequency\n");
    }

    if (cmd_ln_boolean_r(P->config, "-doublebw")) {
        E_INFO("Will use double bandwidth filters\n");
    }

}


globals_t *
fe_parse_options(int32 argc, char **argv)
{
    globals_t *P;
    int32 format;
    char const *endian;

    P = ckd_calloc(1, sizeof(*P));
    P->config = cmd_ln_parse_r(NULL, defn, argc, argv, TRUE);
	if (P->config == NULL)
		return NULL;

    /* Load arguments from a feat.params file if requested. */
    if (cmd_ln_str_r(P->config, "-argfile")) {
        P->config = cmd_ln_parse_file_r(P->config, defn,
                                        cmd_ln_str_r(P->config, "-argfile"),
                                        FALSE);
    }

    P->nskip = P->runlen = -1;
    P->wavfile = cmd_ln_str_r(P->config, "-i");
    if (P->wavfile != NULL) {
        P->is_single = 1;
    }
    P->cepfile = cmd_ln_str_r(P->config, "-o");
    P->ctlfile = cmd_ln_str_r(P->config, "-c");
    if (P->ctlfile != NULL) {
        char const *nskip;
        char const *runlen;

        P->is_batch = 1;

        nskip = cmd_ln_str_r(P->config, "-nskip");
        runlen = cmd_ln_str_r(P->config, "-runlen");
        if (nskip != NULL) {
            P->nskip = atoi(nskip);
        }
        if (runlen != NULL) {
            P->runlen = atoi(runlen);
        }
    }
    P->wavdir = cmd_ln_str_r(P->config, "-di");
    P->cepdir = cmd_ln_str_r(P->config, "-do");
    P->wavext = cmd_ln_str_r(P->config, "-ei");
    P->cepext = cmd_ln_str_r(P->config, "-eo");
    format = cmd_ln_int32_r(P->config, "-raw");
    if (format) {
        P->input_format = RAW;
    }
    format = cmd_ln_int32_r(P->config, "-nist");
    if (format) {
        P->input_format = NIST;
    }
    format = cmd_ln_int32_r(P->config, "-mswav");
    if (format) {
        P->input_format = MSWAV;
    }

    P->nchans = cmd_ln_int32_r(P->config, "-nchans");
    P->whichchan = cmd_ln_int32_r(P->config, "-whichchan");
    P->output_endian = BIG;
    P->blocksize = cmd_ln_int32_r(P->config, "-blocksize");
    endian = cmd_ln_str_r(P->config, "-mach_endian");
    if (!strcmp("big", endian)) {
        P->machine_endian = BIG;
    }
    else {
        if (!strcmp("little", endian)) {
            P->machine_endian = LITTLE;
        }
        else {
            E_FATAL("Machine must be big or little Endian\n");
        }
    }
    endian = cmd_ln_str_r(P->config, "-input_endian");
    if (!strcmp("big", endian)) {
        P->input_endian = BIG;
    }
    else {
        if (!strcmp("little", endian)) {
            P->input_endian = LITTLE;
        }
        else {
            E_FATAL("Input must be big or little Endian\n");
        }
    }

    if (cmd_ln_boolean_r(P->config, "-logspec")
        || cmd_ln_boolean_r(P->config, "-smoothspec"))
        P->logspec = TRUE;
    if (cmd_ln_boolean_r(P->config, "-spec2cep"))
        P->convert = SPEC2CEP;
    if (cmd_ln_boolean_r(P->config, "-cep2spec"))
        P->convert = CEP2SPEC;

    fe_validate_parameters(P);

    return (P);

}

int32
fe_build_filenames(globals_t * P, char *fileroot, char **infilename,
                   char **outfilename)
{
    char chanlabel[32];

    if (P->nchans > 1)
        sprintf(chanlabel, ".ch%d", P->whichchan);

    if (P->is_batch) {
        if (infilename != NULL) {
            *infilename = string_join(P->wavdir, "/",
                                      fileroot, ".",
                                      P->wavext, NULL);
        }

        if (outfilename != NULL) {
            if (P->nchans > 1)
                *outfilename = string_join(P->cepdir, "/",
                                           fileroot, chanlabel,
                                           ".", P->cepext, NULL);
            else
                *outfilename = string_join(P->cepdir, "/",
                                           fileroot, ".",
                                           P->cepext, NULL);
        }
    }
    else if (P->is_single) {
        if (infilename != NULL) {
            *infilename = ckd_salloc(P->wavfile);
        }
        if (outfilename != NULL) {
            *outfilename = ckd_salloc(P->cepfile);
        }
    }
    else {
        E_FATAL("Unspecified Batch or Single Mode\n");
    }

    return 0;
}

int32
fe_openfiles(globals_t * P, fe_t * FE, char *infile, int32 * fp_in,
             int32 * nsamps, int32 * nframes, int32 * nblocks,
             char *outfile, int32 * fp_out)
{
    struct stat filestats;
    int fp = 0, len = 0, outlen, numframes, numblocks, outframes;
    FILE *fp2;
    char line[MAXCHARS];
    int got_it = 0;


    /* Note: this is kind of a hack to read the byte format from the
       NIST header */
    if (P->input_format == NIST) {
        if ((fp2 = fopen(infile, "rb")) == NULL) {
            E_ERROR_SYSTEM("Cannot read %s", infile);
            return (FE_INPUT_FILE_READ_ERROR);
        }
        *line = 0;
        got_it = 0;
        while (strcmp(line, "end_head") && !got_it) {
            fscanf(fp2, "%s", line);
            if (!strcmp(line, "sample_byte_format")) {
                fscanf(fp2, "%s", line);
                if (!strcmp(line, "-s2")) {
                    fscanf(fp2, "%s", line);
                    if (!strcmp(line, "01")) {
                        P->input_endian = LITTLE;
                        got_it = 1;
                    }
                    else if (!strcmp(line, "10")) {
                        P->input_endian = BIG;
                        got_it = 1;
                    }
                    else
                        E_ERROR("Unknown/unsupported byte order\n");
                }
                else
                    E_ERROR("Error determining byte format\n");
            }
        }
        if (!got_it) {
            E_WARN
                ("Can't find byte format in header, setting to machine's endian\n");
            P->input_endian = P->machine_endian;
        }
        fclose(fp2);
    }
    else if (P->input_format == RAW) {
        /*
           P->input_endian = P->machine_endian;
         */
    }
    else if (P->input_format == MSWAV) {
        P->input_endian = LITTLE;       // Default for MS WAV riff files
    }

    /* FIXME: Why aren't we using stdio here??? */
    if ((fp = open(infile, O_RDONLY | O_BINARY, 0644)) < 0) {
        fprintf(stderr, "Cannot open %s\n", infile);
        return (FE_INPUT_FILE_OPEN_ERROR);
    }
    else {
        if (fstat(fp, &filestats) != 0)
            printf("fstat failed\n");

        if (P->input_format == NIST) {
            short *hdr_buf;

            len = (filestats.st_size - HEADER_BYTES) / sizeof(short);
            /* eat header */
            hdr_buf =
                (short *) calloc(HEADER_BYTES / sizeof(short),
                                 sizeof(short));
            if (read(fp, hdr_buf, HEADER_BYTES) != HEADER_BYTES) {
                E_ERROR("Cannot read %s\n", infile);
                return (FE_INPUT_FILE_READ_ERROR);
            }
            free(hdr_buf);
        }
        else if (P->input_format == RAW) {
            len = filestats.st_size / sizeof(int16);
        }
        else if (P->input_format == MSWAV) {
            /* Read the header */
            MSWAV_hdr *hdr_buf = NULL;
            /* MC: read till just before datatag */
            const int hdr_len_to_read = ((char *) (&hdr_buf->datatag))
                - (char *) hdr_buf;
            int data_start;

            if ((hdr_buf =
                 (MSWAV_hdr *) calloc(1, sizeof(MSWAV_hdr))) == NULL) {
                E_ERROR("Cannot allocate for input file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            if (read(fp,hdr_buf,hdr_len_to_read) != hdr_len_to_read){
                E_ERROR("Cannot allocate for input file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            /* Check header */
            if (strncmp(hdr_buf->rifftag, "RIFF", 4) != 0 ||
                strncmp(hdr_buf->wavefmttag, "WAVEfmt", 7) != 0) {
                E_ERROR("Error in mswav file header\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            {
                /* There may be other "chunks" before the data chunk,
                 * which we can ignore. We have to find the start of
                 * the data chunk, which begins with the string
                 * "data".
                 */
                int16 found = 0;
                char readChar;
                char *dataString = "data";
                int16 charPointer = 0;
                while (!found) {
                    if (read(fp, &readChar, sizeof(char)) != sizeof(char)) {
                        E_ERROR("Failed reading wav file.\n");
                        return (FE_INPUT_FILE_READ_ERROR);
                    }
                    if (readChar == dataString[charPointer]) {
                        charPointer++;
                    }
                    if (charPointer == (int) strlen(dataString)) {
                        found = 1;
                        strcpy(hdr_buf->datatag, dataString);
                        if (read
                            (fp,
                             &(hdr_buf->
                               datalength),
                             sizeof(int32)) != sizeof(int32)) {
                            E_ERROR("Failed reading wav file.\n");
                            return (FE_INPUT_FILE_READ_ERROR);
                        }
                    }
                }
            }
            data_start = lseek(fp, 0, SEEK_CUR);
            if (P->input_endian != P->machine_endian) { // If machine is Big Endian
                hdr_buf->datalength = SWAP_INT32(&(hdr_buf->datalength));
                hdr_buf->data_format = SWAP_INT16(&(hdr_buf->data_format));
                hdr_buf->numchannels = SWAP_INT16(&(hdr_buf->numchannels));
                hdr_buf->BitsPerSample =
                    SWAP_INT16(&(hdr_buf->BitsPerSample));
                hdr_buf->SamplingFreq =
                    SWAP_INT32(&(hdr_buf->SamplingFreq));
                hdr_buf->BytesPerSec = SWAP_INT32(&(hdr_buf->BytesPerSec));
            }
            /* Check Format */
            if (hdr_buf->data_format != 1 || hdr_buf->BitsPerSample != 16) {
                E_ERROR("MS WAV file not in 16-bit PCM format\n");
                return (FE_INPUT_FILE_READ_ERROR);
            }
            /* This number may be bogus.  Check for a truncated file. */
            len = hdr_buf->datalength / sizeof(short);
            if (len > (filestats.st_size - data_start) / sizeof(short))
                len = (filestats.st_size - data_start) / sizeof(short);
            
            P->nchans = hdr_buf->numchannels;
            /* DEBUG: Dump Info */
            if (P->verbose) {
                E_INFO("Reading MS Wav file %s:\n", infile);
                E_INFO
                    ("\t16 bit PCM data, %d channels %d samples\n",
                     P->nchans, len);
                E_INFO("\tSampled at %d\n", hdr_buf->SamplingFreq);
            }
            free(hdr_buf);
        }
        else {
            E_ERROR("Unknown input file format\n");
            return (FE_INPUT_FILE_OPEN_ERROR);
        }
    }


    len = len / P->nchans;
    *nsamps = len;
    *fp_in = fp;

    numblocks = (int) ((float) len / (float) P->blocksize);
    if (numblocks * P->blocksize < len)
        numblocks++;

    *nblocks = numblocks;

    if ((fp =
         open(outfile, O_CREAT | O_WRONLY | O_TRUNC | O_BINARY,
              0644)) < 0) {
        E_ERROR("Unable to open %s for writing features\n", outfile);
        return (FE_OUTPUT_FILE_OPEN_ERROR);
    }
    else {
        size_t nsamps = len;
        int frame_shift, frame_size;

        /* Compute number of frames and write cepfile header */
        fe_process_frames(FE, NULL, &nsamps, NULL, &numframes, &outframes);
        /* This is sort of hacky... we need to figure out if there
           will be a trailing frame from fe_end_utt() or not.  */
        fe_get_input_size(FE, &frame_shift, &frame_size);
        /* Don't ask me why this has to be <= rather than <, it just does... */
        if (frame_size + (numframes - 1) * frame_shift <= len)
            ++numframes;

        outlen = numframes * fe_get_output_size(FE);
        if (P->output_endian != P->machine_endian)
            SWAP_INT32(&outlen);
        if (write(fp, &outlen, 4) != 4) {
            E_ERROR("Data write error on %s\n", outfile);
            close(fp);
            return (FE_OUTPUT_FILE_WRITE_ERROR);
        }
        if (P->output_endian != P->machine_endian)
            SWAP_INT32(&outlen);
    }

    *nframes = numframes;
    *fp_out = fp;

    return 0;
}

int32
fe_readblock_spch(globals_t * P, int32 fp, int32 nsamps, int16 * buf)
{
    int32 bytes_read, cum_bytes_read, nreadbytes, actsamps, offset, i,
        j, k;
    int16 *tmpbuf;
    int32 nchans, whichchan;

    nchans = P->nchans;
    whichchan = P->whichchan;

    if (nchans == 1) {
        if (P->input_format == RAW
            || P->input_format == NIST
            || P->input_format == MSWAV) {
            nreadbytes = nsamps * sizeof(int16);
            if ((bytes_read = read(fp, buf, nreadbytes)) != nreadbytes) {
                E_ERROR_SYSTEM("error reading block: %ld != %d",
                               bytes_read, nreadbytes);
                return (0);
            }
        }
        else {
            E_ERROR("unknown input file format\n");
            return (0);
        }
        cum_bytes_read = bytes_read;
    }
    else if (nchans > 1) {

        if (nsamps < P->blocksize) {
            actsamps = nsamps * nchans;
            tmpbuf = (int16 *) calloc(nsamps * nchans, sizeof(int16));
            cum_bytes_read = 0;
            if (P->input_format == RAW
                || P->input_format == MSWAV || P->input_format == NIST) {

                k = 0;
                nreadbytes = actsamps * sizeof(int16);

                if ((bytes_read =
                     read(fp, tmpbuf, nreadbytes)) != nreadbytes) {
                    E_ERROR
                        ("error reading block (got %d not %d)\n",
                         bytes_read, nreadbytes);
                    return (0);
                }

                for (j = whichchan - 1; j < actsamps; j = j + nchans) {
                    buf[k] = tmpbuf[j];
                    k++;
                }
                cum_bytes_read += bytes_read / nchans;
            }
            else {
                E_ERROR("unknown input file format\n");
                return (0);
            }
            free(tmpbuf);
        }
        else {
            tmpbuf = (int16 *) calloc(nsamps, sizeof(int16));
            actsamps = nsamps / nchans;
            cum_bytes_read = 0;

            if (actsamps * nchans != nsamps) {
                E_WARN
                    ("Blocksize %d is not an integer multiple of Number of channels %d\n",
                     nsamps, nchans);
            }

            if (P->input_format == RAW
                || P->input_format == MSWAV || P->input_format == NIST) {
                for (i = 0; i < nchans; i++) {

                    offset = i * actsamps;
                    k = 0;
                    nreadbytes = nsamps * sizeof(int16);

                    if ((bytes_read =
                         read(fp, tmpbuf, nreadbytes)) != nreadbytes) {
                        E_ERROR
                            ("error reading block (got %d not %d)\n",
                             bytes_read, nreadbytes);
                        return (0);
                    }

                    for (j = whichchan - 1; j < nsamps; j = j + nchans) {
                        buf[offset + k] = tmpbuf[j];
                        k++;
                    }
                    cum_bytes_read += bytes_read / nchans;
                }
            }
            else {
                E_ERROR("unknown input file format\n");
                return (0);
            }
            free(tmpbuf);
        }
    }

    else {
        E_ERROR("unknown number of channels!\n");
        return (0);
    }

    if (P->input_endian != P->machine_endian) {
        for (i = 0; i < nsamps; i++)
            SWAP_INT16(&buf[i]);
    }

    return (cum_bytes_read / sizeof(int16));

}

int32
fe_writeblock_feat(globals_t * P, fe_t * FE, int32 fp, int32 nframes,
                   mfcc_t ** feat)
{

    int32 i, length, nwritebytes;
    float32 **ffeat;

    length = nframes * fe_get_output_size(FE);

    ffeat = (float32 **) feat;
    fe_mfcc_to_float(FE, feat, ffeat, nframes);
    if (P->output_endian != P->machine_endian) {
        for (i = 0; i < length; ++i)
            SWAP_FLOAT32(ffeat[0] + i);
    }

    nwritebytes = length * sizeof(float32);
    if (write(fp, ffeat[0], nwritebytes) != nwritebytes) {
        close(fp);
        E_FATAL("Error writing block of features\n");
    }

    return (length);
}


int32
fe_closefiles(int32 fp_in, int32 fp_out)
{
    close(fp_in);
    close(fp_out);
    return 0;
}

int32
fe_convert_with_dct(globals_t * P, fe_t * FE, char *infile, char *outfile)
{
    FILE *ifh, *ofh;
    int32 ifsize, nfloats, swap = 0;
    int32 input_ncoeffs, output_ncoeffs;
    float32 *logspec;

    if ((ifh = fopen(infile, "rb")) == NULL) {
        E_ERROR_SYSTEM("Cannot read %s", infile);
        return (FE_INPUT_FILE_READ_ERROR);
    }
    if ((ofh = fopen(outfile, "wb")) == NULL) {
        E_ERROR_SYSTEM("Unable to open %s for writing features", outfile);
        return (FE_OUTPUT_FILE_OPEN_ERROR);
    }

    fseek(ifh, 0, SEEK_END);
    ifsize = ftell(ifh);
    fseek(ifh, 0, SEEK_SET);
    fread(&nfloats, 4, 1, ifh);
    if (nfloats != ifsize / 4 - 1) {
        E_INFO("Will byteswap %s (%x != %x)\n",
               infile, nfloats, ifsize / 4 - 1);
        SWAP_INT32(&nfloats);
        swap = 1;
    }
    if (nfloats != ifsize / 4 - 1) {
        E_ERROR("Size of file doesn't match header: %d != %d\n",
                nfloats, ifsize / 4 - 1);
        return (FE_INPUT_FILE_READ_ERROR);
    }
    if (P->convert == CEP2SPEC) {
        input_ncoeffs = cmd_ln_int32_r(P->config, "-ncep");
        output_ncoeffs = cmd_ln_int32_r(P->config, "-nfilt");
    }
    else {
        input_ncoeffs = cmd_ln_int32_r(P->config, "-nfilt");
        output_ncoeffs = cmd_ln_int32_r(P->config, "-ncep");
    }
    nfloats = nfloats * output_ncoeffs / input_ncoeffs;

    if (swap)
        SWAP_INT32(&nfloats);
    fwrite(&nfloats, 4, 1, ofh);
    /* Always use the largest size since it's done inplace */
    logspec = ckd_calloc(cmd_ln_int32_r(P->config, "-nfilt"),
                         sizeof(*logspec));

    while (fread(logspec, 4, input_ncoeffs, ifh) == input_ncoeffs) {
        int32 i;
        if (swap) {
            for (i = 0; i < input_ncoeffs; ++i) {
                SWAP_FLOAT32(logspec + i);
            }
        }
        fe_float_to_mfcc(FE, &logspec, (mfcc_t **)&logspec, 1);
        if (P->convert == CEP2SPEC) {
            fe_mfcc_dct3(FE, (mfcc_t *)logspec, (mfcc_t *)logspec);
        }
        else {
            if (0 == strcmp(cmd_ln_str_r(P->config, "-transform"), "legacy"))
                fe_logspec_to_mfcc(FE, (mfcc_t *)logspec, (mfcc_t *)logspec);
            else
                fe_logspec_dct2(FE, (mfcc_t *)logspec, (mfcc_t *)logspec);
        }
        fe_mfcc_to_float(FE, (mfcc_t **)&logspec, &logspec, 1);
        if (swap) {
            for (i = 0; i < output_ncoeffs; ++i) {
                SWAP_FLOAT32(logspec + i);
            }
        }
        if (fwrite(logspec, 4, output_ncoeffs, ofh) < output_ncoeffs) {
            E_ERROR_SYSTEM("Failed to write %d coeffs to %s",
                           output_ncoeffs, outfile);
            ckd_free(logspec);
            return (FE_OUTPUT_FILE_WRITE_ERROR);
        }
    }
    if (!feof(ifh)) {
        E_ERROR("Short read in input file %s\n", infile);
        ckd_free(logspec);
        return (FE_INPUT_FILE_READ_ERROR);
    }
    fclose(ifh);
    fclose(ofh);
    ckd_free(logspec);

    return FE_SUCCESS;
}

/*
 * Log record.  Maintained by RCS.
 *
 * $Log: wave2feat.c,v $
 * Revision 1.35  2006/02/25 00:53:48  egouvea
 * Added the flag "-seed". If dither is being used and the seed is less
 * than zero, the random number generator is initialized with time(). If
 * it is at least zero, it's initialized with the provided seed. This way
 * we have the benefit of having dither, and the benefit of being
 * repeatable.
 *
 * This is consistent with what sphinx3 does. Well, almost. The random
 * number generator is still what the compiler provides.
 *
 * Also, moved fe_init_params to fe_interface.c, so one can initialize a
 * variable of type param_t with meaningful values.
 *
 * Revision 1.34  2006/02/20 23:55:51  egouvea
 * Moved fe_dither() to the "library" side rather than the app side, so
 * the function can be code when using the front end as a library.
 *
 * Revision 1.33  2006/02/17 00:31:34  egouvea
 * Removed switch -melwarp. Changed the default for window length to
 * 0.025625 from 0.256 (so that a window at 16kHz sampling rate has
 * exactly 410 samples). Cleaned up include's. Replaced some E_FATAL()
 * with E_WARN() and return.
 *
 * Revision 1.32  2006/02/16 20:11:20  egouvea
 * Fixed the code that prints a warning if any zero-energy frames are
 * found, and recommending the user to add dither. Previously, it would
 * only report the zero energy frames if they happened in the last
 * utterance. Now, it reports for each utterance.
 *
 * Revision 1.31  2006/02/16 00:18:26  egouvea
 * Implemented flexible warping function. The user can specify at run
 * time which of several shapes they want to use. Currently implemented
 * are an affine function (y = ax + b), an inverse linear (y = a/x) and a
 * piecewise linear (y = ax, up to a frequency F, and then it "breaks" so
 * Nyquist frequency matches in both scales.
 *
 * Added two switches, -warp_type and -warp_params. The first specifies
 * the type, which valid values:
 *
 * -inverse or inverse_linear
 * -linear or affine
 * -piecewise or piecewise_linear
 *
 * The inverse_linear is the same as implemented by EHT. The -mel_warp
 * switch was kept for compatibility (maybe remove it in the
 * future?). The code is compatible with EHT's changes: cepstra created
 * from code after his changes should be the same as now. Scripts that
 * worked with his changes should work now without changes. Tested a few
 * cases, same results.
 *
 */
