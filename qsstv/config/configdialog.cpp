#include "configdialog.h"
#include "ui_configdialog.h"
#include "gallerywidget.h"
#include "mainwindow.h"
#include "txwidget.h"
#include "filewatcher.h"


configDialog::configDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::configDialog)
{
  ui->setupUi(this);
  ui->catWidget->attachRigController(rigControllerPtr);
  ui->configTabWidget->setCurrentIndex(0);
}

configDialog::~configDialog()
{
  delete ui;
}

void configDialog::readSettings()
{
  ui->cwWidget->readSettings();
  ui->directoryWidget->readSettings();
  ui->drmProfilesWidget->readSettings();
  ui->ftpWidget->readSettings();
  ui->guiWidget->readSettings();
  ui->hybridWidget->readSettings();
  ui->hybridNotifyWidget->readSettings();
  ui->operatorWidget->readSettings();
  ui->repeaterWidget->readSettings();
  ui->catWidget->readSettings();
  ui->soundWidget->readSettings();
  ui->waterfallWidget->readSettings();
  ui->freqSelectWidget->readSettings();
}

void configDialog::writeSettings()
{
  ui->operatorWidget->writeSettings();
  ui->directoryWidget->writeSettings();
  ui->soundWidget->writeSettings();
  ui->guiWidget->writeSettings();
  ui->catWidget->writeSettings();
  ui->ftpWidget->writeSettings();
  ui->repeaterWidget->writeSettings();
  ui->hybridWidget->writeSettings();
  ui->hybridNotifyWidget->writeSettings();
  ui->drmProfilesWidget->writeSettings();
  ui->cwWidget->writeSettings();
  ui->waterfallWidget->writeSettings();
  soundNeedsRestart=ui->soundWidget->hasChanged();
  guiNeedsRestart=ui->guiWidget->hasChanged();
  ui->freqSelectWidget->writeSettings();
  if(ui->guiWidget->backGroundColorChanged)
    {
      emit bgColorChanged();
      ui->guiWidget->backGroundColorChanged=false;
    }

}

/**
    Opens the configuration dialog
*/

int configDialog::exec()
{
  if(QDialog::exec())
  {
    writeSettings();
    if(ui->directoryWidget->hasChanged() || ui->guiWidget->hasChanged())
      {
        galleryWidgetPtr->changedMatrix();
      }
    if(ui->drmProfilesWidget->hasChanged())
      {
        txWidgetPtr->reloadProfiles();
      }
    if(ui->directoryWidget->hasChanged())
      {
        fileWatcherPtr->init();
      }
    mainWindowPtr->setSSTVDRMPushButton(transmissionModeIndex==TRXDRM);

    return QDialog::Accepted;
  }
  else
  {
    return QDialog::Rejected;
  }
}


