//This File includes methods of class SignalWidget that deal with markers
//it also contains methods of Marker and MarkerType class.

#include <math.h>
#include <limits.h>
#include <qobject.h>
#include <qpainter.h>
#include "signalview.h"
#include "dialog_progress.h"
#include "pitchwidget.h"
#include "signalmanager.h"
#include "../lib/parser.h"
#include "../lib/markers.h"
#include "../lib/globals.h"
#include "../lib/dialogoperation.h"
#include "../lib/dynamicloader.h"
#include "../libgui/kwavedialog.h"

extern Global globals;

#define	AUTOKORRWIN 320 
//windowsize for autocorellation, propably a little bit to short for
//lower frequencies, but this will get configurable somewhere in another
//dimension or for those of you who can't zap to other dimensions, it will
//be done in future

int findNextRepeat       (int *,int);
int findNextRepeatOctave (int *,int,double =1.005);
int findFirstMark  (int *,int);

float autotable  [AUTOKORRWIN];
float weighttable[AUTOKORRWIN];


//****************************************************************************
void selectMarkers (const char *command)
{
  KwaveParser parser(command);
} 
//****************************************************************************
MarkerType *findMarkerType (const char *txt)
{
  MarkerType *act;
  int cnt=0;

  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
    {
      if (strcmp (act->name->data(),txt)==0) return act;
      cnt++;
    }
  debug ("could not find Markertype %s\n",txt);
  return 0;
}
//****************************************************************************
void SignalWidget::signalinserted (int start, int len)
{
  struct Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next()) 
      if (tmp->pos>start) tmp->pos+=len;
  setRange (start,start+len); 
  refresh ();
}
//****************************************************************************
void SignalWidget::signaldeleted (int start, int len)
{
  struct Marker *tmp;
  for (tmp=markers->first();tmp;tmp=markers->next())
    {
      if ((tmp->pos>start)&&(tmp->pos<start+len)) //if marker position is within selected boundaries
	{
	  markers->remove ();
	  tmp=markers->first();
	}
      if (tmp->pos>=start+len) tmp->pos-=len;  //if it is greater correct position
    }
  setRange (start,start); 
  refresh ();
}
//****************************************************************************
void SignalWidget::deleteMarks ()
{
  if (signalmanage)
    {
      Marker *tmp;
      int l=signalmanage->getLMarker();
      int r=signalmanage->getRMarker();

      for (tmp=markers->first();tmp;tmp=markers->next())  
	if ((tmp->pos>=l)&&(tmp->pos<r))
	  {
	    markers->remove (tmp);
	    tmp=markers->first();
	  }
      refresh ();
    }
}
//****************************************************************************
void SignalWidget::loadMarks ()
{
  markers->clear(); //remove old marks...

  appendMarks ();
}
//****************************************************************************
void SignalWidget::appendMarks ()
{
  char buf[120];
  QString name=QFileDialog::getOpenFileName (0,"*.label",this);
  if (!name.isNull())
    {
      QFile in(name.data());
      in.open (IO_ReadOnly);
      {
	MarkerType *act;
	while ((strncmp (buf,"Labels",6)!=0)&&(in.readLine(buf,120)>0));

	for (act=globals.markertypes.first();act;act=globals.markertypes.next()) act->selected=-1;

	while (in.readLine (buf,120)>0)
	  {
	    if (strncmp(buf,"Type",4)==0)
	      {
		int num;
		int named;
		char name[120];
		int r,g,b;
		int set=false;
		sscanf (buf,"Type %d %s %d %d %d %d",&num,&name[0],&named,&r,&g,&b);
		for (act=globals.markertypes.first();act;act=globals.markertypes.next()) //linear search for label type ...
		  if (strcmp (act->name->data(),name)==0)
		    {
		      set=true;
		      act->selected=num;
		    }
		if (!set) //generate new type...
		  {
		    struct MarkerType *newtype=new MarkerType;
		    if (newtype)
		      {
			newtype->name=new QString (name);
			newtype->named=named;
			newtype->selected=num;
			newtype->color=new QColor (r,g,b);

			addMarkType (newtype);
		      }
		  }
	      }
	    if (strncmp(buf,"Samples",7)==0) break;
	  }
	//left above loop, so begin to pick up marker positions...

	int num;
	int pos;
	char name [120];

	while (in.readLine (buf,120)>0)
	  {
	    name[0]=0;
	    sscanf (buf,"%d %d %s",&num,&pos,&name[0]);

	    struct Marker *newmark=new Marker;

	    if (newmark)
	      {
		newmark->pos=pos;

		if (globals.markertypes.current())
		  {
		    if (globals.markertypes.current()->selected!=num)
		      for (act=globals.markertypes.first();act->selected!=num;act=globals.markertypes.next());   
		  }
		else
		  for (act=globals.markertypes.first();act->selected!=num;act=globals.markertypes.next());   
		newmark->type=globals.markertypes.current();

		if (globals.markertypes.current()->named)
		  if (name[0]!=0) newmark->name=new QString (name);
		  else newmark->name=0;

		markers->append (newmark);
	      }
	  }
      }
    }
  refresh ();
}
//****************************************************************************
void SignalWidget::saveMarks ()
{
  KwaveDialog *dialog =
    DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

  if ((dialog)&&(dialog->exec()))
    {   
      selectMarkers (dialog->getCommand());

      QString name=QFileDialog::getSaveFileName (0,"*.label",this);
      if (!name.isNull())
	{
	  QFile out(name.data());
	  int num=0;
	  char buf[160];
	  out.open (IO_WriteOnly);
	  out.writeBlock ("Labels\n",7);
	  Marker     *tmp;
	  MarkerType *act;

	  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	    //write out all label types
	    if (act->selected)
	      {
		sprintf (buf,"Type %d %s %d %d %d %d\n",num,act->name->data(),act->named,act->color->red(),act->color->green(),act->color->blue());
		act->selected=num++;
		out.writeBlock (&buf[0],strlen(buf));
	      }
	  out.writeBlock ("Samples\n",8);
	  for (tmp=markers->first();tmp;tmp=markers->next())  //write out labels
	    {
	      //type must be named, and qstring name must be non-null
	      if ((tmp->type->named)&&(tmp->name))
		sprintf (buf,"%d %d %s\n",tmp->type->selected,tmp->pos,tmp->name->data());
	      else 
		sprintf (buf,"%d %d\n",tmp->type->selected,tmp->pos);
	      out.writeBlock (&buf[0],strlen(buf));
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::addMark ()
{
  if (signalmanage&&markertype)
    {
      Marker *newmark=new Marker;

      newmark->pos=signalmanage->getLMarker();
      newmark->type=markertype;
      if (markertype->named)
	{
	  KwaveDialog *dialog =
	    DynamicLoader::getDialog ("command",new DialogOperation("Enter name of label :",true));

	  if ((dialog)&&(dialog->exec()))
	    {   
	      newmark->name=new QString (dialog->getCommand());
	      markers->inSort (newmark);
	      delete dialog;
	    }
	  else delete newmark;
	}
      else
	{
	  newmark->name=0;
	  markers->inSort (newmark);
	}
      refresh();
    }
}
//****************************************************************************
void SignalWidget::jumptoLabel ()
//another fine function contributed by Gerhard Zintel
// if lmarker == rmarker (no range selected) cursor jumps to the nearest label
// if lmarker <  rmarker (range is selected) lmarker jumps to next lower label or zero
// rmarker jumps to next higher label or end
{
  if (signalmanage)
    {
      int lmarker=signalmanage->getLMarker(), rmarker=signalmanage->getRMarker();
      bool RangeSelected = (rmarker - lmarker) > 0;
      if (markers)
      {
	struct Marker *tmp;
	int position = 0;
	for (tmp=markers->first();tmp;tmp=markers->next())
	  if (RangeSelected) {
	    if (tmp->pos < lmarker)
	      if (abs(lmarker-position)>abs(lmarker-tmp->pos)) position = tmp->pos;
	}
	else if (abs(lmarker-position)>abs(lmarker-tmp->pos)) position = tmp->pos;
	lmarker = position;
	position = signalmanage->getLength();
	for (tmp=markers->first();tmp;tmp=markers->next())
	  if (tmp->pos > rmarker)
	    if (abs(rmarker-position)>abs(rmarker-tmp->pos)) position = tmp->pos;
	rmarker = position;
	if (RangeSelected) signalmanage->setMarkers (lmarker,rmarker);
	else signalmanage->setMarkers (lmarker,lmarker);
	refresh ();
      }
    }
}   
//****************************************************************************
void SignalWidget::savePeriods ()
{
  if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  selectMarkers (dialog->getCommand());

	  MarkerType *act;
	  Marker *tmp;
	  int last=0;
	  int rate=signalmanage->getRate ();

	  QString name=QFileDialog::getSaveFileName (0,"*.dat",this);
	  if (!name.isNull())
	    {
	      QFile out(name.data());
	      char buf[160];
	      float freq=0,time,lastfreq=0;
	      out.open (IO_WriteOnly);
	      int first=true;

	      for (act=globals.markertypes.first();act;act=globals.markertypes.next())
		//write only selected label type
		if (act->selected)
		  //traverse list of all labels
		  for (tmp=markers->first();tmp;tmp=markers->next())
		    {
		      if (tmp->type==act)
			{
			  freq=tmp->pos-last;
			  time=last*1000/rate;

			  if ((!first)&&(freq!=lastfreq))
			    {
			      lastfreq=freq;
			      freq=1/(freq/rate);
			      sprintf (buf,"%f %f\n",time,freq);
			      out.writeBlock (&buf[0],strlen(buf));
			    }
			  else lastfreq=freq;
			  first=false;
			  last=tmp->pos;
			}
		    }

	      if (!first) //make sure last tone gets its length
		{
		  time=last*1000/rate;
		  sprintf (buf,"%f %f\n",time,freq);
		  out.writeBlock (&buf[0],strlen(buf));
		}

	      out.close ();
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::saveBlocks (int bit)
{
    if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("saveblock",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  KwaveParser parser (dialog->getCommand());

	  QString filename=parser.getFirstParam();
	  QDir *savedir=new QDir (parser.getNextParam());

	  struct MarkerType *start=findMarkerType(parser.getNextParam());
	  struct MarkerType *stop=findMarkerType (parser.getNextParam());
	  
	  struct Marker *tmp;
	  struct Marker *tmp2;
	  int count=0;
	  int l=signalmanage->getLMarker(); //save old marker positions...
	  int r=signalmanage->getRMarker(); //

	  for (tmp=markers->first();tmp;tmp=markers->next())  //traverse list of markers
	    {
	      if (tmp->type==start)
		{
		  for (tmp2=tmp;tmp2;tmp2=markers->next())  //traverse rest of list to find next stop marker
		    if (tmp2->type==stop)
		      {
			char buf[128];
			sprintf (buf,"%s%04d.wav",filename.data(),count);
			//lets hope noone tries to save more than 10000 blocks...

			signalmanage->setMarkers (tmp->pos,tmp2->pos);
			filename=savedir->absFilePath(buf);
			signalmanage->save (&filename,bit,true);  //save selected range...
			count++;
			break;
		      }
		}
	    }
	  signalmanage->setMarkers (l,r);
	}
    }
}
//****************************************************************************
void SignalWidget::markSignal ()
{
  if (signalmanage)
    {

      Marker *newmark;

      KwaveDialog *dialog =
	DynamicLoader::getDialog ("mark",new DialogOperation(signalmanage->getRate(),true));

      if ((dialog)&&(dialog->exec()))
	{   
	  KwaveParser parser (dialog->getCommand());
	  
	  int level=(int) (parser.toDouble()*(1<<23)/100);

	  int len=signalmanage->getLength();
	  int *sam=signalmanage->getSignal()->getSample();
	  struct MarkerType *start=findMarkerType(parser.getNextParam());
	  struct MarkerType *stop=findMarkerType (parser.getNextParam());
	  int time=(int) (parser.toDouble ()*signalmanage->getRate());

	  if (start&&stop)
	    {
	      newmark=new Marker();  //generate initial marker
	      newmark->pos=0;
	      newmark->type=start;
	      newmark->name=0;
	      markers->inSort (newmark);

	      for (int i=0;i<len;i++)
		{
		  if (abs(sam[i])<level)
		    {
		      int j=i;
		      while ((i<len) &&(abs(sam[i])<level)) i++;
		      if (i-j>time)
			{
			  //insert markers...
			  newmark=new Marker();
			  newmark->pos=i;
			  newmark->type=start;
			  newmark->name=0;
			  markers->inSort (newmark);

			  if (start!=stop)
			    {
			      newmark=new Marker();
			      newmark->pos=j;
			      newmark->type=stop;
			      newmark->name=0;
			      markers->inSort (newmark);
			    }
			}
		    }
		}

	      newmark=new Marker();
	      newmark->pos=len-1;
	      newmark->type=stop;
	      newmark->name=0;
	      markers->inSort (newmark);

	      refresh ();
	    }
	}
    }
}
//****************************************************************************
void SignalWidget::markPeriods ()
{
  if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("mark",new DialogOperation(signalmanage->getRate(),true));

      if ((dialog)&&(dialog->exec()))
	{   
	  KwaveParser parser (dialog->getCommand());

	  int high   =signalmanage->getRate()/parser.toInt();
	  int low    =signalmanage->getRate()/parser.toInt();
	  int octave =parser.toBool ("true");
	  double adjust=parser.toDouble ();

	  for (int i=0;i<AUTOKORRWIN;i++)
	    autotable[i]=1-(((double)i*i*i)/(AUTOKORRWIN*AUTOKORRWIN*AUTOKORRWIN)); //generate static weighting function

	  if (octave) for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]=1; //initialise moving weight table

	  Marker *newmark;
	  int next;
	  int len=signalmanage->getLength();
	  int *sam=signalmanage->getSignal()->getSample();
	  struct MarkerType *start=markertype;
	  int cnt=findFirstMark (sam,len);

	  ProgressDialog *dialog=new ProgressDialog (len-AUTOKORRWIN,"Correlating Signal to find Periods:");
	  dialog->show();

	  if (dialog)
	    {
	      newmark=new Marker();
	      newmark->pos=cnt;
	      newmark->type=start;
	      newmark->name=0;
	      markers->inSort (newmark);

	      while (cnt<len-2*AUTOKORRWIN)
		{
		  if (octave)
		    next=findNextRepeatOctave (&sam[cnt],high,adjust);
		  else
		    next=findNextRepeat (&sam[cnt],high);

		  if ((next<low)&&(next>high))
		    {
		      newmark=new Marker();
		      newmark->pos=cnt;
		      newmark->type=start;
		      newmark->name=0;
		      markers->inSort (newmark);
		    }
		  if (next<AUTOKORRWIN) cnt+=next;
		  else
		    if (cnt<len-AUTOKORRWIN)
		      {
			int a=findFirstMark (&sam[cnt],len-cnt);
			if (a>0) cnt+=a;
			else cnt+=high;
		      }
		    else cnt=len;

		  dialog->setProgress (cnt);
		}

	      delete dialog;

	      refresh ();
	    }
	}
    }
}
//*****************************************************************************
int findNextRepeat (int *sample,int high)
  //autocorellation of a windowed part of the sample
  //returns length of period, if found
{
  int	i,j;
  double gmax=0,max,c;
  int	maxpos=AUTOKORRWIN;
  int	down,up;	//flags

  max=0;
  for (j=0;j<AUTOKORRWIN;j++)
    gmax+=((double)sample[j])*sample [j];

  //correlate signal with itself for finding maximum integral

  down=0;
  up=0;
  i=high;
  max=0;
  while (i<AUTOKORRWIN)
    {
      c=0;
      for (j=0;j<AUTOKORRWIN;j++) c+=((double)sample[j])*sample [i+j];
      c=c*autotable[i]; //multiply window with weight for preference of high frequencies
      if (c>max) max=c,maxpos=i;
      i++;
    }
  return maxpos;
} 
//*****************************************************************************
int findNextRepeatOctave (int *sample,int high,double adjust=1.005)
  //autocorellation of a windowed part of the sample
  //same as above only with an adaptive weighting to decrease fast period changes
{
  int	i,j;
  double gmax=0,max,c;
  int	maxpos=AUTOKORRWIN;
  int	down,up;	//flags

  max=0;
  for (j=0;j<AUTOKORRWIN;j++)
    gmax+=((double)sample[j])*sample [j];

  //correlate signal with itself for finding maximum integral

  down=0;
  up=0;
  i=high;
  max=0;
  while (i<AUTOKORRWIN)
    {
      c=0;
      for (j=0;j<AUTOKORRWIN;j++) c+=((double)sample[j])*sample [i+j];
      c=c*autotable[i]*weighttable[i];
      //multiply window with weight for preference of high frequencies
      if (c>max) max=c,maxpos=i;
      i++;
    }
  
    for (int i=0;i<AUTOKORRWIN;i++) weighttable[i]/=adjust;

  weighttable[maxpos]=1;
  weighttable[maxpos+1]=.9;
  weighttable[maxpos-1]=.9;
  weighttable[maxpos+2]=.8;
  weighttable[maxpos-2]=.8;

  float buf[7];

  for (int i=0;i<7;buf[i++]=.1)

    //low pass filter
  for (int i=high;i<AUTOKORRWIN-3;i++)
    {
      buf[i%7]=weighttable[i+3];
       weighttable[i]=(buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5]+buf[6])/7;
    }

  return maxpos;
} 
//*****************************************************************************
int findFirstMark (int *sample,int len)
  //finds first sample that is non-zero, or one that preceeds a zero crossing
{
  int i=1;
  int last=sample[0];
  int act=last;
  if ((last<100)&&(last>-100)) i=0;
  else
    while (i<len)
      {
	act=sample[i];
	if ((act<0)&&(last>=0)) break;
	if ((act>0)&&(last<=0)) break;
	last=act;
	i++;
      }
  return i;
}
//*****************************************************************************
void SignalWidget::setMarkType  (int num)
{
  this->setOp (SELECTMARK+num);
}
//*****************************************************************************
void SignalWidget::addMarkType (struct MarkerType *marker)
{
  globals.markertypes.append (marker);
  if (manage) manage->addNumberedMenuEntry ("Globals.Markertypes",marker->name->data());
}
//*****************************************************************************
void SignalWidget::addMarkType ()
{
  KwaveDialog *dialog = DynamicLoader::getDialog ("marktype",new DialogOperation(signalmanage->getRate(),true));

  if ((dialog)&&(dialog->exec()))
    {   
      MarkerType *marker=new MarkerType(dialog->getCommand());
      if (marker) addMarkType (marker);
    }
}
//*****************************************************************************
void SignalWidget::convertMarkstoPitch ()
{
  if (signalmanage)
    {
      KwaveDialog *dialog =
	DynamicLoader::getDialog ("marksave",new DialogOperation(&globals,signalmanage->getRate(),0,0));

      if ((dialog)&&(dialog->exec()))
	{   
	  selectMarkers (dialog->getCommand());

	  MarkerType *act;
	  Marker     *tmp;
	  int   len=signalmanage->getLength()/2;
	  float *data=new float[len];
	  float freq;
	  float rate=(float)signalmanage->getRate();

	  for (int i=0;i<len;data[i++]=0);

	  for (act=globals.markertypes.first();act;act=globals.markertypes.next())
	    {
	      if (act->selected)
		{
		  int   last=0;
		  //traverse list of all labels of the selected type...
		  for (tmp=markers->first();tmp;tmp=markers->next())
		    {
		      if (tmp->type==act)
			{
			  if (tmp->pos!=last)
			    {
			      freq=rate/(tmp->pos-last);
			    }
			  else freq=0;

			  for (int i=last;i<tmp->pos;i+=2) data[i/2]=freq;

			  last=tmp->pos;
			}
		    }
		  PitchWindow *window=new PitchWindow (signalmanage->getName());
		  window->show ();
		  if (window) window->setSignal (data,len,rate/2);
		}
	    }
	}
    }
}
