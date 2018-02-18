#include "filters.h"
#include "filter.h"

#include <QDebug>



syncFilter:: syncFilter(uint maxLength):
  sync1200(filter::FTFIR,maxLength),sync1900(filter::FTFIR,maxLength),
  vol1200(filter::FTFIR,maxLength),vol1900(filter::FTFIR,maxLength)
{
  init();
}

syncFilter::~syncFilter()
{

}

void syncFilter:: init()
{
  // setup the syncFilters
  sync1200.init();
  sync1200.setupMatchedFilter(1200,60);

  sync1900.init();
  sync1900.setupMatchedFilter(1900,60);

  vol1200.init();
  vol1200.nZeroes=HILBERTTAPS-1;
  vol1200.coefZPtr=(FILTERPARAMTYPE *)hilbertCoef;
  vol1200.gain=HILBERTGAIN;
  vol1200.volumeAttackIntegrator=0.07;
  vol1200.volumeDecayIntegrator=0.05;
  vol1200.allocate();

  vol1900.init();
  vol1900.nZeroes=HILBERTTAPS-1;
  vol1900.coefZPtr=(FILTERPARAMTYPE *)hilbertCoef;
  vol1900.gain=HILBERTGAIN;
  vol1900.volumeAttackIntegrator=0.07;
  vol1900.volumeDecayIntegrator=0.05;
  vol1900.allocate();
  detect1200Ptr= vol1200.volumePtr;
  detect1900Ptr= vol1900.volumePtr;
}

void syncFilter::process(FILTERPARAMTYPE *dataPtr)
{
  sync1200.processFIR(dataPtr,sync1200.filteredPtr);
  sync1900.processFIR(dataPtr,sync1900.filteredPtr);
  vol1200.processHILBVolume(sync1200.filteredPtr);
  vol1900.processHILBVolume(sync1900.filteredPtr);
}






videoFilter::videoFilter(uint maxLength):videoFltr(filter::FTFIR,maxLength),lpFltr(filter::FTFIR,maxLength)
{
  init();
}

videoFilter::~videoFilter()
{
}

void videoFilter::init()
{
  videoFltr.init();
  lpFltr.init();
  videoFltr.volumeAttackIntegrator=0.07;
  videoFltr.volumeDecayIntegrator=0.01;
  videoFltr.nZeroes=VIDEOFIRNUMTAPS-1;
  videoFltr.gain=VIDEOFIRGAIN;
  videoFltr.frCenter=VIDEOFIRCENTER;
  videoFltr.coefZPtr=(FILTERPARAMTYPE *)videoFilterCoefFIR;
  videoFltr.allocate();
  demodPtr=videoFltr.demodPtr;

  lpFltr.setupMatchedFilter(0,1);
}

void videoFilter::process(FILTERPARAMTYPE *dataPtr)
{

  videoFltr.processFIRDemod(dataPtr,videoFltr.filteredPtr);
  lpFltr.processFIRInt(videoFltr.filteredPtr,videoFltr.demodPtr);
}



wfFilter::wfFilter(uint maxLength):wfFltr(filter::FTFIR,maxLength)
{
   init();
}

wfFilter::~wfFilter()
{

}

void wfFilter::init()
{
  wfFltr.init();
  wfFltr.nZeroes=TXWFNUMTAPS-1;
  wfFltr.gain=1;
  wfFltr.coefZPtr=(FILTERPARAMTYPE *)wfFilterCoef;
  wfFltr.volumeAttackIntegrator=0.07;
  wfFltr.volumeDecayIntegrator=0.01;
  wfFltr.allocate();
}

void  wfFilter::process(double *dataPtr, uint dataLength)
{
  wfFltr.dataLen=dataLength;
  wfFltr.processFIR(dataPtr,dataPtr);
}



drmHilbertFilter::drmHilbertFilter(uint maxLength):drmFltr(filter::FTFIR,maxLength)
{
  init();
}

drmHilbertFilter::~drmHilbertFilter()
{
}

void drmHilbertFilter::init()
{
  drmFltr.init();
  drmFltr.nZeroes=DRMHILBERTTAPS-1;
  drmFltr.nPoles=0;
  drmFltr.coefZPtr=(FILTERPARAMTYPE *)drmHilbertCoef;
  drmFltr.gain=DRMHILBERTGAIN;
  drmFltr.allocate();
}

//void drmHilbertFilter::process(float *dataPtr,uint dataLength)
//{

//  process(dataPtr,dataPtr,dataLength);
//}

void drmHilbertFilter::process(FILTERPARAMTYPE *dataPtr, float *outputPtr,uint dataLength)
{
  drmFltr.dataLen=dataLength;
  drmFltr.processIQ(dataPtr,outputPtr);
}

//void drmHilbertFilter::processIQ(FILTERPARAMTYPE *data, FILTERPARAMTYPE *output,uint dataLength)
//{
//  length=dataLength;
//  FILTERPARAMTYPE resQ=0;
//  const FILTERPARAMTYPE *cf1;
//  FILTERPARAMTYPE *fp1;
//  unsigned int i;
//  uint k;
//  for (k=0;k<length;k++)
//    {
//      resQ=0;
//      cf1 = hilbCoef;
//      fp1 = sampleBufferI;
//      memmove(sampleBufferI+1, sampleBufferI,sizeof(FILTERPARAMTYPE)); // newest at index 0
//      sampleBufferI[0]=data[k];
//      for(i=0;i<hilbTaps;i++,fp1++,cf1++)
//        {
//          resQ+=(*fp1)*(*cf1);
//        }
//      output[2*k+1]=sampleBufferI[hilbTaps/2]; // just delay
//      output[2*k]=(FILTERPARAMTYPE)resQ;
//    }
//}


