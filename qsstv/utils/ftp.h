/***************************************************************************
 *   Copyright (C) 2004 by Johan Maes                                      *
 *   on4qz@telenet.be                                                      *
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
#ifndef FTPINTERFACE_H
#define FTPINTERFACE_H

#include "qglobal.h"

# if(QT_VERSION > QT_VERSION_CHECK(5, 0, 0))
#include "qftp.h"
# else
#include <QFtp>
#endif

#include <QObject>
#include <QFile>
#include <QTimer>

enum eftpError {FTPOK,FTPERROR,FTPNAMEERROR,FTPCANCELED,FTPTIMEOUT};
class ftpInterface: public QObject
{
  Q_OBJECT
public:
  ftpInterface(QString id);
  ~ftpInterface();
  void setupConnection(QString tHost,int tPort,QString tUser,QString tPasswd,QString tDirectory);
  void changePath( const QString &newPath );
  eftpError uploadFile(QString fileName, QString fixFilename, bool reconnect);
  eftpError uploadData(const QByteArray & data, QString fixFilename);
  eftpError downloadFile(QString sourceFileName,QString destinationFilename);
  eftpError wait(int timeout = 0);
  eftpError remove(QString path);
  eftpError mremove(QString path);
  eftpError getListing(QString path);
  eftpError startNotifyCheck(QString fn, int interval, int repeats, bool rm);
  QList <QUrlInfo> getListingResults();
  void clearListingResults();
  void closeWhenDone(bool can=true) { canCloseWhenDone=can; }
  void hideProgress(bool hide=true) { displayProgress=!hide; }
  
  bool isUnconnected();
  bool isLoggedIn();
  bool isBusy();
  void dumpState();
  eftpError uploadToRXServer(QString fn);
  QString getLastError() {return ftp->errorString();}
  QString execFTPTest();
  void init();

signals:
  void notification(QString info);
  void listingComplete();
  void commandsDone(bool error);

private slots:
  void ftp_commandStarted(int);
  void ftp_commandFinished(int,bool);
  void ftp_done(bool);
  void ftp_stateChanged(int);
  void ftp_listInfo(const QUrlInfo &);
  void ftp_rawCommandReply(int, const QString &);
  void slotAbort();
  void slotProgress(qint64 total, qint64 bytes);
  void notifyTick();
  void slotTimeout();
  void slotDisconnect();
    
private:
  void connectToHost();
  eftpError doConnect();
  void destroy();

  QFtp *ftp;
  QFile *sourceFn;
  bool ftpDone;
  bool aborting;
  QString host;
  QString user;
  QString passwd;
  QString directory;
  int port;
  bool ftpCommandSuccess;
  QTimer timeoutTimer;
  bool   timeoutExpired;
  QString name;

  QList <int> mremove_listids;
  QList <QUrlInfo> listingResults;
  QList <QString>  notifyList;
  QTimer notifyTimer;
  QTimer disconnectTimer;
  QString notifyMask;
  int notifyTicks;
  int notifyRepeats;
  bool notifyRemove;
  int notifyId;
  bool connectPending;
  bool canCloseWhenDone;
  bool displayProgress;
};

//extern ftpInterface *ftpIntf;

#endif
