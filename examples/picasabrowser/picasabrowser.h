#ifndef PICASABROWSER_H
#define PICASABROWSER_H

#include <QMainWindow>
#include <qtpicasaweb.h>

namespace Ui {
    class PicasaBrowser;
}

class PicasaBrowser : public QMainWindow
{
    Q_OBJECT

public:
    explicit PicasaBrowser(QWidget *parent = 0);
    ~PicasaBrowser();

private:
    Ui::PicasaBrowser *ui;
    QtPicasaWeb *qtPicasaWeb;
};

#endif // PICASABROWSER_H
