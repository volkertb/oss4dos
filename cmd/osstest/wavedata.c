/*
 * Purpose: Test sounds for osstest
 *
 * Nodoc:
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


#include <string.h>

#include "wavedata.h"
extern int play_gain; // 0 to 100

static int
le_int (const unsigned char *p, int l)
{
  int i, val;

  val = 0;

  for (i = l - 1; i >= 0; i--)
    {
      val = (val << 8) | p[i];
    }

  return val;
}

int
uncompress_wave (short *outbuf)
{
#define WAVE_FORMAT_ADPCM		0x0002

  int i, n, dataleft, x, l = sizeof (inbuf);
  const unsigned char *hdr = inbuf;
  typedef struct
  {
    int coeff1, coeff2;
  }
  adpcm_coeff;

  adpcm_coeff coeff[32];
  static int AdaptionTable[] = { 230, 230, 230, 230, 307, 409, 512, 614,
    768, 614, 512, 409, 307, 230, 230, 230
  };

  unsigned char buf[4096];

  int channels = 1;
  int p = 12, outp = 0;
  int nBlockAlign = 2048;
  int wSamplesPerBlock = 2036, wNumCoeff = 7;
  int nib;
  int ppp;

  /* filelen = le_int (&hdr[4], 4); */

  while (p < l - 16 && memcmp (&hdr[p], "data", 4) != 0)
    {
      n = le_int (&hdr[p + 4], 4);

      if (memcmp (&hdr[p], "fmt ", 4) == 0)
	{

	  /* fmt = le_int (&hdr[p + 8], 2); */
	  channels = le_int (&hdr[p + 10], 2);
	  /* speed = le_int (&hdr[p + 12], 4); */
	  nBlockAlign = le_int (&hdr[p + 20], 2);
	  /* bytes_per_sample = le_int (&hdr[p + 20], 2); */

	  wSamplesPerBlock = le_int (&hdr[p + 26], 2);
	  wNumCoeff = le_int (&hdr[p + 28], 2);

	  x = p + 30;

	  for (i = 0; i < wNumCoeff; i++)
	    {
	      coeff[i].coeff1 = (short) le_int (&hdr[x], 2);
	      x += 2;
	      coeff[i].coeff2 = (short) le_int (&hdr[x], 2);
	      x += 2;
	    }

	}

      p += n + 8;
    }

  if (p < l - 16 && memcmp (&hdr[p], "data", 4) == 0)
    {

      dataleft = n = le_int (&hdr[p + 4], 4);
      p += 8;

/*
 * Playback procedure
 */
#define OUT_SAMPLE(s) { \
	if (s>32767)s=32767;else if(s<-32768)s=-32768; \
	outbuf[outp++] = (s*play_gain) / 100; \
	n+=2; \
	}

#define GETNIBBLE \
	((nib==0) ? \
		(buf[x + nib++] >> 4) & 0x0f : \
		buf[x++ + --nib] & 0x0f \
	)

      outp = 0;

      ppp = p;
      while (dataleft > nBlockAlign)
	{
	  int predictor[2], delta[2], samp1[2], samp2[2];

	  int x = 0;

	  memcpy (buf, &inbuf[ppp], nBlockAlign);
	  ppp += nBlockAlign;
	  dataleft -= nBlockAlign;

	  nib = 0;
	  n = 0;

	  for (i = 0; i < channels; i++)
	    {
	      predictor[i] = buf[x];
	      x++;
	    }

	  for (i = 0; i < channels; i++)
	    {
	      delta[i] = (short) le_int (&buf[x], 2);
	      x += 2;
	    }

	  for (i = 0; i < channels; i++)
	    {
	      samp1[i] = (short) le_int (&buf[x], 2);
	      x += 2;
	      OUT_SAMPLE (samp1[i]);
	    }

	  for (i = 0; i < channels; i++)
	    {
	      samp2[i] = (short) le_int (&buf[x], 2);
	      x += 2;
	      OUT_SAMPLE (samp2[i]);
	    }

	  while (n < (wSamplesPerBlock * 2 * channels))
	    for (i = 0; i < channels; i++)
	      {
		int pred, new, error_delta, i_delta;

		pred = ((samp1[i] * coeff[predictor[i]].coeff1)
			+ (samp2[i] * coeff[predictor[i]].coeff2)) / 256;
		i_delta = error_delta = GETNIBBLE;

		if (i_delta & 0x08)
		  i_delta -= 0x10;	/* Convert to signed */

		new = pred + (delta[i] * i_delta);
		OUT_SAMPLE (new);

		delta[i] = delta[i] * AdaptionTable[error_delta] / 256;
		if (delta[i] < 16)
		  delta[i] = 16;

		samp2[i] = samp1[i];
		samp1[i] = new;
	      }
	}

    }

  return outp * 2;
}
