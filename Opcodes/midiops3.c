/*
    midiops3.c:

    Copyright (C) 1997 Gabriel Maldonado

    This file is part of Csound.

    The Csound Library is free software; you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    Csound is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with Csound; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA
*/

/* sliders and other MIDI opcodes by Gabriel Maldonado */

#include "csdl.h"
#include "midiops.h"
#include "midiops3.h"
#include <math.h>

#define f7bit           (FL(127.0))
#define oneTOf7bit      (MYFLT)(1./127.)
#define f14bit          (FL(16383.0))
#define oneTOf14bit     (MYFLT)(1/16383.)
#define f21bit          (FL(2097151.0))
#define oneTOf21bit     (MYFLT)(1/2097151.)

/* This set of macros is rather a cop-out! */
#define SLIDERI_INIT(p, n)                                        \
{                                                                 \
    unsigned char chan = p->slchan = (unsigned char)((*p->ichan)-1); \
    char sbuf[120];                                               \
    if (chan  > 15)  {                                            \
      return initerror(Str(X_840,"illegal channel"));             \
    }                                                             \
    {                                                             \
      MYFLT value;                                                \
      int j = 0;                                                  \
      SLD *sld = p->s;                                            \
      unsigned char *slnum = p->slnum;                            \
      MYFLT *min = p->min, *max= p->max;                          \
      FUNC **ftp = p->ftp;                                        \
      MYFLT *chanblock = (MYFLT *) M_CHNBP[chan]->ctl_val;        \
      while (j++ < n) {                                           \
      *slnum = (unsigned char) *sld->ictlno;                      \
        if (*slnum > 127) {                                       \
          sprintf(sbuf,                                           \
                  Str(X_843,"illegal control number at position n.%d"), j); \
          return initerror(sbuf);                                 \
        }                                                         \
        if ((value=*sld->initvalue) < (*min=*sld->imin) ||        \
            value > (*max=*sld->imax) ) {                         \
          sprintf(sbuf,                                           \
                  Str(X_856,"illegal initvalue at position n.%d"),\
                  j);                                             \
          return initerror(sbuf);                                 \
        }                                                         \
        if (*sld->ifn > 0)   *ftp++ = ftfind(csound, sld->ifn);   \
        else                 *ftp++ = NULL;                       \
        value =  (*(sld++)->initvalue - *min) / (*max++ - *min);  \
        min++;                                                    \
        chanblock[*slnum++] =  (MYFLT)((int)(value * f7bit + FL(0.5)));\
      }                                                           \
    }                                                             \
    return OK;                                                    \
}

#define SLIDER_INIT(p, n)                                         \
{                                                                 \
    MYFLT value;                                                  \
    int j = 0;                                                    \
    FUNC **ftp = p->ftp-1;                                        \
    MYFLT *chanblock = (MYFLT *) M_CHNBP[p->slchan]->ctl_val;     \
    unsigned char  *slnum = p->slnum;                             \
    MYFLT *min = p->min, *max = p->max;                           \
    MYFLT **result = p->r;                                        \
    while (j++ < n) {                                             \
      value = (MYFLT) (chanblock[*slnum++] * oneTOf7bit);         \
      if (*(++ftp))             /* if valid ftable,use value as index   */  \
        value = *((*ftp)->ftable + (long)(value * (*ftp)->flen)); \
                                /* no interpolation */            \
      **result++ = value * (*max++ - *min) + *min;   /* scales the output */ \
      min++;;                                                     \
    }                                                             \
    return OK;                                                    \
}


/*--------------------------------------------------------*/

int slider_i8(ENVIRON *csound, SLIDER8 *p)
{
    SLIDERI_INIT(p, 8);
}


int slider8(ENVIRON *csound, SLIDER8 *p)
{
    SLIDER_INIT(p, 8);
}

int slider_i16(ENVIRON *csound, SLIDER16 *p)
{
    SLIDERI_INIT(p, 16);
}


int slider16(ENVIRON *csound, SLIDER16 *p)
{
    SLIDER_INIT(p, 16);
}

int slider_i32(ENVIRON *csound, SLIDER32 *p)
{
    SLIDERI_INIT(p, 32);
}


int slider32(ENVIRON *csound, SLIDER32 *p)
{
    SLIDER_INIT(p, 32);
}

int slider_i64(ENVIRON *csound, SLIDER64 *p)
{
    SLIDERI_INIT(p, 64);
}

int slider64(ENVIRON *csound, SLIDER64 *p)
{
    SLIDER_INIT(p, 64);
}


/*==============================*/
#define SLIDERIF(p, n)                                            \
{                                                                 \
    unsigned char chan = p->slchan = (unsigned char)((*p->ichan)-1); \
    char sbuf[120];                                               \
    if (chan  > 15)  {                                            \
      return initerror(Str(X_840,"illegal channel"));                    \
    }                                                             \
    {                                                             \
      MYFLT value;                                                \
      int j = 0;                                                  \
      SLDf *sld = p->s;                                           \
      unsigned char *slnum = p->slnum;                            \
      MYFLT *min = p->min, *max= p->max;                          \
      FUNC **ftp = p->ftp;                                        \
      MYFLT     b;                                                \
      MYFLT *yt1 = p->yt1, *c1=p->c1, *c2=p->c2;                  \
      MYFLT *chanblock = (MYFLT *) M_CHNBP[chan]->ctl_val;        \
      while (j++ < 8) {                                           \
      *slnum = (unsigned char) *sld->ictlno;                      \
        if (*slnum > 127) {                                       \
          sprintf(sbuf,                                           \
                  Str(X_843,"illegal control number at position n.%d"), j); \
          return initerror(sbuf);                                        \
        }                                                         \
        if ((value=*sld->initvalue) < (*min=*sld->imin) ||        \
            value > (*max=*sld->imax) ) {                         \
          sprintf(sbuf,                                           \
                  Str(X_856,"illegal initvalue at position n.%d"), j); \
          return initerror(sbuf);                                 \
        }                                                         \
        if (*sld->ifn > 0)   *ftp++ = ftfind(csound, sld->ifn);           \
        else                 *ftp++ = NULL;                       \
        value =  (*sld->initvalue - *min) / (*max++ - *min);      \
        min++;;                                                   \
        chanblock[*slnum++] =  (MYFLT)(int)(value * f7bit + .5);  \
                                                                  \
                /*----- init filtering coeffs*/                   \
        *yt1++ = FL(0.0);                                         \
        b = (MYFLT)(2.0 - cos((double)(*(sld++)->ihp * tpidsr * ksmps))); \
        *c2 = (MYFLT)(b - sqrt((double)(b * b - FL(1.0))));       \
        *c1++ = FL(1.0) - *c2++;                                  \
      }                                                           \
    }                                                             \
    return OK;                                                    \
}

#define SLIDERF(p, n)                                             \
{                                                                 \
    MYFLT value;                                                  \
    int j = 0;                                                    \
    FUNC **ftp = p->ftp-1;                                        \
    MYFLT *chanblock = (MYFLT *) M_CHNBP[p->slchan]->ctl_val;     \
    unsigned char  *slnum = p->slnum;                             \
    MYFLT *min = p->min, *max = p->max;                           \
    MYFLT **result = p->r;                                        \
    MYFLT *yt1 = p->yt1, *c1=p->c1, *c2=p->c2;                    \
    while (j++ < n) {                                             \
      value = chanblock[*slnum++] * oneTOf7bit;                   \
      if (*(++ftp))    /* if valid ftable,use value as index   */ \
        value = *( (*ftp)->ftable + (long)(value * (*ftp)->flen));\
      value = value * (*max++ - *min) + *min; /* scales the output */ \
      min++;                                                      \
      **result++ =                                                \
        *yt1++ = *c1++ * value + *c2++ * *yt1; /* filters the output */ \
    }                                                             \
    return OK;                                                    \
}

int slider_i8f(ENVIRON *csound, SLIDER8f *p)
{
    SLIDERIF(p, 8);
}

int slider8f(ENVIRON *csound, SLIDER8f *p)
{
    SLIDERF(p, 8);
}

int slider_i16f(ENVIRON *csound, SLIDER16f *p)
{
    SLIDERIF(p, 16);
}

int slider16f(ENVIRON *csound, SLIDER16f *p)
{
    SLIDERF(p, 16);
}

int slider_i32f(ENVIRON *csound, SLIDER32f *p)
{
    SLIDERIF(p, 32);
}

int slider32f(ENVIRON *csound, SLIDER32f *p)
{
    SLIDERF(p, 32);
}


int slider_i64f(ENVIRON *csound, SLIDER64f *p)
{
    SLIDERIF(p, 64);
}

int slider64f(ENVIRON *csound, SLIDER64f *p)
{
    SLIDERF(p, 64);
}

/*===================================*/

#define ISLIDER(p, n)                                             \
{                                                                 \
    unsigned char chan= (unsigned char) ((*p->ichan)-1);          \
    char sbuf[120];                                               \
    if (chan  > 15)  {                                            \
      return initerror(Str(X_840,"illegal channel"));             \
    }                                                             \
    {                                                             \
      MYFLT value;                                                \
      int j = 0;                                                  \
      ISLD *sld = p->s;                                           \
      unsigned char slnum;                                        \
      MYFLT *chanblock = (MYFLT *) M_CHNBP[chan]->ctl_val;        \
      FUNC *ftp;                                                  \
      MYFLT **result = p->r;                                      \
                                                                  \
      while (j++ < n) {                                           \
        slnum=(unsigned char) *sld->ictlno;                       \
        if (slnum > 127) {                                        \
          sprintf(sbuf, Str(X_843,"illegal control number at position n.%d"), j); \
          return initerror(sbuf);                                 \
        }                                                         \
        value = chanblock[slnum] * oneTOf7bit;                    \
        if (*sld->ifn > 0)  {                                     \
          ftp = ftfind(csound, sld->ifn);         \
          value = *( ftp->ftable + (long)(value * ftp->flen));    \
                                /* no interpolation */            \
        }                                                         \
        **result++ = value * (*sld->imax - *sld->imin) + *sld->imin;   /* scales the output */ \
        sld++;                                                    \
      }                                                           \
    }                                                             \
    return OK;                                                    \
}


int islider8(ENVIRON *csound, ISLIDER8 *p)
{
    ISLIDER(p, 8);
}

int islider16(ENVIRON *csound, ISLIDER16 *p)
{
    ISLIDER(p, 16);
}

int islider32(ENVIRON *csound, ISLIDER32 *p)
{
    ISLIDER(p, 32);
}

int islider64(ENVIRON *csound, ISLIDER64 *p)
{
    ISLIDER(p, 64);
}



/*-------------------------------*/

#define SLIDERI14(p, n)                                                \
{                                                                      \
    unsigned char chan= p->slchan = (unsigned char)((*p->ichan)-1);    \
    char sbuf[120];                                                    \
    if (chan  > 15)  {                                                 \
      return initerror(Str(X_840,"illegal channel"));                  \
    }                                                                  \
    {                                                                  \
      MYFLT value;                                                     \
      int intvalue, j = 0;                                             \
      SLD14 *sld = p->s;                                               \
      unsigned char *slnum_msb = p->slnum_msb;                         \
      unsigned char *slnum_lsb = p->slnum_lsb;                         \
      MYFLT *min = p->min, *max= p->max;                               \
      FUNC **ftp = p->ftp;                                             \
      MYFLT *chanblock = (MYFLT *) M_CHNBP[chan]->ctl_val;             \
                                                                       \
      while (j++ < n) {                                                \
        *slnum_msb = (unsigned char)*sld->ictlno_msb;                  \
        if (*slnum_msb > 127) {                                        \
          sprintf(sbuf,                                                \
                  Str(X_872,"illegal msb control number at position n.%d"), \
                  j);                                                  \
          return initerror(sbuf);                                      \
        }                                                              \
        *slnum_lsb = (unsigned char)*sld->ictlno_lsb;                  \
        if (*slnum_lsb > 127) {                                        \
          sprintf(sbuf,                                                \
                  Str(X_868,"illegal lsb control number at position n.%d"), \
                  j);                                                  \
          return initerror(sbuf);                                      \
        }                                                              \
        if ((value=*sld->initvalue) < (*min=*sld->imin) ||             \
            value > (*max=*sld->imax) ) {                              \
          sprintf(sbuf,                                                \
                  Str(X_856,"illegal initvalue at position n.%d"), j); \
          return initerror(sbuf);                                      \
        }                                                              \
        if (*sld->ifn > 0)   *ftp++ = ftfind(csound, sld->ifn); \
        else                 *ftp++ = NULL;                            \
        intvalue = (int) (((*(sld++)->initvalue - *min) / (*max++ - *min)) \
                          * f14bit+FL(0.5));                           \
        min++;                                                         \
        chanblock[*slnum_msb++] =  (MYFLT) (intvalue >> 7);            \
        chanblock[*slnum_lsb++] =  (MYFLT) (intvalue & 0x7f);          \
      }                                                                \
    }                                                                  \
    return OK;                                                         \
}

#define SLIDER14(p, n)                                                 \
{                                                                      \
    MYFLT value;                                                       \
    int j = 0;                                                         \
    FUNC **ftp = p->ftp-1;                                             \
    MYFLT *chanblock = (MYFLT *) M_CHNBP[p->slchan]->ctl_val;          \
    unsigned char  *slnum_msb = p->slnum_msb;                          \
    unsigned char  *slnum_lsb = p->slnum_lsb;                          \
    MYFLT *min = p->min, *max = p->max;                                \
    MYFLT **result = p->r;                                             \
                                                                       \
    while (j++ < n) {                                                  \
      value = (MYFLT)((chanblock[*slnum_msb++]  * 128                  \
                       + chanblock[*slnum_lsb++]) * oneTOf14bit);      \
      if (*(++ftp)) {      /* if valid ftable,use value as index   */  \
        MYFLT phase = value * (*ftp)->flen;                            \
        MYFLT *base = (*ftp)->ftable + (long)(phase);                  \
        value = *base + (*(base+1) - *base) * (phase - (long) phase);  \
      }                                                                \
      **result++ = value * (*max++ - *min) + *min; /* scales the output */ \
      min++;                                                           \
    }                                                                  \
    return OK;                                                    \
}

int slider_i16bit14(ENVIRON *csound, SLIDER16BIT14 *p)
{
    SLIDERI14(p, 16);
}

int slider16bit14(ENVIRON *csound, SLIDER16BIT14 *p)
{
    SLIDER14(p, 16);
}

int slider_i32bit14(ENVIRON *csound, SLIDER32BIT14 *p)
{
    SLIDERI14(p, 32);
}


int slider32bit14(ENVIRON *csound, SLIDER32BIT14 *p)
{
    SLIDER14(p, 32);
}

/*--------------------------------*/
#define ISLIDER14(p, n)                                                \
{                                                                      \
    unsigned char chan = (unsigned char)((*p->ichan)-1);               \
    char sbuf[120];                                                    \
    if (chan  > 15)  {                                                 \
      return initerror(Str(X_840,"illegal channel"));                  \
    }                                                                  \
    {                                                                  \
      MYFLT value;                                                     \
      int j = 0;                                                       \
      ISLD14 *sld = p->s;                                              \
      unsigned char slnum_msb;                                         \
      unsigned char slnum_lsb;                                         \
      MYFLT *chanblock = (MYFLT *) M_CHNBP[chan]->ctl_val;             \
      MYFLT **result = p->r;                                           \
                                                                       \
      while (j++ < n) {                                                \
        slnum_msb=(unsigned char)*sld->ictlno_msb;                     \
        if (slnum_msb > 127) {                                         \
          sprintf(sbuf,                                                \
                  Str(X_872,"illegal msb control number at position n.%d"), \
                  j);                                                  \
          return initerror(sbuf);                                      \
        }                                                              \
        slnum_lsb=(unsigned char)*sld->ictlno_lsb;                     \
        if (slnum_lsb > 127) {                                         \
          sprintf(sbuf,                                                \
                  Str(X_868,"illegal lsb control number at position n.%d"), \
                  j);                                                  \
          return initerror(sbuf);                                      \
        }                                                              \
                                                                       \
        value = (MYFLT)((chanblock[slnum_msb]  * 128                   \
                         + chanblock[slnum_lsb]) * oneTOf14bit);       \
        if (*sld->ifn > 0) {    /* linear interpolation routine */     \
          FUNC *ftp= ftfind(csound, sld->ifn);         \
          MYFLT phase = value * ftp->flen;                             \
          MYFLT *base = ftp->ftable + (long)(phase);                   \
          value = *base + (*(base + 1) - *base) * (phase - (long) phase); \
        }                                                              \
                                /* scales the output */                \
        **result++ = value * (*sld->imax - *sld->imin) + *sld->imin;   \
        sld++;                                                         \
      }                                                                \
    }                                                                  \
    return OK;                                                         \
}

int islider16bit14(ENVIRON *csound, ISLIDER16BIT14 *p)
{
    ISLIDER14(p, 16);
}

int islider32bit14(ENVIRON *csound, ISLIDER32BIT14 *p)
{
    ISLIDER14(p, 16);
}

#define S       sizeof

static OENTRY localops[] = {
{ "s16b14", 0xffff,                                                     },
{ "s32b14", 0xffff,                                                     },
{ "slider16", 0xffff,                                                   },
{ "slider32", 0xffff,                                                   },
{ "slider64", 0xffff,                                                   },
{ "slider8", 0xffff,                                                    },
{ "slider8_k", S(SLIDER8), 3, "kkkkkkkk",  "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                              "iiiiiiii", (SUBR)slider_i8, (SUBR)slider8, NULL },
{ "slider8f", S(SLIDER8f), 3, "kkkkkkkk","iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiii",
                                        (SUBR)slider_i8f, (SUBR)slider8f, NULL },
{ "slider8_i", S(SLIDER8), 1, "iiiiiiii", "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                          (SUBR)islider8, NULL, NULL },
{ "slider16_k", S(SLIDER16), 3, "kkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiii",
                                        (SUBR)slider_i16, (SUBR)slider16, NULL },
{ "slider16f", S(SLIDER16f), 3, "kkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)slider_i16f, (SUBR)slider16f, NULL },
{ "slider16_i", S(SLIDER16), 1, "iiiiiiiiiiiiiiii",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)islider16, NULL, NULL       },
{ "slider32_k", S(SLIDER32),  3, "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)slider_i32, (SUBR)slider32, NULL  },
{ "slider32f", S(SLIDER32f), 3, "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiii",
                                        (SUBR)slider_i32f, (SUBR)slider32f, NULL },
{ "slider32_i", S(SLIDER32), 1, "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)islider32, NULL, NULL  },
{ "slider64_k", S(SLIDER64), 3, "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk"
                              "kkkkkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)slider_i64, (SUBR)slider64, NULL  },
{ "slider64f", S(SLIDER64f), 3, "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk"
                                "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)slider_i64f, (SUBR)slider64f, NULL },
{ "slider64_i", S(SLIDER64), 1, "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiii",
                                        (SUBR)islider64, NULL, NULL  },
{ "s16b14_k", S(SLIDER16BIT14), 3, "kkkkkkkkkkkkkkkk",
                                   "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                   "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                   "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                 (SUBR)slider_i16bit14, (SUBR)slider16bit14, NULL},
{ "s32b14_k", S(SLIDER32BIT14), 3, "kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                 (SUBR)slider_i32bit14, (SUBR)slider32bit14, NULL},
{ "s16b14_i", S(ISLIDER16BIT14), 1, "iiiiiiiiiiiiiiii",
                                        "iiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiii",
                                        (SUBR)islider16bit14, NULL, NULL  },
{ "s32b14_i", S(ISLIDER32BIT14), 1, "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii"
                                        "iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii",
                                        (SUBR)islider32bit14, NULL, NULL  },
};

LINKAGE
