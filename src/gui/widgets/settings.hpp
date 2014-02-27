#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include <WinsockWrapper.h>
#include <ExportWrapper.h>

#include <QWidget>

namespace Ui {
class Settings;
}

class Settings : public QWidget
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();

private:
    void showEvent (QShowEvent * event);

private slots:
    void on_pushButtonSave_clicked();

private:
    Ui::Settings *ui;
    QString language;
};

#endif // SETTINGS_HPP
