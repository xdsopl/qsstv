#ifndef TXWIDGET_H
#define TXWIDGET_H


#include "imageviewer.h"
#include "sstvparam.h"
#include "txfunctions.h"
#include "drmtransmitter.h"
#include "ui_txwidget.h"

#include <QWidget>




class drmTransmitter;
namespace Ui {
class txWidget;
}

class txWidget : public QWidget
{
  Q_OBJECT

public:
  explicit txWidget(QWidget *parent = 0);
  ~txWidget();
  void init();
  void startTX(bool st, bool check=true);
  void prepareTx();
  void prepareTxComplete(bool ok);
  void writeSettings();
  void readSettings();
  imageViewer *getImagePtr();
  void repeat(QImage *im,esstvMode sm);
  void setImage(QImage *ima);
  void setImage(QString fn);
  void setProgress(uint prg);
  void setupTemplatesComboBox();
  void setPreviewWidget(QString fn);
  void setSettingsTab();
  txFunctions *functionsPtr() {return txFunctionsPtr;}
  imageViewer *getImageViewerPtr(){ return imageViewerPtr;}
  QString getPreviewFilename();
  void txTestPattern(etpSelect sel);

  void setDRMNotifyText(QString txt) {      
      //ui->txNotificationList->clear();                 
      ui->txNotificationList->setPlainText(txt);
      }
  void appendDRMNotifyText(QString txt) {      
      ui->txNotificationList->appendPlainText(txt);
      }

  //  bool prepareHybrid(QString fn);
  bool prepareText(QString txt);
  void copyProfile(drmTxParams d);
  void setProfileNames();
  void reloadProfiles();
  void changeTransmissionMode(int rxtxMode);


  //  void test();
  //  void sendFIX();
  void sendBSR();
  void sendWfText();
  void sendWFID();
  void sendCWID();

public slots:
  void slotGetTXParams();
  void slotGetParams();
  void slotStart();
  void slotUpload();
  void slotStop();
//  void slotDisplayStatusMessage(QString);

  void slotGenerateSignal();
  void slotSweepSignal();
  void slotGenerateRepeaterTone();
  void slotEdit();
  //  void slotReplay();
  void slotRepeaterTimer();
  void slotFileOpen();

  void slotSnapshot();
  void slotSize(int v);
  void slotSizeApply();
  void slotTransmissionMode(int rxtxMode);
  void slotProfileChanged(int );
  void slotImageChanged();
  void slotModeChanged(int);
  void slotResizeChanged(int);
  void slotBinary();
  void slotHybridToggled();

signals:
  void modeSwitch(int);

private:
  Ui::txWidget *ui;
  txFunctions *txFunctionsPtr;
  void initView();
  void setParams();
  void sendHybrid(QString fn);
  editor *ed;
  QTimer *repeaterTimer;
  int repeaterIndex;
  QImage origImage;
  QImage resultImage;
  void applyTemplate();
  void updateTxTime();
  imageViewer *imageViewerPtr;
  etransmissionMode currentTXMode;
  int sizeRatio;
  bool sizeRatioChanged;
  int drmProfileIdx;
  QString previewFilename;
  int doTx;

  void startTxImage();
};

#endif // TXWIDGET_H















