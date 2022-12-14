/*
 * Purpose: Local input buffer to application export routine for vmix (int)
 */
/*
 * This file is part of Open Sound System
 *
 * Copyright (C) 4Front Technologies 1996-2008.
 *
 * This software is released under the BSD license.
 * See the COPYING file included in the main directory of this source
 * distribution for the license terms and conditions
 */


vmix_mixer_t *mixer = portc->mixer;
vmix_engine_t *eng = &mixer->record_engine;

dmap_t *dmap = audio_engines[portc->audio_dev]->dmap_in;

int frame_size;

int outptr, outmax;
int used_channels;
int i, ch;

/*
 * Initial setup
 */

frame_size = sizeof (*outp);

outmax = dmap->bytes_in_use / frame_size;
outptr = portc->rec_dma_pointer / frame_size;

outp = (BUFFER_TYPE) dmap->dmabuf;

used_channels = portc->channels;
if (used_channels > eng->channels)
  used_channels = eng->channels;

/* ignored_channels = portc->channels - used_channels; */

/*
 * Do the mixing
 */
for (ch = 0; ch < used_channels; ch++)
  {
    int op = outptr + ch;
    int *chbuf = eng->chbufs[ch+portc->rec_choffs];

    for (i = 0; i < nsamples; i++)
      {
	int tmp;

	tmp = *chbuf++;
	/*
	 * Convert the sample to right endianess. 
	 */
	outp[op] = VMIX_BYTESWAP (INT_EXPORT(tmp));

	op = (op + portc->channels) % outmax;
      }
  }

/*
 * Finally save the state variables
 */

portc->rec_dma_pointer =
  (portc->rec_dma_pointer +
   nsamples * frame_size * portc->channels) % dmap->bytes_in_use;
