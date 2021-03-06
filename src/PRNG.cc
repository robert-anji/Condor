// $RCSfile: PRNG.cc,v $
// $Revision: 1.1 $

#include <assert.h>

#include "PRNG.h"

#include "Global.h"

// As per the CMWC algorithm from Wikipedia (see below).
static const uint32_t PHI = 0x9e3779b9;

M_ExternConstructDestructCountInt;

PRNG::PRNG(uint32_t seed) :
  mSeed   (seed),
  c       (362436),
  mCMWC_i (4095)
{
  init_rand(seed);

  M_UpdConstructCount(eType_PRNG);
}

PRNG::~PRNG()
{
  M_UpdDestructCount(eType_PRNG);
}

uint32_t PRNG::RandomValueGet()
{
  return rand_cmwc();
}

uint32_t PRNG::RandomValueGet0To(uint32_t max)
{
  return (rand_cmwc() % max);
}

uint32_t PRNG::RandomValueGetBetween(uint32_t min, uint32_t max)
{
  assert(max >= min);

  uint32_t range = max - min;

  if (range) return (min + rand_cmwc() % range);
  else       return (min);
}

void PRNG::init_rand(uint32_t x)
{
  int i;

  Q[0] = x;
  Q[1] = x + PHI;
  Q[2] = x + PHI + PHI;

  for (i=3; i<4096; i++)
    Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}
 
uint32_t PRNG::rand_cmwc(void)
{
  uint64_t t;
  uint64_t a = 18782LL;
//static uint32_t i = 4095; // have used mCMWC_i instead
  uint32_t x;
  uint32_t r = 0xfffffffe;

  mCMWC_i = (mCMWC_i + 1) & 4095;
  //
  t = a * Q[mCMWC_i] + c;
  c = t >> 32;
  x = t + c;

  if (x < c) { x++; c++; }

  return (Q[mCMWC_i] = r - x);
}

// Complementary-Carry-With_Multiply_Adder PRNG from Wikipedia.
// ************************************************************
//
// #define PHI 0x9e3779b9
//  
// static uint32_t Q[4096], c = 362436;
//  
// void init_rand(uint32_t x)
// {
//         int i;
//  
//         Q[0] = x;
//         Q[1] = x + PHI;
//         Q[2] = x + PHI + PHI;
//  
//         for (i = 3; i < 4096; i++)
//                 Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
// }
//  
// uint32_t rand_cmwc(void)
// {
//         uint64_t t, a = 18782LL;
//         static uint32_t i = 4095;
//         uint32_t x, r = 0xfffffffe;
//         i = (i + 1) & 4095;
//         t = a * Q[i] + c;
//         c = (t >> 32);
//         x = t + c;
//         if (x < c) {
//                 x++;
//                 c++;
//         }
//         return (Q[i] = r - x);
// }

// End
