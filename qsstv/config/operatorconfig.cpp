#include "operatorconfig.h"
#include "ui_operatorconfig.h"

QString myCallsign;
QString myQth;
QString myLocator;
QString myLastname;
QString myFirstname;
QString lastReceivedCall;
bool    onlineStatusEnabled;
QString onlineStatusText;

operatorConfig::operatorConfig(QWidget *parent) :  baseConfig(parent), ui(new Ui::operatorConfig)
{
  ui->setupUi(this);
}

operatorConfig::~operatorConfig()
{
  delete ui;
}

void operatorConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("PERSONAL");
  myCallsign=qSettings.value("callsign",QString("NOCALL")).toString();
  myQth=qSettings.value("qth",QString("NOWHERE")).toString();
  myLastname=qSettings.value("lastname",QString("NONAME")).toString();
  myFirstname=qSettings.value("firstname",QString("NOFIRSTNAME")).toString();
  myLocator=qSettings.value("locator",QString("NOLOCATOR")).toString();
  onlineStatusEnabled=qSettings.value("onlinestatusenabled",true).toBool();
  onlineStatusText=qSettings.value("onlinestatustext",QString("")).toString();
  qSettings.endGroup();
  setParams();
}

void operatorConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("PERSONAL");
  qSettings.setValue("callsign",myCallsign);
  qSettings.setValue("qth",myQth);
  qSettings.setValue("locator",myLocator);
  qSettings.setValue("lastname",myLastname);
  qSettings.setValue("firstname",myFirstname);
  qSettings.setValue("onlinestatusenabled",onlineStatusEnabled);
  qSettings.setValue("onlinestatustext",onlineStatusText);
  qSettings.endGroup();
}

void operatorConfig::getParams()
{
  QString myCallsignCopy=myCallsign;
  QString myQthCopy=myQth;
  QString myLocatorCopy= myLocator;
  QString myLastnameCopy=myLastname;
  QString myFirstnameCopy=myFirstname;
  QString onlineStatusTextCopy=onlineStatusText;
  bool    onlineStatusEnabledCopy=onlineStatusEnabled;

  getValue(myCallsign,ui->callsignLineEdit);
  getValue(myLastname,ui->lastnameLineEdit);
  getValue(myFirstname,ui->firstnameLineEdit);
  getValue(myQth,ui->qthLineEdit);
  getValue(myLocator,ui->locatorLineEdit);
  getValue(onlineStatusText,ui->onlineStatusText);
  getValue(onlineStatusEnabled,ui->onlineStatusCheckbox);

  changed=false;
  if( myCallsignCopy!=myCallsign
      || myQthCopy!=myQth
      || myLocatorCopy!= myLocator
      || myLastnameCopy!=myLastname
      || myFirstnameCopy!=myFirstname
      || onlineStatusEnabledCopy!=onlineStatusEnabled
      || onlineStatusTextCopy!=onlineStatusText)
    changed=true;
}

void operatorConfig::setParams()
{
  setValue(myCallsign,ui->callsignLineEdit);
  setValue(myLastname,ui->lastnameLineEdit);
  setValue(myFirstname,ui->firstnameLineEdit);
  setValue(myQth,ui->qthLineEdit);
  setValue(myLocator,ui->locatorLineEdit);
  setValue(onlineStatusEnabled,ui->onlineStatusCheckbox);
  setValue(onlineStatusText,ui->onlineStatusText);
}
