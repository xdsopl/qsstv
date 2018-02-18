#ifndef DRMPARAMS_H
#define DRMPARAMS_H

#include <QString>
#include <QList>

struct drmTxParams
{
  int robMode;
  int qam;
  int bandwith;
  int interleaver;
  int protection;
  QString callsign;
  int reedSolomon;
};

drmTxParams modeToParams(unsigned int mode);
unsigned int paramsToMode(drmTxParams prm);
extern int numTxFrames;
extern   drmTxParams drmParams;
extern QList<short unsigned int> fixBlockList;

#endif // DRMPARAMS_H
