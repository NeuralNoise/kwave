/***************************************************************************
         SonagramDialog.h  -  dialog for setting up the sonagram window
                             -------------------
    begin                : Fri Jul 28 2000
    copyright            : (C) 2000 by Thomas Eschenbacher
    email                : Thomas.Eschenbacher@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SONAGRAM_DIALOG_H_
#define _SONAGRAM_DIALOG_H_

#include <qdialog.h>

class Slider;
class QComboBox;
class QLabel;
class QPushButton;
class QRadioButton;
class QStrList;
class KwavePlugin;

//*****************************************************************************
class SonagramDialog : public QDialog 
{
    Q_OBJECT

public:
    SonagramDialog(KwavePlugin &p);
    ~SonagramDialog();

    /**
     * Fills the current parameters into a parameter list.
     * The list always is cleared before it gets filled.
     * The first parameter will contain the number of fft points [1...n]
     * The second parameter will contain the id of a window function
     * or zero if no window function was selected ("<none>").
     */
    void parameters(QStrList &list);

public slots:
    void setPoints(int points);
    void setBoxPoints(int num);

private:
    Slider *m_pointslider;
    QLabel *m_pointlabel;
    QLabel *m_windowlabel;
    QLabel *m_bitmaplabel;
    QLabel *m_windowtypelabel;
    QComboBox *m_pointbox;
    QComboBox *m_windowtypebox;
    QRadioButton *m_rbColor;
    QPushButton *m_ok;
    QPushButton *m_cancel;
    unsigned int m_length;
    unsigned int m_rate;
};

#endif /* _SONAGRAM_DIALOG_H_ */

/* end of SonagramDialog.h */
