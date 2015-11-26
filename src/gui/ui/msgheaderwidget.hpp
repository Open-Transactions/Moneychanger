#ifndef MSGHEADERWIDGET_HPP
#define MSGHEADERWIDGET_HPP

#include <QWidget>

namespace Ui {
class MsgHeaderWidget;
}

class MsgHeaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MsgHeaderWidget(QWidget *parent = 0);
    ~MsgHeaderWidget();

    void setSender(QString qstrSender);
    void setRecipient(QString qstrRecipient);
    void setTimestamp(QString qstrTimestamp);
    void setFolder(QString qstrFolder);
    void setSubject(QString qstrSubject);

private:
    Ui::MsgHeaderWidget *ui;
};

#endif // MSGHEADERWIDGET_HPP
