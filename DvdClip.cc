#include "DvdClip.h"
#include "ui_DvdClip.h"

#include <QtGui/QPainter>
#include <QtGui/QBrush>

#include <QtCore/QProcess>
#include <QtCore/QFileInfo>

#include <sys/stat.h>

#include <iostream>

#include "/home/scott/dev/debug.h"

using namespace std;

DvdClip::DvdClip(const QString& dvdDevice,
                 const QString& dvdTitle,
                 const QString& vidLength,
                 QWidget *parent)
   :
     QDialog(parent),
     theUi(new Ui::DvdClip),
     theDvdDevice(dvdDevice),
     theDvdTitle(dvdTitle)
{
   theUi->setupUi(this);

   const int NSECS = vidLength.toInt();
   theUi->theFrameSlider->setMaximum(NSECS);

   theUi->thePrevFrameButton->setDefaultAction(theUi->thePrevFrameAction);
   theUi->theNextFrameButton->setDefaultAction(theUi->theNextFrameAction);

   connect(theUi->theFrameSlider,SIGNAL(valueChanged(int)),SLOT(frameSlide()));

   connect(theUi->theClipBottom,SIGNAL(valueChanged(int)), SLOT(clipChanged()));
   connect(theUi->theClipLeft,  SIGNAL(valueChanged(int)), SLOT(clipChanged()));
   connect(theUi->theClipRight, SIGNAL(valueChanged(int)), SLOT(clipChanged()));
   connect(theUi->theClipTop,   SIGNAL(valueChanged(int)), SLOT(clipChanged()));

   connect(theUi->theAnalyzeButton, SIGNAL(clicked()), SLOT(analyzeImages()));

   connect(theUi->thePrevFrameAction, SIGNAL(triggered()), SLOT(prevFrame()));
   connect(theUi->theNextFrameAction, SIGNAL(triggered()), SLOT(nextFrame()));

   // call qsrand() to initialize random number, but be consistent
   qsrand(NSECS);

   theUi->theFrameSlider->setValue(qrand() % NSECS);
   theUi->theFrameSlider->setValue(qrand() % NSECS);
   theUi->theFrameSlider->setValue(qrand() % NSECS);
   analyzeImages();
}

DvdClip::~DvdClip()
{
   delete theUi;
}

int DvdClip::clipTop() const
{
   return theUi->theClipTop->value();
}

int DvdClip::clipBottom() const
{
   return theUi->theClipBottom->value();
}

int DvdClip::clipLeft() const
{
   return theUi->theClipLeft->value();
}

int DvdClip::clipRight() const
{
   return theUi->theClipRight->value();
}

QImage DvdClip::getFrame(int seconds)
{
   QString tmpDir = "/tmp/dvdclip";
   if (!QFileInfo(tmpDir).exists())
   {
      mkdir(tmpDir.toAscii(), 0700);
   }

   QStringList args;
   args << "-dvd-device" << theDvdDevice
        << ("dvd://" + theDvdTitle)
        << "-vo" << ("png:outdir=" + tmpDir)
        << "-ao" << "null"
        << "-frames" << "1"
        << "-ss" << QString::number(seconds);

//   cout << "DEBUG: args == {" << args.join(",").toStdString() << "}" << endl;

   QProcess proc;
   proc.start("mplayer", args);
   proc.waitForFinished();

   // TODO: get better naming
   QFileInfo frameFile(tmpDir + "/00000001.png");
   if (frameFile.exists())
   {
      QImage frame(frameFile.filePath());
      unlink(frameFile.filePath().toAscii());
      return frame;
   }
   return QImage();
}

void DvdClip::setFrameImage(const QImage& frame)
{
   QImage annotFrame(frame);
   {
      QPainter painter(&annotFrame);
      painter.setPen(Qt::green);

      const int left   = clipLeft();
      const int top    = clipTop();
      const int bottom = clipBottom();
      const int right  = clipRight();

      const int height = annotFrame.height();
      const int width  = annotFrame.width();

      painter.drawLine(left, 0, left, height);
      painter.drawLine(0, top, width, top);
      painter.drawLine(width-right, 0, width-right, height);
      painter.drawLine(0, height-bottom, width, height-bottom);
   }
   theUi->theDisplay->setPixmap(QPixmap::fromImage(annotFrame));
}

void DvdClip::frameSlide()
{
   int seconds = theUi->theFrameSlider->value();

   theUi->theFrameLabel->setText(QString("Time: %1 seconds").arg(seconds));

   if (!theFrameCache.contains(seconds))
   {
      theFrameCache[seconds] = getFrame(seconds);
      theUi->theAnalyzeButton->setEnabled(true);
   }

   setFrameImage(theFrameCache[seconds]);

   // set enabled state on thePrevFrameAction and theNextFrameAction
   theUi->thePrevFrameAction->setEnabled(
      theFrameCache.find(seconds) != theFrameCache.begin());
   theUi->theNextFrameAction->setEnabled(
      theFrameCache.lowerBound(seconds+1) != theFrameCache.end());
}

void DvdClip::clipChanged()
{
   setFrameImage(theFrameCache[theUi->theFrameSlider->value()]);
}

void DvdClip::analyzeImage(const QImage& img,
                           int& left, int& right,
                           int& top,  int& bottom)
{
   const int imgHeight = img.height();
   const int imgWidth  = img.width();

   for(top = 0;
       top < imgHeight && lineIsNull(img, top);
       top += 2);
   for(bottom = 0;
       (imgHeight-bottom-1)>=top && lineIsNull(img, imgHeight-bottom-1);
       bottom += 2);

   for(left = 0;
       left < imgWidth && sampIsNull(img, left);
       left += 2);
   for(right = 0;
       (imgWidth-right-1)>=left && sampIsNull(img, imgWidth-right-1);
       right += 2);
}

void DvdClip::analyzeImages()
{
   int minLeft   = -1;
   int minRight  = -1;
   int minTop    = -1;
   int minBottom = -1;
   unsigned int numFrames = 0;
   int maxWidth = 0;
   int maxHeight = 0;
   foreach(QImage img, theFrameCache.values())
   {
      int l, r, t, b;
      analyzeImage(img, l, r, t, b);
      if (minLeft   < 0 || l < minLeft  ) minLeft   = l;
      if (minRight  < 0 || r < minRight ) minRight  = r;
      if (minTop    < 0 || t < minTop   ) minTop    = t;
      if (minBottom < 0 || b < minBottom) minBottom = b;
      if (maxWidth  < img.width())  maxWidth  = img.width();
      if (maxHeight < img.height()) maxHeight = img.height();
      ++numFrames;
   }

   if (minLeft   >= 0) theUi->theClipLeft->setValue(minLeft);
   if (minRight  >= 0) theUi->theClipRight->setValue(minRight);
   if (minTop    >= 0) theUi->theClipTop->setValue(minTop);
   if (minBottom >= 0) theUi->theClipBottom->setValue(minBottom);

   theUi->theAnalyzeButton->setEnabled(false);

   // compute some kind of confidence, based on how much is clipped,
   // and how many frames were examined
   const int totalClipLR = (theUi->theClipLeft->value() +
                            theUi->theClipRight->value());
   const int totalClipTB = (theUi->theClipTop->value() +
                            theUi->theClipBottom->value());
   double clipPercentLR = double(totalClipLR) / maxWidth;
   double clipPercentTB = double(totalClipTB) / maxHeight;
   const double confidence =
      (1.0 - (clipPercentLR + clipPercentTB)/(numFrames * numFrames));

   theUi->theConfidenceLabel->setText(tr("Confidence: %1% (%2 frame%3)").
                                      arg(confidence*100, 5, 'f', 2).
                                      arg(numFrames).
                                      arg(numFrames == 1 ? "" : "s"));
}

static bool pixNull(QRgb pix)
{
   static const int PIX_THRESHOLD = 16;
   return (qRed(pix)   < PIX_THRESHOLD &&
           qGreen(pix) < PIX_THRESHOLD &&
           qBlue(pix)  < PIX_THRESHOLD);
}

bool DvdClip::lineIsNull(const QImage& img, int line)
{
   const int NSAMPS = img.width();
   for(int samp = 0; samp < NSAMPS; ++samp)
   {
      if (!pixNull(img.pixel(samp, line))) return false;
   }
   return true;
}

bool DvdClip::sampIsNull(const QImage &img, int samp)
{
   const int NLINES = img.height();
   for(int line = 0; line < NLINES; ++line)
   {
      if (!pixNull(img.pixel(samp, line))) return false;
   }
   return true;
}

void DvdClip::prevFrame()
{
   int curFrame = theUi->theFrameSlider->value();
   FrameCache::const_iterator it = theFrameCache.find(curFrame);
   if (it != theFrameCache.end() && it != theFrameCache.begin())
   {
      --it;
      theUi->theFrameSlider->setValue(it.key());
   }
}

void DvdClip::nextFrame()
{
   int curFrame = theUi->theFrameSlider->value();
   FrameCache::const_iterator it = theFrameCache.lowerBound(curFrame+1);
   if (it != theFrameCache.end())
   {
      theUi->theFrameSlider->setValue(it.key());
   }
}
