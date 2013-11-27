#ifndef TRANSLATION_H
#define TRANSLATION_H

#include <QObject>
#include <QApplication>
#include <QTranslator>

#define TRANSLATIONS_DIRECOTRY "translations"

class Translation : public QObject
{
    Q_OBJECT
public:
    explicit Translation(QObject *parent = 0);

    void updateLanguage(QApplication& app,QTranslator& translator);

private:
    QString ui_language;
};

#endif // TRANSLATION_H
