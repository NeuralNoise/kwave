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
const char *name="average";
//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  KwaveParser parser (operation->getCommand());
  int taps=parser.toInt();

  int b=taps/2;
  long int newsam;
  int i,j;

  int *sam=new int[len];
  if (sam)
    {
      for (i=b;i<len-b;i++)
	{
	  newsam=0;
	  for (j=-b;j<b;j++) newsam+=sample[i+j];
	  newsam/=taps;
	  sam[i]=newsam;
	  operation->setCounter(i);
	}
      memcpy (&sample[b],&sam[b],(len-taps)*sizeof(int));
      delete sam;
    }

  operation->done();
  return 0;
}
//**********************************************************













