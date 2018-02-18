#include "syncprocessor.h"
#include "appglobal.h"
#include "sstvparam.h"
#include "configparams.h"
#include "dispatchevents.h"
#include "dispatcher.h"
#include "modes/modes.h"

#ifndef QT_NO_DEBUG
#include "scope/scopeview.h"
#endif

#define LINETOLERANCEMODEDETECT 0.015
#define LINETOLERANCEINSYNC 0.008
#define SYNCAVGINTEGRATOR 0.09
#define FILTERDELAYCORRECTION  -18
#define MINVOLUME 300

//#define DISABLERETRACE
//#define DISABLEDETECT
//#define DISABLENARROW

ssenitivity sensitivityArray[2]=
{
  // minMatchedLines maxLineDistanceModeDetect maxLineDistanceInSync onVolume offVolume  startToMax;
  {         6,                2,                       7,              3000,    2500 ,      1.5},
  {         6,                2,                       7 ,             2000,    1800 ,      1.5}
};


//const QString syncStateStr[syncProcessor::SYNCVALID+1]=
//{
//  "SYNCOFF",
//  "SYNCUP",
//  "SYNCSTART",
//  "SYNCON",
//  "SYNCDOWN",
//  "SYNCEND",
//  "SYNCVALID"
//};


const QString syncStateStr[syncProcessor::SYNCVALID+1]=
{
  "SYNCOFF",
  "SYNCUP",
  "SYNCVALID"
};

const QString stateStr[syncProcessor::RETRACEWAIT+1]=
{
  "Mode detect",
  "In Sync",
  "Sync Lost New Mode",
  "Sync Lost False syncs",
  "Sync Lost Missing Lines",
  "Sync Lost",
  "Retrace Wait"
};


syncProcessor::syncProcessor(bool narrow, QObject *parent) :  QObject(parent),streamDecode(narrow)
{
  detectNarrow=narrow;
  currentModePtr=NULL;
  visMode=NOTVALID;
  maxLineSamples=getMaxLineSamples();
  //  syncFound=false;
  if(!detectNarrow)
    {
      connect(streamDecode.getFskDecoderPtr(),SIGNAL(callReceived(QString)),SLOT(slotNewCall(QString)));
      connect(streamDecode.getVisDecoderPtr(),SIGNAL(visCodeWideDetected(int,uint)),SLOT(slotVisCodeDetected(int,uint)));
    }
  else
    {
      connect(streamDecode.getVisDecoderPtr(),SIGNAL(visCodeNarrowDetected(int,uint)),SLOT(slotVisCodeDetected(int,uint)));
    }

}

syncProcessor::~syncProcessor()
{
  int i;
  for(i=0;i<=ENDNARROW;i++)
    {
      matchArray[i].clear();
    }
  if(currentModePtr!=NULL) delete currentModePtr;
}

void syncProcessor::reset()
{
  sampleCounter=0;
  init();
  if(!detectNarrow) streamDecode.reset();
  //  scopeViewerSyncNarrow->clear();
  //  scopeViewerSyncWide->clear();
#ifndef QT_NO_DEBUG
  scopeViewerSyncNarrow->setCurveName("SYNC VOL",SCDATA1);
  scopeViewerSyncNarrow->setCurveName("SYNC AVG",SCDATA2);
  scopeViewerSyncNarrow->setCurveName("SYNC STATE",SCDATA3);
  scopeViewerSyncNarrow->setCurveName("FREQ",SCDATA4);
  scopeViewerSyncNarrow->setAxisTitles("Samples","int","State");

  scopeViewerSyncWide->setCurveName("SYNC VOL",SCDATA1);
  scopeViewerSyncWide->setCurveName("SYNC AVG",SCDATA2);
  scopeViewerSyncWide->setCurveName("SYNC STATE",SCDATA3);
  scopeViewerSyncWide->setCurveName("FREQ",SCDATA4);
  scopeViewerSyncWide->setAxisTitles("Samples","int","State");
#endif

}

/**
 * @brief
 *
 */
void syncProcessor::init()
{
  enabled=true;
  if(detectNarrow)
    {
      idxStart=STARTNARROW;
      idxEnd=ENDNARROW;
    }
  else
    {
      idxStart=STARTWIDE;
      idxEnd=ENDWIDE;
    }

  if(sstvModeIndexRx!=0)
    {
      if((sstvModeIndexRx>=idxStart) && (sstvModeIndexRx<=idxEnd))
        {
          idxEnd=idxStart=(esstvMode)(sstvModeIndexRx-1);
        }
      else
        {
          enabled=false;
        }
    }
  else if(visMode!=NOTVALID)
    {
      if((visMode>=idxStart) && (visMode<=idxEnd))
        idxEnd=idxStart=visMode;
    }
  visMode=NOTVALID; // and reset the visMode
  syncProcesState=MODEDETECT;
  modifiedClock=rxClock/SUBSAMPLINGFACTOR;
  syncArrayIndex=0;
  syncArray[0].init();
  retraceFlag=false;
  syncState=SYNCOFF;
  displaySyncEvent* ce;
  ce = new displaySyncEvent(0); // reset sync vuMeter
  QApplication::postEvent(dispatcherPtr, ce);
  addToLog("init called",LOGSYNCSTATE);
  clearMatchArray();
  currentMode=NOTVALID;
  lineTolerance=LINETOLERANCEMODEDETECT;
  minMatchedLines=sensitivityArray[squelch].minMatchedLines;
}


//void syncProcessor::reset()
//{
//  sampleCounter=0;
//  init();
//}


void syncProcessor::process()
{
  //  if(!syncFound) syncQuality=0;

#ifdef DISABLENARROW
  if(!detectNarrow)
    {
#endif
      streamDecode.process(freqPtr,sampleCounter);
      if(enabled)
        {
          extractSync();
        }
#ifdef DISABLENARROW
    }
#endif

#ifndef QT_NO_DEBUG
  if(detectNarrow)
    {
      scopeViewerSyncNarrow->addData(SCDATA1,syncVolumePtr,sampleCounter,RXSTRIPE);
//      scopeViewerSyncNarrow->addData(SCDATA2,inputVolumePtr,sampleCounter,RXSTRIPE);
      scopeViewerSyncNarrow->addData(SCDATA3,syncStateBuffer,sampleCounter,RXSTRIPE);
      scopeViewerSyncNarrow->addData(SCDATA4,freqPtr,sampleCounter,RXSTRIPE);
    }
  else
    {
      scopeViewerSyncWide->addData(SCDATA1,syncVolumePtr,sampleCounter,RXSTRIPE);
//      scopeViewerSyncWide->addData(SCDATA2,inputVolumePtr,sampleCounter,RXSTRIPE);
      scopeViewerSyncWide->addData(SCDATA3,syncStateBuffer,sampleCounter,RXSTRIPE);
      scopeViewerSyncWide->addData(SCDATA4,freqPtr,sampleCounter,RXSTRIPE);
    }
#endif
}



void syncProcessor::extractSync()
{
  int i;
  int lastSync;
  for(i=0;i<RXSTRIPE;i++)
    {
      switch(syncState)
        {
          case SYNCVALID:
          case SYNCOFF:
           if(syncVolumePtr[i]>sensitivityArray[squelch].onVolume)
            {
              syncArray[syncArrayIndex].start=sampleCounter+i;
              syncArray[syncArrayIndex].startVolume=syncVolumePtr[i];
              switchSyncState(SYNCACTIVE,sampleCounter+i);
            }
          break;
        case SYNCACTIVE:
          if(syncVolumePtr[i]<sensitivityArray[squelch].offVolume)
            {
              syncArray[syncArrayIndex].end=sampleCounter+i;
            if(validateSync())
                {
                  switchSyncState(SYNCVALID,sampleCounter+i);
                }
                else
                {
                   switchSyncState(SYNCOFF,sampleCounter+i);
                }
          }
        break;
        }
#ifndef QT_NO_DEBUG
      syncStateBuffer[i]=(unsigned char)syncState*STATESCALER;
#endif
    }

  if(syncProcesState==INSYNC)
    {
      lastSync=syncArray[activeChainPtr->last()->to].end;
      if((sampleCounter+RXSTRIPE-RXSTRIPE/7)>(lastSync+10*samplesPerLine))
        {
          tempOutOfSync=true;
        }
      missingLines=(uint) round(((sampleCounter+RXSTRIPE-RXSTRIPE/7)-(lastSync+samplesPerLine))/samplesPerLine+1);
      calcSyncQuality();
    }
  else if(visMode!=NOTVALID)
    {
      if(sampleCounter>visTimeout)
        {
          visMode=NOTVALID;
          init();
        }
    }

}

void syncProcessor::slotNewCall(QString call)
{
  emit callReceived(call);
  retraceFlag=true;
}

void syncProcessor::slotVisCodeDetected(int mode,uint visSampleCounter)
{
  if((mode>=idxStart) && (mode<=idxEnd))
    {
      visMode=(esstvMode)mode;
      idxStart=idxEnd=(esstvMode)mode;
      minMatchedLines=3;
      visTimeout=visSampleCounter+4*getLineLength(visMode,modifiedClock);
    }
  else
    {
      visMode=NOTVALID;
    }
}

bool syncProcessor::validateSync()
{
  bool result;
#ifndef DISABLERETRACE
  if(syncArray[syncArrayIndex].diffStartEnd()>=MINRETRACEWIDTH)
    {
      syncArray[syncArrayIndex].retrace=true;
      result=true;
    }
  else if(syncArray[syncArrayIndex].diffStartEnd()>=MINRETRACEWIDTH/4)
    {
      if((syncArrayIndex>2)
         && (syncArray[syncArrayIndex].end-syncArray[syncArrayIndex-1].start)>=MINRETRACEWIDTH
         && (syncArray[syncArrayIndex].start-syncArray[syncArrayIndex-1].end)<=MINRETRACEWIDTH/5)
        {
          syncArray[syncArrayIndex-1].end=syncArray[syncArrayIndex].end;
          syncArray[syncArrayIndex-1].retrace=true;
          syncArray[syncArrayIndex-1].diffStartEnd(); // just calculate the width;
          syncArrayIndex--;
        }
      result=true;
    }
#endif
  else if((syncArray[syncArrayIndex].diffStartEnd()>=0.004*SAMPLERATE) && (syncArray[syncArrayIndex].diffStartEnd()<=0.025*SAMPLERATE))
    {
      syncArray[syncArrayIndex].retrace=false;
      //      addToLog(QString("index %1, mid:=%2").arg(syncArrayIndex).arg(syncArray[syncArrayIndex].mid),LOGSYNCACCEPTED);
      result=true;
    }
  else
    {
      addToLog(QString("Sync rejected:%1 end:%2 width %3").arg(syncArray[syncArrayIndex].start)
               .arg(syncArray[syncArrayIndex].end).arg(syncArray[syncArrayIndex].diffStartEnd()),LOGSYNCREJECTED);
      result=false;
    }
  if(result)
    {
      checkSyncArray();
      switch(syncProcesState)
        {
        case MODEDETECT:
          if(findMatch())
            {
              visMode=NOTVALID; //reset visMode;
#ifndef DISABLEDETECT
              //we have a new mode
              if(!createModeBase())
                {
                  addToLog("Error creating modeBase",LOGALL);
                  result=false;
                }
              else
                {
                  falseSyncs=0;
                  lineTolerance=LINETOLERANCEINSYNC;
                  // when we have S1,S2 or SDX then we have to set the syncposition at the beginning of the green line
                  //          syncPosition=currentModePtr->adjustSyncPosition(syncArray[0].end)- FILTERDELAYCORRECTION; // type 1 sync end
                  unsigned int syncCorrected;;
                  syncWidth=getSyncWidth(currentMode ,modifiedClock);
                  if(syncArray[0].retrace)
                    {
                      syncCorrected=syncArray[0].end;
                    }
                  else
                    {
                      syncCorrected=(syncArray[0].start+syncArray[0].end)/2+syncWidth/2;
                    }

                  syncPosition=currentModePtr->adjustSyncPosition(syncCorrected,syncArray[0].retrace)+FILTERDELAYCORRECTION; // type 2 sync end
                  tempOutOfSync=false;
                  slantAdjustLine=6;
                  slantAdjust(true);
                  switchProcessState(INSYNC);

                }
#endif
            }
          break;
        case INSYNC:
          trackSyncs();
          break;
        case SYNCLOSTNEWMODE:
        case SYNCLOSTFALSESYNC:
        case SYNCLOSTMISSINGLINES:
        case SYNCLOST:
          addToLog("synclost detected",LOGSYNCSTATE);
          break;
        case RETRACEWAIT:
          break;

        }
      syncArrayIndex++;
      syncArray[syncArrayIndex].init();
    }
  return result;
}


void  syncProcessor::trackSyncs()
{
  //  calcSyncQuality();
  if(addToChain(currentMode,activeChainPtr->last()->to))
    {
      falseSyncs=0;
      missingLines=0;
      if(tempOutOfSync)
        {
          // check if we can resynchronize
          if(activeChainPtr->last()->lineSpacing<=2)
            {
              tempOutOfSync=false;
            }
        }
    }
  else
    {
      falseSyncs++;
    }
  if(retraceFlag)
    {
      switchProcessState(RETRACEWAIT);
      return;
    }
  if(currentModeMatchChanged)
    {
      currentModeMatchChanged=false;
      calculateLineNumber(activeChainPtr->last()->from,activeChainPtr->last()->to);
      lastSyncTest=activeChainPtr->last()->endTo;
      slantAdjust(false);
      if(falseSlantSync>=10)
        {
          switchProcessState(SYNCLOSTFALSESYNC);
        }
    }
}

void  syncProcessor::calcSyncQuality()
{
  int k;
  quint16 fs=0;
  syncQuality=10-(missingLines*5/25);
  //calc false syncs in the last 10
  if(activeChainPtr->count()>=10)
    {
      for(k=activeChainPtr->count()-10;k<activeChainPtr->count()-1;k++)
        {
          fs+=activeChainPtr->at(k)->to-activeChainPtr->at(k)->from-1;
        }
    }

  syncQuality-=((falseSyncs*5)/30);
  syncQuality-=(falseSlantSync*2);
  if(syncQuality<0) syncQuality=0;
  if((syncQuality<=0) && (squelch!=1))
    {
      switchProcessState(SYNCLOST);
    }

}


void  syncProcessor::calculateLineNumber(uint fromIdx,uint toIdx)
{
  quint16 lnbr;
  double fract;
  lineCompare(samplesPerLine,0,toIdx,lnbr,fract);
  syncArray[toIdx].length=syncArray[toIdx].end-syncArray[fromIdx].end;
  syncArray[toIdx].lineNumber=lnbr;
  lastUpdatedSync=toIdx;
}

void syncProcessor::checkSyncArray()
{

  if(syncArray[syncArrayIndex].retrace)
    {
      syncArray[0]=syncArray[syncArrayIndex];
      syncArrayIndex=0;
      //      syncPosition=syncArray[0].end + FILTERDELAYCORRECTION;
      addToLog(QString("Found retrace: start:%1 end:%2  width %3").arg(syncArray[0].start).arg(syncArray[0].end).arg(syncArray[0].end-syncArray[0].start),LOGSYNCACCEPTED);
      if(syncProcesState==INSYNC)
        {
          retraceFlag=true;
          syncProcesState=RETRACEWAIT;
        }
    }

  if(syncArrayIndex>=(MAXSYNCENTRIES-1))
    {
      //shift syncArray
      dropTop();
    }
  addToLog(QString("Found sync: start:%1 end:%2 width %3 at %4").arg(syncArray[syncArrayIndex].start).arg(syncArray[syncArrayIndex].end).arg(syncArray[syncArrayIndex].end-syncArray[syncArrayIndex].start).arg(syncArrayIndex),LOGSYNCACCEPTED);
}



bool syncProcessor::findMatch()
{
  int i,j,k,m;
  int fs;
  uint minTotLines=9999;
  double minFract=1;
  int idx=-1;
  QList<modeMatchList *> changeList;
  QList <int> modeList;
  QList<uint> totalLinesList;
  QList<double> totalFractList;
  addToLog (QString(" checking match for syncArrayIndex %1 at %2").arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
  for(i=idxStart;i<=idxEnd;i++)
    {
      syncWidth=getSyncWidth((esstvMode)i ,modifiedClock);
      if(addToMatch((esstvMode)i))
        {
          for(j=0;j<matchArray[i].count();j++)
            {
              if(matchArray[i][j]->count()>=(int)minMatchedLines)
                {
                  changeList.append(matchArray[i][j]);
                  modeList.append(i);
                }

            }
        }
    }
  for (m=0;m<changeList.count();m++)
    {
      totalLinesList.append(calcTotalLines( changeList.at(m)));
      totalFractList.append(calcTotalFract( changeList.at(m)));
    }
  for (m=0;m<changeList.count();m++)
    {
      if((minTotLines>=totalLinesList.at(m)) && (totalFractList.at(m)<=minFract))
        {
          idx=m;
          minTotLines=totalLinesList.at(m);
          minFract=totalFractList.at(m);
        }
    }

  if(idx>=0)
    {
      currentMode=(esstvMode)modeList.at(idx);
      activeChainPtr=changeList.at(idx);
      samplesPerLine=getLineLength(currentMode,modifiedClock);
      fs=0;
      for(k=0;k<activeChainPtr->count()-1;k++)
        {
          fs+=activeChainPtr->at(k)->to-activeChainPtr->at(k)->from-1;
        }
      if (fs>20)
        {
          clearMatchArray();
          return false;
        }
      cleanupMatchArray();
      return true;
    }

  return false;
}


uint syncProcessor::calcTotalLines(modeMatchList *mlPtr)
{
  int i;
  uint lines=0;
  for(i=0;i<mlPtr->count();i++)
    {
      lines+=mlPtr->at(i)->lineSpacing;
    }
  return lines;
}

double syncProcessor::calcTotalFract(modeMatchList *mlPtr)
{

  int i;
  double fract=0;
  for(i=0;i<mlPtr->count();i++)
    {
      fract+=mlPtr->at(i)->fraction;
    }
  return fract;

}


bool syncProcessor::addToMatch(esstvMode mode)
{
  int  i;
  
  if(syncArrayIndex<1) return false;
  for(i=syncArrayIndex-1;i>=0;i--)
    {

      if(addToChain(mode,i))
        {
          return true;
        }
    }
  return false;
}


bool syncProcessor::addToChain(esstvMode mode,  uint fromIdx)
{
  int i;
  double fract;
  quint16 lnbr;

  samplesPerLine=getLineLength(mode,modifiedClock);
  
  if(!lineCompare(samplesPerLine,fromIdx,syncArrayIndex,lnbr,fract))
    {
      return false;
    }


  if((syncArray[syncArrayIndex].diffStartEnd()<syncWidth*0.75)
     || (syncArray[syncArrayIndex].diffStartEnd()>syncWidth*2)
     || (syncArray[fromIdx].diffStartEnd()<syncWidth*0.75))
    {

      return false;
    }

  if (syncArray[fromIdx].diffStartEnd()>syncWidth*2)
    {
      if(fromIdx!=0 || !syncArray[0].retrace)
        {
          return false;
        }
    }


  if(syncProcesState==MODEDETECT)
    {
      if(lnbr>sensitivityArray[squelch].maxLineDistanceModeDetect)
        {
          return false;
        }
    }

  bool found=false;
  if(matchArray[mode].count()==0) // we don't have a chain yet
    {
      matchArray[mode].append(new modeMatchList);
      matchArray[mode][0]->append(new smatchEntry(fromIdx,syncArrayIndex,lnbr,fract,syncArray[fromIdx].end,syncArray[syncArrayIndex].end));
      addToLog(QString("Match: mode=%1,new chain=%2 syncIndex=%3 end=%4").arg(getSSTVModeNameShort(mode)).arg(matchArray[mode].count()-1).arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
    }
  else
    {
      // can we append this to an existing chain?
      for(i=0;i<matchArray[mode].count();i++)
        {
          if(matchArray[mode][i]->last()->to==fromIdx)
            {
              if((syncProcesState==INSYNC) && (i==0) )
                {
                  currentModeMatchChanged=true;
                }
              matchArray[mode][i]->append(new smatchEntry(fromIdx,syncArrayIndex,lnbr,fract,syncArray[fromIdx].end,syncArray[syncArrayIndex].end));
              addToLog(QString("Match: mode=%1,chain=%2 syncIndex=%3 end=%4").arg(getSSTVModeNameShort(mode)).arg(i).arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
              found=true;
              break;
            }
        }
      if(!found)
        {
          matchArray[mode].append(new modeMatchList);
          matchArray[mode].last()->append(new smatchEntry(fromIdx,syncArrayIndex,lnbr,fract,syncArray[fromIdx].end,syncArray[syncArrayIndex].end));
          addToLog(QString("Match: mode=%1,new chain=%2 syncIndex=%3 end=%4").arg(getSSTVModeNameShort(mode)).arg(matchArray[mode].count()-1).arg(syncArrayIndex).arg(syncArray[syncArrayIndex].end),LOGSYNCMATCH);
        }
      
    }
  return true;
}


void syncProcessor::clearMatchArray()
{
  int i,j;
  for(i=idxStart;i<=idxEnd;i++)
    {
      for(j=0;j<matchArray[i].count();j++)
        {
          removeMatchArrayChain((esstvMode)i,0);
        }
      matchArray[i].clear();
    }
  activeChainPtr=NULL;
  switchProcessState(MODEDETECT);
}

void syncProcessor::removeMatchArrayChain(esstvMode mode,int chainIdx)
{
  int i;
  for(i=0;i<matchArray[mode][chainIdx]->count();i++)
    {
      delete matchArray[mode][chainIdx]->at(i);
    }
  matchArray[mode][chainIdx]->clear();
  delete matchArray[mode][chainIdx];
  matchArray[mode].takeAt(chainIdx);
}

void syncProcessor::cleanupMatchArray()
{
  int i,j;
  //  double fract=0;
  //  quint16 lnbr=0;
  for(i=idxStart;i<=idxEnd;i++)
    {
      for(j=0;j<matchArray[i].count();)
        {
          if(activeChainPtr!=matchArray[i][j])
            {
              removeMatchArrayChain((esstvMode)i,j);
            }
          else
            {
              j++;
            }
        }
    }

  //  if(syncArray[0].retrace)
  //    {
  //      if((currentMode==S1) || (currentMode==S2))
  //        {
  //          //check if this is a valid retrace
  //          // prepend active chainPtr with the retrace position
  //          activeChainPtr->prepend(new smatchEntry(0,activeChainPtr->at(0)->from ,lnbr,fract,syncArray[0].end,syncArray[activeChainPtr->at(0)->to].end));
  //        }
  //    }





  for(i=0;i<activeChainPtr->count();i++)
    {
      syncArray[activeChainPtr->at(i)->from].inUse=true;
      syncArray[activeChainPtr->at(i)->to].inUse=true;
    }
  for(i=0;i<syncArrayIndex;)
    {
      if(!syncArray[i].inUse)
        {
          deleteSyncArrayEntry(i);
        }
      else
        {
          i++;
        }
    }
  for(i=0;i<syncArrayIndex;i++)
    {
      calculateLineNumber(i,i+1);
    }
}

void syncProcessor::dropTop()
{
  deleteSyncArrayEntry(0);
}

void syncProcessor::deleteSyncArrayEntry(uint entry)
{
  int i,j,k;
  modeMatchList *ml;
  //  smatchEntry *tempPtr;
  if(entry>=syncArrayIndex) return;
  // delete or adapt the matchArrays
  for(i=idxStart;i<=idxEnd;i++)   //all modes
    {
      for(j=0;j<matchArray[i].count();) //all chains
        {
          for(k=0;k<matchArray[i][j]->count();)
            {
              ml=matchArray[i][j];
              if (ml->at(k)->from==entry)
                {
                  delete matchArray[i][j]->takeAt(k);
                }
              else
                {
                  if(ml->at(k)->from>entry)
                    {
                      ml->at(k)->from--;
                    }
                  if(ml->at(k)->to>entry)
                    {
                      ml->at(k)->to--;
                    }
                  k++;
                }
            }
          if(matchArray[i][j]->count()==0)
            {
              delete matchArray[i].takeAt(j);
            }
          else
            {
              j++;
            }
        }
    }
  for(i=entry;i<(int)syncArrayIndex;i++)
    {
      syncArray[i]=syncArray[i+1];
    }
  syncArrayIndex--;
}



void syncProcessor::recalculateMatchArray()
{
}

bool syncProcessor::lineCompare(DSPFLOAT samPerLine, int srcIdx, int dstIdx, quint16 &lineNumber, double &fraction)
{
  double delta;
  double intPart;
  delta=(double)(syncArray[dstIdx].end-syncArray[srcIdx].end);
  fraction=modf(delta/samPerLine,&intPart);
  if(fraction>=0.5)
    {
      fraction=(1-fraction);
      intPart+=1.;
    }
  fraction=fraction/intPart;
  lineNumber=(int)intPart;
  return (fraction<lineTolerance);
}

void syncProcessor::resetRetraceFlag()
{
  retraceFlag=false;
  clearMatchArray();
}


bool  syncProcessor::createModeBase()
{
  bool done=false;
  if(currentModePtr) delete currentModePtr;
  currentModePtr=NULL;
  switch (currentMode)
    {
    case M1:
    case M2:
      currentModePtr=new modeGBR(currentMode,RXSTRIPE,false,false);
      break;
    case S1:
    case S2:
    case SDX:
      currentModePtr=new modeGBR2(currentMode,RXSTRIPE,false,false);
      break;
    case R36:
      currentModePtr=new modeRobot1(currentMode,RXSTRIPE,false,false);
      break;
    case R24:
    case R72:
    case MR73:
    case MR90:
    case MR115:
    case MR140:
    case MR175:
    case ML180:
    case ML240:
    case ML280:
    case ML320:
      currentModePtr=new modeRobot2(currentMode,RXSTRIPE,false,false);
      break;
    case SC2_60:
    case SC2_120:
    case SC2_180:
    case P3:
    case P5:
    case P7:
    case MC110N:
    case MC140N:
    case MC180N:
      currentModePtr=new modeRGB(currentMode,RXSTRIPE,false,false);
      break;
    case FAX480:
    case BW8:
    case BW12:
      currentModePtr=new modeBW(currentMode,RXSTRIPE,false,false);
      break;
    case AVT24:
    case AVT90:
    case AVT94:
      currentModePtr=new modeAVT(currentMode,RXSTRIPE,false,false);
      break;
    case PD50:
    case PD90:
    case PD120:
    case PD160:
    case PD180:
    case PD240:
    case PD290:
    case MP73:
    case MP115:
    case MP140:
    case MP175:
      currentModePtr=new modePD(currentMode,RXSTRIPE,false,false);
      break;
    case MP73N:
    case MP110N:
    case MP140N:
      currentModePtr=new modePD(currentMode,RXSTRIPE,false,true);
      break;
    default:
      currentMode=NOTVALID;
      break;
    }
  if (currentMode!=NOTVALID)
    {
      initializeSSTVParametersIndex(currentMode,false);
      QString s=getSSTVModeNameLong(currentMode);
      addToLog(QString("create RX mode: %1").arg(getSSTVModeNameShort(currentMode)),LOGSYNCSTATE);
      currentModePtr->init(modifiedClock);
      startImageRXEvent* ce = new startImageRXEvent(QSize(currentModePtr->imagePixels(),currentModePtr->imageLines()));
      ce->waitFor(&done);
      QApplication::postEvent(dispatcherPtr, ce);
      while(!done)
        {
          QApplication::processEvents();
        }
    }
  return (currentMode!=NOTVALID);
}


void syncProcessor::regression(DSPFLOAT &a,DSPFLOAT &b,bool initial)
{
  /* calculate linear regression
    formula x=a+by
    b=sum((x[i]-xm)*(y[i]-ym))/sum((y[i]-ym)*(y[i]-ym))
    a=xm-b*ym
  */
  int j;

  int count=activeChainPtr->count();
  falseSlantSync=0;
  DSPFLOAT sum_x,sum_y,sum_xx,sum_xy;
  sum_x=sum_y=sum_xx=sum_xy=a=b=0;
  unsigned int endZero;
  unsigned int tempCount=0;
  //  if(currentMode==S1 || currentMode==S2)
  //    {
  //      j=1;
  //      endZero=syncArray[1].end-samplesPerLine;
  //    }
  //  else
  {
    j=0;
    endZero=syncArray[0].end;
  }
  for(;j<count;j++)
    {

      if((activeChainPtr->at(j)->fraction>0.006)&&(!initial))
        {
          continue;
        }

      slantXYArray[tempCount].y=(DSPFLOAT)(syncArray[activeChainPtr->at(j)->to].end-endZero);
      slantXYArray[tempCount].x= syncArray[activeChainPtr->at(j)->to].lineNumber*samplesPerLine;
      addToLog(QString("pos: %1, x=%2 y=%3 syncIndex:%4, diff %5").arg(tempCount).arg(slantXYArray[tempCount].x).arg(slantXYArray[tempCount].y).arg(activeChainPtr->at(j)->to).arg(slantXYArray[tempCount].x-slantXYArray[tempCount].y) ,LOGSLANT);
      if((fabs(slantXYArray[tempCount].x-slantXYArray[tempCount].y)>150.)&&(!initial))
        {
          falseSlantSync++;
          continue;
        }
      sum_x+=slantXYArray[tempCount].x;
      sum_y+=slantXYArray[tempCount].y;
      sum_xx+=slantXYArray[tempCount].x*slantXYArray[tempCount].x;
      sum_xy+=slantXYArray[tempCount].x*slantXYArray[tempCount].y;
      lastValidSyncCounter=syncArray[activeChainPtr->at(j)->to].end;
      tempCount++;
    }
  b=((tempCount)*sum_xy-(sum_x*sum_y))/((tempCount)*sum_xx-(sum_x*sum_x));
  a=sum_y/(tempCount)-(b*sum_x)/(tempCount);
}



bool syncProcessor::slantAdjust(bool initial)
{
  DSPFLOAT a,b;
  if ((currentMode>=AVT24) && (currentMode <= AVT94)) return true;
  if(currentMode==NOTVALID) return true;
  falseSlantSync=0;
  if(!initial)
    {
      if(syncArray[activeChainPtr->last()->to].lineNumber<slantAdjustLine) return false;
    }

  regression(a,b,initial);

  //  addToLog(QString("regr. params line %1 a:%2 b:%3").arg(slantAdjustLine).arg(a).arg(b),LOGSLANT);
  slantAdjustLine+=5;
  if(!autoSlantAdjust) return false;
  if(initial)
    {
      if((fabs(1.-b)>0.02) || (fabs(a)>100)) return false;
    }
  else
    {
      if((fabs(1.-b)>0.005) || (fabs(a)>50)) return false;
    }
  if  (((fabs(1.-b)>0.00001)  || (fabs(a)>1)  )    && autoSlantAdjust)
    {
      newClock=true;
      modifiedClock*=b;
      samplesPerLine=getLineLength(currentMode,modifiedClock); //recalculate the samples per line
      addToLog(QString("new clock accepted: %1 a=%2,b=%3").arg(modifiedClock).arg(a).arg(b),LOGSLANT);

      syncArray[0].end+=(long)round(a);
      syncArray[0].start+=(long)round(a);

      unsigned int syncCorrected;
      if(syncArray[0].retrace)
        {
          syncCorrected=syncArray[0].end;
        }
      else
        {
          syncCorrected=(syncArray[0].start+syncArray[0].end)/2+getSyncWidth(currentMode,modifiedClock)/2;
        }

      syncPosition=currentModePtr->adjustSyncPosition(syncCorrected,syncArray[0].retrace)+FILTERDELAYCORRECTION; // type 2 sync end

      recalculateMatchArray();
      addToLog(QString("slantAdjust: modified  syncpos:=%1").arg(syncPosition),LOGSLANT);

      return true;
    }

  return false;
}



void syncProcessor::switchSyncState(esyncState newState,quint32 sampleCntr)
{
  Q_UNUSED(sampleCntr)
  if(syncState!=newState)
    {
      addToLog(QString("switching from %1 to %2 at %3").arg(syncStateStr[syncState]).arg(syncStateStr[newState]).arg(sampleCntr),LOGSYNCSTATE);
      syncState=newState;
    }
}

void syncProcessor::switchProcessState(esyncProcessState  newState)
{
  addToLog(QString("syncProcessState %1 to %2").arg(stateStr[syncProcesState]).arg(stateStr[newState]),LOGSYNCPROCESSSTATE);
  if((newState==SYNCLOSTFALSESYNC) || (newState==SYNCLOSTNEWMODE)||(newState==SYNCLOSTMISSINGLINES) || (newState==SYNCLOST))
    {
      newState=SYNCLOST;
    }
  syncProcesState=newState;
}

#ifndef QT_NO_DEBUG
void syncProcessor::setOffset(unsigned int dataScopeOffset)
{
  xOffset=dataScopeOffset;
  scopeViewerSyncNarrow->setOffset(xOffset);
  scopeViewerSyncWide->setOffset(xOffset);
}

void syncProcessor::clear()
{
  scopeViewerSyncNarrow->clear();
  scopeViewerSyncWide->clear();
  xOffset=0;
  scopeViewerSyncNarrow->setOffset(xOffset);
  scopeViewerSyncWide->setOffset(xOffset);
}


#endif
