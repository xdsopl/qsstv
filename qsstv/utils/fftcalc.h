#ifndef FFTCALC_H
#define FFTCALC_H
#include <fftw3.h>
#include <math.h>

class fftCalc
{
public:
  fftCalc();
  ~fftCalc();
  void init(int length,int nblocks,int isamplingrate);
  void realFFT(double *data);
  double *out;
private:
  void createHamming();
  void doFFT();
  double *hammingBuffer;
  double *dataBuffer;
  int windowSize;
  int fftLength;
  int samplingrate;
  unsigned int blocks;
  unsigned int blockIndex;
  fftw_plan plan;
};

#endif // FFTCALC_H
