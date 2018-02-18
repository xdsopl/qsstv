#include "fftcalc.h"

fftCalc::fftCalc()
{
  plan=NULL;
  out=NULL;
  dataBuffer=NULL;
}

fftCalc::~fftCalc()
{
  if(plan)fftw_destroy_plan(plan);
  if(out) fftw_free(out);
  if(dataBuffer) fftw_free(dataBuffer);
}

void fftCalc::init(int length,int nblocks,int isamplingrate)
{
  windowSize=length;
  fftLength=windowSize*nblocks;
  blocks=nblocks;
  blockIndex=0;
  createHamming();
  samplingrate=isamplingrate;
  //prepare fft
  if(plan)fftw_destroy_plan(plan);
  if(out) fftw_free(out);
  if(dataBuffer) fftw_free(dataBuffer);
  out =         (double *)fftw_malloc(fftLength * sizeof(double));
  dataBuffer  = (double *)fftw_malloc(fftLength * sizeof(double));
  // create the fftw plan
  plan = fftw_plan_r2r_1d(fftLength, dataBuffer, out, FFTW_R2HC, FFTW_ESTIMATE);
}

void fftCalc::createHamming()
{
  int i;
  hammingBuffer= new double[fftLength];
  for(i=0;i<fftLength;i++)
    {
      hammingBuffer[i]=0.54-(0.46*cos(2*M_PI*((double)i/((double)(fftLength-1)))));
    }

}

void fftCalc::realFFT(double *data)
{
  int i,j;
  for(i=0,j=windowSize*blockIndex;i<windowSize;i++,j++)
    {
      dataBuffer[j]=data[i]*hammingBuffer[i];

    }
  doFFT();
}

void fftCalc::doFFT()
{
  blockIndex++;
  if(blockIndex<blocks) return;
  blockIndex=0;
  fftw_execute(plan);
}


