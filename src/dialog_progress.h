#ifndef _DIALOGS_PROGRESS_H_
#define _DIALOGS_PROGRESS_H_ 1

#include <qdialog.h>
#include <qtimer.h>
#include <../lib/timeoperation.h>

class ProgressDialog : public QDialog
{
 Q_OBJECT

 public:
	ProgressDialog 	(TimeOperation *,char *caption);
 	ProgressDialog 	(int max=100,char*caption="Progress");
 	~ProgressDialog ();
 void	setProgress(int);

 signals:

 void done ();

 public slots:

 void	timedProgress();

 protected:

 void paintEvent  (QPaintEvent *);

 private:

 int max;
 int act;
 int last;
 int lastx,lasty;
 int oldw;
 TimeOperation *operation;
 QTimer        *timer;
};
#endif
