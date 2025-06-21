//eout.hpp
//Defines the eout class for loading, saving and de-/encrypting the eout.bin file.
//This is only for file i/o convenience. Parsing and heavy lifting is done inside the iidxStyle and iidxSong classes.
//Encryption and decryption were shamelessly pilfered from tau's crackeout and slightly optimized/simplified.

#ifndef __EOUT_HPP
#define __EOUT_HPP

#include <string>
#include <fstream>
#include "stdint.h"
#include "string.h"

#include "iidxcommon.hpp"

namespace iidx
{

class eout
{
public:

  //std::string path;
  //std::string filename;

  uint8_t *data;
  uint32_t length;

  //eout();
  //constructor for loading an existing file
  eout(std::string file);
  ~eout();
  //constructor for creating or updating a file
  //void eout(std::string root, iidxVersion version, uint32_t songcount);

  void Decrypt();
  void Encrypt();
  void Save(std::string where);

private:
  bool encrypted;

  //helper functions for encryption
  enum eoutconstants {
    EOUT_MODE_DEC = 0,
    EOUT_MODE_ENC = 1,
    IDX_CURRENT_KEY = 54,
    IDX_LAST = 55
  };

  void crypt(uint32_t mode);
  void update_state(uint8_t *state, uint8_t src_byte);
};

void eout::update_state(uint8_t *state, uint8_t src)
{
  uint8_t tmp;
  uint32_t i;

  tmp = 1;

  state[IDX_CURRENT_KEY] = src;

  for(i = 0; i < IDX_CURRENT_KEY*21; i += 21)
  {
    state[(i+20) % 55] = tmp;
    tmp = src - tmp;
    src = state[(i+20) % 55];
  }

  for(i = 0; i < 4*IDX_LAST; i++)
    state[i % 55] -= state[(i+31) % 55];
}

void eout::crypt(uint32_t mode)
{
  static const uint8_t key[] =
  {
    0x55, 0x37, 0x9F, 0xCC, 0xE3, 0xA7, 0x7D, 0x99,
    0xDD, 0xAA, 0xBB, 0xCF, 0xFC, 0x67, 0x43, 0x17
  };

  uint8_t state[IDX_LAST];
  uint8_t states[16][16];
  uint8_t quo;
  uint8_t rem;

  if(mode != EOUT_MODE_ENC && mode != EOUT_MODE_DEC)
    return;

  //precompute the 16 possible states
  for(uint8_t i = 0; i < 16; i++)
  {
    update_state(state, key[i]);
    memcpy(states[i], state, 16);

    if(mode == EOUT_MODE_ENC)
      for(uint8_t k = 0; k < 16; k++)
        states[i][k] = -states[i][k];
  }

  for(uint32_t i = 0; i < length; i++)
  {
    quo = (i / 0x10) % 0x10;
    rem = i % 0x10;

    data[i] -= states[quo][rem];
  }
}

void eout::Encrypt()
{
  if(encrypted == false)
    crypt(EOUT_MODE_ENC);

  encrypted = true;
}

void eout::Decrypt()
{
  if(encrypted == true)
    crypt(EOUT_MODE_DEC);

  encrypted = false;
}

void eout::Save(std::string where)
{
  std::ofstream out(where.c_str());

  if(!out)
    return;

  out.write((char*)data, length);
  out.close();
}

/*
eout::eout()
{
  length = 0;
  encrypted = false;
  data = NULL;
}*/

eout::eout(std::string file)
{
  std::ifstream in;

  length = 0;
  encrypted = false;
  data = NULL;

  in.open(file.c_str(), std::ios::binary );

  if(!in)
    throw filenotfound();

  //read in the file
  in.seekg(0, std::ios::end);
  length = in.tellg();

  //FIXME: better length check
  if(length < 4)
  {
    in.close();
    length = 0;
    throw invalidfile();
  }

  data = new uint8_t [length];
  in.seekg(0, std::ios::beg);
  in.read((char*)data, length);
  in.close();
  
  //check if eout.bin is encrypted
  if(memcmp((char*)data, "IIDX", 4))
  {
    encrypted = true;
    Decrypt();
  }

  if(memcmp((char*)data, "IIDX", 4))
  {
    //not a valid eout.bin
    delete data;
    length = 0;
    throw invalidfile();
  }

  //path = root;
  //filename = "eout.bin";
}

eout::~eout()
{
  if(length > 0)
    delete data;
}

}
#endif
