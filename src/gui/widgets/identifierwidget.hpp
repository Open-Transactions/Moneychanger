#ifndef IDENTIFIERWIDGET_HPP
#define IDENTIFIERWIDGET_HPP

#include <opentxs/WinsockWrapper.h>
#include <opentxs/ExportWrapper.h>

#include <QWidget>
#include <QString>

namespace Ui {
class MTIdentifierWidget;
}

class MTIdentifierWidget : public QWidget
{
    Q_OBJECT

    QString m_qstrId;

public:
    explicit MTIdentifierWidget(QWidget *parent = 0);
    ~MTIdentifierWidget();

    QString GetId() const { return m_qstrId; }

    void SetLabel(QString qstrLabelText);

private slots:
    void on_lineEdit_textChanged(const QString &arg1);

private:
    Ui::MTIdentifierWidget *ui;
};

#endif // IDENTIFIERWIDGET_HPP
