#include "application.h"

/*
 * Project wot-controller
 * Description:
 * Author:
 * Date:
 */

int stripMode = 0;

int setStripMode(String args);
int setColors(String args);
int startChase(String args);

void setup()
{
  // Put initialization like pinMode and begin functions here.
  Particle.function("setMode", setStripMode);
  Particle.function("setColors", setColors);
  Particle.function("chase", startChase);
  Particle.variable("stripMode", stripMode);
}

void loop()
{
}

int setStripMode(String args)
{
  Mesh.publish("setMode", args);
  stripMode = args.toInt();

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