#include "guiconfig.h"
#include "ui_guiconfig.h"
#include <QImageWriter>
#include <QColorDialog>


int galleryRows;
int galleryColumns;
bool imageStretch;
QColor backGroundColor;
QColor imageBackGroundColor;
bool slowCPU;
bool lowRes;

guiConfig::guiConfig(QWidget *parent) : baseConfig(parent),  ui(new Ui::guiConfig)
{
  ui->setupUi(this);
  backGroundColorChanged=false;
  connect(ui->backgroundColorPushButton,SIGNAL(clicked()),SLOT(slotBGColorSelect()));
  connect(ui->imageBackgroundColorPushButton,SIGNAL(clicked()),SLOT(slotIBGColorSelect()));
}

guiConfig::~guiConfig()
{
  delete ui;
}

void guiConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("GUI");
  galleryRows=qSettings.value("galleryRows",4).toInt();
  galleryColumns=qSettings.value("galleryColumns",4).toInt();
  imageStretch=qSettings.value("imageStretch",true).toBool();
  backGroundColor=qSettings.value("backGroundColor",QColor(128,128,128)).value<QColor>();
  imageBackGroundColor=qSettings.value("imageBackGroundColor",QColor(0,0,128)).value<QColor>();
  slowCPU=qSettings.value("slowCPU",false).toBool();
  lowRes=qSettings.value("lowRes",false).toBool();
  qSettings.endGroup();
  setParams();
}

void guiConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("GUI");
  qSettings.setValue("galleryRows",galleryRows);
  qSettings.setValue("galleryColumns",galleryColumns);
  qSettings.setValue("imageStretch",imageStretch);
  qSettings.setValue("backGroundColor",backGroundColor);
  qSettings.setValue("imageBackGroundColor",imageBackGroundColor);
  qSettings.setValue("slowCPU",slowCPU);
  qSettings.setValue("lowRes",lowRes);
  qSettings.endGroup();
}

void guiConfig::getParams()
{
  int galleryRowsCopy=galleryRows;
  int galleryColumnsCopy=galleryColumns;
  getValue(galleryRows,ui->rowsSpinBox);
  getValue(galleryColumns, ui->columnsSpinBox);
  changed=false;
  if( galleryRowsCopy!=galleryRows || galleryColumnsCopy!=galleryColumns)
    changed=true;
  getValue(imageStretch,ui->stretchCheckBox);
  getValue(slowCPU,ui->slowCPUCheckBox);
  getValue(lowRes,ui->lowResCheckBox);
}

void guiConfig::setParams()
{
  setValue(galleryRows,ui->rowsSpinBox);
  setValue(galleryColumns, ui->columnsSpinBox);
  setValue(imageStretch,ui->stretchCheckBox);
  setColorLabel(backGroundColor,false);
  setColorLabel(imageBackGroundColor,true);
  setValue(slowCPU,ui->slowCPUCheckBox);
  setValue(lowRes,ui->lowResCheckBox);
}


void guiConfig::slotBGColorSelect()
{
  QColor c;
  c=QColorDialog::getColor(backGroundColor,this,"",QColorDialog::ShowAlphaChannel);
  setColorLabel(c,false);
}

void guiConfig::slotIBGColorSelect()
{
  QColor c;
  c=QColorDialog::getColor(imageBackGroundColor,this,"",QColorDialog::ShowAlphaChannel);
  setColorLabel(c,true);
}


void guiConfig::setColorLabel(QColor c,bool image)
{
if (c.isValid())
  {
    if(!image)
      {
        backGroundColorChanged=true;
        QPalette palette = ui->backGroundColorLabel->palette();
        palette.setColor(ui->backGroundColorLabel->backgroundRole(), c);
        ui->backGroundColorLabel->setPalette(palette);
        backGroundColor=c;
      }
    else
      {
        QPalette palette = ui->imageBackGroundColorLabel->palette();
        palette.setColor(ui->imageBackGroundColorLabel->backgroundRole(), c);
        ui->imageBackGroundColorLabel->setPalette(palette);
        imageBackGroundColor=c;
      }
   }
}
