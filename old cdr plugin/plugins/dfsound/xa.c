/***************************************************************************
                            xa.c  -  description
                             -------------------
    begin                : Wed May 15 2002
    copyright            : (C) 2002 by Pete Bernert
    email                : BlackDove@addcom.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version. See also the license.txt file for *
 *   additional informations.                                              *
 *                                                                         *
 ***************************************************************************/

//*************************************************************************//
// History of changes:
//
// 2003/02/18 - kode54
// - added gaussian interpolation
//
// 2002/05/15 - Pete
// - generic cleanup for the Peops release
//
//*************************************************************************//

#include "stdafx.h"

#define _IN_XA

// will be included from spu.c
#ifdef _IN_SPU

////////////////////////////////////////////////////////////////////////
// XA GLOBALS
////////////////////////////////////////////////////////////////////////

xa_decode_t   * xapGlobal=0;

int iXAInterp=0;
unsigned long * XAFeed  = NULL;
unsigned long * XAPlay  = NULL;
unsigned long * XAStart = NULL;
unsigned long * XAEnd   = NULL;
unsigned long   XARepeat  = 0;
unsigned long   XALastVal = 0;
unsigned long		CDDARepeat = 0;

int             iLeftXAVol  = 0x4000;
int             iRightXAVol = 0x4000;



static int gauss_ptr = 0;
static int gauss_window[8] = {0, 0, 0, 0, 0, 0, 0, 0};

#define gvall0 gauss_window[gauss_ptr]
#define gvall(x) gauss_window[(gauss_ptr+x)&3]
#define gvalr0 gauss_window[4+gauss_ptr]
#define gvalr(x) gauss_window[4+((gauss_ptr+x)&3)]


static int pete_simple_l[5];
static int pete_simple_r[5];

#define pete_svall(x) pete_simple_l[x-28]
#define pete_svalr(x) pete_simple_r[x-28]


unsigned int * CDDAFeed  = NULL;
unsigned int * CDDAPlay  = NULL;
unsigned int * CDDAStart = NULL;
unsigned int * CDDAEnd   = NULL;


long cdxa_dbuf_ptr;
int lastxa_lc, lastxa_rc;
int lastcd_lc, lastcd_rc;



INLINE void XASimpleInterpolateUpL(int sinc)
{
 if( sinc == 0 ) sinc=0x10000;


 if(pete_svall(32)==1)                               // flag == 1? calc step and set flag... and don't change the value in this pass
  {
   const int id1=pete_svall(30)-pete_svall(29);    // curr delta to next val
   const int id2=pete_svall(31)-pete_svall(30);    // and next delta to next-next val :)

   pete_svall(32)=0;

   if(id1>0)                                           // curr delta positive
    {
     if(id2<id1)
      {pete_svall(28)=id1;pete_svall(32)=2;}
     else
     if(id2<(id1<<1))
      pete_svall(28)=(id1*sinc)/0x10000L;
     else
      pete_svall(28)=(id1*sinc)/0x20000L; 
    }
   else                                                // curr delta negative
    {
     if(id2>id1)
      {pete_svall(28)=id1;pete_svall(32)=2;}
     else
     if(id2>(id1<<1))
      pete_svall(28)=(id1*sinc)/0x10000L;
     else
      pete_svall(28)=(id1*sinc)/0x20000L; 
    }
  }
 else
 if(pete_svall(32)==2)                               // flag 1: calc step and set flag... and don't change the value in this pass
  {
   pete_svall(32)=0;

   pete_svall(28)=(pete_svall(28)*sinc)/0x20000L;
   if(sinc<=0x8000)
        pete_svall(29)=pete_svall(30)-(pete_svall(28)*((0x10000/sinc)-1));
   else pete_svall(29)+=pete_svall(28);
  }
 else                                                  // no flags? add bigger val (if possible), calc smaller step, set flag1
  pete_svall(29)+=pete_svall(28);
}


INLINE void XASimpleInterpolateUpR(int sinc)
{
 if( sinc == 0 ) sinc=0x10000;


 if(pete_svalr(32)==1)                               // flag == 1? calc step and set flag... and don't change the value in this pass
  {
   const int id1=pete_svalr(30)-pete_svalr(29);    // curr delta to next val
   const int id2=pete_svalr(31)-pete_svalr(30);    // and next delta to next-next val :)

   pete_svalr(32)=0;

   if(id1>0)                                           // curr delta positive
    {
     if(id2<id1)
      {pete_svalr(28)=id1;pete_svalr(32)=2;}
     else
     if(id2<(id1<<1))
      pete_svalr(28)=(id1*sinc)/0x10000L;
     else
      pete_svalr(28)=(id1*sinc)/0x20000L; 
    }
   else                                                // curr delta negative
    {
     if(id2>id1)
      {pete_svalr(28)=id1;pete_svalr(32)=2;}
     else
     if(id2>(id1<<1))
      pete_svalr(28)=(id1*sinc)/0x10000L;
     else
      pete_svalr(28)=(id1*sinc)/0x20000L; 
    }
  }
 else
 if(pete_svalr(32)==2)                               // flag 1: calc step and set flag... and don't change the value in this pass
  {
   pete_svalr(32)=0;

   pete_svalr(28)=(pete_svalr(28)*sinc)/0x20000L;
   if(sinc<=0x8000)
        pete_svalr(29)=pete_svalr(30)-(pete_svalr(28)*((0x10000/sinc)-1));
   else pete_svalr(29)+=pete_svall(28);
  }
 else                                                  // no flags? add bigger val (if possible), calc smaller step, set flag1
  pete_svalr(29)+=pete_svalr(28);
}



INLINE int XAGetInterpolationVal(int spos, int sinc)
{
 int lc,rc;

 switch(iXAInterp)
  {
   case 6:
    {
		 // SPU2-X - Catmull-Rom

		 int y3,y2,y1,y0,mu;
		 int l_val, r_val;

		 int l_a3, l_a2, l_a1, l_a0;
		 int r_a3, r_a2, r_a1, r_a0;

		 mu = spos;


		 // voice more clear this way
		 y0 = gvall(0);
		 y1 = gvall(1);
		 y2 = gvall(2);
		 y3 = gvall(3);

 		 l_a3 = (-  y0 + 3*y1 - 3*y2 + y3)>>1;
		 l_a2 = ( 2*y0 - 5*y1 + 4*y2 - y3)>>1;
		 l_a1 = (-  y0        +   y2     )>>1;
		 l_a0 = (        2*y1            )>>1;

		 l_val = ((l_a3  ) * mu) >> 16;
		 l_val = ((l_a2 + l_val) * mu) >> 16;
		 l_val = ((l_a1 + l_val) * mu) >> 16;

		 lc = l_a0 + l_val;


		 // voice more clear this way
		 y0 = gvalr(0);
		 y1 = gvalr(1);
		 y2 = gvalr(2);
		 y3 = gvalr(3);

 		 r_a3 = (-  y0 + 3*y1 - 3*y2 + y3)>>1;
		 r_a2 = ( 2*y0 - 5*y1 + 4*y2 - y3)>>1;
		 r_a1 = (-  y0        +   y2     )>>1;
		 r_a0 = (        2*y1            )>>1;

		 r_val = ((r_a3  ) * mu) >> 16;
		 r_val = ((r_a2 + r_val) * mu) >> 16;
		 r_val = ((r_a1 + r_val) * mu) >> 16;

		 rc = r_a0 + r_val;
    } break;
   //--------------------------------------------------//
   case 5:
    {
		 // SPU2-X - Hermite

#define HERMITE_TENSION 32768/2

		 int y3,y2,y1,y0,mu;
		 int l_val, r_val;

		 int l_m00,l_m01,l_m0,l_m10,l_m11,l_m1;
		 int r_m00,r_m01,r_m0,r_m10,r_m11,r_m1;

		 mu = spos;


		 // voice more clear this way
		 y0 = gvall(0);
		 y1 = gvall(1);
		 y2 = gvall(2);
		 y3 = gvall(3);

		 l_m00 = ((y1-y0)*HERMITE_TENSION) >> 16; // 16.0
		 l_m01 = ((y2-y1)*HERMITE_TENSION) >> 16; // 16.0
		 l_m0  = l_m00 + l_m01;

		 l_m10 = ((y2-y1)*HERMITE_TENSION) >> 16; // 16.0
		 l_m11 = ((y3-y2)*HERMITE_TENSION) >> 16; // 16.0
		 l_m1  = l_m10 + l_m11;

		 l_val = ((  2*y1 +   l_m0 + l_m1 - 2*y2) * mu) >> 16; // 16.0
		 l_val = ((l_val - 3*y1 - 2*l_m0 - l_m1 + 3*y2) * mu) >> 16; // 16.0
		 l_val = ((l_val        +   l_m0            ) * mu) >> 16; // 16.0

		 lc = l_val + (y1<<0);



		 // voice more clear this way
		 y0 = gvalr(0);
		 y1 = gvalr(1);
		 y2 = gvalr(2);
		 y3 = gvalr(3);

		 r_m00 = ((y1-y0)*HERMITE_TENSION) >> 16; // 16.0
		 r_m01 = ((y2-y1)*HERMITE_TENSION) >> 16; // 16.0
		 r_m0  = r_m00 + r_m01;

		 r_m10 = ((y2-y1)*HERMITE_TENSION) >> 16; // 16.0
		 r_m11 = ((y3-y2)*HERMITE_TENSION) >> 16; // 16.0
		 r_m1  = r_m10 + r_m11;

		 r_val = ((  2*y1 +   r_m0 + r_m1 - 2*y2) * mu) >> 16; // 16.0
		 r_val = ((r_val - 3*y1 - 2*r_m0 - r_m1 + 3*y2) * mu) >> 16; // 16.0
		 r_val = ((r_val        +   r_m0            ) * mu) >> 16; // 16.0

		 rc = r_val + (y1<<0);
    } break;
   //--------------------------------------------------//
   case 4:                                             // cubic interpolation
    {
     int xd;

     xd = (spos >> 1)+1;

     lc  = gvall(3) - 3*gvall(2) + 3*gvall(1) - gvall0;
     lc *= (xd - (2<<15)) / 6;
     lc >>= 15;
     lc += gvall(2) - gvall(1) - gvall(1) + gvall0;
     lc *= (xd - (1<<15)) >> 1;
     lc >>= 15;
     lc += gvall(1) - gvall0;
     lc *= xd;
     lc >>= 15;
     lc = lc + gvall0;

     rc  = gvalr(3) - 3*gvalr(2) + 3*gvalr(1) - gvalr0;
     rc *= (xd - (2<<15)) / 6;
     rc >>= 15;
     rc += gvalr(2) - gvalr(1) - gvalr(1) + gvalr0;
     rc *= (xd - (1<<15)) >> 1;
     rc >>= 15;
     rc += gvalr(1) - gvalr0;
     rc *= xd;
     rc >>= 15;
     rc = rc + gvalr0;

    } break;
   //--------------------------------------------------//
		case 3:
		{
			/*
			ADPCM interpolation (4-tap FIR)

			y[n] = (x[n-3] * 4807 + x[n-2] * 22963 + x[n-1] * 4871 - x[n]) >> 15;

			- Dr. Hell (Xebra PS1 emu)
			*/

			lc = (gvall(3) * 4807 + gvall(2) * 22963 + gvall(1) * 4871 - gvall(0)) >> 15;
			rc = (gvalr(3) * 4807 + gvalr(2) * 22963 + gvalr(1) * 4871 - gvalr(0)) >> 15;
		} break;
   //--------------------------------------------------//
   case 2:                                             // gauss interpolation
    {
     spos = (spos >> 6) & ~3;

		 lc=(gauss[spos]*gvall(0))&~2047;
     lc+=(gauss[spos+1]*gvall(1))&~2047;
     lc+=(gauss[spos+2]*gvall(2))&~2047;
     lc+=(gauss[spos+3]*gvall(3))&~2047;

		 rc=(gauss[spos]*gvalr(0))&~2047;
     rc+=(gauss[spos+1]*gvalr(1))&~2047;
     rc+=(gauss[spos+2]*gvalr(2))&~2047;
     rc+=(gauss[spos+3]*gvalr(3))&~2047;

		 // clip bits first!!
		 lc = lc>>11;
		 rc = rc>>11;
    } break;
   //--------------------------------------------------//
   case 1:                                             // simple interpolation
    {
     XASimpleInterpolateUpL(sinc);                     // --> interpolate up
     XASimpleInterpolateUpR(sinc);                     // --> interpolate up

     lc=pete_svall(29);
     rc=pete_svalr(29);
    } break;
   //--------------------------------------------------//
   default:                                            // no interpolation
    {
		 lc = gvall0;
		 rc = gvalr0;
    } break;
   //--------------------------------------------------//
  }

 // clip to 16-bits
 if( lc > 32767 ) lc = 32767;
 if( rc > 32767 ) rc = 32767;
 if( lc < -32768 ) lc = -32768;
 if( rc < -32768 ) rc = -32768;

 // clip bits again
 // - Megaman Legends 2
 lc &= 0xffff;
 rc &= 0xffff;

 return lc | (rc<<16);
}


INLINE void XAStoreInterpolationVal(int val_l, int val_r)
{
 if((spuCtrl&0x4000)==0) {
	val_l=0;                       // muted?
	val_r=0;
 }


 gvall0 = val_l;
 gvalr0 = val_r;
 

 if(iXAInterp>=2)                            // gauss/cubic interpolation
 {
	gauss_ptr = (gauss_ptr+1) & 3;
 }
 else
 if(iXAInterp==1)                            // simple interpolation
 {
	pete_svall(28) = 0;
	pete_svall(29) = pete_svall(30);
	pete_svall(30) = pete_svall(31);
	pete_svall(31) = val_l;
	pete_svall(32) = 1;

	pete_svalr(28) = 0;
	pete_svalr(29) = pete_svalr(30);
	pete_svalr(30) = pete_svalr(31);
	pete_svalr(31) = val_r;
	pete_svalr(32) = 1;
 }
 else {
	 //pChannel->SB[29]=fa;                           // no interpolation
	}
}

////////////////////////////////////////////////////////////////////////
// MIX XA & CDDA
////////////////////////////////////////////////////////////////////////

/*
Attenuation
- Blade_Arma (edgbla) (PCSX-reloaded)
- accurate (!)


s32 lc = (spsound[i ] * attenuators.val0 + spsound[i+1] * attenuators.val3]) / 128;
s32 rc = (spsound[i+1] * attenuators.val2 + spsound[i ] * attenuators.val1]) / 128;
*/


INLINE void MixXA(void)
{
 int ns;
 unsigned char val0,val1,val2,val3;
 short l,r;
 int lc,rc;
 unsigned long cdda_l;

 val0 = (iLeftXAVol>>8)&0xff;
 val1 = iLeftXAVol&0xff;
 val2 = (iRightXAVol>>8)&0xff;
 val3 = iRightXAVol&0xff;

 lc = 0;
 rc = 0;

 for(ns=0;ns<NSSIZE && XAPlay!=XAFeed;ns++)
  {
	 XALastVal=*XAPlay++;
   if(XAPlay==XAEnd) XAPlay=XAStart;

	 l = XALastVal&0xffff;
	 r = (XALastVal>>16) & 0xffff;

   lc=(l * val0 + r * val3) / 128;
   rc=(r * val2 + l * val1) / 128;

	 if( lc < -32768 ) lc = -32768;
	 if( rc < -32768 ) rc = -32768;
	 if( lc > 32767 ) lc = 32767;
	 if( rc > 32767 ) rc = 32767;

	 SSumL[ns]+=lc;
	 SSumR[ns]+=rc;

	 // improve crackle - buffer under
	 // - not update fast enough
	 lastxa_lc = lc;
	 lastxa_rc = rc;


	 // Tales of Phantasia - voice meter
	 if( cdxa_dbuf_ptr >= 0x800 )
		 cdxa_dbuf_ptr = 0;
	 spuMem[ cdxa_dbuf_ptr++ ] = lc;
	 spuMem[ cdxa_dbuf_ptr++ ] = rc;
  }

 if(XAPlay==XAFeed && XARepeat)
  {
   //XARepeat--;
   for(;ns<NSSIZE;ns++)
    {
		 SSumL[ns]+=lastxa_rc;
		 SSumR[ns]+=lastxa_rc;


		 // Tales of Phantasia - voice meter
		 if( cdxa_dbuf_ptr >= 0x800 )
			 cdxa_dbuf_ptr = 0;
		 spuMem[ cdxa_dbuf_ptr++ ] = lastxa_rc;
		 spuMem[ cdxa_dbuf_ptr++ ] = lastxa_rc;
    }
  }

 for(ns=0;ns<NSSIZE && CDDAPlay!=CDDAFeed && (CDDAPlay!=CDDAEnd-1||CDDAFeed!=CDDAStart);ns++)
  {
   cdda_l=*CDDAPlay++;
   if(CDDAPlay==CDDAEnd) CDDAPlay=CDDAStart;

	 l = cdda_l&0xffff;
	 r = (cdda_l>>16) & 0xffff;

   lc=(l * val0 + r * val3) / 128;
   rc=(r * val2 + l * val1) / 128;

	 if( lc < -32768 ) lc = -32768;
	 if( rc < -32768 ) rc = -32768;
	 if( lc > 32767 ) lc = 32767;
	 if( rc > 32767 ) rc = 32767;

	 SSumL[ns]+=lc;
	 SSumR[ns]+=rc;

	 // improve crackle - buffer under
	 // - not update fast enough
	 lastcd_lc = lc;
	 lastcd_rc = rc;
	}


 if(CDDAPlay==CDDAFeed && XARepeat)
  {
   //XARepeat--;
   for(;ns<NSSIZE;ns++)
    {
		 SSumL[ns]+=lastcd_lc;
		 SSumR[ns]+=lastcd_rc;
    }
  }
}


////////////////////////////////////////////////////////////////////////
// FEED XA 
////////////////////////////////////////////////////////////////////////

/*
44100 <-- 37800 / 18900

441(00)
- 378(00) = 1.1666666666
- 189(00) = 2.3333333333

- 37800 @ 1 - 1/6
- 18900 @ 2 - 2/6
*/

INLINE void FeedXA(xa_decode_t *xap)
{
 int i,iSize,iPlace;
 int fa;
 long spos,sinc;

 if(!bSPUIsOpen) return;

 xapGlobal = xap;                                      // store info for save states
 XARepeat  = 25000;                                      // set up repeat

 iSize=((44100*xap->nsamples)/xap->freq);              // get size
 if(!iSize) return;                                    // none? bye

 //----------------------------------------------------//
 if(iXAPitch)                                          // pitch change option?
  {
   static DWORD dwLT=0;
   static DWORD dwFPS=0;
   static int   iFPSCnt=0;
   static int   iLastSize=0;
   static DWORD dwL1=0;
   DWORD dw=timeGetTime(),dw1,dw2;

   iPlace=iSize;

   dwFPS+=dw-dwLT;iFPSCnt++;

   dwLT=dw;
                                       
   if(iFPSCnt>=10)
    {
     if(!dwFPS) dwFPS=1;
     dw1=1000000/dwFPS; 
     if(dw1>=(dwL1-100) && dw1<=(dwL1+100)) dw1=dwL1;
     else dwL1=dw1;
     dw2=(xap->freq*100/xap->nsamples);
     if((!dw1)||((dw2+100)>=dw1)) iLastSize=0;
     else
      {
       iLastSize=iSize*dw2/dw1;
       if(iLastSize>iPlace) iLastSize=iPlace;
       iSize=iLastSize;
      }
     iFPSCnt=0;dwFPS=0;
    }
   else
    {
     if(iLastSize) iSize=iLastSize;
    }
  }

 //----------------------------------------------------//

 spos=0x10000L;
 sinc = (xap->nsamples << 16) / iSize;                 // calc freq by num / size

 if(xap->stereo)
  {
   unsigned long * pS=(unsigned long *)xap->pcm;
   long l=0;

   if(iXAPitch)
    {
     long l1,l2;short s;

     for(i=0;i<iSize;i++)
     {
       while(spos>=0x10000L) {
				 l = *pS++;
         spos -= 0x10000L;

				 s=(short)(LOWORD(l));
				 l1=s;
				 l1=(l1*iPlace)/iSize;
				 
				 s=(short)(HIWORD(l));
				 l2=s;
				 l2=(l2*iPlace)/iSize;

				 XAStoreInterpolationVal(l1,l2);
       }

			 fa = XAGetInterpolationVal(spos,sinc);

			 *XAFeed++=fa;

			 if(XAFeed==XAEnd) XAFeed=XAStart;
			 if(XAFeed==XAPlay)  {
				 XAPlay++;
				 if( XAPlay==XAEnd ) XAPlay = XAStart;
			 }

       spos += sinc;
		 } // end samples
   } // end pitch
   else {
     long l1,l2;

     for(i=0;i<iSize;i++)
      {
       while(spos>=0x10000L)
       {
        l = *pS++;
        spos -= 0x10000L;

				l1 = (short)(LOWORD(l));
				l2 = (short)(HIWORD(l));

				XAStoreInterpolationVal(l1,l2);
       }

			 fa = XAGetInterpolationVal(spos,sinc);

			 *XAFeed++=fa;

			 if(XAFeed==XAEnd) XAFeed=XAStart;
			 if(XAFeed==XAPlay) 
				{
				 XAPlay++;
				 if( XAPlay==XAEnd ) XAPlay = XAStart;
				}

       spos += sinc;
		 } // end samples
   } // end pitch
  }
 else
  {
   unsigned short *pS=(unsigned short *)xap->pcm;
   long l;short s=0;

   if(iXAPitch)
    {
     long l1;

     for(i=0;i<iSize;i++)
      {
       while(spos>=0x10000L)
       {
        s = *pS++;
        spos -= 0x10000L;

				
				l1 = (short) s;
        l1=(l1*iPlace)/iSize;

			  XAStoreInterpolationVal(l1,l1);
       }

			 fa = XAGetInterpolationVal(spos,sinc);

				 
			 *XAFeed++=fa;

			 if(XAFeed==XAEnd) XAFeed=XAStart;
			 if(XAFeed==XAPlay) 
				{
				 XAPlay++;
				 if( XAPlay==XAEnd ) XAPlay = XAStart;
				}

       spos += sinc;
			}
    }
   else
	 {
     for(i=0;i<iSize;i++)
      {
       while(spos>=0x10000L)
       {
        s = *pS++;
        spos -= 0x10000L;

				l = (short) s;

				XAStoreInterpolationVal(l,l);
       }

			 fa = XAGetInterpolationVal(spos,sinc);


			 *XAFeed++=fa;

			 if(XAFeed==XAEnd) XAFeed=XAStart;
			 if(XAFeed==XAPlay) 
				{
				 XAPlay++;
				 if( XAPlay==XAEnd ) XAPlay = XAStart;
				}

       spos += sinc;
		 } // end samples
	 } // end pitch
 } // end stereo
}


/*
////////////////////////////////////////////////////////////////////////
// FEED CDDA
////////////////////////////////////////////////////////////////////////

INLINE void FeedCDDA(unsigned char *pcm, int nBytes)
{
 int l1,l2;
 unsigned short *src;
 int temp;

 CDDARepeat = 2500;
 src = (unsigned short *) pcm;

 // reuse XA code
 temp = iXAInterp;
 iXAInterp = iCDDAInterp;


 while(nBytes>0)
  {
	 l1 = (short)(*src++);
	 l2 = (short)(*src++);

	 XAStoreInterpolationVal(l1,l2);
	 *CDDAFeed++ = XAGetInterpolationVal(0x3000,0x3000);



   //*CDDAFeed++=(*pcm | (*(pcm+1)<<8) | (*(pcm+2)<<16) | (*(pcm+3)<<24));
   //pcm+=4;
   nBytes-=4;

	 if(CDDAFeed==CDDAEnd) CDDAFeed=CDDAStart;
	 if(CDDAFeed==CDDAPlay) {
		 CDDAPlay++;
		 if( CDDAPlay==CDDAEnd ) CDDAPlay = CDDAStart;
	 }
  }


	iXAInterp = temp;
}
*/

////////////////////////////////////////////////////////////////////////
// FEED CDDA
////////////////////////////////////////////////////////////////////////

unsigned int cdda_ptr;

INLINE void FeedCDDA(unsigned char *pcm, int nBytes)
{
 while(nBytes>0)
  {
   if(CDDAFeed==CDDAEnd) CDDAFeed=CDDAStart;
   while(CDDAFeed==CDDAPlay-1||
         (CDDAFeed==CDDAEnd-1&&CDDAPlay==CDDAStart))
   {
#ifdef _XBOX
    if (!iUseTimer) Sleep(1);
    else return;
#else
    if (!iUseTimer) usleep(1000);
    else return;
#endif
   }
   *CDDAFeed++=(*pcm | (*(pcm+1)<<8) | (*(pcm+2)<<16) | (*(pcm+3)<<24));
   nBytes-=4;
   pcm+=4;

 
#if 0
	 /*
	 Vib Ribbon
	 $00000-$003ff  CD audio left
	 $00400-$007ff  CD audio right
	 */

	 // TIMING: perform in PCSX-r
	 // - gets data from reverb buffer, only update this at intervals (not real-time)

	 // remember: 16-bit ptrs
	 spuMem[ cdda_ptr ] = (l >> 0) & 0xffff;
	 spuMem[ cdda_ptr+0x200 ] = (l >> 16) & 0xffff;

	 cdda_ptr++;
	 cdda_ptr &= 0x1ff;
#endif
 }
}

unsigned long timeGetTime_spu()
{

 struct timeval tv;
 gettimeofday(&tv, 0);                                 // well, maybe there are better ways
 return tv.tv_sec * 1000 + tv.tv_usec/1000;            // to do that, but at least it works

//	return __mftb()/500;//(500 cpu tick speed)
}

#endif
