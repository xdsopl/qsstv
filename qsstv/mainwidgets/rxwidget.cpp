#include "rxwidget.h"
#include "appglobal.h"
#include "dispatcher.h"
#include "dirdialog.h"
#include "rxfunctions.h"
#include "imageviewer.h"
#include "sstvparam.h"
#include "filterparam.h"
#include "mainwindow.h"
#include "soundbase.h"
#include "vumeter.h"
#include "sstvrx.h"
#include "guiconfig.h"
#include "mainwindow.h"
#include "configparams.h"
#include "ftp.h"


rxWidget::rxWidget(QWidget *parent):QWidget(parent),ui(new Ui::rxWidget)
{
  int i;
  ui->setupUi(this);
  rxFunctionsPtr=new rxFunctions();
  ui->syncWidget->setHorizontal(false);
  ui->syncWidget->setLabelText("S");

  ui->vuWidget->setHorizontal(true);
  ui->vuWidget->setLabelText("V");
  imageViewerPtr=ui->imageFrame;


  ui->sstvModeComboBox->addItem("Auto");
  for(i=0;i<NUMSSTVMODES-1;i++)
    {
      ui->sstvModeComboBox->addItem(getSSTVModeNameLong((esstvMode)i));
    }
  foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
      QString text = tr("%1").arg(QString(format));
      ui->defaultImageFormatComboBox->addItem(text);
    }
  connect(&rxFunctionsPtr->sstvRxPtr->syncWideProc,SIGNAL(callReceived(QString)),SLOT(slotNewCall(QString)));
  connect(rxFunctionsPtr->sstvRxPtr,SIGNAL(resetCall()),SLOT(slotResetCall()));
  connect(ui->logPushButton,SIGNAL(clicked()),SLOT(slotLogCall()));
  connect(ui->whoPushButton,SIGNAL(clicked()),SLOT(slotWho()));
  notifyRXIntf = new ftpInterface("RX Notification FTP");

}

rxWidget::~rxWidget()
{
  writeSettings();
  rxFunctionsPtr->terminate();
  delete rxFunctionsPtr;
  delete notifyRXIntf;
}

void rxWidget::init()
{
  splashStr+=QString( "Setting up RX" ).rightJustified(25,' ')+"\n";
  splashPtr->showMessage ( splashStr ,Qt::AlignLeft,Qt::white);

  qApp->processEvents();
  readSettings();
  //  imageViewerPtr->createImage(QSize(320,256),QColor(0,0,128),imageStretch);
  imageViewerPtr->createImage(QSize(320,256),imageBackGroundColor,imageStretch);
  imageViewerPtr->setType(imageViewer::RXIMG);
  setSettingsTab();

  rxFunctionsPtr->init();


  // make connections after initialization
  connect(ui->startToolButton, SIGNAL(clicked()),SLOT(slotStart()));
  connect(ui->stopToolButton, SIGNAL(clicked()),SLOT(slotStop()));
  connect(ui->resyncToolButton,SIGNAL(clicked()),SLOT(slotResync()));
  connect(ui->autoSlantAdjustCheckBox,SIGNAL(clicked()),SLOT(slotGetParams()));
  connect(ui->squelchComboBox,SIGNAL(currentIndexChanged(int)),SLOT(slotGetParams()));
  connect(ui->settingsTableWidget,SIGNAL(currentChanged(int)),this, SLOT(slotTransmissionMode(int)));
  connect(ui->eraseToolButton, SIGNAL(clicked()),SLOT(slotErase()));
  connect(ui->saveToolButton, SIGNAL(clicked()),SLOT(slotSave()));
  if(slowCPU)
    {
      ui->drmFACLabel->hide();
      ui->drmMSCLabel->hide();
      ui->drmMSCWidget->hide();
      ui->drmFACWidget->hide();
    }
  if(slowCPU || lowRes)
    {
      ui->rxNotificationList->hide();
      ui->whoPushButton->hide();
//      ui->whoSpacer->hide();
    }
}

void rxWidget::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("RX");
  useVIS=qSettings.value("useVIS",false).toBool();
  autoSlantAdjust=qSettings.value("autoSlantAdjust",false).toBool();
  autoSave=qSettings.value("autoSave",true).toBool();
  squelch=qSettings.value("squelch",1).toInt();
  //  filterIndex=qSettings.value("filterIndex",0).toInt();
  sstvModeIndexRx=(esstvMode)qSettings.value("sstvModeIndexRx",0).toInt();
  defaultImageFormat=qSettings.value("defaultImageFormat","png").toString();
  minCompletion=qSettings.value("minCompletion",25).toInt();
  setParams();
  qSettings.endGroup();

}

void rxWidget::writeSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("RX");
  getParams();
  qSettings.setValue("useVIS",useVIS);
  qSettings.setValue("autoSlantAdjust",autoSlantAdjust);
  qSettings.setValue("autoSave",autoSave);
  qSettings.setValue("squelch",squelch);
  qSettings.setValue("sstvModeIndexRx",sstvModeIndexRx);
  qSettings.setValue("defaultImageFormat",defaultImageFormat);
  qSettings.setValue("minCompletion",minCompletion);
  qSettings.endGroup();
}

void rxWidget::getParams()
{
  int temp;
  getValue(useVIS,ui->useVISCheckBox);
  getValue(autoSlantAdjust,ui->autoSlantAdjustCheckBox);
  getValue(autoSave,ui->autoSaveCheckBox);
  getIndex(squelch,ui->squelchComboBox);
  getIndex(temp,ui->sstvModeComboBox);
  sstvModeIndexRx=(esstvMode)temp;
  getValue(defaultImageFormat,ui->defaultImageFormatComboBox);
  getValue(minCompletion,ui->completeSpinBox);
}

void rxWidget::setParams()
{
  setValue(useVIS,ui->useVISCheckBox);
  setValue(autoSlantAdjust,ui->autoSlantAdjustCheckBox);
  setValue(autoSave,ui->autoSaveCheckBox);
  setIndex(squelch,ui->squelchComboBox);
  setIndex(sstvModeIndexRx,ui->sstvModeComboBox);
  setValue(defaultImageFormat,ui->defaultImageFormatComboBox);
  setValue(minCompletion,ui->completeSpinBox);
}

void rxWidget::slotGetParams()
{
  getParams();
}


void rxWidget::slotStart()
{
  getParams();
  dispatcherPtr->startRX();
}

void rxWidget::slotStop()
{
  getParams();
  dispatcherPtr->idleAll();
}

void rxWidget::slotResync()
{
  rxFunctionsPtr->restartRX();
}

void rxWidget::slotTransmissionMode(int rxtxMode)
{
  emit modeSwitch(rxtxMode);
}


void rxWidget::changeTransmissionMode(int rxtxMode)
{
  transmissionModeIndex=(etransmissionMode)rxtxMode;
  dispatcherPtr->idleAll();
  setSettingsTab();
  switch(transmissionModeIndex)
    {
    case TRXSSTV:
      mainWindowPtr->setSSTVDRMPushButton(false);
      ui->resyncToolButton->setEnabled(true);
      break;
    case TRXDRM:
      mainWindowPtr->setSSTVDRMPushButton(true);
      ui->resyncToolButton->setEnabled(false);
      break;
    default:
      break;
    }
  dispatcherPtr->startRX();
}

void rxWidget::slotLogCall()
{
  QString call;
  call=ui->callLineEdit->text().toUpper();
  dispatcherPtr->logSSTV(call,false);
}

void rxWidget::slotNewCall(QString call)
{
  ui->callLineEdit->setText(call);
  dispatcherPtr->logSSTV(call,true);
}

void rxWidget::slotResetCall()
{
  ui->callLineEdit->clear();
}

void rxWidget::slotErase()
{
  rxFunctionsPtr->eraseImage();
  imageViewerPtr->createImage(QSize(320,256),imageBackGroundColor,imageStretch);
}


void rxWidget::slotSave()
{
  QDateTime dt(QDateTime::currentDateTime().toUTC()); //this is compatible with QT 4.6
  QString path;
  QString info;
  dirDialog d(this);
  if(transmissionModeIndex==TRXSSTV)
  {
    path=rxSSTVImagesPath;

  }
  else
  {
    QMessageBox::information(this,"Saving image","Not available in DRM mode");
    return;
  }

  info="";
  QString fileName=d.saveFileName(path,"*","png");
  if (fileName==QString::null) return ;
  getImageViewerPtr()->save(fileName,defaultImageFormat,true,false);
  dispatcherPtr->saveImage(fileName,info);
}

void rxWidget::slotWho()
{
  dispatcherPtr->who();
}

void rxWidget::setSettingsTab()
{

  int i;
  if((transmissionModeIndex>=0)&&(transmissionModeIndex<TRXNOMODE))
    {
      for(i=0;i<TRXNOMODE;i++)
        {
          if(i!=transmissionModeIndex) ui->settingsTableWidget->widget(i)->setEnabled(false);
        }
      ui->settingsTableWidget->widget(transmissionModeIndex)->setEnabled(true);
      ui->settingsTableWidget->setCurrentIndex(transmissionModeIndex);
    }
  ui->vuWidget->setMaximum(100);
  ui->vuWidget->setMinimum(0);
  ui->vuWidget->setValue(-15);
  if(transmissionModeIndex==TRXDRM)
    {
      ui->syncWidget->setColors(QColor(0,90,0),QColor(0,190,0),Qt::green);
      ui->syncWidget->setMaximum(25.);
      ui->syncWidget->setMinimum(5.);
      ui->syncWidget->setValue(0.);
      ui->vuWidget->setColors(QColor(255,50,0),Qt::green,Qt::red);
      mainWindowPtr->spectrumFramePtr->displaySettings(true);
    }
  else
    {
      ui->syncWidget->setColors(Qt::red,QColor(255,165,0),Qt::green);
      ui->syncWidget->setMaximum(10.);
      ui->syncWidget->setMinimum(0.);
      ui->syncWidget->setValue(0.);
      ui->vuWidget->setColors(QColor(255,50,0),Qt::green,Qt::red);
      mainWindowPtr->spectrumFramePtr->displaySettings(false);
    }
}

void rxWidget::startRX(bool st)
{
  if(st)
    {
      getParams();
      dispatcherPtr->startRX();
      addToLog("starting rxfunction run",LOGRXMAIN);
    }
  else
    {
      dispatcherPtr->idleAll();
    }
}

void rxWidget::setSSTVStatusText(QString txt)
{
  ui->sstvStatusLineEdit->setText(txt);
}


vuMeter *rxWidget::vMeterPtr()
{
  return ui->vuWidget;
}

vuMeter *rxWidget::sMeterPtr()
{
  return ui->syncWidget;
}

