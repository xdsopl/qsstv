#ifndef HYBRIDNOTIFYCONFIG_H
#define HYBRIDNOTIFYCONFIG_H

#include "baseconfig.h"

extern bool enableHybridNotify;
extern bool enableHybridNotifySnoop;
extern int hybridNotifyPort;
extern QString hybridNotifyRemoteHost;
extern QString hybridNotifyRemoteDir;
extern QString hybridNotifyLogin;
extern QString hybridNotifyPassword;
extern QString hybridNotifyDir;


namespace Ui {
class hybridNotifyConfig;
}

class hybridNotifyConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit hybridNotifyConfig(QWidget *parent = 0);
  ~hybridNotifyConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();
private slots:
  void slotTestNotifyPushButton();
  
private:
  Ui::hybridNotifyConfig *ui;
};

#endif // HYBRIDNOTIFYCONFIG_H
