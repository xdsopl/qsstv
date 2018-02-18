#ifndef REPEATERCONFIG_H
#define REPEATERCONFIG_H

#include "baseconfig.h"
#include "sstv/sstvparam.h"


extern bool repeaterEnable;
extern int repeaterImageInterval;
extern esstvMode repeaterTxMode;
extern QString repeaterImage1;
extern QString repeaterImage2;
extern QString repeaterImage3;
extern QString repeaterImage4;
extern QString repeaterAcknowledge;
extern QString repeaterTemplate;
extern QString idleTemplate;

namespace Ui {
class repeaterConfig;
}

class repeaterConfig : public baseConfig
{
  Q_OBJECT
  
public:
  explicit repeaterConfig(QWidget *parent = 0);
  ~repeaterConfig();
  void readSettings();
  void writeSettings();
  void getParams();
  void setParams();
  
private:
  Ui::repeaterConfig *ui;
};

#endif // REPEATERCONFIG_H
