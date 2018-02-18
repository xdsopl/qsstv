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
#ifndef DISPATCHER_H
#define DISPATCHER_H
#include "dispatchevents.h"
#include "appglobal.h"
#include <QByteArray>
#include "textdisplay.h"
#include "txfunctions.h"
class editor;
class imageViewer;
#include <QProgressDialog>

enum eftpResult {DFTPWAITING,DFTPOK,DFTPERROR};

/**
@author Johan Maes
*/
class dispatcher : public QObject
{
  Q_OBJECT

public:

  dispatcher();
  ~dispatcher();
  void init();
  void idleAll();
  void startRX();
  void startTX(txFunctions::etxState state);
  void prepareTX(txFunctions::etxState state);
  void readSettings();
  void writeSettings();
  void customEvent( QEvent * e );
  void startDRMFIXTx(QByteArray ba);
  void startDRMTxBinary();
  //  void startDRMHybridTx(QString fn);
  //  void startDRMHybridText(QString txt);
  //  void sendSweepTone(double duration,double lowerFreq,double upperFreq);
  void saveImage(QString fileName, QString infotext);
  void uploadToRXServer(QString remoteDir, QString fn);
  void logSSTV(QString call, bool fromFSKID);
  void setOnlineStatus(bool online, QString info="");
  void who();
  eftpResult notifyRXDone;
  eftpResult hybridTxDone;
  eftpResult notifyTxDone;


private slots:
  void slotRXNotification(QString info);
  void slotTXNotification(QString);
//  void slotWhoResult();
  void slotHybridTxDone(bool error);


private:
  void saveRxSSTVImage(QString shortModeName);
  void timerEvent(QTimerEvent *event);
  bool editorActive;
  editor *ed;
  imageViewer *iv;
  int txTimeCounter;
  int prTimerIndex;
  int logTimerIndex;
  textDisplay *infoTextPtr;
  QMessageBox *mbox;
  QProgressDialog *progressFTP;
  QString lastFileName;
  QString lastCallsign;
  QDateTime saveTimeStamp;
};
#endif

