/**************************************************************************
*   Copyright (C) 2000-2012 by Johan Maes                                 *
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

#include "logging.h"

#include <QDebug>
#include <QTextStream>
#include <QFileInfo>
#include <QStringList>
#include <QDir>
#include "ui_loggingform.h"








/*! class logFile
  \brief utility class to enable logging facilities

  Create an instance of this class giving the basename of the logfile.
  By default the log is disabled. call setEnabled(true) to enable logging


*/
//logFile logfile;


logFile::logFile()
{
#ifdef ENABLELOGGING
  lf=new QFile;
  auxFile=new QFile;
#endif
  logCount=0;
  savedLogEntry="";
  savedPosMask=0;
  mask.set(); //all masks set:this will enable all logfile messages
}

/*!
  creates logfile with name=logname, and opens it for writing
*/

#ifdef ENABLELOGGING
bool logFile::open(QString logname)
{

  lf->setFileName(QDir::homePath()+"/"+logname);
  auxFile->setFileName(QDir::homePath()+"/aux_"+logname);
  return reopen();

}
#else
bool logFile::open(QString ) { return true;}
#endif


/*!
  closes the logfile
*/

logFile::~logFile()
{
 close();
}

void logFile::close()
{
#ifdef ENABLELOGGING
 errorOut() << "closing logfile";
 add("End of logfile",LOGALL);
 add("....,",LOGALL);
 delete ts;
 delete auxTs;
 lf->close();
 auxFile->close();
#endif
}

void logFile::reset()
{
  close();
  reopen();
}

bool logFile::reopen()
{
#ifdef ENABLELOGGING
  setEnabled(false);
  QFileInfo finf(*lf);
  QFileInfo finfaux(*auxFile);
  errorOut() << "opening logfile--: " << finf.absoluteFilePath();
  if(!lf->open(QIODevice::WriteOnly))
    {
     errorOut() << "logfile creation failed";
      return false;
    }
  errorOut() << "opening logfile: " << finfaux.absoluteFilePath();
  if(!auxFile->open(QIODevice::WriteOnly))
    {
      errorOut() << "auxillary file creation failed";
      lf->close();
      return false;
    }
  setEnabled(true);
  ts= new QTextStream( lf );
  auxTs= new QTextStream( auxFile);
  savedLogEntry="";
  logCount=0;
  timer.start();
  *ts<< "Time \tElapsed  \t  Level  \t  Count\t          Info\n";
  ts->flush();
#endif
  return true;
}

/*!
  \brief Writes to the logfile

  The output is flushed after every access.Identical messages are only logged once. The count indicates the number of duplicate messages.
*/

#ifdef ENABLELOGGING
void logFile::add(QString t,short unsigned int posMask)
{
  if(!(posMask==LOGALL)) // always show messages with DBALL
    {
      if (!mask.test(posMask)) return;
    }
  if (!enabled) return;
  mutex.lock();
  if(logCount==0)
    {
      logCount=1;
      savedLogEntry=t;
      timer.restart();
      tmp=QString("%1 ").arg(timer.elapsed(),5);
      tmp2=timer.currentTime().toString("HH:mm:ss:zzz ");
      savedPosMask=posMask;
    }
  if ((t==savedLogEntry) &&(deduplicate)) logCount++;
  else
    {
      if(!deduplicate)
        {
          savedLogEntry=t;
          tmp=QString("%1 ").arg(timer.elapsed(),5);
          tmp2 = timer.currentTime().toString("HH:mm:ss:zzz ");
          savedPosMask=posMask;
        }
      if(savedPosMask==LOGALL)
        {
          *ts << tmp2<< "\t" << tmp << "\tALL     \t" << logCount << "\t" << savedLogEntry <<"\n";
        }
      else
        {
          *ts << tmp2<< "\t" << tmp << "\t" << levelStr[savedPosMask] <<"\t" << logCount << "\t" << savedLogEntry <<"\n";
        }
      tmp=QString("%1 ").arg(timer.elapsed(),5);
      tmp2 = timer.currentTime().toString("HH:mm:ss:zzz ");
      timer.restart();;
      savedLogEntry=t;
      savedPosMask=posMask;
      logCount=1;
    }
  ts->flush();
  lf->flush();
  mutex.unlock();
}
#else
void logFile::add(QString ,short unsigned int) {}
#endif

void logFile::add(const char *fileName,const char *functionName, int line, QString t,short unsigned int posMask)
{
  QString s;
  s=QString(fileName)+":"+QString(functionName)+":"+QString::number(line)+" "+ t;
  add(s,posMask);
}

#ifdef ENABLELOGGING
void logFile::addToAux(QString t)
{
  if (!enabled) return;
  mutex.lock();
  *auxTs << t << "\n";
  auxTs->flush();
  auxFile->flush();
  mutex.unlock();
}
#else
void logFile::addToAux(QString ){}
#endif
/*!
  if enable=true logging wil be performed
  \return previous logging state (true if logging was enabled)
*/

bool logFile::setEnabled(bool enable)
{
  bool t=enabled;
  enabled=enable;
  return t;
}

void logFile::setLogMask(std::bitset<NUMDEBUGLEVELS> logMask)
{
  mask=logMask;
}

void logFile::maskSelect(QWidget *wPtr)
{
  int i,j;
   QDialog lf(wPtr);
   QCheckBox *cb;
 //  QTableWidgetItem *item;
   Ui::loggingForm ui;
   ui.setupUi(&lf);
   ui.maskTableWidget->setRowCount((NUMDEBUGLEVELS+1)/2);
   for(i=0;i<ui.maskTableWidget->rowCount();i++)
     {
       for(j=0;(j<2)&(i*2+j<NUMDEBUGLEVELS);j++)
       {
         cb=new QCheckBox(levelStr[i*2+j]);
         cb->setChecked(mask.test(i*2+j));
         ui.maskTableWidget->setCellWidget(i,j,cb);
       }
     }
   ui.deduplicateCheckBox->setChecked(deduplicate);
   if(lf.exec()==QDialog::Accepted)
     {
       for(i=0;i<ui.maskTableWidget->rowCount();i++)
         {
           for(j=0;(j<2)&(i*2+j<NUMDEBUGLEVELS);j++)
           {
             cb=(QCheckBox *)ui.maskTableWidget->cellWidget(i,j);
             mask.set(i*2+j,cb->isChecked());
           }
         }
       deduplicate=ui.deduplicateCheckBox->isChecked();
      }
}

void logFile::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("logging");
  mask=qSettings.value("mask",1).toULongLong();
  deduplicate=qSettings.value("deduplicate",true).toBool();
  qSettings.endGroup();
}

void logFile::writeSettings()
{
  QSettings qSettings;
  qSettings.beginGroup ("logging");
  qSettings.setValue ( "mask", (qulonglong)mask.to_ulong());
  qSettings.setValue ( "deduplicate", deduplicate);
  qSettings.endGroup();
}


