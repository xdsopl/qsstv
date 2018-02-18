/***************************************************************************
 *   Copyright (C) 2000-2008 by Johan Maes                                 *
 *   on4qz@telenet.be                                                      *
 *   http://users.telenet.be/on4qz                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QtGui>
#include "editorscene.h"
#include "ui_editorform.h"


/**
@author Johan Maes - ON4QZ
*/
class editorForm;
enum eactionType {SELECT,COLORPICK,RECTANGLE,ELLIPSE,LINE,IMAGE,TEXT};
enum ePickMode {NOPICK, PICKFILLCOLOR,PICKLINECOLOR};

/** Widget to display the various canvasItems */
class editorView : public QWidget,private Ui::editorForm
{
Q_OBJECT
public:
	
	editorView(QWidget *parent = 0);
	~editorView();
	void readSettings();
	void writeSettings();
	
	bool isModified() {return modified;}
	bool open(QFile &f);
	void save(QFile &f,bool templ);

	QImage *getImage() { return image;}
	void setImage(QImage *ima);
	editorScene *getScene() {return scene;}

public slots:
  void slotChangeCanvasSize();
	void slotArrow();
	void slotRectangle();
	void slotCircle();
	void slotColorPicker();
	void slotColorPicked(const QPointF &p);
	void slotText();
	void slotImage();
	void slotReplay();
	void slotLine();
	void slotClearAll();

//Font
	void slotFontChanged(const QFont &);
	void slotFontSizeChanged(int);
	void slotPenWidthChanged(double);
	void slotBold(bool);
	void slotItalic(bool);
	void slotUnderline(bool);

//Color
	void slotColorDialog();
	void slotGradientDialog();
	void slotButtonTriggered();

//Transform
	void slotRotateChanged(int);
	void slotShearChanged(int);
//	void slotShearRotateChanged(double,int);
//item feedback
	void slotItemSelected(itemBase*);
//Debug

	void slotTextReturnPressed(const QString &);
private:
	editorScene *scene;
	bool modified;
	QImage *image;
	void setTransform();
  QIcon createColorToolButtonIcon(const QString &imageFile, QColor color);
	QMenu *createColorMenu(const char *,int,QString text);
	int canvasSizeIndex;
	int currentPointSize;
	int currentFontIndex;
	double currentPenWidth;
	QString txt;
	QImage *colorPickImage;
	ePickMode pickMode;
  int canvasWidth;
  int canvasHeight;
  void changeCanvasSize();
  void dump();
};

#endif
