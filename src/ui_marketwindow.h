/********************************************************************************
** Form generated from reading UI file 'marketwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MARKETWINDOW_H
#define UI_MARKETWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>

QT_BEGIN_NAMESPACE

class Ui_MarketWindow
{
public:
    QTextEdit *textEdit;
    QPushButton *pushButton;
    QPushButton *pushButton_2;

    void setupUi(QDialog *MarketWindow)
    {
        if (MarketWindow->objectName().isEmpty())
            MarketWindow->setObjectName(QStringLiteral("MarketWindow"));
        MarketWindow->resize(400, 300);
        textEdit = new QTextEdit(MarketWindow);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(20, 20, 361, 191));
        pushButton = new QPushButton(MarketWindow);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(30, 230, 114, 32));
        pushButton_2 = new QPushButton(MarketWindow);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));
        pushButton_2->setGeometry(QRect(260, 230, 114, 32));
        textEdit->raise();
        pushButton->raise();
        pushButton_2->raise();
        pushButton_2->raise();

        retranslateUi(MarketWindow);

        QMetaObject::connectSlotsByName(MarketWindow);
    } // setupUi

    void retranslateUi(QDialog *MarketWindow)
    {
        MarketWindow->setWindowTitle(QApplication::translate("MarketWindow", "Advanced Markets", 0));
        textEdit->setHtml(QApplication::translate("MarketWindow", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:'Lucida Grande'; font-size:13pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">This is only a test, please ignore.</p></body></html>", 0));
        pushButton->setText(QApplication::translate("MarketWindow", "Apply", 0));
        pushButton_2->setText(QApplication::translate("MarketWindow", "Close", 0));
    } // retranslateUi

};

namespace Ui {
    class MarketWindow: public Ui_MarketWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MARKETWINDOW_H
