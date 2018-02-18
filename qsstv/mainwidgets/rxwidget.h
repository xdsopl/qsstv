#ifndef RXWIDGET_H
#define RXWIDGET_H

#include <QWidget>
#include <QFrame>
#include "spectrumwidget.h"
#include "ui_rxwidget.h"

class rxFunctions;
class imageViewer;
class spectrumWidget;
class vuMeter;


namespace Ui {
class rxWidget;
}

class rxWidget : public QWidget
{
  Q_OBJECT
  
public:
  explicit rxWidget(QWidget *parent = 0);
  ~rxWidget();
  void readSettings();
  void writeSettings();
  void startRX(bool st);
  rxFunctions *functionsPtr() {return rxFunctionsPtr;}
  imageViewer *getImageViewerPtr(){ return imageViewerPtr;}
  //  spectrumWidget *fftDisplayPtr() ;
  vuMeter *vMeterPtr();
  vuMeter *sMeterPtr();
  drmConstellationFrame *mscWdg() {return ui->drmMSCWidget;}
  drmConstellationFrame *facWdg() {return ui->drmFACWidget;}
  void setDRMStatusText(QString txt)
  {
    ui->drmStatusLineEdit->clear();
    ui->drmStatusLineEdit->appendPlainText(txt);
  }
  void setDRMNotifyText(QString txt) {
    //ui->rxNotificationList->clear();
    ui->rxNotificationList->setPlainText(txt);
  }
  void appendDRMNotifyText(QString txt) {
    ui->rxNotificationList->appendPlainText(txt);
  }
  //  drmPSDFrame *psdWdg() {return ui->drmPSDWidget;}
  drmStatusFrame *statusWdg() {return ui->drmStatusWidget;}
  //  int getFilterIndex();
  void init();
  void setSSTVStatusText(QString txt);
  void setSettingsTab();
  void changeTransmissionMode(int rxtxMode);

private slots:
  void slotStart();
  void slotStop();
  void slotResync();
  void slotGetParams();
  void slotTransmissionMode(int rxtxMode);
  void slotNewCall(QString);
  void slotResetCall();
  void slotLogCall();
  void slotErase();
  void slotSave();
  void slotWho();

signals:
  void modeSwitch(int);


private:
  Ui::rxWidget *ui;
  rxFunctions *rxFunctionsPtr;
  imageViewer *imageViewerPtr;
  void getParams();
  void setParams();
};
#endif // RXWIDGET_H
