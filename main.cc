#include <QtGui/QApplication>
#include "DvdClip.h"

#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);

   if (argc < 4)
   {
      cerr << "Usage: " << *argv << " DEVICE TITLE_NUM TITLE_LEN" << endl;
      return EXIT_FAILURE;
   }

   DvdClip w(argv[1], argv[2], argv[3]);
   w.show();

   if (w.exec() != QDialog::Accepted) return EXIT_FAILURE;

   cout << w.clipLeft() << ":"
        << w.clipTop() << ":"
        << w.clipRight() << ":"
        << w.clipBottom() << endl;
   return EXIT_SUCCESS;
}
