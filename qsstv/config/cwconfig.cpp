#include "cwconfig.h"
#include "ui_cwconfig.h"

QString cwText;
int cwTone;
int cwWPM;
//bool enableCW;

cwConfig::cwConfig(QWidget *parent) :  baseConfig(parent), ui(new Ui::cwConfig)
{
  ui->setupUi(this);
}

cwConfig::~cwConfig()
{
  delete ui;
}

void cwConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("CW");
  cwText=qSettings.value("cwtext","").toString();
  cwTone=qSettings.value("cwtone",800).toInt();
  cwWPM=qSettings.value("cwWPM",12).toInt();
//  enableCW=qSettings.value("enableCW",false).toBool();
  qSettings.endGroup();
  setParams();
}

void cwConfig::writeSettings()
{
  QSettings qSettings;
  getParams();

  qSettings.beginGroup("CW");
  qSettings.setValue("cwtext",cwText);
  qSettings.setValue("cwtone",cwTone);
  qSettings.setValue("cwWPM",cwWPM);
//  qSettings.setValue("enableCW",enableCW);
  qSettings.endGroup();
}

void cwConfig::getParams()
{
  QString cwTextCopy=cwText;
  int cwToneCopy=cwTone;
  int cwWPMCopy=cwWPM;
  getValue(cwText,ui->cwTextLineEdit);
  getValue(cwTone,ui->cwToneSpinBox);
  getValue(cwWPM,ui->cwWPMSpinBox);
//  getValue(enableCW,ui->enableCWCheckBox);
  changed=false;
  if(cwTextCopy!=cwText
     || cwToneCopy!=cwTone
     || cwWPMCopy!=cwWPM)
    changed=true;
}

void cwConfig::setParams()
{
  setValue(cwText,ui->cwTextLineEdit);
  setValue(cwTone,ui->cwToneSpinBox);
  setValue(cwWPM,ui->cwWPMSpinBox);
//  setValue(enableCW,ui->enableCWCheckBox);

}
