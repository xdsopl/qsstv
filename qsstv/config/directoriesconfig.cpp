#include "directoriesconfig.h"
#include "ui_directoriesconfig.h"


QString rxSSTVImagesPath;
QString rxDRMImagesPath;
QString txSSTVImagesPath;
QString txDRMImagesPath;
QString txStockImagesPath;
QString templatesPath;
QString audioPath;
bool saveTXimages;
QString docURL;



directoriesConfig::directoriesConfig(QWidget *parent) :baseConfig(parent),
  ui(new Ui::directoriesConfig)
{
  ui->setupUi(this);
  connect(ui->rxSSTVImagesPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseRxSSTVImagesPath()));
  connect(ui->rxDRMImagesPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseRxDRMImagesPath()));
  connect(ui->txSSTVImagesPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseTxSSTVImagesPath()));
  connect(ui->txDRMImagesPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseTxDRMImagesPath()));
  connect(ui->txStockImagesPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseTxStockImagesPath()));
  connect(ui->templatesPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseTemplatesPath()));
  connect(ui->audioPathBrowseButton,SIGNAL(clicked()),SLOT(slotBrowseAudioPath()));
}

directoriesConfig::~directoriesConfig()
{
  delete ui;
}

void directoriesConfig::readSettings()
{
  QSettings qSettings;
  qSettings.beginGroup("DIRECTORIES");
  rxSSTVImagesPath=qSettings.value("rxSSTVImagesPath",QString(getenv("HOME"))+"/qsstv/rx_sstv/").toString();
  rxDRMImagesPath=qSettings.value("rxDRMImagesPath",QString(getenv("HOME"))+"/qsstv/rx_drm/").toString();
  txSSTVImagesPath=qSettings.value("txSSTVImagesPath",QString(getenv("HOME"))+"/qsstv/tx_sstv/").toString();
  txDRMImagesPath=qSettings.value("txDRMImagesPath",QString(getenv("HOME"))+"/tx_drm/").toString();
  txStockImagesPath=qSettings.value("txStockImagesPath",QString(getenv("HOME"))+"/tx_stock/").toString();
  templatesPath=qSettings.value("templatesPath",QString(getenv("HOME"))+"/templates/").toString();
  audioPath=qSettings.value("audioPath",QString(getenv("HOME"))+"/audio/").toString();
  docURL=qSettings.value("docURL","http://users.telenet.be/on4qz/qsstv/manual").toString();
  saveTXimages=qSettings.value("saveTXimages",false).toBool();
  qSettings.endGroup();
  setParams();
}

void directoriesConfig::writeSettings()
{
  QSettings qSettings;
  getParams();
  qSettings.beginGroup("DIRECTORIES");
  qSettings.setValue("rxSSTVImagesPath",rxSSTVImagesPath);
  qSettings.setValue("rxDRMImagesPath",rxDRMImagesPath);
  qSettings.setValue("txSSTVImagesPath",txSSTVImagesPath);
  qSettings.setValue("txDRMImagesPath",txDRMImagesPath);
  qSettings.setValue("txStockImagesPath",txStockImagesPath);
  qSettings.setValue("templatesPath",templatesPath);
  qSettings.setValue("audioPath",audioPath);
  qSettings.setValue("docURL",docURL);
  qSettings.setValue("saveTXimages",saveTXimages);

  qSettings.endGroup();
}

void directoriesConfig::getParams()
{
  QString rxSSTVImagePathSaved=rxSSTVImagesPath;
  QString rxDRMImagePathSaved=rxDRMImagesPath;
  QString txSSTVImagePathSaved=txSSTVImagesPath;
  QString txDRMImagePathSaved=txDRMImagesPath;
  QString txStockImagePathSaved=txStockImagesPath;
  QString templatesPathSaved=templatesPath;
  getValue(rxSSTVImagesPath,ui->rxSSTVImagesPathLineEdit);
  getValue(rxDRMImagesPath,ui->rxDRMImagesPathLineEdit);
  getValue(txSSTVImagesPath,ui->txSSTVImagesPathLineEdit);
  getValue(txDRMImagesPath,ui->txDRMImagesPathLineEdit);
  getValue(txStockImagesPath,ui->txStockImagesPathLineEdit);
  getValue(templatesPath,ui->templatesPathLineEdit);
  getValue(audioPath,ui->audioPathLineEdit);
  getValue(docURL,ui->docPathLineEdit);
  getValue(saveTXimages,ui->saveTXcheckBox);
  changed=false;
  if(rxSSTVImagePathSaved!=rxSSTVImagesPath || rxDRMImagePathSaved!=rxDRMImagesPath ||
     txSSTVImagePathSaved!=txSSTVImagesPath ||
     txDRMImagePathSaved!=txDRMImagesPath ||
     txStockImagePathSaved!=txStockImagesPath ||
     templatesPathSaved!=templatesPath
     )
    changed=true; // always save it
}

void directoriesConfig::setParams()
{
  setValue(rxSSTVImagesPath,ui->rxSSTVImagesPathLineEdit);
  setValue(rxDRMImagesPath,ui->rxDRMImagesPathLineEdit);
  setValue(txSSTVImagesPath,ui->txSSTVImagesPathLineEdit);
  setValue(txDRMImagesPath,ui->txDRMImagesPathLineEdit);
  setValue(txStockImagesPath,ui->txStockImagesPathLineEdit);
  setValue(templatesPath,ui->templatesPathLineEdit);
  setValue(audioPath,ui->audioPathLineEdit);
  setValue(docURL,ui->docPathLineEdit);
  setValue(saveTXimages,ui->saveTXcheckBox);
  // create directories if not exist
  createDir(rxSSTVImagesPath);
  createDir(rxDRMImagesPath);
  createDir(txSSTVImagesPath);
  createDir(txDRMImagesPath);
  createDir(txStockImagesPath);
  createDir(templatesPath);

  createDir(rxSSTVImagesPath+"/cache");
  createDir(rxDRMImagesPath+"/cache");
  createDir(txSSTVImagesPath+"/cache");
  createDir(txDRMImagesPath+"/cache");
  createDir(txStockImagesPath+"/cache");
  createDir(audioPath);
}

void directoriesConfig::createDir(QString path)
{
  QDir dd(path);
  if(!dd.exists())
    {
      dd.mkpath(path);
    }
}


/**
  Browse function for path where the rximages are stored
*/


void directoriesConfig::slotBrowseRxSSTVImagesPath()
{
  browseDir(ui->rxSSTVImagesPathLineEdit,rxSSTVImagesPath);
}

void directoriesConfig::slotBrowseRxDRMImagesPath()
{
  browseDir(ui->rxDRMImagesPathLineEdit,rxDRMImagesPath);
}

/**
  Browse function for path where the tximages are stored
*/

void directoriesConfig::slotBrowseTxSSTVImagesPath()
{
  browseDir(ui->txSSTVImagesPathLineEdit,txSSTVImagesPath);
}

void directoriesConfig::slotBrowseTxDRMImagesPath()
{
  browseDir(ui->txDRMImagesPathLineEdit,txDRMImagesPath);
}

void directoriesConfig::slotBrowseTxStockImagesPath()
{
  browseDir(ui->txStockImagesPathLineEdit,txStockImagesPath);
}

/**
  Browse function for path where the templates are stored
*/

void directoriesConfig::slotBrowseTemplatesPath()
{
  browseDir(ui->templatesPathLineEdit,templatesPath);
}

/**
  Browse function for audio path
*/

void directoriesConfig::slotBrowseAudioPath()
{
  browseDir(ui->audioPathLineEdit,audioPath);
}
