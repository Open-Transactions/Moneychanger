
#include <gui/widgets/qrtoolbutton.hpp>
#include <ui_qrtoolbutton.h>

#include <QPainter>
#include <QStylePainter>
#include <QStyleOption>
#include <QImage>

QrToolButton::QrToolButton(QWidget *parent) :
    QToolButton(parent),
    ui(new Ui::QrToolButton)
{
    ui->setupUi(this);
}

QrToolButton::~QrToolButton()
{
    delete ui;

    if (nullptr != qr)
    {
        QRcode_free(qr);
        qr = nullptr;
    }
}

bool QrToolButton::asImage(QImage & output, int size)
{
    if (size != 0)
    {
        QImage image(size, size, QImage::Format_Mono);
        QPainter painter(&image);
        QColor background(Qt::white);
        painter.setBrush(background);
        painter.setPen(Qt::NoPen);
        painter.drawRect(0, 0, size, size);
        if (nullptr != qr)
        {
            draw(painter, size, size);
        }
        output = image;
        return true;
    }

    return false;
}

void QrToolButton::setString(QString str)
{
    string = str;

    if (qr != nullptr)
    {
        QRcode_free(qr);
        qr = nullptr;
    }
    qr = QRcode_encodeString(string.toStdString().c_str(),
                             1,
                             QR_ECLEVEL_L,
                             QR_MODE_8,
                             1);
    update();
}

int QrToolButton::getQRWidth() const
{
    if (qr != nullptr)
    {
        return qr->width;
    }

    return 0;
}

//bool QrToolButton::saveImage(QString fileName, int size)
//{
//    if (size != 0 && !fileName.isEmpty())
//    {
//        QImage image(size, size, QImage::Format_Mono);
//        QPainter painter(&image);
//        QColor background(Qt::white);
//        painter.setBrush(background);
//        painter.setPen(Qt::NoPen);
//        painter.drawRect(0, 0, size, size);
//        if (nullptr != qr)
//        {
//            draw(painter, size, size);
//        }
//        return image.save(fileName);
//    }
//    else
//    {
//        return false;
//    }
//}

void QrToolButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionToolButton sotb;
    sotb.initFrom(this);

    p.drawComplexControl(QStyle::CC_ToolButton, sotb);
//  void QStylePainter::drawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex &opt)


    QPainter & painter(p);
//  QPainter painter(this);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width(), height());
    if (nullptr != qr)
    {
        draw(painter, width(), height());
    }
}

QSize QrToolButton::sizeHint() const
{
    QSize s;
    if (nullptr != qr)
    {
        int qr_width = qr->width > 0 ? qr->width : 1;
        s = QSize(qr_width * 4, qr_width * 4);
    }
    else
    {
        s = QSize(50, 50);
    }
    return s;
}

QSize QrToolButton::minimumSizeHint() const
{
    QSize s;
    if (nullptr != qr)
    {
        int qr_width = qr->width > 0 ? qr->width : 1;
        s = QSize(qr_width, qr_width);
    }
    else
    {
        s = QSize(50, 50);
    }
    return s;
}


void QrToolButton::draw(QPainter &painter, int width, int height)
{
    QColor foreground(Qt::black);
    painter.setBrush(foreground);
    const int qr_width = qr->width > 0 ? qr->width : 1;

    const double scale_qr_width = qr_width;
    const double scale_width    = width;
    const double scale_height   = height;

    const double double_scale_x = scale_width  / scale_qr_width;
    const double double_scale_y = scale_height / scale_qr_width;

    for (int y = 0; y < qr_width; y ++)
    {
        const double scale_y = y;

        for (int x = 0; x < qr_width; x++)
        {
            const double scale_x = x;

            unsigned char b = qr->data[y * qr_width + x];

            if (b & 0x01)
            {
                QRectF r(scale_x * double_scale_x, scale_y * double_scale_y, double_scale_x, double_scale_y);
                painter.drawRects(&r, 1);
            }
        }
    }
}
