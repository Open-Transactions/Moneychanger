
#include <gui/widgets/qrwidget.hpp>
#include <ui_qrwidget.h>

#include <QPainter>
#include <QImage>


QrWidget::QrWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QrWidget)
{
    ui->setupUi(this);
}

QrWidget::~QrWidget()
{
    delete ui;

    if (nullptr != qr)
    {
        QRcode_free(qr);
        qr = nullptr;
    }
}

void QrWidget::setString(QString str)
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

int QrWidget::getQRWidth() const
{
    if (qr != nullptr)
    {
        return qr->width;
    }

    return 0;
}

bool QrWidget::asImage(QImage & output, int size)
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

bool QrWidget::saveImage(QString fileName, int size)
{
    if (size != 0 && !fileName.isEmpty())
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
        return image.save(fileName);
    }
    else
    {
        return false;
    }
}


void QrWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width(), height());
    if (nullptr != qr)
    {
        draw(painter, width(), height());
    }
}

QSize QrWidget::sizeHint() const
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

QSize QrWidget::minimumSizeHint() const
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



void QrWidget::draw(QPainter &painter, int width, int height)
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


//void QrWidget::draw(QPainter &painter, int width, int height)
//{
//    QColor foreground(Qt::black);
//    painter.setBrush(foreground);
//    const int qr_width = qr->width > 0 ? qr->width : 1;
//    double scale_x = width / qr_width;
//    double scale_y = height / qr_width;

//    for (int y = 0; y < qr_width; y ++)
//    {
//        for (int x = 0; x < qr_width; x++)
//        {
//            unsigned char b = qr->data[y * qr_width + x];

//            if (b & 0x01)
//            {
//                QRectF r(x * scale_x, y * scale_y, scale_x, scale_y);
//                painter.drawRects(&r, 1);
//            }
//        }
//    }
//}
