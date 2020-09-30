

#ifndef LOGGING_H
#define LOGGING_H
#include <QString>
#include <QFile>
#include <QMutex>
#include <QTime>
#include <QSettings>
#include <QBitArray>
#include "loggingparams.h"

#ifndef QT_NO_DEBUG
#define ENABLELOGGING
#endif

#define errorOut() qDebug()
#ifdef ENABLELOGGING
#define addToLog(x,y) logFilePtr->add(__FILE__,__func__,__LINE__,x,y,false)
#define addToLogDebug(x,y) logFilePtr->add(__FILE__,__func__,__LINE__,x,y,true)
#else
#define addToLog(x,y) {}
#define addToLogDebug(x,y) {}
#endif



class QTextStream;

class logFile
{
  public:
    logFile();
    logFile(QString logname);
    ~logFile();
    bool open(QString logname);
    void add(QString t, short unsigned int posMask, bool debug);
    void add(const char *fileName,const char *functionName, int line, QString t,short unsigned int posMask,bool debug=false);
    void dummyAdd(QString,int) {}
    void addToAux(QString t);
    bool setEnabled(bool e);
    void setLogMask(QBitArray logMask);
    void maskSelect(QWidget *wPtr=0);
    void readSettings();
    void writeSettings();
    void close();
    void reset();
    bool reopen();

  private:
    QString tmp;
    QString tmp2;
    QFile *lf;
    QTextStream *ts;
    QFile *auxFile;
    QTextStream *auxTs;
    bool enabled;
    QMutex mutex;
    QTime timer;
    QBitArray maskBA;
    QString savedLogEntry;
    int logCount;
    int savedPosMask;
    bool deduplicate;
};






#endif
