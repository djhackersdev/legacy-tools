#ifndef __IIDXSONG_HPP
#define __IIDXSONG_HPP

#include <string>
#include <fstream>
#include <list>
#include "string.h"

#include "iidxcommon.hpp"
#include "iidxchart.hpp"

namespace iidx
{

class iidxSong
{
  //name to be displayed on led screen
  //also used for sorting
public:
  uint8_t name[64];
  uint32_t id;
  uint16_t highscorepointer;

  //filenames to be used for graphics
  uint8_t label[32]; //songwheel
  uint8_t title[32]; //banner
  uint8_t genre[32]; //genre
  uint8_t artist[32]; //artist
  uint8_t license[32]; //label the song was licensed from (top right of the banner ingame)

  //sorting options
  uint16_t style; //style folder
  uint16_t sort_others; //non-alphabet starting letter
  uint16_t sort_bemani; //add also into bemani folder
  uint16_t sort_bemani2; //add also into bemani folder (again?)

  //charts
  iidxChart charts[8];
  uint8_t suffix[8]; //file suffix used for .2dx files

  //misc
  uint32_t volume;
  uint32_t movieoffset; //offset in ms?
  uint8_t  moviefile[32]; //sans .4
  uint8_t  bgname[32];
  uint8_t  graphicsfolder[32];

  uint32_t layerinfo;
  uint8_t  layer[9][32];

  std::string songroot;
  
  //iidxSong() { onefile = NULL; }
  uint16_t GetId() { return id; };
  bool operator<(iidxSong *comp) { return (id < comp->GetId()); };
  void LoadOne();
};

typedef std::list<iidxSong> iidxSonglist;

void iidxSong::LoadOne()
{
  std::ifstream in;
  uint8_t songid;
  uint8_t *data = NULL;
  uint32_t length = 0;

  snprintf((char*)songid, 5, "%04d", id);

  in.open((songroot + (char*)songid).c_str(), std::ios::binary );
  if(!in)
    throw filenotfound();

  //read in the file
  in.seekg(0, std::ios::end);
  length = in.tellg();

  //FIXME: better length check
  if(length < 92)
  {
    in.close();
    length = 0;
    throw invalidfile();
  }

  data = new uint8_t [length];
  in.seekg(0, std::ios::beg);
  in.read((char*)data, length);
  in.close();
  
  

}

}

#endif
