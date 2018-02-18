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
#include "graphicitems.h"
#include "appglobal.h"
#include "editorscene.h"

/*!

*/
static QPainterPath qt_graphicsItem_shapeFromPath(const QPainterPath &path, const QPen &pen)
{
  // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
  // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
  const qreal penWidthZero = qreal(0.00000001);

  if (path == QPainterPath()) return path;
  QPainterPathStroker ps;
  ps.setCapStyle(pen.capStyle());
  if (pen.widthF() <= 0.0) ps.setWidth(penWidthZero);
  else  ps.setWidth(pen.widthF());
  ps.setJoinStyle(pen.joinStyle());
  ps.setMiterLimit(pen.miterLimit());
  QPainterPath p = ps.createStroke(path);
  p.addPath(path);
  return p;
}


itemBase::itemBase(QMenu *cntxtMenu)
{
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
  setAcceptHoverEvents (true);
  param.locked=false;
  param.modified=true;
  param.menu=cntxtMenu;
}

void itemBase::highlightSelected(QPainter *painter,const QStyleOptionGraphicsItem *option)
{

  ///	qreal itemPenWidth = pen().widthF();
  //		const qreal pad = itemPenWidth / 2;
  //	const qreal pad = itemPenWidth;
  const qreal penWidth = 0; // cosmetic pen
  const QColor fgcolor = option->palette.windowText().color();
  const QColor bgcolor( // ensure good contrast against fgcolor
                        fgcolor.red()   > 127 ? 0 : 255,
                        fgcolor.green() > 127 ? 0 : 255,
                        fgcolor.blue()  > 127 ? 0 : 255);
  painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
  painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(boundingRect());
  //	painter->drawRect(boundingRect().adjusted(pad, pad, -pad, -pad));
}

QPainterPath itemBase::shape() const
{
  QPainterPath path;
  path.addRect(param.rct);
  return qt_graphicsItem_shapeFromPath(path,pen());
}

void itemBase::setBrush(QColor c)
{
  param.fillColor=c;
  QAbstractGraphicsShapeItem::setBrush(param.fillColor);
}

void itemBase::load(QDataStream &str)
{
  //	str << type(); this item is already read by the loader
  QTransform f;
  QPointF p;
  QRectF r;
  QColor c;
  QPen pn;
  QBrush br;
  QString t;
  QFont fnt;
  qreal z;
  str >> param.rotation;
  str >> param.hShear;
  str >> param.vShear;
  str >> z;
  setZValue(z);
  param.zValue=z;
  str >> p;
  setPos(p);
  param.position=p;
  str >> r;
  setRect(r);
  str >> c;
  param.fillColor=c;
  setBrush(c);
  str >> pn;
  setPen(pn);
  str >> br;
  QAbstractGraphicsShapeItem::setBrush(br);
  str >> param.locked;
  str >>param.im;
  str >>t;
  setText(t);
  str >>fnt;
  setFont(fnt);
  str >> param.line;
  param.gradient.load(str);
  setTransform ();
}

void itemBase::save(QDataStream &str)
{
  str << type();
  str << param.rotation;
  str << param.hShear;
  str << param.vShear;
  str	<< zValue();
  str << pos();
  str << rect();
  str << param.fillColor;
  str << pen();
  str << brush();
  str << param.locked;
  str << param.im;
  str << param.txt;
  str << param.font;
  str << param.line;
  param.gradient.save(str);


}

void itemBase::setTransform ()
{
  QTransform tx;
  tx.translate(rect().x()+rect().width()/2,rect().y()+rect().height()/2);
  tx.shear(param.hShear,param.vShear);
  tx.rotate(param.rotation);
  tx.translate(-rect().x()-rect().width()/2,-rect().y()-rect().height()/2);
  QAbstractGraphicsShapeItem::setTransform(tx,false);
  update();
}

void itemBase::setTransform ( int rot,double hs,double vs)
{
  param.rotation=rot;
  param.hShear=hs;
  param.vShear=vs;
  setTransform ();
}

void itemBase::hoverMoveEvent ( QGraphicsSceneHoverEvent * event )
{
  if(((editorScene*)scene())->mode==editorScene::PICK)
    {
      QPixmap pm(":/icons/colorpicker.png");
      QCursor cpCursor(pm,0,pm.height()-1);
      setCursor(cpCursor);
      return;
    }
  if(((editorScene*)scene())->mode==editorScene::INSERT)
    {
      setCursor(Qt::ArrowCursor);
      return;
    }


  if(param.locked)
    {
      grab = NO;
      setCursor(Qt::ForbiddenCursor);
    }
  else if(type()!=LINE)
    {
      grab = getCorner(event->pos());
      if(type()==TEXT) grab=NO;
      if ((grab == CUL)|| (grab == CDR)) setCursor(Qt::SizeFDiagCursor);
      if ((grab == CUR)|| (grab == CDL)) setCursor(Qt::SizeBDiagCursor);
      if ((grab == HU) || (grab == HD))  setCursor(Qt::SizeVerCursor);
      if ((grab == VL) || (grab == VR))  setCursor(Qt::SizeHorCursor);
      if (grab == NO) setCursor(Qt::OpenHandCursor);
    }
  else
    {
      grab=NO;
      setCursor(Qt::CrossCursor);
    }
  //QAbstractGraphicsShapeItem::hoverEnterEvent(event);
  QAbstractGraphicsShapeItem::hoverMoveEvent(event);
}




itemBase::corner itemBase::getCorner( QPointF mouse)
{
  double x = rect().x();
  double y = rect().y();
  double h = rect().height();
  double w = rect().width();

  double diff;
  diff=w; if (diff>h) diff=h;
  diff/=10;
  if (diff>10) diff=10;
  else if (diff<1) diff=1;

  QRectF cul (x,y, diff,diff);
  QRectF cur (x+w-diff,y, diff,diff);
  QRectF cdl (x,y+h-diff,diff,diff);
  QRectF cdr (x+w-diff,y+h-diff,diff,diff);

  QRectF hu (x+diff,y,w-(2*diff),diff);
  QRectF hd (x+diff,y+h-diff,w-(2*diff),diff);
  QRectF vl (x,y+diff,diff,h-(2*diff));
  QRectF vr (x+w-diff,y+diff,diff,h-(2*diff));

  if ( cul.contains(mouse) ) return CUL;
  if ( cur.contains(mouse) ) return CUR;
  if ( cdl.contains(mouse) ) return CDL;
  if ( cdr.contains(mouse) ) return CDR;
  if ( hu.contains(mouse) ) return HU;
  if ( hd.contains(mouse) ) return HD;
  if ( vl.contains(mouse) ) return VL;
  if ( vr.contains(mouse) ) return VR;

  return NO;
}


void itemBase::mouseMoveEvent ( QGraphicsSceneMouseEvent * event )
{
  if(((editorScene*)scene())->mode==editorScene::PICK) return;
  if(((editorScene*)scene())->mode==editorScene::INSERT) return;
  if(param.locked) return;
  QPointF mouse = event->pos();
  //resize!
  prepareGeometryChange();
  param.modified=true;
  switch ( grab)
    {
    case NO : QAbstractGraphicsShapeItem::mouseMoveEvent(event);
    break;
    case HD :
      if(mouse.y()-rect().y()>1)
        setRect(rect().x(),rect().y(),rect().width(),mouse.y()-rect().y());
    break;
    case HU :
      if(rect().height()+(rect().y()-mouse.y())>0)
        setRect(rect().x(),mouse.y(),rect().width(),rect().height()+(rect().y()-mouse.y()));
    break;
    case VR :
      if(mouse.x()-rect().x()>1)
        setRect(rect().x(),rect().y(),mouse.x()-rect().x(),rect().height());
    break;
    case VL :
      if(rect().width()+(rect().x()-mouse.x())>1)
        setRect(mouse.x(),rect().y(),rect().width()+(rect().x()-mouse.x()),rect().height());
    break;
    case CDR :
      if(((mouse.x()-rect().x())>1)&&((mouse.y()-rect().y())>1))
        setRect(rect().x(),rect().y(),mouse.x()-rect().x(),mouse.y()-rect().y());
    break;
    case CUR :
      if(((mouse.x()-rect().x())>1)&&((rect().height()+(rect().y()-mouse.y()))>1))
        setRect(rect().x(),mouse.y(),mouse.x()-rect().x(),rect().height()+(rect().y()-mouse.y()));
    break;
    case CDL :
      if(((rect().width()+(rect().x()-mouse.x()))>1)&&((mouse.y()-rect().y())>1))
        setRect(mouse.x(),rect().y(),rect().width()+(rect().x()-mouse.x()),mouse.y()-rect().y());
    break;
    case CUL :
      if(((rect().width()+(rect().x()-mouse.x()))>1)&&((rect().height()+(rect().y()-mouse.y()))>1))
        setRect(mouse.x(),mouse.y(),rect().width()+(rect().x()-mouse.x()),rect().height()+(rect().y()-mouse.y()));
    break;
    }
  update();
}

QString itemBase::getTypeStr()
{
  QString tp;
  switch(type())
    {
    case BASE: tp="Base"; break;
    case RECTANGLE: tp="Rectangle";  break;
    case ELLIPSE: tp="Ellipse"; break;
    case IMAGE: tp="Image";  break;
    case LINE: tp="Line";  break;
    case TEXT: tp="Text"; break;
    case REPLAY: tp="Replay";  break;
    case SBORDER: tp="SBorder";  break;
    }
  return tp;
}


void itemBase::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
  if(((editorScene*)scene())->mode==editorScene::PICK) return;
  setSelected(true);
  param.menu->exec(event->screenPos());
}


// Text graphics

itemText::itemText(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  param.font.setFamily("Times");
  param.font.setPointSize(24);
  param.font.setStyleStrategy(QFont::ForceOutline);
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
  setAcceptHoverEvents (true);
}

itemText::~itemText()
{
}


void itemText::setText(const QString &t)
{
  //  QPainterPath tt;
  //  prepareGeometryChange();
  param.modified=true;
  param.txt=t;
  //  tt.addText(0, 0, param.font, param.txt);
  //  param.rct=tt.controlPointRect();
  update();
}

void itemText::setFont(QFont f)
{
  //  QPainterPath tt;
  //  prepareGeometryChange();
  param.modified=true;
  param.font=f;
  param.font.setStyleStrategy(QFont::ForceOutline);
  //  tt.addText(0, 0, param.font, param.txt);
  //  param.rct=tt.controlPointRect();
  update();
}


static QRectF setupTextLayout(QTextLayout *layout)
{
  layout->setCacheEnabled(true);
  layout->beginLayout();
  while (layout->createLine().isValid())
    ;
  layout->endLayout();
  qreal maxWidth = 0;
  qreal y = 0;
  for (int i = 0; i < layout->lineCount(); ++i) {
      QTextLine line = layout->lineAt(i);
      maxWidth = qMax(maxWidth, line.naturalTextWidth());
      line.setPosition(QPointF(0, y));
      y += line.height();
    }
  return QRectF(0, 0, maxWidth, y);
}

void itemText::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  QPainterPath tt;
  QString tmp=param.txt;
  tmp.replace(QLatin1Char('\n'), QChar::LineSeparator);
  QTextLayout layout(tmp, param.font);

  QPen p;
  if (param.modified)
    {
      param.modified=false;
      if(param.gradient.type!=sgradientParam::NONE)
        {
          QAbstractGraphicsShapeItem::setBrush(buildGradient(param.gradient,rect()));
        }
      else
        {
          QAbstractGraphicsShapeItem::setBrush(param.fillColor);
        }
    }
  p.setBrush(brush());
  painter->setPen(p);
  QTextLayout::FormatRange range;
  range.start = 0;
  range.length = layout.text().length();
  range.format.setTextOutline(pen());
  QList<QTextLayout::FormatRange> formats;
  formats.append(range);
  layout.setAdditionalFormats(formats);
  param.rct=setupTextLayout(&layout);
  layout.draw(painter, QPointF(0, 0));
  if (option->state & QStyle::State_Selected)  highlightSelected(painter,option);
}














itemRectangle::itemRectangle(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  setRect(0,0,100,100);
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
}



void itemRectangle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  //	if (resized ) prepareGeometryChange();
  if (param.modified)
    {
      param.modified=false;
      if(param.gradient.type!=sgradientParam::NONE)
        {
          QAbstractGraphicsShapeItem::setBrush(buildGradient(param.gradient,rect()));
        }
      else
        {
          QAbstractGraphicsShapeItem::setBrush(param.fillColor);
        }
    }
  painter->setPen(pen());
  painter->setBrush(brush());
  painter->drawRect(param.rct);
  if (option->state & QStyle::State_Selected)  highlightSelected(painter,option);
}


itemEllipse::itemEllipse(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  setRect(0,0,100,100);
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
}



void itemEllipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  //	if(resized) prepareGeometryChange();
  if (param.modified)
    {
      param.modified=false;
      if(param.gradient.type!=sgradientParam::NONE)
        {
          QAbstractGraphicsShapeItem::setBrush(buildGradient(param.gradient,rect()));
        }
      else
        {
          QAbstractGraphicsShapeItem::setBrush(param.fillColor);
        }
    }
  painter->setPen(pen());
  painter->setBrush(brush());
  painter->drawEllipse(param.rct);
  if (option->state & QStyle::State_Selected)  highlightSelected(painter,option);
}




itemImage::itemImage(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  setRect(0,0,100,100);
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
}



void itemImage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  QImage tim;
  tim=param.im.scaled(param.rct.width(),param.rct.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
  qreal pad=pen().widthF()/2;
  painter->drawImage(param.rct.adjusted(pad,pad,-pad,-pad),tim);
  painter->setBrush(Qt::NoBrush);
  painter->setPen(pen());
  //painter->drawRect(param.rct);
  if (option->state & QStyle::State_Selected)  highlightSelected(painter,option);
}


itemLine::itemLine(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  //	setRect(0,0,100,100);

  param.line.setPoints(QPoint(0,0),QPoint(100,0));
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
}

QPainterPath itemLine::shape() const

{
  QPainterPath path;
  if (param.line.isNull()) return path;
  path.moveTo(param.line.p1());
  path.lineTo(param.line.p2());
  return qt_graphicsItem_shapeFromPath(path,pen());
}


QRectF itemLine::boundingRect() const
{
  if (pen().widthF() == 0.0)
    {
      const qreal x1 = param.line.p1().x();
      const qreal x2 = param.line.p2().x();
      const qreal y1 = param.line.p1().y();
      const qreal y2 = param.line.p2().y();
      qreal lx = qMin(x1, x2);
      qreal rx = qMax(x1, x2);
      qreal ty = qMin(y1, y2);
      qreal by = qMax(y1, y2);
      return QRectF(lx, ty, rx - lx, by - ty);
    }
  return shape().controlPointRect();
}

void itemLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  //	if (resized ) prepareGeometryChange();
  painter->setPen(pen());
  painter->setBrush(brush());
  painter->drawLine(param.line);
  if (option->state & QStyle::State_Selected)
    //  highlightSelected(painter,option);
    {
      const qreal penWidth = 0; // cosmetic pen
      const QColor fgcolor = option->palette.windowText().color();
      const QColor bgcolor( // ensure good contrast against fgcolor
                            fgcolor.red()   > 127 ? 0 : 255,
                            fgcolor.green() > 127 ? 0 : 255,
                            fgcolor.blue()  > 127 ? 0 : 255);
      painter->setPen(QPen(bgcolor, penWidth, Qt::SolidLine));
      painter->setBrush(Qt::NoBrush);
      painter->strokePath(shape(),QPen(bgcolor, penWidth, Qt::SolidLine));
      painter->setPen(QPen(option->palette.windowText(), 0, Qt::DashLine));
      painter->setBrush(Qt::NoBrush);
      painter->strokePath(shape(),QPen(option->palette.windowText(), 0, Qt::DashLine));
    }
}

itemReplayImage::itemReplayImage(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  setRect(0,0,100,100);
  setFlags(QGraphicsItem::ItemIsSelectable|QGraphicsItem::ItemIsMovable);
}



void itemReplayImage::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  QImage tim;
  if (param.im.isNull())
    {
      QBrush b(Qt::black,Qt::Dense5Pattern);
      painter->setPen(pen());
      painter->setBrush(b);
      painter->drawRect(param.rct);
    }
  else
    {
      tim=param.im.scaled(param.rct.width(),param.rct.height(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
      qreal pad=pen().widthF()/2;
      //    painter->drawImage(param.rct.adjusted(pad,pad,-pad,-pad), param.im, param.im.rect());
      painter->drawImage(param.rct.adjusted(pad,pad,-pad,-pad), tim);
      painter->setBrush(Qt::NoBrush);
      painter->setPen(pen());
    }
  if (option->state & QStyle::State_Selected)  highlightSelected(painter,option);
}


itemBorder::itemBorder(QMenu *cntxtMenu): itemBase(cntxtMenu)
{
  setRect(0,0,100,100);
  setFlags(QGraphicsItem::QGraphicsItem::ItemIgnoresTransformations);
}



void itemBorder::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *)
{
  //	if (resized ) prepareGeometryChange();
  if (param.modified)
    {
      param.modified=false;
      if(param.gradient.type!=sgradientParam::NONE)
        {
          QAbstractGraphicsShapeItem::setBrush(buildGradient(param.gradient,rect()));
        }
      else
        {
          QAbstractGraphicsShapeItem::setBrush(param.fillColor);
        }
    }
  painter->setPen(pen());
  painter->setBrush(Qt::NoBrush);
  painter->drawRect(param.rct);
  if (option->state & QStyle::State_Selected)  highlightSelected(painter,option);
}



