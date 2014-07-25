#ifndef __FFT__
#define __FFT__
#include <complex>
#include <fftw3.h>

using namespace std;

struct Data
{
  short int I;
  short int Q;
};

struct FFT
{
  int              nSize;
  int              nCount;
  int              nSets;
  fftw_plan        plan;
  fftw_complex    *data;
  long             nsp1;
  double          *spectra1;
  long             nsp2;
  double          *spectra2;
  FFT(int nd=2048);
 ~FFT();

  bool addData(char state, Data *d, int nd);
  void clear();
};
#endif
