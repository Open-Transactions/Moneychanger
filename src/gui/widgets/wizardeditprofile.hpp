#ifndef WIZARDEDITPROFILE_HPP
#define WIZARDEDITPROFILE_HPP

#include "core/WinsockWrapper.h"
#include "core/ExportWrapper.h"

#include <gui/widgets/pagenym_altlocation.hpp>

#include <QWizard>

namespace Ui {
class WizardEditProfile;
}

class WizardEditProfile : public QWizard
{
    Q_OBJECT

public:
    explicit WizardEditProfile(QWidget *parent = 0);
    ~WizardEditProfile();

private:
    Ui::WizardEditProfile *ui;

public:
    listContactDataTuples & listContactDataTuples_;
};

#endif // WIZARDEDITPROFILE_HPP
