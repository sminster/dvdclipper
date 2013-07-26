#ifndef DVDCLIP_H
#define DVDCLIP_H

#include <QtGui/QDialog>
#include <QtCore/QMap>

#include <memory>

namespace Ui { class DvdClip; }

class DvdClip : public QDialog
{
    Q_OBJECT

public:
   explicit DvdClip(const QString& dvdDevice,
                    const QString& dvdTitle,
                    const QString& vidLength,
                    QWidget *parent = 0);
   ~DvdClip();

   int clipTop() const;
   int clipBottom() const;
   int clipLeft() const;
   int clipRight() const;

private slots:
   void prevFrame();
   void nextFrame();
   void frameSlide();
   void clipChanged();
   void analyzeImages();

private:
   void setFrameImage(const QImage& frame);
   QImage getFrame(int seconds);

   void analyzeImage(const QImage& img,
                     int& left, int& right,
                     int& top,  int& bottom);

   bool lineIsNull(const QImage& img, int line);
   bool sampIsNull(const QImage& img, int samp);

   Ui::DvdClip* theUi;

   QString theDvdDevice;
   QString theDvdTitle;

   typedef QMap<int,QImage> FrameCache;

   FrameCache theFrameCache;
};

#endif // DVDCLIP_H
