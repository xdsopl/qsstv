/***************************************************************************
 *   Copyright (C) 2000-2008 by Johan Maes                                 *
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
#include "downsamplefilter.h"
#include "appglobal.h"
#include "arraydumper.h"
#include "logging.h"

#include <QDebug>



#include <math.h>

#define VOLINTEGRATOR 0.5



downsampleFilter::downsampleFilter()
{
  filteredDataBuffer=0;
  volumeBuffer=0;
  filterParams=0;
  samplesI=0;
  samplesQ=0;
  volSamples=0;
  filterLength=0;

}

downsampleFilter::downsampleFilter(unsigned int len, bool scaled)
{
  filteredDataBuffer=0;
  volumeBuffer=0;
  filterParams=0;
  samplesI=0;
  samplesQ=0;
  volSamples=0;
  filterLength=0;
  setFilterParams(scaled);
  allocate(len);
  init();
}

downsampleFilter::~downsampleFilter()
{
  if(filteredDataBuffer) delete [] filteredDataBuffer;
  if(filterParams) delete [] filterParams;
  if(samplesI) delete [] samplesI;
  if(samplesQ) delete [] samplesQ;
  if(volSamples) delete [] volSamples;
}



void downsampleFilter::allocate(unsigned int len)
{
  length=len;
  if(filteredDataBuffer) delete [] filteredDataBuffer;
  filteredDataBuffer=new FILTERPARAMTYPE [length];
  volumeBuffer=new unsigned int [length/4+CONVDELAY];
}


void downsampleFilter::init()
{
  unsigned int i;
  first=true;
  for(i=0;i<filterLength;i++)
    {
      samplesI[i]=0;
      samplesQ[i]=0;
    }
  for(i=0;i<length;i++)
    {
      filteredDataBuffer[i]=0;
    }
  for(i=0;i<CONVLENGTH;i++)
    {
      volSamples[i]=0;
    }
  for(i=0;i<length/4+CONVDELAY;i++)
    {
      volumeBuffer[i]=0;
    }
  avgVolumeDb=0;
//  rxBytes=0;
//  avgVolume=0;
}

void downsampleFilter::setFilterParams(bool scaled)
{
  unsigned int i;

  filterLength=DSAMPLEFILTERLEN;
  if(filterParams) delete [] filterParams;
  filterParams=new FILTERPARAMTYPE [filterLength];
  if(samplesI) delete [] samplesI;
  if(samplesQ) delete [] samplesQ;
  samplesI=new FILTERPARAMTYPE[filterLength];
  samplesQ=new FILTERPARAMTYPE[filterLength];
  volSamples=new DSPFLOAT[CONVLENGTH];
  zeroes=filterLength-1;
  ssize=(zeroes)*sizeof(DSPFLOAT);


  DSPFLOAT gain=0;
  for(i=0;i<filterLength;i++)
    {
      gain+=downSampleFilterParam[i];
    }
  for(i=0;i<filterLength;i++)
    {
      if(scaled) filterParams[i]=downSampleFilterParam[i]/gain;
      else filterParams[i]=downSampleFilterParam[i];
    }
  addToLog(QString("filtergain:=%1").arg(gain),LOGPERFORM);
}



void downsampleFilter::downSample4(short int *data)
{
  unsigned int i,k;
  DSPFLOAT tmpVol;
  FILTERPARAMTYPE res;
  const FILTERPARAMTYPE *cf1;
  FILTERPARAMTYPE *fp1;
  FILTERPARAMTYPE res0,res1,res2,res3;
//  arrayDump("din",data,RXSTRIPE,true,false);
  memmove(volumeBuffer,volumeBuffer+length/4,CONVDELAY*sizeof(unsigned int));

  for (k=0;k<length;k+=4)
    {
      res0=res1=res2=res3=0;
      cf1 = filterParams;
      fp1 = samplesI;
      memmove(samplesI+4, samplesI, (filterLength-4)*sizeof(FILTERPARAMTYPE));
      samplesI[3]=data[k];
      samplesI[2]= data[k+1];
      samplesI[1]= data[k+2];
      samplesI[0]= data[k+3];
      for(i=0;i<filterLength;i+=4,fp1+=4,cf1+=4)
        {
          res0+=(*fp1)*(*cf1);
          res1+=(*(fp1+1))*(*(cf1+1));
          res2+=(*(fp1+2))*(*(cf1+2));
          res3+=(*(fp1+3))*(*(cf1+3));
        }
      res=res0+res1+res2+res3;
      filteredDataBuffer[k/4]=res;
      memmove(volSamples+1,volSamples,(CONVLENGTH-1)*sizeof(DSPFLOAT));
      volSamples[0]=sqrt(res*res)*1.35;
      tmpVol=0;
      for(i=0;i<CONVLENGTH;i++)
        {
          tmpVol+=volSamples[i];
        }
      volumeBuffer[k/4+CONVDELAY]=(unsigned int)rint(tmpVol/CONVLENGTH);

    }
  avgVolumeDb=20*log(volumeBuffer[length/8])-130;
}



// rFIR
// localSamplingrate = 48000.000000
// number of taps = 180
// Band0 Lower=0.000000, Upper=2800.000000, Desired=2.000000, Weight=1.000000
// Band1 Lower=3500.000000, Upper=24000.000000, Desired=0.000000, Weight=10.000000
//Coefficients:
const FILTERPARAMTYPE downSampleFilterParam[DSAMPLEFILTERLEN]=
{
  -0.000726805371518309,
  0.00212924341308074,
  0.00209607515518649,
  0.00258616863859503,
  0.00312451941666017,
  0.00351845737963003,
  0.00365705393269608,
  0.00347249738871369,
  0.00294091277303658,
  0.0020876784816062,
  0.000989968644394825,
  -0.000229536963193281,
  -0.00141602838562286,
  -0.00240355817861836,
  -0.00303972686902264,
  -0.00321109351395581,
  -0.00286490657890866,
  -0.00202335724664101,
  -0.000787353876891696,
  0.000672176418542286,
  0.00213509217135987,
  0.00336342008182743,
  0.00413819652648567,
  0.00429713243771618,
  0.00376510708747164,
  0.00257360789092004,
  0.000863764615466942,
  -0.001127792766604,
  -0.00309663047609688,
  -0.00472159091592147,
  -0.0056999360876337,
  -0.00583031339786136,
  -0.00501710522705826,
  -0.00331719263251476,
  -0.000939525687925027,
  0.00178104445626093,
  0.00442648054106535,
  0.00655781811862848,
  0.00778470951795523,
  0.00783457506289861,
  0.00660582869791214,
  0.00419854005559208,
  0.000913611071336271,
  -0.00278058458658644,
  -0.00631166219807734,
  -0.00908863549508422,
  -0.0105968380602393,
  -0.0104880827011458,
  -0.00865059863609764,
  -0.00524633117469452,
  -0.000706169042697737,
  0.00431853795460012,
  0.00904473602072652,
  0.0126740048689635,
  0.0145210513202929,
  0.0141339163209834,
  0.0113860335748772,
  0.00652568850180913,
  0.000164013828194059,
  -0.00678721495092159,
  -0.0132450608783663,
  -0.0181056223037124,
  -0.0204244364948377,
  -0.0195817855693473,
  -0.0154088583976291,
  -0.00825851456253797,
  0.00100265183405794,
  0.0110798178134256,
  0.0204127669048817,
  0.0273919133377699,
  0.0306039642231267,
  0.0290707628785015,
  0.0224466971136873,
  0.0111424375360727,
  -0.00364878906373264,
  -0.0200329664559621,
  -0.0356023553816112,
  -0.0477088371694707,
  -0.053792701556172,
  -0.0517250893361419,
  -0.0401198095407566,
  -0.0185705541637443,
  0.0122205143308298,
  0.0504455099076038,
  0.093330253141237,
  0.13739759113622,
  0.178828272483524,
  0.213871703505128,
  0.239265440205684,
  0.252600612879519,
  0.252600612879519,
  0.239265440205684,
  0.213871703505128,
  0.178828272483524,
  0.13739759113622,
  0.093330253141237,
  0.0504455099076038,
  0.0122205143308298,
  -0.0185705541637443,
  -0.0401198095407566,
  -0.0517250893361419,
  -0.053792701556172,
  -0.0477088371694707,
  -0.0356023553816112,
  -0.0200329664559621,
  -0.00364878906373264,
  0.0111424375360727,
  0.0224466971136873,
  0.0290707628785015,
  0.0306039642231267,
  0.0273919133377699,
  0.0204127669048817,
  0.0110798178134256,
  0.00100265183405794,
  -0.00825851456253797,
  -0.0154088583976291,
  -0.0195817855693473,
  -0.0204244364948377,
  -0.0181056223037124,
  -0.0132450608783663,
  -0.00678721495092159,
  0.000164013828194059,
  0.00652568850180913,
  0.0113860335748772,
  0.0141339163209834,
  0.0145210513202929,
  0.0126740048689635,
  0.00904473602072652,
  0.00431853795460012,
  -0.000706169042697737,
  -0.00524633117469452,
  -0.00865059863609764,
  -0.0104880827011458,
  -0.0105968380602393,
  -0.00908863549508422,
  -0.00631166219807734,
  -0.00278058458658644,
  0.000913611071336271,
  0.00419854005559208,
  0.00660582869791214,
  0.00783457506289861,
  0.00778470951795523,
  0.00655781811862848,
  0.00442648054106535,
  0.00178104445626093,
  -0.000939525687925027,
  -0.00331719263251476,
  -0.00501710522705826,
  -0.00583031339786136,
  -0.0056999360876337,
  -0.00472159091592147,
  -0.00309663047609688,
  -0.001127792766604,
  0.000863764615466942,
  0.00257360789092004,
  0.00376510708747164,
  0.00429713243771618,
  0.00413819652648567,
  0.00336342008182743,
  0.00213509217135987,
  0.000672176418542286,
  -0.000787353876891696,
  -0.00202335724664101,
  -0.00286490657890866,
  -0.00321109351395581,
  -0.00303972686902264,
  -0.00240355817861836,
  -0.00141602838562286,
  -0.000229536963193281,
  0.000989968644394825,
  0.0020876784816062,
  0.00294091277303658,
  0.00347249738871369,
  0.00365705393269608,
  0.00351845737963003,
  0.00312451941666017,
  0.00258616863859503,
  0.00209607515518649,
  0.00212924341308074,
  -0.000726805371518309
};




