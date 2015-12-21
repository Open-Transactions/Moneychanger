#ifndef QRWIDGET_HPP
#define QRWIDGET_HPP

#include <QWidget>

#include <qrencode/qrencode.h>

namespace Ui {
class QrWidget;
}

class QrWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QrWidget(QWidget *parent = 0);
    ~QrWidget();

    //This is where you pass the string to encode
    void setString (QString str);
    int  getQRWidth() const;
    bool saveImage (QString name, int size);
    bool asImage(QImage & output, int size);

signals:

protected:
    void paintEvent(QPaintEvent *);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

private:
    //This is where you paint on the widget
    //ZXing is doing all the heavy lifting here.
    void draw(QPainter &painter, int width, int height);
    QString string;
    QRcode *qr=nullptr;

    Ui::QrWidget *ui;
};

#endif // QRWIDGET_HPP

