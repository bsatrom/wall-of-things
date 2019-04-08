#include "soundtocolor.h"
#include "math.h"

// Color frequency constants, in Hertz
double lightFreqRedLower = 400000000000000;
double speedOfLightVacuum = 299792458; // m/sec

double wavelength(double frequency, double speedOfLight);
double frequency(double wavelength, double speedOfLight);
color getColorFromWavelength(double wavelength);
double factorAdjust(int color, double factor, int intensityMax, double gamma);

color mapNoteToColor(double soundFrequency)
{
  struct color noteColor;
  double lightWavelength, lightWavelengthNM;

  double lightFrequency = soundFrequency;

  while (lightFrequency < lightFreqRedLower)
  {
    lightFrequency *= 2;
  }

  lightWavelength = wavelength(lightFrequency, speedOfLightVacuum);
  lightWavelengthNM = lightWavelength * 1000000000;

  noteColor = getColorFromWavelength(lightWavelengthNM);

  return noteColor;
}

color getColorFromWavelength(double wavelength)
{
  struct color RGBVals;
  double gamma = 1.00;
  int intensityMax = 255;
  double factor;

  double Red, Green, Blue;

  if (wavelength >= 350 && wavelength < 440)
  {
    // From Purple (1, 0, 1) to Blue (0, 0, 1), with increasing intensity (set below)
    Red = -(wavelength - 440) / (440 - 350);
    Green = 0.0;
    Blue = 1.0;
  }
  else if (wavelength >= 440 && wavelength < 490)
  {
    // From Blue (0, 0, 1) to Cyan (0, 1, 1)
    Red = 0.0;
    Green = (wavelength - 440) / (490 - 440);
    Blue = 1.0;
  }
  else if (wavelength >= 490 && wavelength < 510)
  {
    // From  Cyan (0, 1, 1)  to  Green (0, 1, 0)
    Red = 0.0;
    Green = 1.0;
    Blue = -(wavelength - 510) / (510 - 490);
  }
  else if (wavelength >= 510 && wavelength < 580)
  {
    // From  Green (0, 1, 0)  to  Yellow (1, 1, 0)
    Red = (wavelength - 510) / (580 - 510);
    Green = 1.0;
    Blue = 0.0;
  }
  else if (wavelength >= 580 && wavelength < 645)
  {
    // From  Yellow (1, 1, 0)  to  Red (1, 0, 0)
    Red = 1.0;
    Green = -(wavelength - 645) / (645 - 580);
    Blue = 0.0;
  }
  else if (wavelength >= 645 && wavelength <= 780)
  {
    // Solid Red (1, 0, 0), with decreasing intensity (set below)
    Red = 1.0;
    Green = 0.0;
    Blue = 0.0;
  }
  else
  {
    Red = 0.0;
    Green = 0.0;
    Blue = 0.0;
  }

  // Intensity factor goes through the range:
  // 0.1 (350-420 nm) 1.0 (420-645 nm) 1.0 (645-780 nm) 0.2
  if (wavelength >= 350 && wavelength < 420)
  {
    factor = 0.1 + 0.9 * (wavelength - 350) / (420 - 350);
  }
  else if (wavelength >= 420 && wavelength < 645)
  {
    factor = 1.0;
  }
  else if (wavelength >= 645 && wavelength <= 780)
  {
    factor = 0.2 + 0.8 * (780 - wavelength) / (780 - 645);
  }
  else
  {
    factor = 0.0;
  }

  RGBVals.Red = factorAdjust(Red, factor, intensityMax, gamma);
  RGBVals.Green = factorAdjust(Green, factor, intensityMax, gamma);
  RGBVals.Blue = factorAdjust(Blue, factor, intensityMax, gamma);

  return RGBVals;
}

double factorAdjust(int color, double factor, int intensityMax, double gamma)
{
  if (color == 0.0)
  {
    return 0;
  }
  else
  {
    return round(intensityMax * pow(color * factor, gamma));
  }
}

double wavelength(double frequency, double speedOfLight)
{
  return (speedOfLight / frequency);
}

double frequency(double wavelength, double speedOfLight)
{
  return (speedOfLight / wavelength);
}