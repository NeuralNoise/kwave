#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../lib/interpolation.h"
#include "../../../lib/menuitem.h"
#include "../../../lib/timeoperation.h"
#include "../../../lib/curve.h"
#include "../../../lib/parser.h"         

#define PROGRESS_SIZE 512*3*4

const char *version="1.0";
const char *author="Martin Wilz";
const char *name="amplify";
//**********************************************************
int operation (TimeOperation *operation)
{
  int *sample=operation->getSample();
  int len=operation->getLength();
  KwaveParser parse(operation->getCommand());

  double mult=parse.toDouble();
  int j;
	
  for (int i=0;i<len;)
    {
      if (i<len-PROGRESS_SIZE) j=i+PROGRESS_SIZE;
      else j=len;

      for (;i<j;i++)
	sample[i]=(int) (sample[i]*mult);

      operation->setCounter(i);
    }

  operation->done();
  return 0;
}
//**********************************************************













