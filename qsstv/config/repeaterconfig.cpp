#include "repeaterconfig.h"
#include "ui_repeaterconfig.h"


bool repeaterEnable;
int repeaterImageInterval;
esstvMode repeaterTxMode;
QString repeaterImage1;
QString repeaterImage2;
QString repeaterImage3;
QString repeaterImage4;
QString repeaterAcknowledge;
QString repeaterTemplate;
QString idleTemplate;

repeaterConfig::repeaterConfig(QWidget *parent) : baseConfig(parent),  ui(new Ui::repeaterConfig)
{
  ui->setupUi(this);
}

repeaterConfig::~repeaterConfig()
{
  delete ui;
}

void repeaterConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("REPEATER");
  repeaterImageInterval=qSettings.value("repeaterImageInterval",10).toInt();
  repeaterEnable=qSettings.value("repeaterEnable",false).toBool();
  repeaterTxMode=(esstvMode)qSettings.value("repeaterTxMode",0).toInt();
  repeaterImage1=qSettings.value("repeaterImage1","").toString();
  repeaterImage2=qSettings.value("repeaterImage2","").toString();
  repeaterImage3=qSettings.value("repeaterImage3","").toString();
  repeaterImage4=qSettings.value("repeaterImage4","").toString();
  repeaterAcknowledge=qSettings.value("repeaterAcknowledge","").toString();
  repeaterTemplate=qSettings.value("repeaterTemplate","").toString();
  idleTemplate=qSettings.value("idleTemplate","").toString();
  qSettings.endGroup();
  setParams();
}

void repeaterConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("REPEATER");
  qSettings.setValue("repeaterImageInterval",repeaterImageInterval);
  qSettings.setValue("repeaterEnable",repeaterEnable);
  qSettings.setValue("repeaterTxMode",repeaterTxMode);
  qSettings.setValue("repeaterImage1",repeaterImage1);
  qSettings.setValue("repeaterImage2",repeaterImage2);
  qSettings.setValue("repeaterImage3",repeaterImage3);
  qSettings.setValue("repeaterImage4",repeaterImage4);
  qSettings.setValue("repeaterAcknowledge",repeaterAcknowledge);
  qSettings.setValue("repeaterTemplate",repeaterTemplate);
  qSettings.setValue("idleTemplate",idleTemplate);
  qSettings.endGroup();
}

void repeaterConfig::getParams()
{
  int temp;
  bool repeaterEnableCopy=repeaterEnable;
  int repeaterImageIntervalCopy=repeaterImageInterval;
  esstvMode repeaterTxModeCopy=repeaterTxMode;
  QString repeaterImage1Copy=repeaterImage1;
  QString repeaterImage2Copy=repeaterImage2;
  QString repeaterImage3Copy=repeaterImage3;
  QString repeaterImage4Copy=repeaterImage4;
  QString repeaterAcknowledgeCopy=repeaterAcknowledge;
  QString repeaterTemplateCopy=repeaterTemplate;
  QString idleTemplateCopy=idleTemplate;
  getValue(repeaterImageInterval,ui->imageIntervalSpinBox);
  getValue(repeaterEnable,ui->repeaterEnableCheckBox);
  getIndex(temp,ui->repeaterTxModeComboBox);
  repeaterTxMode=(esstvMode)temp;
  getValue(repeaterImage1,ui->repeaterImage1LineEdit);
  getValue(repeaterImage2,ui->repeaterImage2LineEdit);
  getValue(repeaterImage3,ui->repeaterImage3LineEdit);
  getValue(repeaterImage4,ui->repeaterImage4LineEdit);
  getValue(idleTemplate,ui->idleTemplateLineEdit);
  getValue(repeaterTemplate,ui->repeaterTemplateLineEdit);
  changed=false;
  if(  repeaterEnableCopy!=repeaterEnable
       || repeaterImageIntervalCopy!=repeaterImageInterval
       || repeaterTxModeCopy!=repeaterTxMode
       || repeaterImage1Copy!=repeaterImage1
       || repeaterImage2Copy!=repeaterImage2
       || repeaterImage3Copy!=repeaterImage3
       || repeaterImage4Copy!=repeaterImage4
       || repeaterAcknowledgeCopy!=repeaterAcknowledge
       || repeaterTemplateCopy!=repeaterTemplate
       || idleTemplateCopy!=idleTemplate)
    changed=true;
}

void repeaterConfig::setParams()
{;
  setValue(repeaterImageInterval,ui->imageIntervalSpinBox);
  setValue(repeaterEnable,ui->repeaterEnableCheckBox);
  setIndex(repeaterTxMode,ui->repeaterTxModeComboBox);
  setValue(repeaterImage1,ui->repeaterImage1LineEdit);
  setValue(repeaterImage2,ui->repeaterImage2LineEdit);
  setValue(repeaterImage3,ui->repeaterImage3LineEdit);
  setValue(repeaterImage4,ui->repeaterImage4LineEdit);
  setValue(idleTemplate,ui->idleTemplateLineEdit);
  setValue(repeaterTemplate,ui->repeaterTemplateLineEdit);
}
