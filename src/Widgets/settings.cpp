#include "settings.h"
#include "ui_settings.h"
#include "Handlers/DBHandler.h"
#include "translation.h"

#include <QDir>
#include <QMessageBox>
#include <QLocale>

Settings::Settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    language = DBHandler::getInstance()->queryString("SELECT `parameter1` FROM `settings` WHERE `setting`='language'", 0, 0);

    QDir translationDir(TRANSLATIONS_DIRECOTRY);
    QStringList filters;
    filters << "*.qm";
    translationDir.setNameFilters(filters);
    QStringList translationFiles = translationDir.entryList();
    QString translationName;
    foreach(translationName, translationFiles)
    {
        translationName.chop(3);
        QLocale nativTranslation(translationName);
        ui->comboBoxLanguage->addItem(nativTranslation.nativeLanguageName(),translationName);
    }

    ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->findData(language));

}

Settings::~Settings()
{
    delete ui;
}

void Settings::showEvent (QShowEvent * event)
{
    ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->findData(language));
}

void Settings::on_pushButtonSave_clicked()
{
    qDebug() << ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString();
    if(language != ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString())
    {
        language = ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString();
        DBHandler::getInstance()->runQuery(
                                            QString("UPDATE `settings` SET `setting`='language', `parameter1`= '%1' WHERE `setting`='language'")
                                            .arg(language)
                                          );
        QMessageBox::information(this, "Settings saved","The language change will take effect after a restart of Moneychanger.");
    }
    hide();
}
