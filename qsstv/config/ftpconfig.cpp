#include "ftpconfig.h"
#include "ui_ftpconfig.h"
#include "ftp.h"

#include <QImageWriter>
#include <QMessageBox>


bool enableFTP;
int ftpPort;
QString ftpRemoteHost;
QString ftpRemoteSSTVDirectory;
QString ftpRemoteDRMDirectory;
QString ftpLogin;
QString ftpPassword;
QString ftpDefaultImageFormat;
eftpSaveFormat ftpSaveFormat;
int ftpNumImages;

ftpConfig::ftpConfig(QWidget *parent) :  baseConfig (parent),  ui(new Ui::ftpConfig)
{
  ui->setupUi(this);
  foreach (QByteArray format, QImageWriter::supportedImageFormats())
    {
      QString text = tr("%1").arg(QString(format));
      ui->ftpDefaultImageFormatComboBox->addItem(text);
    }
  connect(ui->testFTPPushButton,SIGNAL(clicked()),SLOT(slotTestFTPPushButton()));
}

ftpConfig::~ftpConfig()
{
  delete ui;
}

void ftpConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("FTPCONFIG");
  enableFTP=qSettings.value("enableFTP",false).toBool();
  ftpPort=qSettings.value("ftpPort",21).toInt();
  ftpRemoteHost=qSettings.value("ftpRemoteHost","").toString();
  ftpRemoteSSTVDirectory=qSettings.value("ftpRemoteSSTVDirectory","").toString();
  ftpRemoteDRMDirectory=qSettings.value("ftpRemoteDRMDirectory","").toString();
  ftpLogin=qSettings.value("ftpLogin","").toString();
  ftpPassword=qSettings.value("ftpPassword","").toString();
  ftpDefaultImageFormat=qSettings.value("ftpDefaultImageFormat","png").toString();
  ftpSaveFormat=(eftpSaveFormat)qSettings.value("ftpSaveFormat",0).toInt();
  ftpNumImages=qSettings.value("ftpNumImages",30).toInt();
  qSettings.endGroup();
  setParams();
}

void ftpConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("FTPCONFIG");
  qSettings.setValue("enableFTP",enableFTP);
  qSettings.setValue("ftpPort",ftpPort);
  qSettings.setValue("ftpRemoteHost",ftpRemoteHost);
  qSettings.setValue("ftpRemoteSSTVDirectory",ftpRemoteSSTVDirectory);
  qSettings.setValue("ftpRemoteDRMDirectory",ftpRemoteDRMDirectory);
  qSettings.setValue("ftpLogin",ftpLogin);
  qSettings.setValue("ftpPassword",ftpPassword);
  qSettings.setValue("ftpDefaultImageFormat",ftpDefaultImageFormat);
  qSettings.setValue("ftpSaveFormat",(int)ftpSaveFormat);
  qSettings.setValue("ftpNumImages",ftpNumImages);
  qSettings.endGroup();
}

void ftpConfig::getParams()
{
  bool enableFTPCopy=enableFTP;
  int ftpPortCopy=ftpPort;
  QString ftpRemoteHostCopy=ftpRemoteHost;
  QString ftpRemoteSSTVDirectoryCopy=ftpRemoteSSTVDirectory;
  QString ftpRemoteDRMDirectoryCopy=ftpRemoteDRMDirectory;
  QString ftpLoginCopy=ftpLogin;
  QString ftpPasswordCopy=ftpPassword;
  QString ftpDefaultImageFormatCopy=ftpDefaultImageFormat;
  eftpSaveFormat ftpSaveFormatCopy=ftpSaveFormat;


  getValue(enableFTP,ui->enableFTPCheckBox);
  getValue(ftpPort,ui->ftpPortSpinBox);
  getValue(ftpRemoteHost,ui->remoteHostLineEdit);
  getValue(ftpNumImages,ui->ftpNumImagesSpinBox);
  getValue(ftpRemoteSSTVDirectory,ui->remoteSSTVDirectoryLineEdit);
  getValue(ftpRemoteDRMDirectory,ui->remoteDRMDirectoryLineEdit);
  getValue(ftpLogin,ui->ftpLoginLineEdit);
  getValue(ftpPassword,ui->ftpPasswordLineEdit);
  getValue(ftpDefaultImageFormat,ui->ftpDefaultImageFormatComboBox);
  if(ui->imageRadioButton->isChecked())
    {
      ftpSaveFormat=FTPIM;
    }
  else
    {
      ftpSaveFormat=FTPFILE;
    }
  changed=false;
  if(enableFTPCopy!=enableFTP
     || ftpPortCopy!=ftpPort
     || ftpRemoteHostCopy!=ftpRemoteHost
     || ftpRemoteSSTVDirectoryCopy!=ftpRemoteSSTVDirectory
     || ftpRemoteDRMDirectoryCopy!=ftpRemoteDRMDirectory
     || ftpLoginCopy!=ftpLogin
     || ftpPasswordCopy!=ftpPassword
     || ftpDefaultImageFormatCopy!=ftpDefaultImageFormat
     || ftpSaveFormatCopy!=ftpSaveFormat)
    changed=true;
}

void ftpConfig::setParams()
{
  setValue(enableFTP,ui->enableFTPCheckBox);
  setValue(ftpPort,ui->ftpPortSpinBox);
  setValue(ftpRemoteHost,ui->remoteHostLineEdit);
  setValue(ftpNumImages,ui->ftpNumImagesSpinBox);
  setValue(ftpRemoteSSTVDirectory,ui->remoteSSTVDirectoryLineEdit);
  setValue(ftpRemoteDRMDirectory,ui->remoteDRMDirectoryLineEdit);
  setValue(ftpLogin,ui->ftpLoginLineEdit);
  setValue(ftpPassword,ui->ftpPasswordLineEdit);
  if(ftpSaveFormat==FTPIM)
    {
      ui->imageRadioButton->setChecked(true);
    }
  else
    {
      ui->filenameRadioButton->setChecked(true);
    }
  setValue(ftpDefaultImageFormat,ui->ftpDefaultImageFormatComboBox);
}


void ftpConfig::slotTestFTPPushButton()
{
  QString r1,r2;
  QApplication::setOverrideCursor(Qt::WaitCursor);
  ftpInterface fInt("TestUploadConnection");
  ui->testFTPPushButton->setDisabled(true);
  getParams();
  fInt.setupConnection(ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,ftpRemoteSSTVDirectory);
  r1=fInt.execFTPTest();
  fInt.init();
  fInt.setupConnection(ftpRemoteHost,ftpPort,ftpLogin,ftpPassword,ftpRemoteDRMDirectory);
  r2=fInt.execFTPTest();
  QApplication::restoreOverrideCursor();
  ui->testFTPPushButton->setDisabled(false);
  QMessageBox::information(this,"Testing Connection","",QString("SSTV: %1\nDRM: %2").arg(r1).arg(r2));
}

