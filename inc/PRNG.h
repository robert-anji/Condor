// $RCSfile: PRNG.h,v $
// $Revision: 1.1 $

#ifndef PRNG_CLASS
#define PRNG_CLASS

// CMCA algorithm as mentioned on Wikipedia.

#include <stdint.h> // needed for uint32_t, uint64_t

class PRNG
{
public:
  PRNG(uint32_t seed);
 ~PRNG();

  uint32_t RandomValueGet();
  uint32_t RandomValueGet0To(uint32_t max);
  uint32_t RandomValueGetBetween(uint32_t min, uint32_t max);

private:
  uint32_t mSeed;

  uint32_t Q[4096];
  uint32_t c;
  uint32_t mCMWC_i;
  //
  void     init_rand(uint32_t x);
  uint32_t rand_cmwc(void);
};

#endif
