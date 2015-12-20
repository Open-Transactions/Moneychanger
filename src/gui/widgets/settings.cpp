#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/settings.hpp>
#include <ui_settings.h>

#include <core/translation.hpp>
#include <core/handlers/DBHandler.hpp>
#include <core/handlers/contacthandler.hpp>

#include <QDir>
#include <QMessageBox>
#include <QDebug>


void Settings::on_comboBoxLanguage_currentIndexChanged(int index)
{
    ui->pushButtonSave->setEnabled(true);
}

Settings::Settings(QWidget *parent) :
    QWidget(parent, Qt::Window),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    // ----------------------------------------------
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    language = DBHandler::getInstance()->queryString("SELECT `parameter1` FROM `settings` WHERE `setting`='language'", 0, 0);

    QDir translationDir(TRANSLATIONS_DIRECOTRY);
    QStringList filters;
    filters << "*.qm";
    translationDir.setNameFilters(filters);
    QStringList translationFiles = translationDir.entryList();
    QString translationName;
    ui->comboBoxLanguage->blockSignals(true);
    foreach(translationName, translationFiles)
    {
        translationName.chop(3);
        QLocale nativTranslation(translationName);
        ui->comboBoxLanguage->addItem(nativTranslation.nativeLanguageName(),translationName);
    }
    ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->findData(language));
    ui->comboBoxLanguage->blockSignals(false);
#endif
    // ----------------------------------------------
    if (DBHandler::getInstance()->querySize("SELECT `setting`,`parameter1` FROM `settings` WHERE `setting`='expertmode'") <= 0)
    {
        DBHandler::getInstance()->runQuery(QString("INSERT INTO `settings` (`setting`, `parameter1`) VALUES('expertmode','off')"));
        qDebug() << "expertmode setting wasn't set in the database. Setting to 'off'.";
    }
    // ----------------------------------------------
    expertmode_ = MTContactHandler::getInstance()->
            GetValueByID("expertmode", "parameter1", "settings", "setting");

    const bool bExpertMode = (0 == expertmode_.compare("on"));

    ui->checkBoxExpertMode->blockSignals(true);
    if (bExpertMode)
        ui->checkBoxExpertMode->setChecked(true);
    else
        ui->checkBoxExpertMode->setChecked(false);
    ui->checkBoxExpertMode->blockSignals(false);
    // ----------------------------------------------
}

Settings::~Settings()
{
    delete ui;
}

void Settings::showEvent (QShowEvent * event)
{
    // ----------------------------------------------
//#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
//    ui->comboBoxLanguage->blockSignals(true);
//    ui->comboBoxLanguage->setCurrentIndex(ui->comboBoxLanguage->findData(language));
//    ui->comboBoxLanguage->blockSignals(false);
//#endif
//    // ----------------------------------------------
//    QString qstrExpertMode = MTContactHandler::getInstance()->
//            GetValueByID("expertmode", "parameter1", "settings", "setting");

//    const bool bExpertMode = (0 == qstrExpertMode.compare("on"));

//    ui->checkBoxExpertMode->blockSignals(true);
//    if (bExpertMode)
//        ui->checkBoxExpertMode->setChecked(true);
//    else
//        ui->checkBoxExpertMode->setChecked(false);
//    ui->checkBoxExpertMode->blockSignals(false);
    // ----------------------------------------------
}


void Settings::on_checkBoxExpertMode_toggled(bool checked)
{
//    QString GetValueByID(QString qstrID, QString column, QString table, QString id_name);
//    bool    SetValueByID(QString qstrID, QString value,  QString column, QString table, QString id_name);
//    QString settings = "CREATE TABLE IF NOT EXISTS settings (setting TEXT PRIMARY KEY, parameter1 TEXT)";

//    expertmode_ = checked ? QString("on") : QString("off");

    ui->pushButtonSave->setEnabled(true);
}


void Settings::on_pushButtonSave_clicked()
{
    bool bExpertModeChanged = false;
    bool bSettingsChanged   = false;
    // ----------------------------------------------
    QString qstrExpertMode = ui->checkBoxExpertMode->isChecked() ? QString("on") : QString("off");
    // ----------------------------------------------
    if (0 != expertmode_.compare(qstrExpertMode))
    {
        //bSettingsChanged = true;
        bExpertModeChanged = true;
        expertmode_ = qstrExpertMode;
        MTContactHandler::getInstance()->SetValueByID("expertmode", qstrExpertMode,  "parameter1", "settings", "setting");
    }
    // ----------------------------------------------
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

    qDebug() << ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString();
    if (language != ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString())
    {
        bSettingsChanged = true;
        language = ui->comboBoxLanguage->itemData(ui->comboBoxLanguage->currentIndex()).toString();
        DBHandler::getInstance()->runQuery(
                                            QString("UPDATE `settings` SET `setting`='language', `parameter1`= '%1' WHERE `setting`='language'")
                                            .arg(language)
                                          );
    }
#endif
    // ----------------------------------------------
    ui->pushButtonSave->setEnabled(false);

    if (bSettingsChanged)
        QMessageBox::information(this, "Settings saved","The change will take effect after a restart of Moneychanger.");
    // ----------------------------------------------
    if (bExpertModeChanged)
        emit expertModeUpdated(ui->checkBoxExpertMode->isChecked());
    // ----------------------------------------------
//  hide();
}

