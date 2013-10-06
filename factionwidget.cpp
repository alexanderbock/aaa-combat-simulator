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

#include "factionwidget.h"

#include "combatwidget.h"
#include "unit.h"
#include "unitwidget.h"

#include <QComboBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>


DetailedInformationWidget::DetailedInformationWidget(QWidget* parent)
    : QWidget(parent)
{
}

InformationWidget::InformationWidget(QWidget* parent)
    : QGroupBox(parent)
    , _winResult(nullptr)
    , _drawResult(nullptr)
    , _unitLeft(nullptr)
    , _ipcLoss(nullptr)
    , _detailedInformationWidget(nullptr)
{
    _detailedInformationWidget = new DetailedInformationWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* line1 = new QHBoxLayout;
    QLabel* winLabel = new QLabel("Win:");
    line1->addWidget(winLabel);
    _winResult = new QLabel("");
    line1->addWidget(_winResult);
    line1->addStretch(-1);
    QLabel* drawLabel = new QLabel(":Draw");
    _drawResult = new QLabel("");
    line1->addWidget(_drawResult);
    line1->addWidget(drawLabel);
    mainLayout->addLayout(line1);

    QHBoxLayout* line2 = new QHBoxLayout;
    QLabel* unitLabel = new QLabel("Units left:");
    line2->addWidget(unitLabel);
    _unitLeft = new QLabel("");
    line2->addWidget(_unitLeft);
    line2->addStretch(-1);
    QLabel* ipcLabel = new QLabel(":IPC Loss");
    _ipcLoss = new QLabel("");
    line2->addWidget(_ipcLoss);
    line2->addWidget(ipcLabel);
    mainLayout->addLayout(line2);

    QPushButton* detailedInformationButton = new QPushButton("Detailed Information");
    detailedInformationButton->setCheckable(true);
    connect(detailedInformationButton, SIGNAL(clicked(bool)), this, SLOT(detailedInformationToggeled(bool)));
    mainLayout->addWidget(detailedInformationButton);
}

void InformationWidget::setResult(QList<CombatThread*>* results, float winPercentage, float drawPercentage,
                                  bool doesWin, float averageUnitLeft, int totalUnitsAtStart, float averageIPCLoss)
{
    QString win = QString::number(winPercentage * 100) + "%";
    QString draw = QString::number(drawPercentage * 100) + "%";

    if (doesWin)
        _winResult->setText("<font color=#00AA00>" + win + "</font>");
    else
        _winResult->setText("<font color=#AA0000>" + win + "</font>");

    _drawResult->setText(draw);

    _unitLeft->setText(QString::number(averageUnitLeft) + " (" + QString::number(static_cast<float>(totalUnitsAtStart) - averageUnitLeft) + " loss)");
    _ipcLoss->setText(QString::number(averageIPCLoss));
}

void InformationWidget::clearResults() {
    _winResult->setText("");
    _drawResult->setText("");
    _unitLeft->setText("");
    _ipcLoss->setText("");
}

void InformationWidget::detailedInformationToggeled(bool b) {
    _detailedInformationWidget->setVisible(b);
}


FactionWidget::FactionWidget(CombatWidget* parent, const QString& title, FactionSide side)
    : QGroupBox(parent)
    , _unitsLayout(nullptr)
    , _factionBox(nullptr)
    , _parent(parent)
    , _side(side)
    , _infoWidget(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel(title);
    layout->addWidget(titleLabel);

    _factionBox = new QComboBox;
    initFactions();
    connect(_factionBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(recreateUnits()));
    layout->addWidget(_factionBox);

    QGroupBox* unitsGroupBox = new QGroupBox;
    QScrollArea* unitsScrollArea = new QScrollArea;
    _unitsLayout = new QGridLayout(unitsGroupBox);
    createUnits();
    unitsScrollArea->setMinimumWidth(300);
    unitsScrollArea->setWidgetResizable(true);
    unitsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _unitsLayout->setVerticalSpacing(0);
    unitsScrollArea->setWidget(unitsGroupBox);
    unitsGroupBox->setLayout(_unitsLayout);
    layout->addWidget(unitsScrollArea);

    _infoWidget = new InformationWidget(this);
    _infoWidget->setEnabled(false);
    layout->addWidget(_infoWidget);
}

void FactionWidget::initFactions() {
    QStringList factions = _parent->factions();

    foreach (const QString& faction, factions)
        _factionBox->addItem(_parent->flag(faction), faction);

    _factionBox->insertSeparator(_factionBox->count());

    QStringList groups = _parent->groups();
    foreach (const QString& group, groups) {
        _factionBox->addItem(group);
    }
}

void FactionWidget::createUnits() {
    const QString& faction = _factionBox->currentText();
    if (_parent->groups().contains(faction)) {
        QStringList factions = _parent->factionsForGroup(faction);
        foreach (const QString& f, factions)
            createUnits(f);
    }
    else
        createUnits(faction);
}

void FactionWidget::createUnits(const QString& faction) {
    QList<Unit*> units = _parent->units();

    foreach (Unit* unit, units) {
        QPixmap pm = _parent->unitIcon(faction, unit->id());
        if (pm.isNull())
            continue;

        if ((_side == FactionSideAttacker) && (unit->attackValue() == 0) && !unit->canAttack())
            continue;

        if ((_side == FactionSideDefender) && (unit->defenseValue() == 0))
            continue;

        if (_parent->isLandBattle()) {
            if (unit->isSea()) {
                if (_side == FactionSideDefender)
                    continue;
            else if (!unit->canBombard())
                continue;
            }
        }
        else {
            if (!unit->isSea() && !unit->isAir())
                continue;
        }

        UnitWidget* unitWidget = new UnitWidget(this, unit, pm);
        _unitsLayout->addWidget(unitWidget, _unitWidgets.size() / 2, _unitWidgets.size() % 2);
        _unitWidgets.append(unitWidget);
    }
}

void FactionWidget::deleteUnits() {
    foreach (UnitWidget* unitWidget, _unitWidgets) {
        _unitsLayout->removeWidget(unitWidget);
        delete unitWidget;
    }
    _unitWidgets.clear();
}

void FactionWidget::recreateUnits() {
    deleteUnits();
    createUnits();
}

QList<QPair<Unit*, int> > FactionWidget::getUnits() const {
    QList<QPair<Unit*, int> > result;
    foreach (UnitWidget* unitWidget, _unitWidgets) {
        QPair<Unit*, int> units = unitWidget->units();
        if (units.second > 0)
            result.append(units);
    }
    return result;
}

void FactionWidget::setUnits(const QList<QPair<Unit*, int> >& units) {
    QPair<Unit*, int> p;
    foreach (p, units) {
        Unit* u = p.first;
        int n = p.second;
        foreach (UnitWidget* w, _unitWidgets) {
            if (w->widgetUnit() == u) {
                w->setAmount(n);
                break;
            }
        }
    }
}

void FactionWidget::clear() {
    foreach (UnitWidget* unitWidget, _unitWidgets) {
        unitWidget->setAmount(0);
    }
}


QString FactionWidget::faction() const {
    return _factionBox->currentText();
}

void FactionWidget::setFaction(const QString& faction) {
    int index = _factionBox->findText(faction);
    _factionBox->setCurrentIndex(index);
}

void FactionWidget::setResults(QList<CombatThread*>* results, float winPercentage, float drawPercentage,
                               bool doesWin, float averageUnitLeft, int totalUnitsAtStart, float averageIPCLoss)
{
    _infoWidget->setEnabled(true);
    _infoWidget->setResult(results, winPercentage, drawPercentage,
        doesWin, averageUnitLeft, totalUnitsAtStart,averageIPCLoss);
    //QString win = QString::number(winPercentage * 100) + "%";
    //QString draw = QString::number(drawPercentage * 100) + "%";
    //
    //if (doesWin)
    //    winResult_->setText("<font color=#00AA00>" + win + "</font>");
    //else
    //    winResult_->setText("<font color=#AA0000>" + win + "</font>");
    //
    //drawResult_->setText(draw);
    //
    //unitLeft_->setText(QString::number(averageUnitLeft) + " (" + QString::number(static_cast<float>(totalUnitsAtStart) - averageUnitLeft) + " loss)");
    //ipcLoss_->setText(QString::number(averageIPCLoss));
}

void FactionWidget::clearResults() {
    _infoWidget->clearResults();
}
