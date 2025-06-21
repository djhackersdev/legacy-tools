
#ifndef __IIDX_HPP
#define __IIDX_HPP


#include <string>
#include <list>
#include "stdint.h"

#include "iidxcommon.hpp"
#include "eout.hpp"
#include "iidxsong.hpp"

//↓debug
#include <iostream>

namespace iidx
{

class iidxStyle
{
public:
  std::string path;
  uint16_t songcount;
  uint16_t highscorecount;
  iidxVersion version;

  eout *Eout;

  iidxSonglist songs;
  
  iidxStyle(std::string root);
  ~iidxStyle();
  
  bool HasSongID(uint32_t id);
  iidxSong& GetSongByID(uint32_t id);

};


iidxStyle::iidxStyle(std::string root)
{
  Eout = NULL;

  //this will throw an exception on failure
  Eout = new eout(root + "/data/info/eout.bin");

  path = root;

  uint8_t songid[5];

  //============
  //parse eout header
  //============
  uint8_t *data = Eout->data;
  uint16_t *hsp = (uint16_t*)(&data[14]);

  version = (iidxVersion)*(uint32_t*)(&data[4]);
  songcount = *(uint16_t*)(&data[8]);
  highscorecount = *(uint16_t*)(&data[10]);

  //============
  //load song data
  //============
  iidxSong song;
  //advance to the beginning of the song infos
  data += 16 + 2*highscorecount;

  for(int i = 0, j = 0; i < songcount; i++)
  {
    //find the next highscore pointer entry
    while(*(++hsp) == 0xffff && j++ <= highscorecount);

    //std::cout << *hsp << "   ";
    if(j <= highscorecount)
      song.highscorepointer = *hsp;
    else
      //broken header; this won't really fix it...
      song.highscorepointer = 0;

    memcpy(song.name, data, 64); data += 64;
    memcpy(song.label, data, 32); data += 32;
    memcpy(song.title, data, 32); data += 32;
    memcpy(song.license, data, 32); data += 32;
    memcpy(song.genre, data, 32); data += 32;
    memcpy(song.artist, data, 32); data += 32;

    memcpy(&song.style, data, 2); data += 2;
    memcpy(&song.sort_others, data, 2); data += 2;
    memcpy(&song.sort_bemani, data, 2); data += 2;
    memcpy(&song.sort_bemani2, data, 2); data += 2;

    memcpy(&song.charts[NORMAL7].level, data, 1); data += 1;
    memcpy(&song.charts[HYPER7].level, data, 1); data += 1;
    memcpy(&song.charts[ANOTHER7].level, data, 1); data += 1;
    memcpy(&song.charts[NORMAL14].level, data, 1); data += 1;
    memcpy(&song.charts[HYPER14].level, data, 1); data += 1;
    memcpy(&song.charts[ANOTHER14].level, data, 1); data += 1;
    memcpy(&song.charts[BEGINNER].level, data, 1); data += 1;
    memcpy(&song.charts[EX].level, data, 1); data += 1;

    data += 128; //bunch of zeros
    if(version >= SIRIUS) //even more zeroes beginning with Sirius
      data += 32;
    data += 20;  //bunch of 0xFF

    memcpy(&song.id, data, 4); data += 4;
    memcpy(&song.volume, data, 4); data += 4;

    memcpy(&song.suffix[NORMAL7], data, 1); data += 1;
    memcpy(&song.suffix[HYPER7], data, 1); data += 1;
    memcpy(&song.suffix[ANOTHER7], data, 1); data += 1;
    memcpy(&song.suffix[NORMAL14], data, 1); data += 1;
    memcpy(&song.suffix[HYPER14], data, 1); data += 1;
    memcpy(&song.suffix[ANOTHER14], data, 1); data += 1;
    memcpy(&song.suffix[BEGINNER], data, 1); data += 1;
    memcpy(&song.suffix[EX], data, 1); data += 1;

    memcpy(&song.movieoffset, data, 4); data += 4;
    memcpy(song.moviefile, data, 32); data += 32;

    memcpy(song.bgname, data, 32); data += 32;
    memcpy(song.graphicsfolder, data, 32); data += 32;

    memcpy(&song.layerinfo, data, 4); data += 4;
    memcpy(song.layer, data, 9*32); data += 9*32;

    //std::cout << song.id << " (" << song.highscorepointer << ")  " << song.name << std::endl;

    snprintf((char*)songid, 5, "%04d", song.id);
    song.songroot = path + "/data/sd_data/" + (char*)songid;

    songs.push_back(song);
  }
  
}

iidxStyle::~iidxStyle()
{
  if(Eout)
    delete Eout;
}

bool iidxStyle::HasSongID(uint32_t id)
{
  iidxSonglist::iterator pos;

  for(pos = songs.begin(); pos != songs.end(); pos++)
    if(pos->id == id)
      return true;

  return false;
}

iidxSong& iidxStyle::GetSongByID(uint32_t id)
{
  iidxSonglist::iterator pos;

  for(pos = songs.begin(); pos != songs.end(); pos++)
    if(pos->id == id)
      return *pos;

  throw songnotfound();
}

}

#endif
