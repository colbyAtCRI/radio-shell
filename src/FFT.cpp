#include "FFT.h"

FFT::FFT(int nd)
{
  nSize = nd;
  nSets = 10;
  data = new fftw_complex[nd];
  spectra1 = new double[nd];
  spectra2 = new double[nd];
  plan = fftw_plan_dft_1d(nSize,data,data,FFTW_BACKWARD,FFTW_ESTIMATE);
  clear();
}

FFT::~FFT()
{
  delete[] data;
  delete[] spectra1;
  delete[] spectra2;
  fftw_destroy_plan(plan);
}

void FFT::clear()
{
  nCount = 0;
  nsp1 = 0;
  nsp2 = 0;
  for (int n = 0; n < nSize; n++) {
    data[n][0] = 0.0;
    data[n][1] = 0.0;
    spectra1[n] = 0.0;
    spectra2[n] = 0.0;
  }
}

bool FFT::addData(char state, Data *d, int nd)
{
  for (int n = 0; n < nSize; n++) {
    data[n][0] = (double)d[n].I;
    data[n][1] = (double)d[n].Q;
  }
  nCount = nCount + 1;
  fftw_execute(plan);
  if ( state == '1' ) {
    for (int n = 0; n < nSize; n++)
      spectra1[n] += data[n][0]*data[n][0] + data[n][1]*data[n][1];
    nsp1++;
  }
  else {
    for (int n = 0; n < nSize; n++)
      spectra2[n] += data[n][0]*data[n][0] + data[n][1]*data[n][1];
    nsp2++;
  }
  if (!(nsp1 == nsp2 && nCount > nSets))
    return false;
  else {
    for (int n = 0; n < 2048; n++) {
      spectra1[n] /= (double) nsp1;
      spectra2[n] /= (double) nsp2;
    }
    return true;
  }
}

