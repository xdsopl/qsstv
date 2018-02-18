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

#include "editorview.h"
#include "appglobal.h"
#include "utils/dirdialog.h"
#include "graphicitems.h"
#include "gradientdialog.h"
#include "canvassizeform.h"
#include "ui_textform.h"


#define TBFILL 0
#define TBLINE 1
#define TBGRAD 2

#define NUMCOLORSELECTORS 12

static QColor defaultColors[NUMCOLORSELECTORS]= 
{
  qRgba(  0,  0,  0,255),
  qRgba(255,255,255,255),
  qRgba(255,  0,  0,255),
  qRgba(  0,255,  0,255),
  qRgba(  0,  0,255,255),
  qRgba(128,128,128,255),
  qRgba(255,  0,  0,128),
  qRgba(  0,255,  0,128),
  qRgba(  0,  0,255,128),
  qRgba(255,255,  0,128),
  qRgba(  0,255,255,128),
  qRgba(255,  0,255,128)
};




struct sCanvasSize
{
  const QString s;
  int width;
  int height;
};
#define SIZES 8
sCanvasSize canvasSizeArray[SIZES]=
{
  {"160x120",160,120},
  {"320x240",320,240},
  {"320x256",320,256},
  {"500x400",500,400},
  {"500x496",500,496},
  {"640x496",640,496},
  {"800x616",800,616},
  {"1024x800",1024,800}
};

//int s;

#define BORDER 4
/** editorview */
editorView::editorView(QWidget *parent):QWidget(parent), Ui::editorForm()
{
  setupUi(this);
  scene=new editorScene(canvas);
  canvas->setScene(scene);
  // canvas->setFixedSize(800,700);;
  pickMode=NOPICK;
  //border= new QGraphicsRectItem(0,0,320,256);
  //	scene->border=border->rect();
  //	scene->addItem(border);

  scene->setMode(editorScene::MOVE);
  scene->setItemType(itemBase::BASE);
  //  for (int i=0; i<SIZES;i++) sizeComboBox->addItem(canvasSizeArray[i].s);

  connect(imageSizePushButton,SIGNAL(clicked()),SLOT(slotChangeCanvasSize()));

  readSettings();
  //  sizeComboBox->setCurrentIndex(canvasSizeIndex);
//  scene->setSceneRect(0,0,canvasSizeArray[canvasSizeIndex].width,canvasSizeArray[canvasSizeIndex].height);
  connect(scene,SIGNAL(itemSelected(itemBase*)),SLOT(slotItemSelected(itemBase*)));
  //  connect(sizeComboBox, SIGNAL(activated(int)), SLOT(slotChangeCanvasSize(int)));
  connect(arrowPushButton,SIGNAL(clicked()),SLOT(slotArrow()));
  connect(rectanglePushButton,SIGNAL(clicked()),SLOT(slotRectangle()));
  connect(circlePushButton,SIGNAL(clicked()),SLOT(slotCircle()));
  connect(replayPushButton,SIGNAL(clicked()),SLOT(slotReplay()));
  connect(imagePushButton,SIGNAL(clicked()),SLOT(slotImage()));
  connect(linePushButton,SIGNAL(clicked()),SLOT(slotLine()));
  connect(textPushButton,SIGNAL(clicked()),SLOT(slotText()));
  rotateLCD->display( "  0'" );
  connect(rotateDial, SIGNAL(valueChanged(int)),SLOT(slotRotateChanged(int)) );
  hshearLCD->display( "0.00" );
  vshearLCD->display( "0.00" );
  connect(hshearSlider, SIGNAL(valueChanged(int)),SLOT(slotShearChanged(int)) );
  connect(vshearSlider, SIGNAL(valueChanged(int)),SLOT(slotShearChanged(int)) );
  //connect (scene,SIGNAL(changeSize(int,int)),SLOT(slotChangeCanvasSize(int,int)));

  //	connect(textLineEdit, SIGNAL(textChanged(const QString &)),SLOT(slotTextReturnPressed(const QString &)) );
  //	slotRotateChanged(0);
  //	slotShearChanged(0);
  fontComboBox->setCurrentIndex(currentFontIndex);

  connect(fontComboBox,SIGNAL(currentFontChanged(const QFont &)),SLOT(slotFontChanged(const QFont &)));

  fontSizeSpinBox->setRange(6, 180);
  fontSizeSpinBox->setValue(currentPointSize);
  scene->font.setPointSize(fontSizeSpinBox->value());
  connect(fontSizeSpinBox,SIGNAL( valueChanged (int)),SLOT(slotFontSizeChanged(int)));

  penWidthSpinBox->setRange(0,99);
  penWidthSpinBox->setValue(currentPenWidth);
  connect(penWidthSpinBox,SIGNAL( valueChanged (double)),SLOT(slotPenWidthChanged(double)));


  boldButton->setChecked(scene->font.bold());
  italicButton->setChecked(scene->font.italic());
  underlineButton->setChecked(scene->font.underline());
  connect(boldButton,SIGNAL( clicked(bool)),SLOT(slotBold(bool)));
  connect(italicButton,SIGNAL( clicked(bool)),SLOT(slotItalic(bool)));
  connect(underlineButton,SIGNAL( clicked(bool)),SLOT(slotUnderline(bool)));


  QAction *action;
  action = new QAction("Color Picker", this);
  action->setData(TBFILL);
  connect(action, SIGNAL(triggered()),this,SLOT(slotColorPicker()));

  fillToolButton->setMenu(createColorMenu(SLOT(slotColorDialog()),TBFILL,"Select Color"));

  fillToolButton->menu()->addAction(action);
  // scene->fillColor=QColor(127,127,0);
  fillToolButton->setIcon(createColorToolButtonIcon(":/icons/colorfill.png", scene->fillColor));
  connect(fillToolButton, SIGNAL(clicked()),this, SLOT(slotButtonTriggered()));

  action = new QAction("Color Picker", this);
  action->setData(TBLINE);
  connect(action, SIGNAL(triggered()),this,SLOT(slotColorPicker()));
  lineToolButton->setMenu(createColorMenu(SLOT(slotColorDialog()),TBLINE,"Select Color"));
  lineToolButton->menu()->addAction(action);
  lineToolButton->setIcon(createColorToolButtonIcon(":/icons/colorline.png", scene->lineColor));
  connect(lineToolButton, SIGNAL(clicked()),this, SLOT(slotButtonTriggered()));
  connect(scene,SIGNAL(colorSelected( const QPointF &)),this,SLOT(slotColorPicked(const QPointF &)));

  gradientToolButton->setMenu(createColorMenu(SLOT(slotGradientDialog()),TBGRAD,"Select Gradient"));
  gradientToolButton->setIcon(createColorToolButtonIcon(":/icons/gradient.png", scene->gradientColor));
  connect(gradientToolButton, SIGNAL(clicked()),this, SLOT(slotButtonTriggered()));
  // setup the defaults
  slotRotateChanged(0);

  setTransform();
  slotFontChanged(fontComboBox->currentFont());
  slotPenWidthChanged(currentPenWidth);
  changeCanvasSize();
#ifndef QT_NO_DEBUG
  dump();
#endif

  modified=false;
}


editorView::~editorView()
{
  writeSettings();
}

/*! 
  reads the settings (saved images for tx,rx,templates)
*/

void editorView::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("Editor");
  canvasWidth=qSettings.value("canvasWidth", 800 ).toInt();
  canvasHeight=qSettings.value("canvasHeight", 600 ).toInt();
  currentFontIndex=qSettings.value("currentfontindex", 2 ).toInt();
  currentPointSize=qSettings.value("currentpointSize", 24).toInt();
  currentPenWidth=qSettings.value("currentpenwidth", 1).toDouble();
  scene->font = qSettings.value("fillcolor", qApp->font()).value<QFont>();
  scene->fillColor = qSettings.value("fillcolor", QColor(Qt::white)).value<QColor>();
  scene->lineColor = qSettings.value("linecolor", QColor(Qt::black )).value<QColor>();
  scene->gradientColor = qSettings.value("gradientcolor",QColor( Qt::red )).value<QColor>();
  qSettings.endGroup();
}

/*! 
  writes the settings (saved images for tx,rx,templates)
*/
void editorView::writeSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("Editor" );
//  qSettings.setValue ("canvassizeindex", sizeComboBox->currentIndex());
  qSettings.setValue ("currentfontindex", fontComboBox->currentIndex());
  qSettings.setValue ("currentpenwidth", penWidthSpinBox->value());
  qSettings.setValue ("currentpointsize",fontSizeSpinBox->value());
  qSettings.setValue ("fillcolor", scene->fillColor);
  qSettings.setValue ("linecolor", scene->lineColor);
  qSettings.setValue ("gradientcolor", scene->gradientColor);
  qSettings.setValue ("canvasWidth",canvasWidth);
  qSettings.setValue ("canvasHeight",canvasHeight);
  qSettings.endGroup();
}



void editorView::slotArrow()
{

}

void editorView::slotRectangle()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(itemBase::RECTANGLE);
  modified=true;
}


void editorView::slotCircle()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(itemBase::ELLIPSE);
  modified=true;
}

void editorView::slotText()
{
  QDialog d(this);
  Ui::textForm t;
  t.setupUi(&d);
  t.plainTextEdit->setPlainText(txt);
  if(d.exec()==QDialog::Accepted)
    {
      scene->setMode(editorScene::INSERT);
      scene->setItemType(itemBase::TEXT);
      scene->text=t.plainTextEdit->toPlainText();
      txt=t.plainTextEdit->toPlainText();
      scene->apply(editorScene::DTEXT);
    }
  modified=true;
}

void editorView::slotLine()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(itemBase::LINE);
  modified=true;
}

void editorView::slotColorPicker()
{
  int tp;
  colorPickImage=scene->renderImage(0,0);
  addToLog(QString("colorpicker triggered size %1 x %2").arg(colorPickImage->width()).arg(colorPickImage->height()),LOGEDIT);
  QAction *act;
  act=qobject_cast<QAction *>(sender());
  tp=act->data().toInt();
  if (tp==TBFILL)
    {
      pickMode=PICKFILLCOLOR;

    }
  else if (tp==TBLINE)
    {
      pickMode=PICKLINECOLOR;
    }
  scene->setMode(editorScene::PICK);
  //setCursor(Qt::ArrowCursor);
}

void editorView::slotColorPicked(const QPointF &p)
{
  QRgb c=colorPickImage->pixel(p.x(),p.y());
  addToLog(QString("Picked color r=%1,g=%2,b=%3 alpha=%4").arg(qRed(c)).arg(qGreen(c)).arg(qBlue(c)).arg(qAlpha(c)),LOGEDIT);
  if(pickMode==PICKFILLCOLOR)
    {
      scene->fillColor.setRgba(c);
      fillToolButton->setIcon(createColorToolButtonIcon(":/icons/colorfill.png",scene->fillColor));
    }
  else
    {
      scene->lineColor.setRgba(c);
      lineToolButton->setIcon(createColorToolButtonIcon(":/icons/colorline.png",scene->lineColor));
    }
  canvas->setCursor(Qt::ArrowCursor);
}


void editorView::slotImage()
{
  QString fileName;
  dirDialog dd((QWidget *)this,"editor");
  scene->fl=dd.openFileName(QString());
  scene->setMode(editorScene::INSERT);
  scene->setItemType(itemBase::IMAGE);
  modified=true;
}

void editorView::slotReplay()
{
  scene->setMode(editorScene::INSERT);
  scene->setItemType(itemBase::REPLAY);
  modified=true;
}

void editorView::changeCanvasSize()
{
  rotateDial->setValue(0);
  hshearSlider->setValue(0);
  vshearSlider->setValue(0);
  setTransform();
  scene->addBorder(canvasWidth,canvasHeight);
  canvas->setSceneRect(0,0,canvasWidth,canvasHeight);
  modified=true;
}


void editorView::slotChangeCanvasSize()
{
  QRect r;
  canvasSizeForm csize;
  csize.setSize(canvasWidth,canvasHeight);
  if(csize.exec()==QDialog::Accepted)
    {
      r=csize.getSize();
      canvasWidth=r.width();
      canvasHeight=r.height();
      changeCanvasSize();
    }
}

void editorView::slotFontChanged(const QFont &f)
{
  scene->font=f;
  scene->font.setPointSize(fontSizeSpinBox->value());
  scene->font.setBold(boldButton->isChecked());
  scene->font.setItalic(italicButton->isChecked());
  scene->font.setUnderline(underlineButton->isChecked());
  scene->apply(editorScene::DFONT);
  modified=true;
}

void editorView::slotFontSizeChanged(int sz)
{
  scene->font.setPointSize(sz);
  scene->apply(editorScene::DFONT);
  modified=true;
}

void editorView::slotPenWidthChanged(double pw)
{
  scene->penWidth=pw;
  scene->apply(editorScene::DPEN);
  modified=true;
}
void editorView::slotBold(bool b)
{
  scene->font.setBold(b);
  scene->apply(editorScene::DFONT);
  modified=true;
}
void editorView::slotItalic(bool b)
{
  scene->font.setItalic(b);
  scene->apply(editorScene::DFONT);
  modified=true;
}
void editorView::slotUnderline(bool b)
{
  scene->font.setUnderline(b);
  scene->apply(editorScene::DFONT);
  modified=true;
}

/*! \todo image insert
    check if this is used
*/
void editorView::setImage(QImage *)
{

  scene->setMode(editorScene::INSERT);
  scene->setItemType(itemBase::IMAGE);
  modified=true;
}


void editorView::slotClearAll()
{
  scene->clearAll();
}


void editorView::slotButtonTriggered()
{
  QToolButton *act;
  act=qobject_cast<QToolButton *>(sender());
  if (act==fillToolButton)
    {
      scene->apply(editorScene::DFILLCOLOR);
    }
  else if (act==lineToolButton)
    {
      scene->apply(editorScene::DLINECOLOR);
    }
  else if (act==gradientToolButton)
    {
      scene->apply(editorScene::DGRADIENT);
    }
  else
    {
      return;
    }
}

void editorView::slotColorDialog()
{
  int tp;
  QAction *act;
  QColor c;
  act=qobject_cast<QAction *>(sender());
  tp=act->data().toInt();
  switch(tp)
    {
    case TBFILL:
      c=QColorDialog::getColor(scene->fillColor,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid())
        {
          scene->fillColor=c;
          fillToolButton->setIcon(createColorToolButtonIcon(":/icons/colorfill.png",scene->fillColor));
        }
    break;
    case TBLINE:
      c=QColorDialog::getColor(scene->lineColor,this,"",QColorDialog::ShowAlphaChannel);
      if (c.isValid())
        {
          scene->lineColor=c;
          lineToolButton->setIcon(createColorToolButtonIcon(":/icons/colorline.png",scene->lineColor));
        }
    break;
    default:
    break;
    }
}

void editorView::slotGradientDialog()
{
  gradientDialog gDiag(this);
  gDiag.selectGradient();
}

void editorView::save(QFile &f,bool templ)
{
  scene->save(f,templ);
  modified=false;
}

bool editorView::open(QFile &f)
{
  if(!scene->load(f)) return false;
  canvasWidth=scene->border.width();
  canvasHeight=scene->border.height();
  changeCanvasSize();
#ifndef QT_NO_DEBUG
  dump();
#endif
  return true;
}


void editorView::setTransform()
{
  //	int r=450-rotateDial->value();
  //	if ( r >= 360 )	r-=360;
  int r=rotateDial->value();
  scene->rotate=r;
  scene->hShear=(double)hshearSlider->value()/10.;
  scene->vShear=(double)vshearSlider->value()/10.;
  scene->apply(editorScene::DTRANSFORM);
}

/*! \todo  check if used
*/
void editorView::slotTextReturnPressed(const QString &)
{

}


void editorView::slotRotateChanged(int)
{
  QString tmp;
  //	int r=450-rotateDial->value();
  //	if ( r >= 360 )	r-=360;
  int r=rotateDial->value();
  tmp.sprintf( "%3i'", r );
  rotateLCD->display( tmp );
  setTransform();
}

void editorView::slotShearChanged(int)
{
  QString tmp;
  double shearVal;
  QSlider *sl;
  sl=qobject_cast<QSlider *>(sender());
  shearVal=((double)sl->value())/10;
  tmp.sprintf( "%1.3f", shearVal  );
  if ( shearVal >= 0 )
    tmp.insert( 0, " " );
  if(sl==hshearSlider)
    {
      hshearLCD->display( tmp );
    }
  else
    {
      vshearLCD->display( tmp );
    }
  setTransform();
}

QIcon editorView::createColorToolButtonIcon(const QString &imageFile, QColor color)
{
  QPixmap pixmap(22, 30);
  pixmap.fill(Qt::transparent);
  QPainter painter(&pixmap);
  QPixmap image(imageFile);
  QRect target(0, 0, 22, 22);
  QRect source(0, 0, 22, 22);
  painter.fillRect(QRect(0, 22, 22, 8), color);
  painter.drawPixmap(target, image, source);
  return QIcon(pixmap);
}

QMenu *editorView::createColorMenu(const char * slot,int type,QString text)
{
  QMenu *colorMenu = new QMenu;
  QAction *action = new QAction(text, this);
  action->setData(type);
  connect(action, SIGNAL(triggered()),this,slot);
  colorMenu->addAction(action);
  return colorMenu;
}

void editorView::slotItemSelected(itemBase* ib)
{
  sitemParam p;
  p=ib->getParam();
  if(p.type==itemBase::TEXT)
    {
      boldButton->setChecked(p.font.bold());
      underlineButton->setChecked(p.font.underline());
      italicButton->setChecked(p.font.italic());
      fontComboBox->setCurrentFont(p.font);
      fontSizeSpinBox->setValue(p.font.pointSize());
    }
  penWidthSpinBox->setValue(p.pen.widthF());
  int rot=p.rotation;
  rotateDial->setValue(rot);
  hshearSlider->setValue((int)(p.hShear*10));
  vshearSlider->setValue((int)(p.vShear*10));
}


#ifndef QT_NO_DEBUG
void editorView::dump()
{
  QString t;
  int i;
  QList<QGraphicsItem *> l=scene->items();
  itemBase *b;
  addToLog(QString("dump editorView of items: %1").arg(l.count()),LOGEDIT); //exclude border
  for(i=0;i<l.count();i++)
    {
      //      if(l.at(i)->type()>=itemBase::BASE)
      {
        b=qgraphicsitem_cast<itemBase *>(l.at(i));
        switch((int)b->type())
          {
          case itemBase::TEXT:
            t="Text:";
          break;
          case itemBase::LINE:
            t="Line:";
          break;
          case itemBase::IMAGE:
            t="Image:";
          break;
          case itemBase::RECTANGLE:
            t="Rectangle:";
          break;
          case itemBase::ELLIPSE:
            t="Ellipse:";
          break;
          case itemBase::SBORDER:
            t="Border:";
          break;
          default:
            t=QString("Ill: %1").arg(l.at(i)->type());
          break;
          }
        addToLog(QString("editorViewItems %1 pos=%2,%3 rectxy=%4,%5 size=%6x%7 depth=%8")
                 .arg(t)
                 .arg(b->pos().x()).arg(b->pos().y())
                 .arg(b->rect().x()).arg(b->rect().y())
                 .arg(b->rect().width()).arg(b->rect().height())
                 .arg(b->zValue()),LOGEDIT);
      }
    }

}
#endif
