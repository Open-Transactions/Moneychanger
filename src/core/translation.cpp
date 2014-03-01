#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <core/translation.hpp>
#include <core/handlers/DBHandler.hpp>

#include <QDebug>


Translation::Translation(QObject *parent) :
    QObject(parent)
{
    if (DBHandler::getInstance()->querySize("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='language'") <= 0)
    {
        DBHandler::getInstance()->runQuery(QString("INSERT INTO `settings` (`setting`, `parameter1`) VALUES('language','%1')").arg(QLocale::system().name()));
        ui_language = QLocale::system().name();
        qDebug() << "Lanugage Setting wasn't set in the database. Inserting system default: " << ui_language;
    }
    else
    {
        if (DBHandler::getInstance()->runQuery("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='language'"))
        {
            ui_language = DBHandler::getInstance()->queryString("SELECT `parameter1` FROM `settings` WHERE `setting`='language'", 0, 0);
        }
        if (ui_language.isEmpty())
        {
            ui_language = QLocale::system().name();
            qDebug() << "Error loading language from SQL, using system default: " << ui_language;
        }
    }
}


void Translation::updateLanguage(QApplication& app, QTranslator& translator)
{
    QString translationFile = QString("%1.qm").arg(ui_language);
    app.removeTranslator(&translator);
    if (translator.load(translationFile, TRANSLATIONS_DIRECOTRY))
    {
        app.installTranslator(&translator);
    }
    else
    {
        qDebug() << "Could not update language to: " << translationFile;
    }
}
