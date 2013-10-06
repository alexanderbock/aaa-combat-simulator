/**************************************************************************************************
 *                                                                                                *
 * AAA Combat Simulator                                                                           *
 *                                                                                                *
 * Copyright (c) 2011 Alexander Bock                                                              *
 *                                                                                                *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software  *
 * and associated documentation files (the "Software"), to deal in the Software without           *
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,     *
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the  *
 * Software is furnished to do so, subject to the following conditions:                           *
 *                                                                                                *
 * The above copyright notice and this permission notice shall be included in all copies or       *
 * substantial portions of the Software.                                                          *
 *                                                                                                *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING  *
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND     *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,   *
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, *
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 *                                                                                                *
 *************************************************************************************************/

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
    , _landBattle(nullptr)
    , _oneLandUnitMustSurvive(nullptr)
    , _amphibiousCombat(nullptr)
    , _oolType(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    _oneLandUnitMustSurvive = new QCheckBox("One attacking land\nunit must live");
    _amphibiousCombat = new QCheckBox("Amphibious Combat");

    _landBattle = new QCheckBox("Land Battle");
    connect(_landBattle, SIGNAL(stateChanged(int)), this, SLOT(landBattleCheckboxChanged(int)));
    connect(_landBattle, SIGNAL(stateChanged(int)), this, SIGNAL(landBattleCheckboxDidChange()));
    _landBattle->setChecked(true);
    layout->addWidget(_landBattle);
    
    layout->addWidget(_amphibiousCombat);
    
    layout->addWidget(_oneLandUnitMustSurvive);

    layout->addStretch(-1);

    QLabel* oolTypeLabel = new QLabel("Order of loss");
    layout->addWidget(oolTypeLabel);
    _oolType = new QComboBox;
    _oolType->addItem(OOLVALUE);
    _oolType->addItem(OOLIPC);
    _oolType->setToolTip("By Combat Value: First the unit with the lesser attack/defense value is chosen. Default in TripleA\nBy IPC: First the cheaper unit is taken as casualty");
    layout->addWidget(_oolType);

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
    return _landBattle->isChecked();
}

bool ControlWidget::isAmphibiousCombat() const {
    return _amphibiousCombat->isChecked();
}

bool ControlWidget::landUnitMustLive() const {
    return _oneLandUnitMustSurvive->isChecked();
}

OrderOfLoss ControlWidget::orderOfLoss() const {
    const QString& oolString = _oolType->currentText();
    if (oolString == OOLIPC)
        return OrderOfLossIPC;
    else /*if (oolString == OOLVALUE)*/
        return OrderOfLossValue;
}

void ControlWidget::landBattleCheckboxChanged(int state) {
    if (state == 0) { // not Land Battle
        _oneLandUnitMustSurvive->setDisabled(true);
        _amphibiousCombat->setDisabled(true);
        _oneLandUnitOldValue = _oneLandUnitMustSurvive->isChecked();
        _amphibiousOldValue = _amphibiousCombat->isChecked();
        _oneLandUnitMustSurvive->setChecked(false);
        _amphibiousCombat->setChecked(false);
    }
    else {
        _oneLandUnitMustSurvive->setDisabled(false);
        _oneLandUnitMustSurvive->setChecked(_oneLandUnitOldValue);
        _amphibiousCombat->setDisabled(false);
        _amphibiousCombat->setChecked(_amphibiousOldValue);
    }
}
