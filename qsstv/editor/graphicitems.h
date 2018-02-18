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
#ifndef GRAPHICITEMS_H
#define GRAPHICITEMS_H
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif
#include "appdefs.h"
#include "gradientdialog.h"



struct sitemParam
	{
		// must be set before returning parameters
		qreal zValue;
		int type;
		QPen pen;
		QBrush brush;
		QPointF position;
// are used dynamically, no need to setup
		QFont font;
		QString txt;
		int rotation;
		double hShear;
		double vShear;
		QImage im;
		sgradientParam gradient;
		QRectF rct;
		bool locked;
		QLineF line;
		bool modified;
		QColor fillColor;
		QMenu *menu;

};

class itemBase : public QAbstractGraphicsShapeItem
{
public:
  enum egraphType {BASE=QGraphicsItem::UserType+1,RECTANGLE,ELLIPSE,IMAGE,LINE,TEXT,REPLAY,SBORDER};
	itemBase(QMenu *cntxtMenu);
	virtual QRectF rect() { return param.rct;}
	virtual void setRect( const QRectF & rectangle )
		{
			param.rct=rectangle;
			param.modified=true;
		}
	virtual void setRect( qreal x, qreal y, qreal width, qreal height )
		{
			param.rct=QRectF(x,y,width,height);
			param.modified=true;
		}
	void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
	void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual QRectF boundingRect() const
		{
   		 return shape().controlPointRect();
    }
  QString getTypeStr();
  void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	void setLocked(bool b) {param.locked=b;}
	void setGradient(sgradientParam pm) { param.modified=true; param.gradient=pm;}
	void setBrush(QColor c);
	void setImage(QImage ima) {param.im=ima;}
	virtual QPainterPath shape() const;
	virtual void setText(const QString &) {};
	virtual void setFont(QFont) {	}
	QString text() const { return param.txt;}
	void load(QDataStream &str); 
	void save(QDataStream &str);
	void setTransform ( int rot,double hs,double vs);
	sitemParam getParam()
		{
			param.zValue=zValue();
			param.type=type();
			param.pen=pen();
			param.brush=brush();
			param.position=pos();
			return param;
		}
	void setParam(sitemParam sp)
		{
			setPen(sp.pen);
			setBrush(sp.fillColor);
			setFont(sp.font);
		// are used dynamically, no need to setup
			param.txt=sp.txt;
			param.rotation=sp.rotation;
			param.hShear=sp.hShear;
			param.vShear=sp.vShear;
			setTransform (param.rotation,param.hShear,param.vShear);
			param.im=sp.im;
			param.gradient=sp.gradient;
			param.rct=sp.rct;
			param.locked=sp.locked;
			param.line=sp.line;
			param.modified=true;
			param.fillColor=sp.fillColor;
			param.menu=sp.menu;
		}
		
	
	
protected:
	void highlightSelected(QPainter *painter,const QStyleOptionGraphicsItem *option);
  sitemParam param;


private:
	enum corner {CUL,CUR,CDL,CDR,HU,HD,VL,VR,NO};
  void setTransform ();

	corner getCorner( QPointF mouse);
	bool grabbed;
	corner grab;
};





class itemText : public itemBase
{
public:
	itemText(QMenu *cntxtMenu);
	~itemText();
	void setText(const QString &t);
	void setFont(QFont f);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
  //QRectF boundingRect() const;
	int type() const {return  TEXT;}
//  void hover(){;}
};


class itemRectangle : public itemBase
{
public:
	itemRectangle(QMenu *cntxtMenu);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
	int type() const {return  RECTANGLE;}
};


class itemEllipse : public itemBase
{
public:
	itemEllipse(QMenu *cntxtMenu);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
//	QRectF boundingRect() const;
	int type() const {return  ELLIPSE;}
};

class itemImage : public itemBase
{
public:
	itemImage(QMenu *cntxtMenu);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
//	QRectF boundingRect() const;
	int type() const {return  IMAGE;}
};

class itemLine : public itemBase
{
public:
	itemLine(QMenu *cntxtMenu);
	void setRect( const QRectF & rectangle )
		{
			prepareGeometryChange();
			param.rct=rectangle;
			param.line.setPoints(param.rct.topLeft(),param.rct.bottomRight());
			param.modified=true;
		}
	void setRect( qreal x, qreal y, qreal width, qreal height )
		{
			prepareGeometryChange();
			param.rct=QRectF(x,y,width,height);
			param.line.setLine(x,y,width,height);
			param.modified=true;
		}
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
	QPainterPath shape() const;
	int type() const {return  LINE;}
};

class itemReplayImage : public itemBase
{
public:
	itemReplayImage(QMenu *cntxtMenu);
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
	int type() const {return  REPLAY;}
};

class itemBorder : public itemBase
{
public:
  itemBorder(QMenu *cntxtMenu);
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,QWidget *widget);
  int type() const {return  SBORDER;}
};

#endif
