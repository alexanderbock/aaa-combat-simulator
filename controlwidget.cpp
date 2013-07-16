#include "controlwidget.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>

#define OOLIPC "By IPC"
#define OOLVALUE "By Combat Value"

ControlWidget::ControlWidget(QWidget* parent)
    : QWidget(parent)
    , landBattle_(0)
    , oneLandUnitMustSurvive_(0)
    , amphibiousCombat_(0)
    , oolType_(0)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    oneLandUnitMustSurvive_ = new QCheckBox("One attacking land\nunit must live");
    amphibiousCombat_ = new QCheckBox("Amphibious Combat");

    landBattle_ = new QCheckBox("Land Battle");
    connect(landBattle_, SIGNAL(stateChanged(int)), this, SLOT(landBattleCheckboxChanged(int)));
    connect(landBattle_, SIGNAL(stateChanged(int)), this, SIGNAL(landBattleCheckboxDidChange()));
    landBattle_->setChecked(true);
    layout->addWidget(landBattle_);
    
    layout->addWidget(amphibiousCombat_);
    
    layout->addWidget(oneLandUnitMustSurvive_);

    layout->addStretch(-1);

    QLabel* oolTypeLabel = new QLabel("Order of loss");
    layout->addWidget(oolTypeLabel);
    oolType_ = new QComboBox;
    oolType_->addItem(OOLVALUE);
    oolType_->addItem(OOLIPC);
    oolType_->setToolTip("By Combat Value: First the unit with the lesser attack/defense value is chosen. Default in TripleA\nBy IPC: First the cheaper unit is taken as casualty");
    layout->addWidget(oolType_);

    layout->addStretch(-1);

    QPushButton* clearButton = new QPushButton("Clear");
    connect(clearButton, SIGNAL(clicked(bool)), this, SIGNAL(clear()));
    layout->addWidget(clearButton);

    QPushButton* switchSidesButton = new QPushButton("Switch attacker\nand defender");
    connect(switchSidesButton, SIGNAL(clicked(bool)), this, SIGNAL(switchSides()));
    layout->addWidget(switchSidesButton);
    
    QPushButton* fightButton = new QPushButton("Fight!");
    fightButton->setDefault(true);
    connect(fightButton, SIGNAL(clicked(bool)), this, SIGNAL(startCombat()));
    layout->addWidget(fightButton);
}

bool ControlWidget::isLandBattle() const {
    return landBattle_->isChecked();
}

bool ControlWidget::isAmphibiousCombat() const {
    return amphibiousCombat_->isChecked();
}

bool ControlWidget::landUnitMustLive() const {
    return oneLandUnitMustSurvive_->isChecked();
}

OrderOfLoss ControlWidget::orderOfLoss() const {
    const QString& oolString = oolType_->currentText();
    if (oolString == OOLIPC)
        return OrderOfLossIPC;
    else /*if (oolString == OOLVALUE)*/
        return OrderOfLossValue;
}

void ControlWidget::landBattleCheckboxChanged(int state) {
    if (state == 0) { // not Land Battle
        oneLandUnitMustSurvive_->setDisabled(true);
        amphibiousCombat_->setDisabled(true);
        oneLandUnitOldValue_ = oneLandUnitMustSurvive_->isChecked();
        amphibiousOldValue_ = amphibiousCombat_->isChecked();
        oneLandUnitMustSurvive_->setChecked(false);
        amphibiousCombat_->setChecked(false);
    }
    else {
        oneLandUnitMustSurvive_->setDisabled(false);
        oneLandUnitMustSurvive_->setChecked(oneLandUnitOldValue_);
        amphibiousCombat_->setDisabled(false);
        amphibiousCombat_->setChecked(amphibiousOldValue_);
    }
}
