#include "dlgcreatevariable.hpp"
#include "ui_dlgcreatevariable.h"

DlgCreateVariable::DlgCreateVariable(QWidget *parent) :
    QDialog(parent),
    m_bFirstRun(true),
    m_nAccessIndex(1),
    m_nTypeIndex(0),
    m_bValue(true),
    ui(new Ui::DlgCreateVariable)
{
    ui->setupUi(this);

    this->installEventFilter(this);
    // ----------------------------------
    ui->comboBoxVariableAccess->addItem(QString(tr("Constant")),   QVariant(QString("constant")));
    ui->comboBoxVariableAccess->addItem(QString(tr("Persistent")), QVariant(QString("persistent")));
    ui->comboBoxVariableAccess->addItem(QString(tr("Important")),  QVariant(QString("important")));
    // ----------------------------------
    ui->comboBoxVariableValue->addItem(QString(tr("True")),  QVariant(QString("true")));
    ui->comboBoxVariableValue->addItem(QString(tr("False")), QVariant(QString("false")));
    // ----------------------------------
    ui->comboBoxVariableType->addItem(QString(tr("String")),   QVariant(QString("string")));
    ui->comboBoxVariableType->addItem(QString(tr("Long Int")), QVariant(QString("integer")));
    ui->comboBoxVariableType->addItem(QString(tr("Boolean")),  QVariant(QString("bool")));
    // ----------------------------------
}

DlgCreateVariable::~DlgCreateVariable()
{
    delete ui;
}

//virtual
void DlgCreateVariable::showEvent(QShowEvent * event)
{
    if (m_bFirstRun)
    {
        m_bFirstRun = false;
        // ----------------------
        ui->lineEditVariableName->setText(m_qstrName);
        ui->comboBoxVariableValue ->setCurrentIndex(m_bValue ? 1 : 0);
        ui->comboBoxVariableAccess->setCurrentIndex(m_nAccessIndex); // 1 is "persistent"
        ui->comboBoxVariableType  ->setCurrentIndex(m_nTypeIndex); // 0 is "string" (I think.)
        // ----------------------------------
        switch (m_nTypeIndex)
        {
        case 0: // string
            ui->plainTextEditVariableValue->setPlainText(m_qstrValue);
            ui->lineEditVariableValue->setVisible(false);
            ui->comboBoxVariableValue->setVisible(false);
            break;

        case 1: // int64_t
            ui->lineEditVariableValue->setText(m_qstrValue);
            ui->plainTextEditVariableValue->setVisible(false);
            ui->comboBoxVariableValue->setVisible(false);
            break;

        case 2: // bool
            ui->comboBoxVariableValue->setCurrentIndex(m_bValue ? 1 : 0);
            ui->lineEditVariableValue->setVisible(false);
            ui->plainTextEditVariableValue->setVisible(false);
            break;
        } //switch
    } // first run.

    // ----------------------------------
    // call inherited method
    //
    QDialog::showEvent(event);
}

bool DlgCreateVariable::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape)
        {
            close(); // This is caught by this same filter.
            return true;
        }
    }
    // standard event processing
    return QDialog::eventFilter(obj, event);
}


void DlgCreateVariable::on_buttonBox_accepted()
{
    m_qstrName   = ui->lineEditVariableName->text();
    m_qstrAccess = ui->comboBoxVariableAccess->currentData().toString();
    m_qstrType   = ui->comboBoxVariableType  ->currentData().toString();
    // ----------------------------------------------
    m_nAccessIndex = ui->comboBoxVariableAccess->currentIndex();
    m_nTypeIndex   = ui->comboBoxVariableType->currentIndex();
    // ----------------------------------------------
    switch (m_nTypeIndex)
    {
    case 0: // string
        m_qstrValue = ui->plainTextEditVariableValue->toPlainText();
        break;
    case 1: // int64_t
        m_qstrValue = ui->lineEditVariableValue->text();
        break;
    case 2: // bool
        m_qstrValue = ui->comboBoxVariableValue->currentData().toString();
        break;
    }
    // ----------------------------------------------
    QDialog::accept();
}

void DlgCreateVariable::on_comboBoxVariableType_currentIndexChanged(int index)
{
    switch (index)
    {
    case 0: // String
        ui->lineEditVariableValue->setVisible(false);
        ui->comboBoxVariableValue->setVisible(false);
        ui->plainTextEditVariableValue->setVisible(true);
        break;
    case 1: // Integer
        ui->lineEditVariableValue->setVisible(true);
        ui->comboBoxVariableValue->setVisible(false);
        ui->plainTextEditVariableValue->setVisible(false);
         break;
    case 2: // Bool
        ui->lineEditVariableValue->setVisible(false);
        ui->comboBoxVariableValue->setVisible(true);
        ui->plainTextEditVariableValue->setVisible(false);
        break;
    default:
        ui->lineEditVariableValue->setVisible(false);
        ui->comboBoxVariableValue->setVisible(false);
        ui->plainTextEditVariableValue->setVisible(false);
        break;
    }
}
