/***************************************************************************
 *   Copyright (C) 2004 by Johan Maes - ON4QZ                              *
 *   on4qz@telenet.be                                                      *
 *   http://users.telenet.be/on4qz                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef FILTERPARAM_H
#define FILTERPARAM_H
#include "appdefs.h"
#include <QString>

#define FILTERPARAMTYPE double


#define TXWFNUMTAPS 121
#define NUMTAPSPOST 0
#define NUMRXFILTERS 1

#define SYNCBPNUMZEROES 8
#define SYNCBPNUMPOLES  8
#define SYNCBPGAIN 103035.237413
extern const double z_sync_bp[SYNCBPNUMZEROES+1];
extern const double p_sync_bp[SYNCBPNUMPOLES];
#define VIDEOBPNUMZEROES 8
#define VIDEOBPNUMPOLES 8
#define VIDEOBPGAIN 2284.945151
extern const double z_video_bp[VIDEOBPNUMZEROES+1];
extern const double p_video_bp[VIDEOBPNUMPOLES];


#define VIDEOFIRNUMTAPS 181
#define VIDEOFIRGAIN 10.000014
#define VIDEOFIRCENTER 1900
extern const double videoFilterCoefFIR[VIDEOFIRNUMTAPS];


#define HILBERTTAPS 121
#define HILBERTGAIN 1.569474
extern const double hilbertCoef[HILBERTTAPS];


#define DRMHILBERTTAPS 153
#define DRMHILBERTGAIN 1.569749
extern const FILTERPARAMTYPE drmHilbertCoef[DRMHILBERTTAPS];

enum efilterType {FNARROW,FWIDE};
enum epostFilterType {NONE};
extern const  FILTERPARAMTYPE wfFilterCoef[TXWFNUMTAPS];

DSPFLOAT calculateGain(const DSPFLOAT *fp,unsigned int len);

struct sfilters
{
  const QString filterName;
  const FILTERPARAMTYPE *filterPtr;
  DSPFLOAT centerFrequency;
};

//extern sfilters filterStruct[NUMRXFILTERS];


 
 #endif
