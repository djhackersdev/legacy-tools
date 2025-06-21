#include "libiidx.hpp"
#include <iostream>

using namespace iidx;

int main()
{
  iidxStyle *style, *style2;

  std::string path("D:/Spiele/IIDX/IIDX16/I00");
  std::string path2("D:/Spiele/IIDX/IIDX15/HDD");
  try
  {
    std::cout << "Trying to load IIDX AC style from " << path << "...";
    style = new iidxStyle(path);
    std::cout << "ok" << std::endl;
    std::cout << "Version: "<< style->version << std::endl;
    std::cout << "Songcount: "<< style->songcount << std::endl;
    std::cout << "Size of highscore entry pointer table: "<< style->highscorecount << std::endl;

    std::cout << "Trying to load IIDX AC style from " << path2 << "...";
    style2 = new iidxStyle(path2);
    std::cout << "ok" << std::endl;
    std::cout << "Version: "<< style2->version << std::endl;
    std::cout << "Songcount: "<< style2->songcount << std::endl;
    std::cout << "Size of highscore entry pointer table: "<< style2->highscorecount << std::endl;
  }
  catch(invalidfile)
  {
    std::cout << "invalid eout file" << std::endl;
    return 1;
  }
  catch(filenotfound)
  {
    std::cout << "eout not found" << std::endl;
    return 2;
  }   

  //iidxSong &test = style->GetSongByID(100);

  //std::cout << test.id << "  " << test.name << std::endl;

  iidxSonglist::iterator pos;
  
  for(pos = style2->songs.begin(); pos != style2->songs.end(); pos++)
  {
    if(!(style->HasSongID(pos->id)))
      std::cout << "Missing song (ID " << pos->id << "): " << pos->name << std::endl;
  }

  for(pos = style2->songs.begin(); pos != style2->songs.end(); pos++)
  {
    if(style->HasSongID(pos->id))
    {
      iidxSong &tmp = style->GetSongByID(pos->id);
      for(int i = 0; i < 8; i++)
        if(pos->charts[i].level != tmp.charts[i].level)
          std::cout << "Changed difficulty (chart " << i << ", " << (int)pos->charts[i].level << " to " << (int)tmp.charts[i].level << ") (ID " << pos->id << ", sort " << pos->style << "): " << pos->name << std::endl;
    }
  }

  delete style;

  return 0;
}
