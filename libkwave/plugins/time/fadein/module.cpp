#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../src/interpolation.h"
#include "../../../src/menuitem.h"
#include "../../../src/timeoperation.h"
#include "../../../src/curve.h"
#include "../../../src/parser.h"         

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="fadein";
//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  KwaveParser parser(operation->getCommand());
  double curve=parser.toDouble();
  if (curve) curve*=10;

  int i=0;
  if (curve==0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)(((long long) (sample[i]))*i/len);
	operation->setCounter (i);
      }
  else if (curve<0)
    for (;i<len;i++)
      {
	sample[i]=
	  (int)((double)sample[i]*
		log10(1+(-curve*((double)i)/len))/log10(1-curve));
	operation->setCounter (i);
      }
  else 
    {
      for (;i<len;i++)
	sample[i]=
	  (int)((double)sample[i]*
		(1-log10(1+(curve*((double)len-i)/len))/log10(1+curve)));
      operation->setCounter (i);
    }

  operation->done();
  return 0;
}
//**********************************************************













