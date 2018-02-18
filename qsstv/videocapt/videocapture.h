/*
 *      videoCapture.h -- Kapture
 *
 *      Copyright (C) 2006-2007
 *          Detlev Casanova (detlev.casanova@gmail.com)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 */

#ifndef _VIDEOCAPTURE_H
#define _VIDEOCAPTURE_H

#include <QObject>
#include <QImage>
#include <linux/videodev2.h>
#include "appdefs.h"
#include <libv4lconvert.h>

struct tableFormat
{
  QString description;
  unsigned int val1;
  unsigned int val2;
};
extern tableFormat sizeList[];
extern tableFormat fpsList[];


class videoCapture : public QObject
{
	Q_OBJECT

public:
  videoCapture();
  ~videoCapture();
	
	void close();
  bool  open(QString videoDev);
//  QList<int>   getFormatList(QList<QString> &description);
//	QList<QSize> getSizesList() const;
//  QList<int> formatList;
//  QList<QSize> sizeList;
  QList<QString> descripList;

  bool setFormat(v4l2_format &fmt);
  bool getFormat(v4l2_format &fmt);
  int getFrame();
  int currentWidth(v4l2_format fmt) const;
  int currentHeight(v4l2_format fmt) const;
  int currentPixelFormat(v4l2_format fmt) const;
//	int changeCtrl(int ctrl, int value = 0);
//	int defaultCtrlVal(unsigned int control, int &defaultValue);
//  bool isOpened() const {return opened;};
  QImage *getImage() {return localImage;}
  bool captureStart();
  bool captureStop();
  bool stopStreaming();
  bool startSnapshots();
  bool init(int pixelFormat, int width, int height);
  QString getErrorString() {return errorString;}

private:
	int dev;
  v4l2_format srcFmt;
  v4l2_format dstFmt;
	v4l2_buffer buf;
	v4l2_requestbuffers rb;
	bool allocated;
	
  uchar *mem[4];
	size_t bufLength;
  QImage *localImage;
  bool opened;
  bool mmapped;
  bool streaming;
  void dumpCaps(v4l2_capability &cap);
  int numBuffers;
  QString pixelFormatStr(int pixelFormat);
  struct v4lconvert_data *convertData;
  bool convert(unsigned char *src);
  enum QImage::Format checkConversionNeeded ();
  enum QImage::Format qFmt;
  bool needsConversion;
  QString videoDevice;
  int allocateBuffers(int numBufs);
  QString errorString;

};
#endif

