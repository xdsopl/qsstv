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
/*!
  The dispatcher is the central system that routes all messages from the different threads.
It also starts, stops and synchronizes the threads.

*/
#include "dispatcher.h"
#include "appglobal.h"
#include "configparams.h"
#include "rxwidget.h"
#include "txwidget.h"
#include "gallerywidget.h"
#include "widgets/spectrumwidget.h"
#include "widgets/vumeter.h"
#include "rxfunctions.h"
#include "mainwindow.h"
#include "utils/ftp.h"
#include "rigcontrol.h"
#include "logbook/logbook.h"
#include "dirdialog.h"
#include <QSettings>
#include <QMessageBox>



/*!
creates dispatcher instance
 */

dispatcher::dispatcher()
{
  mbox=NULL;
  progressFTP=NULL;
  lastFileName.clear();
  prTimerIndex=0;
}

/*!
delete dispatcher instance
 */

dispatcher::~dispatcher()
{
}

void dispatcher::init()
{
  editorActive=false;
  infoTextPtr=new textDisplay(mainWindowPtr);
  mainWindowPtr->spectrumFramePtr->init(RXSTRIPE,1,BASESAMPLERATE/SUBSAMPLINGFACTOR);
  infoTextPtr->hide();
  setOnlineStatus(true, onlineStatusText);
  connect(notifyRXIntf, SIGNAL(notification(QString)), this, SLOT(slotRXNotification(QString)));
  connect(notifyTXIntf, SIGNAL(notification(QString)), this, SLOT(slotTXNotification(QString)));
  connect(hybridTxIntf, SIGNAL(commandsDone(bool)), this, SLOT(slotHybridTxDone(bool)));

}

void dispatcher::setOnlineStatus(bool online, QString info)
{
  txWidgetPtr->functionsPtr()->setOnlineStatus(online, info);

  // TODO: move to rxWidget->functions->?
  if (!online && enableFTP)
    {
      ftpInterface rxftp("RX Image Cleanup");
      rxftp.setupConnection(ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,ftpRemoteSSTVDirectory);
      rxftp.uploadToRXServer(""); // mark SSTV offline
      rxftp.changePath(ftpRemoteDRMDirectory);
      rxftp.uploadToRXServer(""); // mark DSSTV/DRM offline
    }
}

void dispatcher::who()
{
  txWidgetPtr->functionsPtr()->who();
}


/*!
  All communication between the threads are passed via this eventhandler.
*/

void dispatcher::customEvent( QEvent * e )
{
  dispatchEventType type;
  ftpInterface * iPtr;
  QString fn;
  type=(dispatchEventType)e->type();
  addToLog(((baseEvent*)e)->description,LOGDISPATCH);
  switch(type)
    {
    case displayFFT:
      addToLog("dispatcher: displayFFT",LOGDISPATCH);
      mainWindowPtr->spectrumFramePtr->realFFT(((displayFFTEvent*)e)->data());
      rxWidgetPtr->vMeterPtr()->setValue(soundIOPtr->getVolumeDb());
      //      addToLog(QString::number(soundIOPtr->getVolumeDb()),LOGALL);

    break;
    case displaySync:
      // addToLog("dispatcher: displaySync",LOGDISPATCH);
      uint s;
      ((displaySyncEvent*)e)->getInfo(s);
      rxWidgetPtr->sMeterPtr()->setValue((double)s);
    break;
    case rxSSTVStatus:
      rxWidgetPtr->setSSTVStatusText(((rxSSTVStatusEvent*)e)->getStr());
    break;

    case startImageRX:
      addToLog("dispatcher: clearing RxImage",LOGDISPATCH);
      //      rxWidgetPtr->getImageViewerPtr()->createImage( ((startImageRXEvent*)e)->getSize(),QColor(0,0,128),imageStretch);
      rxWidgetPtr->getImageViewerPtr()->createImage( ((startImageRXEvent*)e)->getSize(),imageBackGroundColor,imageStretch);
      lastCallsign="";
    break;

    case lineDisplay:
      {
        rxWidgetPtr->getImageViewerPtr()->displayImage();
      }
    break;
    case endSSTVImageRX:
      if(autoSave)
        {
          addToLog("dispatcher:endImage savingRxImage",LOGDISPATCH);
          saveRxSSTVImage(((endImageSSTVRXEvent*)e)->getModeName());
        }
    break;


    case rxDRMStatus:
      rxWidgetPtr->setDRMStatusText(((rxDRMStatusEvent*)e)->getStr());

    break;

    case statusBarMsg:
      statusBarPtr->showMessage(((statusBarMsgEvent*)e)->getStr());
    break;
    case callEditor:
      if(editorActive) break;
      editorActive=true;
      ed=new editor();
      ed->show();
      iv=((callEditorEvent*)e)->getImageViewer();
      addToLog (QString(" callEditorEvent imageViewPtr: %1").arg(QString::number((ulong)iv,16)),LOGDISPATCH);
      addToLog(QString("editor: filename %1").arg(((callEditorEvent*)e)->getFilename()),LOGDISPATCH);
      ed->openFile(((callEditorEvent*)e)->getFilename());
    break;

    case rxDRMNotify:
      rxWidgetPtr->setDRMNotifyText(((rxDRMNotifyEvent*)e)->getStr());
    break;
    case rxDRMNotifyAppend:
      rxWidgetPtr->appendDRMNotifyText(((rxDRMNotifyAppendEvent*)e)->getStr());
    break;
    case txDRMNotify:
      txWidgetPtr->setDRMNotifyText(((txDRMNotifyEvent*)e)->getStr());
    break;
    case txDRMNotifyAppend:
      txWidgetPtr->appendDRMNotifyText(((txDRMNotifyAppendEvent*)e)->getStr());
    break;
    case txPrepareComplete:
      txWidgetPtr->prepareTxComplete(((txPrepareCompleteEvent *)e)->ok());
    break;

    case editorFinished:
      if(!editorActive) break;
      if(((editorFinishedEvent*)e)->isOK())
        {
          addToLog (QString(" editorFinishedEvent imageViewPtr: %1").arg(QString::number((ulong)iv,16)),LOGDISPATCH);
          iv->reload();
        }
      editorActive=false;
      delete ed;
    break;

    case templatesChanged:
      galleryWidgetPtr->changedMatrix(imageViewer::TEMPLATETHUMB);
      txWidgetPtr->setupTemplatesComboBox();
    break;

    case progressTX:
      txTimeCounter=0;
      addToLog(QString("dispatcher: progress duration=%1").arg(((progressTXEvent*)e)->getInfo()),LOGDISPATCH);
      prTimerIndex=startTimer(((progressTXEvent*)e)->getInfo()*10); // time in seconds -> times 1000 for msec,divide by 100 for progress
    break;

    case stoppingTX:
      addToLog("dispatcher: endTXImage",LOGDISPATCH);
    break;

    case endImageTX:
      //addToLog("dispatcher: endTXImage",LOGDISPATCH);
      while(soundIOPtr->isPlaying())
        {
          qApp->processEvents();
        }
      addToLog("dispatcher: endTXImage",LOGDISPATCH);
      startRX();
    break;

    case displayDRMInfo:
      if(!slowCPU)
        {
          rxWidgetPtr->mscWdg()->setConstellation(MSC);
          rxWidgetPtr->facWdg()->setConstellation(FAC);
        }
      rxWidgetPtr->statusWdg()->setStatus();
    break;

    case displayDRMStat:
      DSPFLOAT s1;
      ((displayDRMStatEvent*)e)->getInfo(s1);
      rxWidgetPtr->sMeterPtr()->setValue(s1);
    break;

    case loadRXImage:
      {
        QString fn=((loadRXImageEvent *)e)->getFilename();
        rxWidgetPtr->getImageViewerPtr()->openImage(fn,false,false,false);
      }
    break;
    case moveToTx:
      {
        txWidgetPtr->setImage(((moveToTxEvent *)e)->getFilename());
      }
    break;
    case saveDRMImage:
      {
        QString info;
        ((saveDRMImageEvent*)e)->getFilename(fn);
        ((saveDRMImageEvent*)e)->getInfo(info);
        if(!rxWidgetPtr->getImageViewerPtr()->openImage(fn,false,false,false))
          {
            if(mbox==NULL) delete mbox;
            mbox = new QMessageBox(mainWindowPtr);
            mbox->setWindowTitle("Received file");
            mbox->setText(QString("Saved file %1").arg(fn));
            mbox->show();
            QTimer::singleShot(4000, mbox, SLOT(hide()));
            break;
          }
        saveImage(fn, info);
      }
    break;

    case prepareFix:
      addToLog("prepareFix",LOGDISPATCH);
      startDRMFIXTx( ((prepareFixEvent*)e)->getData());
    break;
    case displayText:
      infoTextPtr->clear();
      infoTextPtr->setWindowTitle(QString("Received from %1").arg(drmCallsign));
      infoTextPtr->append(((displayTextEvent*)e)->getStr());
      infoTextPtr->show();
    break;

    case displayMBox:
      if(mbox==NULL) delete mbox;
      mbox = new QMessageBox(mainWindowPtr);
      mbox->setWindowTitle(((displayMBoxEvent*)e)->getTitle());
      mbox->setText(((displayMBoxEvent*)e)->getStr());
      mbox->show();
      QTimer::singleShot(4000, mbox, SLOT(hide()));
    break;

    case displayProgressFTP:
      {
        if(((displayProgressFTPEvent*)e)->getTotal()==0)
          {
            delete progressFTP;
            progressFTP=NULL;
            break;
          }
        if(progressFTP==NULL)
          {
            progressFTP=new QProgressDialog("FTP Transfer","Cancel",0,0,mainWindowPtr);
          }
        progressFTP->show();
        progressFTP->setMaximum(((displayProgressFTPEvent*)e)->getTotal());
        progressFTP->setValue(((displayProgressFTPEvent*)e)->getBytes());
      }
    break;
    case  ftpSetup:

      iPtr=((ftpSetupEvent*)e)->getFtpIntfPtr();
      if(iPtr==notifyRXIntf)
        {
          notifyRXDone=DFTPWAITING;
        }
      else if(iPtr==hybridTxIntf)
        {
          hybridTxDone=DFTPWAITING;
        }
      else if(iPtr==notifyTXIntf)
        {
          notifyTxDone=DFTPWAITING;
        }
        iPtr->setupConnection(
            ((ftpSetupEvent*)e)->getHost(),
            ((ftpSetupEvent*)e)->getPort(),
            ((ftpSetupEvent*)e)->getUser(),
            ((ftpSetupEvent*)e)->getPassword(),
            ((ftpSetupEvent*)e)->getDir());
    break;

    case ftpUploadFile:

      ((ftpUploadFileEvent*)e)->getFtpIntfPtr()->uploadFile(
            ((ftpUploadFileEvent*)e)->getSrcFn(),
            ((ftpUploadFileEvent*)e)->getDstFn(),
            ((ftpUploadFileEvent*)e)->getReconnect()
            );
    break;


    case notifyAction:
      notifyRXIntf->mremove(((notifyActionEvent*)e)->getToRemove());
      notifyRXIntf->uploadData(((notifyActionEvent*)e)->getMsg().toLatin1(), ((notifyActionEvent*)e)->getFilename());
    break;

    case notifyCheck:


      iPtr=((ftpSetupEvent*)e)->getFtpIntfPtr();

      iPtr->startNotifyCheck(
            ((notifyCheckEvent*)e)->getFilename(),
            ((notifyCheckEvent*)e)->getInterval(),
            ((notifyCheckEvent*)e)->getRepeats(),
            ((notifyCheckEvent*)e)->getToRemove()
            );
    break;
    default:
      addToLog(QString("unsupported event: %1").arg(((baseEvent*)e)->description), LOGALL);
    break;
    }
  ((baseEvent *)e)->setDone();
}


void dispatcher::idleAll()
{
  if(prTimerIndex>=0)
    {
      killTimer(prTimerIndex);
      prTimerIndex=-1;
      txWidgetPtr->setProgress(0);
    }
  rigControllerPtr->activatePTT(false);
  rxWidgetPtr->functionsPtr()->stopAndWait();
  txWidgetPtr->functionsPtr()->stopAndWait();
}


void dispatcher::startRX()
{
  idleAll();
  soundIOPtr->startCapture();
  rxWidgetPtr->functionsPtr()->startRX();
}

void dispatcher::startTX(txFunctions::etxState state)
{
  idleAll();
  rigControllerPtr->activatePTT(true);
  soundIOPtr->startPlayback();
  txWidgetPtr->functionsPtr()->startTX(state);
}

void dispatcher::prepareTX(txFunctions::etxState state)
{
  txWidgetPtr->functionsPtr()->prepareTX(state);
}

void dispatcher::startDRMFIXTx(QByteArray ba)
{
  if(!txWidgetPtr->functionsPtr()->prepareFIX(ba)) return;
  startTX(txFunctions::TXSENDDRMFIX);
}

void dispatcher::startDRMTxBinary()
{
  //TODO: this whole thing should probably live in txWidget::slotBinary
  QFileInfo finfo;
  int txtime=0;
  QMessageBox mbox(mainWindowPtr);
  QPushButton *sendButton;

  dirDialog d((QWidget *)mainWindowPtr,"Binary File");
  QString filename=d.openFileName("","*");
  if(filename.isEmpty()) return;
  if(!txWidgetPtr->functionsPtr()->prepareBinary(filename)) return;

  txtime = txWidgetPtr->functionsPtr()->calcTxTime(true,0);
  finfo.setFile(filename);

  if (txtime > (3*60))
    mbox.setIcon(QMessageBox::Warning);

  mbox.setWindowTitle("TX Binary File");
  mbox.setText(QString("'%1'").arg(filename));
  mbox.setInformativeText(QString("The file is %1Kb and will take %2 seconds on air to send").
                          arg(finfo.size()/1000.0,0,'f',0).arg(txtime));

  if (useHybrid)
    sendButton = mbox.addButton(tr("Upload ready to transmit"), QMessageBox::AcceptRole);
  else
    sendButton = mbox.addButton(tr("Start Transmitting"), QMessageBox::AcceptRole);
  mbox.setStandardButtons(QMessageBox::Cancel);

  mbox.exec();
  if (mbox.clickedButton() == sendButton) {
      txWidgetPtr->functionsPtr()->prepareTX(txFunctions::TXPREPAREDRMBINARY);
    }
}



void dispatcher::logSSTV(QString call,bool fromFSKID)
{
  if(lastFileName.isEmpty())
    {
      return;
    }
  if(fromFSKID)
    {
      QDateTime dt(QDateTime::currentDateTime().toUTC());
      int diffsec=saveTimeStamp.secsTo(dt);
      if(diffsec<2)
        {
          logBookPtr->logQSO(call,"SSTV",lastFileName);
        }
      lastCallsign=call;
    }
  else
    {
      logBookPtr->logQSO(call,"SSTV","");
    }

}


void dispatcher::saveRxSSTVImage(QString shortModeName)
{
  QString info,s,fileName;
  int m;
  QDateTime dt(QDateTime::currentDateTime().toUTC()); //this is compatible with QT 4.6
  dt.setTimeSpec(Qt::UTC);
  if (shortModeName.isEmpty())
    {
      lastFileName.clear();
      return;
    }
  if(!autoSave)
    {
      lastFileName=shortModeName;
    }
  else
    {
      fileName=QString("%1/%2_%3.%4").arg(rxSSTVImagesPath).arg(shortModeName).arg(dt.toString("yyyyMMdd_HHmmss")).arg(defaultImageFormat);
      addToLog(QString("dispatcher: saveRxImage():%1 ").arg(fileName),LOGDISPATCH);
      rxWidgetPtr->getImageViewerPtr()->save(fileName,defaultImageFormat,true,false);

      info="";
      m=0;
      while (m<=NUMSSTVMODES && shortModeName!=SSTVTable[m].shortName) m++;
      if (m<=NUMSSTVMODES)
        info += SSTVTable[m].name;
      else
        info += shortModeName;

      if (!lastCallsign.isEmpty())
        info += " de "+lastCallsign;

      saveImage(fileName, info);
      lastFileName=QString("%1_%2.%3").arg(shortModeName).arg(dt.toString("yyyyMMdd_HHmmss")).arg(defaultImageFormat);
      saveTimeStamp= dt;
    }
}

void dispatcher::saveImage(QString fileName, QString infotext)
{
  QFileInfo info(fileName);
  QString fn="/tmp/"+info.baseName()+"."+ftpDefaultImageFormat;
  galleryWidgetPtr->putRxImage(fileName);
  txWidgetPtr->setPreviewWidget(fileName);
  if(enableFTP)
    {
      QImage *imp = rxWidgetPtr->getImageViewerPtr()->getImagePtr();
      QImage im;

      if (imp && imp->width()) {
          // the original source image is available, possibly in higher quality
          // than the displayed image. Only for DRM Images.
          im = QImage(imp->convertToFormat(QImage::Format_RGB32));
        }
      else {
          // Uses the displayed image in whatever quality is displayed.
          // Applies to SSTV images.
          rxWidgetPtr->getImageViewerPtr()->save(fn,ftpDefaultImageFormat,true,false);
          im = QImage(fn);
        }
      QString text, remoteDir;
      QPainter p;
      double freq=0;
      int pixelSize, height, width;

      rigControllerPtr->getFrequency(freq);

      text = QString("%1 UTC %2 kHz ").
          arg(QDateTime::currentDateTime().toUTC().toString("hh:mm ddd MMM d, yyyy")).
          arg(freq/1000,1,'f',0);

      if (transmissionModeIndex==TRXSSTV) {
          remoteDir = ftpRemoteSSTVDirectory;
        }
      else {
          remoteDir = ftpRemoteDRMDirectory;
        }

      if (!infotext.isEmpty()) text += " "+infotext;

      // Limit uploaded size
      if ((im.width() > 960) || (im.height() > 768)) {
          im = im.scaled(960,768, Qt::KeepAspectRatio);
        }

      // Stamp text over the top left of the image
      // and keep it the same portion, unless the
      // font would be unreadable
      QFont font("Arial");
      pixelSize = 9 * im.width()/320;
      if (pixelSize<8) pixelSize=8;
      font.setPixelSize(pixelSize);
      QFontMetrics fontm(font);

      width = fontm.width(text) + 6;
      height= fontm.height() + 2;

      p.begin(&im);
      p.setPen(Qt::black);
      p.fillRect(0,0,width,height, Qt::black);
      p.setPen(Qt::white);
      p.setBrush(Qt::white);
      p.setFont(font);
      p.drawText(2,height-fontm.descent()-1, text);

      im.save(fn, ftpDefaultImageFormat.toUpper().toLatin1().data());
      p.end();

      uploadToRXServer(remoteDir, fn);
      QFile::remove(fn);
    }
}

void dispatcher::uploadToRXServer(QString remoteDir, QString fn)
{
  displayMBoxEvent *stmb=0;
  eftpError ftpResult;

  ftpInterface ftpIntf("Save RX Image");

  ftpIntf.setupConnection(ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,remoteDir);

  ftpResult=ftpIntf.uploadToRXServer(fn);
  switch(ftpResult)
    {
    case FTPOK:
    break;
    case FTPERROR:
      stmb= new displayMBoxEvent("FTP Error",QString("Host: %1: %2").arg(ftpRemoteHost).arg(ftpIntf.getLastError()));
    break;
    case FTPNAMEERROR:
      stmb= new displayMBoxEvent("FTP Error",QString("Host: %1, Error in filename").arg(ftpRemoteHost));
    break;
    case FTPCANCELED:
      stmb= new displayMBoxEvent("FTP Error",QString("Connection to %1 Canceled").arg(ftpRemoteHost));
    break;
    case FTPTIMEOUT:
      stmb= new displayMBoxEvent("FTP Error",QString("Connection to %1 timed out").arg(ftpRemoteHost));
    break;
    default:
      stmb= new displayMBoxEvent("FTP Error",QString("Unknown FTP error: %1").arg(ftpResult));
    break;
    }
  if(ftpResult!=FTPOK)
    {
      QApplication::postEvent( dispatcherPtr, stmb );  // Qt will delete it when done
    }
}



void dispatcher::timerEvent(QTimerEvent *event)
{
  if(event->timerId()==prTimerIndex)
    {
      txWidgetPtr->setProgress(++txTimeCounter);
      if(txTimeCounter>=100)
        {
          if(prTimerIndex>=0)
            {
              killTimer(prTimerIndex);
              prTimerIndex=-1;
              txWidgetPtr->setProgress(0);
            }
        }
      txWidgetPtr->setProgress(txTimeCounter);
    }
}

void dispatcher::slotRXNotification(QString info)
{
  if (info != "")
    {
      rxWidgetPtr->appendDRMNotifyText(info);
    }
}

void dispatcher::slotTXNotification(QString info)
{
  if (info != "")
    {
      txWidgetPtr->appendDRMNotifyText(info);
    }
}


void dispatcher::slotHybridTxDone(bool error)
{
  if(error)
    {
      hybridTxDone=DFTPERROR;
    }
  else
    {
      hybridTxDone=DFTPOK;
    }

}


