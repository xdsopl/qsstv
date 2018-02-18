#include "hybridnotifyconfig.h"
#include "ui_hybridnotifyconfig.h"
#include "ftp.h"

#include <QMessageBox>

bool enableHybridNotify;
bool enableHybridNotifySnoop;
int hybridNotifyPort;
QString hybridNotifyRemoteHost;
QString hybridNotifyRemoteDir;
QString hybridNotifyLogin;
QString hybridNotifyPassword;
QString hybridNotifyDir;


hybridNotifyConfig::hybridNotifyConfig(QWidget *parent) :baseConfig(parent), ui(new Ui::hybridNotifyConfig)
{
  ui->setupUi(this);
//  connect(testFTPPushButton,SIGNAL(clicked()),SLOT(slotTestFTPPushButton()));
  connect(ui->testNotifyPushButton,SIGNAL(clicked()),SLOT(slotTestNotifyPushButton()));
}

hybridNotifyConfig::~hybridNotifyConfig()
{
  delete ui;
}

void hybridNotifyConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("HYBRID_NOTIFY");
  enableHybridNotify=qSettings.value("enableHybridNotify",true).toBool();
  enableHybridNotifySnoop=qSettings.value("enableHybridNotifySnoop",true).toBool();
  hybridNotifyPort=qSettings.value("hybridNotifyPort",21).toInt();
  hybridNotifyRemoteHost=qSettings.value("hybridNotifyRemoteHost","").toString();
  hybridNotifyRemoteDir=qSettings.value("hybridNotifyRemoteDir","").toString();
  hybridNotifyLogin=qSettings.value("hybridNotifyLogin","").toString();
  hybridNotifyPassword=qSettings.value("hybridNotifyPassword","").toString();
  hybridNotifyDir=qSettings.value("hybridNotifyDirectory","RxOkNotifications1").toString();
  
  qSettings.endGroup();
  setParams();
}

void hybridNotifyConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("HYBRID_NOTIFY");
  qSettings.setValue("enableHybridNotify",enableHybridNotify);
  qSettings.setValue("enableHybridNotifySnoop",enableHybridNotifySnoop);
  qSettings.setValue("hybridNotifyPort",hybridNotifyPort);
  qSettings.setValue("hybridNotifyRemoteHost",hybridNotifyRemoteHost);
  qSettings.setValue("hybridNotifyRemoteDir",hybridNotifyRemoteDir);
  qSettings.setValue("hybridNotifyLogin",hybridNotifyLogin);
  qSettings.setValue("hybridNotifyPassword",hybridNotifyPassword);
  qSettings.setValue("hybridNotifyDir",hybridNotifyDir);
  qSettings.endGroup();
}

void hybridNotifyConfig::getParams()
{
  bool enableHybridNotifyCopy=enableHybridNotify;
  bool enableHybridNotifySnoopCopy=enableHybridNotifySnoop;
  int hybridNotifyPortCopy=hybridNotifyPort;
  QString hybridNotifyRemoteHostCopy=hybridNotifyRemoteHost;
  QString hybridNotifyRemoteDirCopy=hybridNotifyRemoteDir;
  QString hybridNotifyDirCopy=hybridNotifyDir;
  QString hybridNotifyLoginCopy=hybridNotifyLogin;
  QString hybridNotifyPasswordCopy=hybridNotifyPassword;


  getValue(enableHybridNotify,ui->enableHybridNotifyCheckBox);
  getValue(enableHybridNotifySnoop,ui->enableHybridNotifySnoopCheckBox);
  getValue(hybridNotifyPort,ui->hybridNotifyPortSpinBox);
  getValue(hybridNotifyRemoteHost,ui->hybridNotifyRemoteHostLineEdit);
  getValue(hybridNotifyRemoteDir,ui->hybridNotifyRemoteDirLineEdit);
  getValue(hybridNotifyLogin,ui->hybridNotifyLoginLineEdit);
  getValue(hybridNotifyPassword,ui->hybridNotifyPasswordLineEdit);
  getValue(hybridNotifyDir,ui->hybridNotifyDirLineEdit);
  changed=false;
  if(  enableHybridNotifyCopy!=enableHybridNotify
       || enableHybridNotifySnoopCopy!=enableHybridNotifySnoop
       || hybridNotifyPortCopy!=hybridNotifyPort
       || hybridNotifyRemoteHostCopy!=hybridNotifyRemoteHost
       || hybridNotifyRemoteDirCopy!=hybridNotifyRemoteDir
       || hybridNotifyDirCopy!=hybridNotifyDir
       || hybridNotifyLoginCopy!=hybridNotifyLogin
       || hybridNotifyPasswordCopy!=hybridNotifyPassword)
    changed=true;




}

void hybridNotifyConfig::setParams()
{
  setValue(enableHybridNotify,ui->enableHybridNotifyCheckBox);
  setValue(enableHybridNotifySnoop,ui->enableHybridNotifySnoopCheckBox);
  setValue(hybridNotifyPort,ui->hybridNotifyPortSpinBox);
  setValue(hybridNotifyRemoteHost,ui->hybridNotifyRemoteHostLineEdit);
  setValue(hybridNotifyRemoteDir,ui->hybridNotifyRemoteDirLineEdit);
  setValue(hybridNotifyLogin,ui->hybridNotifyLoginLineEdit);
  setValue(hybridNotifyPassword,ui->hybridNotifyPasswordLineEdit);
  setValue(hybridNotifyDir,ui->hybridNotifyDirLineEdit);
}


void hybridNotifyConfig::slotTestNotifyPushButton()
{
  QString msg;
  ftpInterface fInt("TestNotifyConnection");
  ui->testNotifyPushButton->setDisabled(true);
  getParams();
  fInt.setupConnection(hybridNotifyRemoteHost,hybridNotifyPort,
          hybridNotifyLogin,hybridNotifyPassword,hybridNotifyRemoteDir+"/"+hybridNotifyDir);
  msg=fInt.execFTPTest();
  ui->testNotifyPushButton->setDisabled(false);
  
  QMessageBox::information(this,"Test Connection",msg);
}
