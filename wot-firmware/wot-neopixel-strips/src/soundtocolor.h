#ifndef SOUNDCOLOR_H
#define SOUNDCOLOR_H

struct color
{
  unsigned int Red = 0;
  unsigned int Green = 0;
  unsigned int Blue = 0;
};

color mapNoteToColor(double soundFrequency);

#endif