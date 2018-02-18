#include "hybridconfig.h"
#include "ui_hybridconfig.h"
#include "ftp.h"
#include "hybridcrypt.h"

#include <QMessageBox>

bool enableHybridRx;
//bool enableSpecialServer;
int hybridFtpPort;
QString hybridFtpRemoteHost;
QString hybridFtpRemoteDirectory;
QString hybridFtpLogin;
QString hybridFtpPassword;
QString hybridFtpHybridFilesDirectory;


hybridConfig::hybridConfig(QWidget *parent) :baseConfig(parent), ui(new Ui::hybridConfig)
{
    ui->setupUi(this);
    //  connect(testFTPPushButton,SIGNAL(clicked()),SLOT(slotTestFTPPushButton()));
    connect(ui->testHybridPushButton,SIGNAL(clicked()),SLOT(slotTestHybridPushButton()));
}

hybridConfig::~hybridConfig()
{
    delete ui;
}

void hybridConfig::readSettings()
{
    QSettings qSettings;
    qSettings.beginGroup("HYBRID");
    enableHybridRx=qSettings.value("enableHybridRx",true).toBool();
//    enableSpecialServer=qSettings.value("enableSpecialServer",false).toBool();
    hybridFtpPort=qSettings.value("hybridFtpPort",21).toInt();
    hybridFtpRemoteHost=qSettings.value("hybridFtpRemoteHost","").toString();
    hybridFtpRemoteDirectory=qSettings.value("hybridFtpRemoteDirectory","").toString();
    hybridFtpLogin=qSettings.value("hybridFtpLogin","").toString();
    hybridFtpPassword=qSettings.value("hybridFtpPassword","").toString();
    hybridFtpHybridFilesDirectory=qSettings.value("hybridFtpHybridFilesDirectory","HybridFiles1").toString();

    qSettings.endGroup();
    setParams();
}

void hybridConfig::writeSettings()
{
    QSettings qSettings;
    getParams();
    qSettings.beginGroup("HYBRID");
    qSettings.setValue("enableHybridRx",enableHybridRx);
//    qSettings.setValue("enableSpecialServer",enableSpecialServer);
    qSettings.setValue("hybridFtpPort",hybridFtpPort);
    qSettings.setValue("hybridFtpRemoteHost",hybridFtpRemoteHost);
    qSettings.setValue("hybridFtpRemoteDirectory",hybridFtpRemoteDirectory);
    qSettings.setValue("hybridFtpLogin",hybridFtpLogin);
    qSettings.setValue("hybridFtpPassword",hybridFtpPassword);
    qSettings.setValue("hybridFtpHybridFilesDirectory",hybridFtpHybridFilesDirectory);
    qSettings.endGroup();
}

void hybridConfig::getParams()
{
    bool enableHybridRxCopy=enableHybridRx;
//    bool enableSpecialServerCopy=enableSpecialServer;
    int hybridFtpPortCopy=hybridFtpPort;
    QString hybridFtpRemoteHostCopy=hybridFtpRemoteHost;
    QString hybridFtpRemoteDirectoryCopy=hybridFtpRemoteDirectory;
    QString hybridFtpLoginCopy=hybridFtpLogin;
    QString hybridFtpPasswordCopy=hybridFtpPassword;


    getValue(enableHybridRx,ui->enableHybridRxCheckBox);
//    getValue(enableSpecialServer,ui->enableSpecialServerCheckBox);
    getValue(hybridFtpPort,ui->hybridFtpPortSpinBox);
    getValue(hybridFtpRemoteHost,ui->hybridRemoteHostLineEdit);
    getValue(hybridFtpRemoteDirectory,ui->hybridRemoteDirectoryLineEdit);
    getValue(hybridFtpLogin,ui->hybridFtpLoginLineEdit);
    getValue(hybridFtpPassword,ui->hybridFtpPasswordLineEdit);
    getValue(hybridFtpHybridFilesDirectory,ui->hybridFilesDirectoryLineEdit);
    changed=false;
    if(  enableHybridRxCopy!=enableHybridRx
//         || enableSpecialServerCopy!=enableSpecialServer
         || hybridFtpPortCopy!=hybridFtpPort
         || hybridFtpRemoteHostCopy!=hybridFtpRemoteHost
         || hybridFtpRemoteDirectoryCopy!=hybridFtpRemoteDirectory
         || hybridFtpLoginCopy!=hybridFtpLogin
         || hybridFtpPasswordCopy!=hybridFtpPassword)
        changed=true;




}

void hybridConfig::setParams()
{
    setValue(enableHybridRx,ui->enableHybridRxCheckBox);
//    setValue(enableSpecialServer,ui->enableSpecialServerCheckBox);
    setValue(hybridFtpPort,ui->hybridFtpPortSpinBox);
    setValue(hybridFtpRemoteHost,ui->hybridRemoteHostLineEdit);
    setValue(hybridFtpRemoteDirectory,ui->hybridRemoteDirectoryLineEdit);
    setValue(hybridFtpLogin,ui->hybridFtpLoginLineEdit);
    setValue(hybridFtpPassword,ui->hybridFtpPasswordLineEdit);
    setValue(hybridFtpHybridFilesDirectory,ui->hybridFilesDirectoryLineEdit);
}


void hybridConfig::slotTestHybridPushButton()
{
    QString msg;
    ftpInterface fInt("TestHybridConnection");
    ui->testHybridPushButton->setDisabled(true);
    getParams();
    fInt.setupConnection(hybridFtpRemoteHost,hybridFtpPort,hybridFtpLogin,hybridFtpPassword,hybridFtpRemoteDirectory+"/"+hybridFtpHybridFilesDirectory);
//    fInt.setupConnection(hc.host(),hc.port(),hc.user(),hc.passwd(),hc.dir()+"/"+hybridFtpHybridFilesDirectory);
    msg=fInt.execFTPTest();
    ui->testHybridPushButton->setDisabled(false);
    QMessageBox::information(this,"Test Connection",msg);
}
