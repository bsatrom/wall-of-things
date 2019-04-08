#include "application.h"

/*
 * Project wot-controller
 * Description:
 * Author:
 * Date:
 */

int setStripMode(String args);
int setColors(String args);
int startChase(String args);

// setup() runs once, when the device is first turned on.
void setup()
{
  // Put initialization like pinMode and begin functions here.
  Particle.function("setMode", setStripMode);
  Particle.function("setColors", setColors);
  Particle.function("chase", startChase);
}

// loop() runs over and over again, as quickly as it can execute.
void loop()
{
  // The core of your code will likely live here.
}

int setStripMode(String args)
{
  Mesh.publish("setMode", args);

  return 1;
}

int setColors(String args)
{
  Mesh.publish("setColors", args);

  return 1;
}

int startChase(String args)
{
  Mesh.publish("startOne", args);

  return 1;
}