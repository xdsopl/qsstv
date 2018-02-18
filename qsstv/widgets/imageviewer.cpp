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
#include "imageviewer.h"
#include "appglobal.h"
#include "utils/logging.h"
#include "configparams.h"
#include "dispatcher.h"
#include "dirdialog.h"
#include "extviewer.h"
#include "jp2io.h"
#include <configdialog.h>


#include <QtGui>
#include <QMenu>

#define RATIOSCALE 1.


/**
  \class imageViewer

  The image is stored in it's original format and size.
  All interactions are done on the original image.
  A scaled version is used to display the contents.
*/	


imageViewer::imageViewer(QWidget *parent): QLabel(parent)
{
  addToLog("image creation",LOGIMAG);
  validImage=false;
  setFrameStyle(QFrame::Sunken | QFrame::Panel);
  QBrush b;
  QPalette palette;
  b.setTexture(QPixmap::fromImage(QImage(":/icons/transparency.png")));
  palette.setBrush(QPalette::Active,QPalette::Base,b);
  palette.setBrush(QPalette::Inactive,QPalette::Base,b);
  palette.setBrush(QPalette::Disabled,QPalette::Base,b);
  setPalette(palette);
  setBackgroundRole(QPalette::Base);
  setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  setAspectMode(Qt::IgnoreAspectRatio);
  setBackgroundRole(QPalette::Dark);

  popup=new QMenu (this);
  newAct = new QAction(tr("&New"),this);
  connect(newAct, SIGNAL(triggered()), this, SLOT(slotNew()));
  loadAct = new QAction(tr("&Load"), this);
  connect(loadAct, SIGNAL(triggered()), this, SLOT(slotLoad()));
  toTXAct = new QAction(tr("&To TX"), this);
  connect(toTXAct, SIGNAL(triggered()), this, SLOT(slotToTX()));
  editAct = new QAction(tr("&Edit"), this);
  connect(editAct, SIGNAL(triggered()), this, SLOT(slotEdit()));
  printAct = new QAction(tr("&Print"), this);
  connect(printAct, SIGNAL(triggered()), this, SLOT(slotPrint()));
  uploadAct = new QAction(tr("&Upload to FTP"), this);
  connect(uploadAct, SIGNAL(triggered()), this, SLOT(slotUploadFTP()));
  deleteAct = new QAction(tr("&Delete"), this);
  connect(deleteAct, SIGNAL(triggered()), this, SLOT(slotDelete()));
  viewAct = new QAction(tr("&View"), this);
  connect(viewAct, SIGNAL(triggered()), this, SLOT(slotView()));
  propertiesAct = new QAction(tr("Propert&ies"), this);
  connect(propertiesAct, SIGNAL(triggered()), this, SLOT(slotProperties()));
  zoomInAct = new QAction(tr("Zoom In (&+)"), this);
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(slotZoomIn()));
  zoomOutAct = new QAction(tr("Zoom Out (&-)"), this);
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(slotZoomOut()));
  connect(configDialogPtr,SIGNAL(bgColorChanged()), SLOT(slotBGColorChanged()));
  clickTimer.setSingleShot(true);
  clickTimer.setInterval(40);
  connect(&clickTimer, SIGNAL(timeout()), this, SLOT(slotLeftClick()));

  init(RXIMG);
  activeMovie=false;
  stretch=false;
  //
}

imageViewer::~imageViewer()
{
}

void imageViewer::init(thumbType tp)
{
  setScaledContents(false);
  setAlignment(Qt::AlignCenter);
  setAutoFillBackground(true);
  slotBGColorChanged();
  addToLog(QString("image creation %1").arg(tp),LOGIMAG);
  setType(tp);
  setPixmap(QPixmap());
  clear();
}

bool imageViewer::openImage(QString &filename,QString start,bool ask,bool showMessage,bool emitSignal,bool fromCache)
{
  QImage tempImage;
  QFile fi(filename);
  QFileInfo finf(filename);
  QString cacheFileName;
  jp2IO jp2;
  displayMBoxEvent *stmb=0;
  editorScene ed;
  bool success=false;
  bool cacheHit=false;

  if(activeMovie)
    {
      activeMovie=false;
      qm.stop();
    }
  if (filename.isEmpty()&&!ask) return false;
  if(ask)
    {
      dirDialog dd((QWidget *)this,"Browse");
      filename=dd.openFileName(start,"*");
    }
  if(filename.isEmpty())
    {
      imageFileName="";
      return false;
    }

  if(fromCache)
    {
      cacheFileName=finf.absolutePath()+"/cache/"+finf.baseName()+finf.created().toString()+".png";
      if(tempImage.load(cacheFileName))
        {
          cacheHit=true;
          success=true;
          orgWidth=tempImage.text("orgWidth").toInt();
          orgHeight=tempImage.text("orgHeight").toInt();
        }
    }
  if(!success)
    {
      if(jp2.check(filename))
        {
          tempImage=jp2.decode(filename);
          if(!tempImage.isNull())
            {
              success=true;
            }
        }
      else if(tempImage.load(filename))
        {
          success=true;
        }
      else if(ed.load(fi))
        {
          success=true;
          tempImage=QImage(ed.renderImage(0,0)->copy());
        }
    }
  if(!success)
    {
      if(showMessage)
        {
          stmb= new displayMBoxEvent("Image Loader",QString("Unable to load image:\n%1").arg(filename));
          QApplication::postEvent( dispatcherPtr, stmb );  // Qt will delete it when done
        }
      validImage=false;
      imageFileName="";
      return false;
    }

  if(fromCache)
    {
      sourceImage=QImage();

      if(!cacheHit)
        {
          orgWidth=tempImage.width();
          orgHeight=tempImage.height();
          tempImage=tempImage.scaledToWidth(120, Qt::FastTransformation);
          // save cacheImage for next time
          tempImage.setText("orgWidth",QString::number(orgWidth));
          tempImage.setText("orgHeight",QString::number(orgHeight));
          tempImage.save(cacheFileName,"PNG");
        }
      stretch=true;
      displayedImage=tempImage;
    }
  else
    {
      sourceImage=tempImage.convertToFormat(QImage::Format_ARGB32_Premultiplied);
      orgWidth=tempImage.width();
      orgHeight=tempImage.height();
      displayedImage=sourceImage;
    }
  view=QRect();
  imageFileName=filename;

  QFileInfo finfo(filename);
  if (finfo.suffix().toLower()=="gif")
    {
      //we will try a animated gif
      qm.setFileName(filename);
      if(qm.isValid())
        {
          if(qm.frameCount()>1)
            {
              activeMovie=true;
              setMovie(&qm);
              qm.start();
              displayedImage=QImage();
            }
          else
            {
              displayImage();  // we have a single image gif
            }
        }
    }
  else
    {
      displayImage();
    }
  validImage=true;
  if (emitSignal) emit imageChanged();
  return true;
}

bool imageViewer::openImage(QString &filename,bool showMessage,bool emitSignal,bool fromCache)
{
  return openImage(filename,"",false,showMessage,emitSignal,fromCache);
}

bool imageViewer::openImage(QImage im)
{
  imageFileName="";
  if(!im.isNull())
    {
      validImage=true;
      sourceImage=im;
      displayedImage=im;
      compressedImageData.clear();
      displayImage();
      return true;
    }
  validImage=false;
  return false;
}

bool imageViewer::openImage(QByteArray *ba)
{
  QImage tempImage;
  QBuffer buffer(ba);
  buffer.open(QIODevice::ReadOnly);
  if(tempImage.load(&buffer,NULL))
    {
      return  openImage(tempImage.convertToFormat(QImage::Format_ARGB32_Premultiplied));
    }
  validImage=false;
  return false;
}

void imageViewer::clear()
{
  validImage=false;
  imageFileName.clear();
  sourceImage=QImage();
  displayedImage=QImage();
  compressedImageData.clear();
  view=QRect();
  setPixmap(QPixmap());
  targetWidth=0;
  targetHeight=0;
  templateFileName.clear();
  useTemplate=false;
}

bool imageViewer::hasValidImage()
{
  return validImage;
}

void imageViewer::createImage(QSize sz,QColor fill,bool scale)
{
  clear();
  displayedImage=QImage(sz,QImage::Format_ARGB32_Premultiplied);
  if(!displayedImage.isNull())
    {
      displayedImage.fill(fill.rgb());
      useCompression=false;
    }
  stretch=scale;
  displayImage();
  emit imageChanged();
}

void imageViewer::copy(imageViewer *src)
{
  imageFileName=src->imageFileName;
  ttype=src->ttype;
  openImage(imageFileName,false,false,false);
}



QRgb *imageViewer::getScanLineAddress(int line)
{
  return (QRgb *)displayedImage.scanLine(line);
}



void imageViewer::displayImage()
{
  if(displayedImage.isNull())
    {
      return;
    }
  if (view.isNull()) {
      if(hasScaledContents() || (displayedImage.width()>width()) || (displayedImage.height()>height()) || stretch)
        {
          QPixmap mp;
          mp=QPixmap::fromImage(displayedImage.scaled(width()-2,height()-2,Qt::KeepAspectRatio,Qt::SmoothTransformation));
          setPixmap(QPixmap::fromImage(displayedImage.scaled(width()-2,height()-2,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
          //    qDebug()<< "mp size" << mp.width() << mp.height() << hasScaledContents() << "display width" << width() << height() << imageFileName;
        }
      else
        {
          //    qDebug()<< "xy" << width() << height() << hasScaledContents();
          setPixmap(QPixmap::fromImage(displayedImage));
        }
    }
  else {
      QImage im = displayedImage.copy(view);
      setPixmap(QPixmap::fromImage(im.scaled(width()-2,height()-2,Qt::KeepAspectRatio,Qt::SmoothTransformation)));
    }
}

void imageViewer::zoom(const QPoint centre, int dlevel)
{
  addToLog(QString("centre=%1,%2 dlevel=%3").arg(centre.x()).arg(centre.y()).arg(dlevel),LOGIMAG);
  if (view.isNull()) {
      view=displayedImage.rect();
    }

  while ((dlevel!=0) && (view.width()<=displayedImage.width())) {
      if (dlevel>0) {
          // halve the size of the viewed area
          if ((view.width()>300) && (view.height()>300)) {
              addToLog("zoom in",LOGIMAG);
              view.adjust(+view.width()/4, +view.height()/4, -view.width()/4, -view.height()/4);
            }
          dlevel--;
        }
      else {
          // double the size of the viewed area
          addToLog("zoom out",LOGIMAG);
          view.adjust(-view.width()/2, -view.height()/2, +view.width()/2, +view.height()/2);
          dlevel++;
        }
    }

  if ((view.width()  > displayedImage.width()) ||
      (view.height() > displayedImage.height())
      ) {
      view=displayedImage.rect();
    }
  else {
      view.moveCenter(centre);

      // ensure the view is within the image
      if (view.x()<0) view.moveLeft(0);
      if (view.y()<0) view.moveTop(0);
      if (view.x()+view.width() > displayedImage.width()) view.moveRight(displayedImage.width());
      if (view.y()+view.height() > displayedImage.height()) view.moveBottom(displayedImage.height());
    }
  addToLog(QString("View:%1,%2,%3,%4").arg(view.x()).arg(view.y()).arg(view.width()).arg(view.height()),LOGIMAG);
  displayImage();
}

QPoint imageViewer::mapToImage(const QPoint &pos)
{
  QRect cr = contentsRect();
  cr.adjust(margin(),margin(),-margin(),-margin());
  QRect aligned = QStyle::alignedRect(QApplication::layoutDirection(), QFlag(alignment()), pixmap()->size(), cr);
  QRect inter = aligned.intersected(cr);

  QPoint c = pos;
  c-=inter.topLeft();

  if (view.isNull()) {
      c.setX(displayedImage.width()  * c.x()/inter.width());
      c.setY(displayedImage.height() * c.y()/inter.height());
    }
  else {
      c.setX(view.width()  * c.x()/inter.width());
      c.setY(view.height() * c.y()/inter.height());
    }

  c+=view.topLeft();
  return c;
}

void imageViewer::setType(thumbType tp)
{
  ttype=tp;
  switch(ttype)
    {
    case RXIMG:
    case EXTVIEW:
    case PREVIEW:
    case RXSSTVTHUMB:
      imageFilePath=rxSSTVImagesPath;
    break;
    case RXDRMTHUMB:
      imageFilePath=rxDRMImagesPath;
    break;

    case TXSSTVTHUMB:
      imageFilePath=txSSTVImagesPath;
    break;
    case TXDRMTHUMB:
      imageFilePath=txDRMImagesPath;
    break;
    case TXIMG:
    case TXSTOCKTHUMB:
      imageFilePath=txStockImagesPath;
    break;
    case TEMPLATETHUMB:
      imageFilePath=templatesPath;
    break;

    }
  if((tp==RXSSTVTHUMB) || (tp==RXDRMTHUMB) || (tp==TXSSTVTHUMB) || (tp==TXDRMTHUMB) ||(tp==TXSTOCKTHUMB) ||(tp==TEMPLATETHUMB))
    {
      setScaledContents(false);
      setAlignment(Qt::AlignCenter);
    }
  popup->removeAction(newAct);
  popup->removeAction(loadAct);
  popup->removeAction(toTXAct);
  popup->removeAction(editAct);
  popup->removeAction(printAct);
  popup->removeAction(uploadAct);
  popup->removeAction(deleteAct);
  popup->removeAction(viewAct);
  popup->removeAction(propertiesAct);
  switch(tp)
    {
    case EXTVIEW:
      popup->addAction(zoomInAct);
      popup->addAction(zoomOutAct);
      popup->addAction(propertiesAct);
    break;

    case RXIMG:
      popup->addAction(viewAct);
      popup->addAction(propertiesAct);
    break;

    case TXIMG:
      popup->addAction(newAct);
      popup->addAction(loadAct);
      popup->addAction(editAct);
      popup->addAction(printAct);
      popup->addAction(viewAct);
      popup->addAction(propertiesAct);
    break;

    case PREVIEW:
      popup->addAction(loadAct);
      popup->addAction(toTXAct);
      popup->addAction(viewAct);
      popup->addAction(propertiesAct);
    break;

    case RXSSTVTHUMB:
    case RXDRMTHUMB:
      popup->addAction(uploadAct);
    case TXSSTVTHUMB:
    case TXDRMTHUMB:
      popup->addAction(toTXAct);
      popup->addAction(printAct);
      popup->addAction(deleteAct);
      popup->addAction(viewAct);
      popup->addAction(propertiesAct);
    break;

    case TXSTOCKTHUMB:
    case TEMPLATETHUMB:
      popup->addAction(newAct);
      popup->addAction(loadAct);
      popup->addAction(toTXAct);
      popup->addAction(editAct);
      popup->addAction(printAct);
      popup->addAction(deleteAct);
      popup->addAction(viewAct);
      popup->addAction(propertiesAct);
    break;
    }
  popupEnabled=true;

}

void imageViewer::mousePressEvent( QMouseEvent *e )
{
  if (e->button() == Qt::LeftButton)
    {
      if (e->type() == QEvent::MouseButtonDblClick)
        {
          clickTimer.stop();
          if (ttype==EXTVIEW) {
              if (pixmap()) {
                  QPoint c = mapToImage(e->pos());
                  if (e->modifiers() & Qt::ShiftModifier)
                    zoom(c, -1);
                  else
                    zoom(c, +1);
                }
            }
          else {
              if (hasValidImage()) slotView();
            }
        }
      else if (e->type() == QEvent::MouseButtonPress)
        {
          if (ttype==EXTVIEW) {
              if (pixmap()) {
                  clickPos = mapToImage(e->pos());
                  clickTimer.start();
                }
            }
        }
    }
  else if (e->button() == Qt::RightButton)
    {
      if(popupEnabled)
        {
          if (pixmap()) clickPos = mapToImage(e->pos());
          popup->popup(QCursor::pos());
        }
    }
}

void imageViewer::slotLeftClick()
{
  switch (ttype)
    {
    case EXTVIEW:
      zoom(clickPos, 0);
    break;
    default:
    break;
    }
}

void imageViewer::slotDelete()
{
  if(imageFileName.isEmpty()) return;
  if(QMessageBox::question(this,"Delete file","Do you want to delete the file and\n move it to the trash folder?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
    {
      trash(imageFileName,true);
    }
  imageFileName="";
  emit layoutChanged();
}

void imageViewer::slotEdit()
{
  if(imageFileName.isEmpty())
    {
      slotLoad();
      if (imageFileName.isEmpty()) return;
    }
  callEditorEvent *ce = new callEditorEvent( this,imageFileName );
  QApplication::postEvent(dispatcherPtr, ce );  // Qt will delete it when done
}


void imageViewer::slotLoad()
{
  QString fileNameTmp;
  dirDialog dd((QWidget *)this,"Browse");
  fileNameTmp=dd.openFileName(imageFilePath);
  if(openImage(fileNameTmp,true,false,false))
    {
      imageFileName=fileNameTmp;
      if(ttype==TEMPLATETHUMB)
        {
          templatesChangedEvent *ce = new templatesChangedEvent( );
          QApplication::postEvent(dispatcherPtr, ce );  // Qt will delete it when done
        }
      else if((ttype==TXIMG) ||(ttype==PREVIEW))
        {
          emit imageChanged();
        }

    }
}



void imageViewer::slotNew()
{
  callEditorEvent *ce = new callEditorEvent( this,NULL);
  QApplication::postEvent(dispatcherPtr, ce );  // Qt will delete it when done
}


void imageViewer::slotPrint()
{
}


void imageViewer::slotUploadFTP()
{
  QString remoteDir;
  switch (ttype) {
    case RXSSTVTHUMB:
      remoteDir = ftpRemoteSSTVDirectory;
    break;
    case RXDRMTHUMB:
      remoteDir = ftpRemoteDRMDirectory;
    break;
    default:
    break;
    }
  if (!remoteDir.isEmpty())
    dispatcherPtr->uploadToRXServer(remoteDir, imageFileName);
}


void imageViewer::slotView()
{
  extViewer vm(this);
  vm.setup(imageFileName);
  vm.exec();
}


void imageViewer::slotBGColorChanged()
{
  QPalette mpalette;
  mpalette.setColor(QPalette::Window,backGroundColor);
  setBackgroundRole(QPalette::Window);
  mpalette.setColor(QPalette::WindowText, Qt::yellow);
  setPalette(mpalette);
}

void imageViewer::slotProperties()
{
  QFileInfo fi(imageFileName);
  if(fi.exists())
    {
      QMessageBox::information(this,"Image Properties",
                               "File: " + imageFileName
                               + "\n File size:     " + QString::number(fi.size())
                               + "\n Image width:   " + QString::number(orgWidth)
                               + "\n Image height:  " + QString::number(orgHeight)
                               + "\n Last Modified: " + fi.lastModified().toString()
                               ,QMessageBox::Ok);
    }
  else
    {
      QMessageBox::information(this,"Image Properties",
                               " Image width:   " + QString::number(orgWidth)
                               + "\n Image height:  " + QString::number(orgHeight)
                               ,QMessageBox::Ok);
    }

}

void imageViewer::slotZoomIn()
{
zoom(clickPos,+1);
}

void imageViewer::slotZoomOut()
{
zoom(clickPos,-1);
}


void imageViewer::slotToTX()
{
  moveToTxEvent *mt=0;
  mt=new moveToTxEvent(imageFileName);
  QApplication::postEvent(dispatcherPtr, mt); // Qt will delete it when done
}


void imageViewer::save(QString fileName,QString fmt,bool convertRGB, bool source)
{
  QImage im;
  if(source)
    {
      if(sourceImage.isNull()) return;
    }
  else
    {
      if(displayedImage.isNull()) return;
    }
  if(!convertRGB)
    {
      if(source) im=sourceImage;
      else im=displayedImage;
    }
  else
    {
      if(source) im=sourceImage.convertToFormat(QImage::Format_RGB32);
      else im=displayedImage.convertToFormat(QImage::Format_RGB32);;
    }
  im.save(fileName,fmt.toUpper().toLatin1().data());
}

bool imageViewer::copyToBuffer(QByteArray *ba)
{
  QImage im;
  jp2IO jp2;
  int fileSize;
  if(displayedImage.isNull())
    {
      return false;
    }
  if (compressedImageData.isEmpty())
    {
      compressedImageData=jp2.encode(displayedImage.convertToFormat(QImage::Format_RGB32),im,fileSize,compressionRatio);
    }
  *ba = compressedImageData;
  if (compressedImageData.isEmpty())
    {
      return false;
    }
  return true;
}


uint  imageViewer:: setSizeRatio(int sizeRatio,bool usesCompression)
{
  if(!usesCompression) return sourceImage.byteCount();
  compressionRatio=sizeRatio;
  return applyTemplate();
}

bool imageViewer::reload()
{
  return openImage(imageFileName ,true,false,false);
}


void imageViewer::setParam(QString templateFn,bool usesTemplate,int width,int height)
{
  targetWidth=width;
  targetHeight=height;
  templateFileName=templateFn;
  useTemplate=usesTemplate;
  applyTemplate();
  displayImage();
}

void imageViewer::setAspectMode(Qt::AspectRatioMode mode)
{
  aspectRatioMode = mode;
}

int imageViewer::applyTemplate()
{
  QImage *resultImage;
  jp2IO jp2;
  QImage overlayedImage;
  int tWidth=targetWidth,tHeight=targetHeight;
  int compRatio;

  if(sourceImage.isNull()) return 0;
  QFile fi(templateFileName);
  if(ttype!=TXIMG) return 0;
  editorScene tscene(0);
  resultImage=&sourceImage;
  if(transmissionModeIndex==TRXDRM)
    {
      useCompression=true;
     }
  else
    {
      useCompression=false;
    }

  compRatio=compressionRatio;

  if (tWidth==0 && tHeight==0 && useCompression &&
     (sourceImage.width()>1000 || sourceImage.height()>1000)
      ) {
      // if this is going DRM, and its not already a small image
      // and the size slider is set for smaller sizes
      // we can pre-scale the image to smaller dimensions to
      // improve the compression speed
      // Changing the ratio makes the slider work consistently
      // About every 50 is where the compression ratio stops
         // making much difference to the size. It's also the point where
         // you are losing significant detail due to compression anyway.
         addToLog(QString("CompressionRatio=%1").arg(compressionRatio),LOGIMAG);
         if (compressionRatio > 150) {

          tWidth =sourceImage.width() / 4;
          tHeight=sourceImage.height() / 4;
          compRatio = ((compRatio - 151) * 3) + 45;
        }
      else if (compressionRatio > 100) {
          tWidth =sourceImage.width() / 3;
          tHeight=sourceImage.height() / 3;
          compRatio -= 73;
        }
      else if (compressionRatio > 50) {
          tWidth =sourceImage.width() / 2;
          tHeight=sourceImage.height() / 2;
          compRatio -= 38;
        }
    }

  if((fi.fileName().isEmpty())  || (!useTemplate))
    {
      addToLog(QString("No Template, targetW,H=%1,%2").arg(targetWidth).arg(targetHeight), LOGIMAG);
      if(tWidth!=0 && tHeight!=0)
        {
          QImage scaledImage = QImage(sourceImage
                                      .scaled(tWidth,
                                              tHeight,
                                              aspectRatioMode,
                                              Qt::SmoothTransformation
                                              )
                                      );
          // Crop to intended dimensions at the centre of the image
          displayedImage = QImage(scaledImage
                                  .copy((scaledImage.width()-tWidth)/2,
                                        (scaledImage.height()-tHeight)/2,
                                        tWidth,
                                        tHeight
                                        )
                                  );
        }
      else
        {
          displayedImage=sourceImage;
        }
      resultImage=&displayedImage;
    }
  else
    {
      addToLog("apply temlate",LOGIMAG);
      //  sconvert cnv;

      if((!fi.fileName().isEmpty())  && (useTemplate))
        {
          tscene.load(fi);
          tscene.addConversion('c',toCall,true);
          tscene.addConversion('r',rsv);
          tscene.addConversion('o',toOperator);
          tscene.addConversion('t',QDateTime::currentDateTime().toUTC().toString("hh:mm"));
          tscene.addConversion('d',QDateTime::currentDateTime().toUTC().toString("yyyy/MM/dd"));
          tscene.addConversion('m',myCallsign);
          tscene.addConversion('q',myQth);
          tscene.addConversion('l',myLocator);
          tscene.addConversion('n',myLastname);
          tscene.addConversion('f',myFirstname);
          tscene.addConversion('v',qsstvVersion);
          tscene.addConversion('x',comment1);
          tscene.addConversion('y',comment2);
          tscene.addConversion('z',comment3);

          addToLog(QString("Template size=%1,%2, SourceW,H=%3,%4 TargetW,H=%5,%6").
                   arg(tscene.width()).arg(tscene.height()).
                   arg(sourceImage.width()).arg(sourceImage.height()).
                   arg(tWidth).arg(tHeight),
                   LOGIMAG);
          if(tWidth!=0 && tHeight!=0)
            {
              QImage scaledImage = QImage(sourceImage
                                          .scaled(tWidth,
                                                  tHeight,
                                                  aspectRatioMode,
                                                  Qt::SmoothTransformation
                                                  )
                                          );
              overlayedImage= QImage(scaledImage
                                     .copy((scaledImage.width()-tWidth)/2,
                                           (scaledImage.height()-tHeight)/2,
                                           tWidth,
                                           tHeight
                                           )
                                     );
              tscene.overlay(&overlayedImage);
            }
          else
            {
              tscene.overlay(&sourceImage);
            }
          resultImage=tscene.getImagePtr();
          addToLog(QString("resultImageW,H=%1,%2").arg(resultImage->width()).arg(resultImage->height()), LOGIMAG);
        }
    }
  if(useCompression)
    {
      bool useOriginal=false;
      int fileSize;
      compressedImageData=jp2.encode(resultImage->convertToFormat(QImage::Format_RGB32),compressedImage,fileSize,compRatio);
      if (!useTemplate && !imageFileName.isEmpty())
        {
          QFile original(imageFileName);
          if (original.open(QIODevice::ReadOnly) && (original.size() < fileSize))
            {
              useOriginal=true;
              compressedFilename=imageFileName;
              addToLog(QString("Using original image data (%1 bytes)").arg(original.size()),LOGIMAG);
              statusBarPtr->showMessage("Using original Image (smaller)");
              compressedImageData = original.readAll();
              displayedImage.load(imageFileName);
            }
        }
      if (!useOriginal)
        {
          displayedImage=compressedImage;
          int pos;
          pos=imageFileName.lastIndexOf(".",-1);
          compressedFilename=imageFileName.left(pos)+".jp2";
          statusBarPtr->showMessage("");
          addToLog(QString("Image Compressed to %1 bytes").arg(compressedImageData.size()),LOGIMAG);
          addToLog(QString("displayedImageW,H=%1,%2  compRatio=%3").
                   arg(displayedImage.width()).arg(displayedImage.height()).
                   arg(compRatio),
                   LOGIMAG);
        }
      addToLog(QString("compressed size %1").arg(compressedImageData.size()),LOGIMAG);
      return fileSize;
    }
  else
    {
      displayedImage=resultImage->convertToFormat(QImage::Format_RGB32);
      compressedImageData.clear();
      return displayedImage.byteCount();
    }
}


void imageViewer::resizeEvent(QResizeEvent *)
{
  displayImage();
}


