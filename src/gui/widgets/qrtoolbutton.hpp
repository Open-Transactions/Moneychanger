#ifndef QRTOOLBUTTON_HPP
#define QRTOOLBUTTON_HPP

#include <QToolButton>

#include <qrencode/qrencode.h>

namespace Ui {
class QrToolButton;
}

class QrToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit QrToolButton(QWidget *parent = 0);
    ~QrToolButton();

    //This is where you pass the string to encode
    void setString (QString str);
    int  getQRWidth() const;
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

private:
    Ui::QrToolButton *ui;
};

#endif // QRTOOLBUTTON_HPP

