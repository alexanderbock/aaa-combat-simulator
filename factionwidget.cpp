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
    , winResult_(0)
    , drawResult_(0)
    , unitLeft_(0)
    , ipcLoss_(0)
    , detailedInformationWidget_(0)
{
    detailedInformationWidget_ = new DetailedInformationWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* line1 = new QHBoxLayout;
    QLabel* winLabel = new QLabel("Win:");
    line1->addWidget(winLabel);
    winResult_ = new QLabel("");
    line1->addWidget(winResult_);
    line1->addStretch(-1);
    QLabel* drawLabel = new QLabel(":Draw");
    drawResult_ = new QLabel("");
    line1->addWidget(drawResult_);
    line1->addWidget(drawLabel);
    mainLayout->addLayout(line1);

    QHBoxLayout* line2 = new QHBoxLayout;
    QLabel* unitLabel = new QLabel("Units left:");
    line2->addWidget(unitLabel);
    unitLeft_ = new QLabel("");
    line2->addWidget(unitLeft_);
    line2->addStretch(-1);
    QLabel* ipcLabel = new QLabel(":IPC Loss");
    ipcLoss_ = new QLabel("");
    line2->addWidget(ipcLoss_);
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
        winResult_->setText("<font color=#00AA00>" + win + "</font>");
    else
        winResult_->setText("<font color=#AA0000>" + win + "</font>");

    drawResult_->setText(draw);

    unitLeft_->setText(QString::number(averageUnitLeft) + " (" + QString::number(static_cast<float>(totalUnitsAtStart) - averageUnitLeft) + " loss)");
    ipcLoss_->setText(QString::number(averageIPCLoss));
}

void InformationWidget::clearResults() {
    winResult_->setText("");
    drawResult_->setText("");
    unitLeft_->setText("");
    ipcLoss_->setText("");
}

void InformationWidget::detailedInformationToggeled(bool b) {
    detailedInformationWidget_->setVisible(b);
}


FactionWidget::FactionWidget(CombatWidget* parent, const QString& title, FactionSide side)
    : QGroupBox(parent)
    , unitsLayout_(0)
    , factionBox_(0)
    , parent_(parent)
    , side_(side)
    , infoWidget_(0)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* titleLabel = new QLabel(title);
    layout->addWidget(titleLabel);

    factionBox_ = new QComboBox;
    initFactions();
    connect(factionBox_, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(recreateUnits()));
    layout->addWidget(factionBox_);

    QGroupBox* unitsGroupBox = new QGroupBox;
    QScrollArea* unitsScrollArea = new QScrollArea;
    unitsLayout_ = new QGridLayout(unitsGroupBox);
    createUnits();
    unitsScrollArea->setMinimumWidth(300);
    unitsScrollArea->setWidgetResizable(true);
    unitsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    unitsLayout_->setVerticalSpacing(0);
    unitsScrollArea->setWidget(unitsGroupBox);
    unitsGroupBox->setLayout(unitsLayout_);
    layout->addWidget(unitsScrollArea);

    infoWidget_ = new InformationWidget(this);
    infoWidget_->setEnabled(false);
    layout->addWidget(infoWidget_);
}

void FactionWidget::initFactions() {
    QStringList factions = parent_->getFactions();

    foreach (const QString& faction, factions)
        factionBox_->addItem(parent_->getFlag(faction), faction);

    factionBox_->insertSeparator(factionBox_->count());

    QStringList groups = parent_->getGroups();
    foreach (const QString& group, groups) {
        factionBox_->addItem(group);
    }
}

void FactionWidget::createUnits() {
    const QString& faction = factionBox_->currentText();
    if (parent_->getGroups().contains(faction)) {
        QStringList factions = parent_->getFactionsForGroup(faction);
        foreach (const QString& f, factions)
            createUnits(f);
    }
    else
        createUnits(faction);
}

void FactionWidget::createUnits(const QString& faction) {
    QList<Unit*> units = parent_->getUnits();

    foreach (Unit* unit, units) {
        QPixmap pm = parent_->getUnitIcon(faction, unit->getID());
        if (pm.isNull())
            continue;

        if ((side_ == FactionSideAttacker) && (unit->getAttack() == 0) && !unit->canAttack())
            continue;

        if ((side_ == FactionSideDefender) && (unit->getDefense() == 0))
            continue;

        if (parent_->isLandBattle()) {
            if (unit->isSea()) {
                if (side_ == FactionSideDefender)
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
        unitsLayout_->addWidget(unitWidget, unitWidgets_.size() / 2, unitWidgets_.size() % 2);
        unitWidgets_.append(unitWidget);
    }
}

void FactionWidget::deleteUnits() {
    foreach (UnitWidget* unitWidget, unitWidgets_) {
        unitsLayout_->removeWidget(unitWidget);
        delete unitWidget;
    }
    unitWidgets_.clear();
}

void FactionWidget::recreateUnits() {
    deleteUnits();
    createUnits();
}

QList<QPair<Unit*, int> > FactionWidget::getUnits() const {
    QList<QPair<Unit*, int> > result;
    foreach (UnitWidget* unitWidget, unitWidgets_) {
        QPair<Unit*, int> units = unitWidget->getUnits();
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
        foreach (UnitWidget* w, unitWidgets_) {
            if (w->getWidgetUnit() == u) {
                w->setAmount(n);
                break;
            }
        }
    }
}

void FactionWidget::clear() {
    foreach (UnitWidget* unitWidget, unitWidgets_) {
        unitWidget->setAmount(0);
    }
}


QString FactionWidget::getFaction() const {
    return factionBox_->currentText();
}

void FactionWidget::setFaction(const QString& faction) {
    int index = factionBox_->findText(faction);
    factionBox_->setCurrentIndex(index);
}

void FactionWidget::setResults(QList<CombatThread*>* results, float winPercentage, float drawPercentage,
                               bool doesWin, float averageUnitLeft, int totalUnitsAtStart, float averageIPCLoss)
{
    infoWidget_->setEnabled(true);
    infoWidget_->setResult(results, winPercentage, drawPercentage,
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
    infoWidget_->clearResults();
}
