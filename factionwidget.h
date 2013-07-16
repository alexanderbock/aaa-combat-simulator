#ifndef BOCK_FACTIONWIDGET_H
#define BOCK_FACTIONWIDGET_H

#include <QGroupBox>

#include "unit.h"

class CombatThread;
class CombatWidget;
class QComboBox;
class QGridLayout;
class QLabel;
class UnitWidget;

enum FactionSide {
    FactionSideAttacker,
    FactionSideDefender
};

class InformationWidget;

class DetailedInformationWidget : public QWidget{
public:
    DetailedInformationWidget(QWidget* parent);

};

class InformationWidget : public QGroupBox {
Q_OBJECT
public:
    InformationWidget(QWidget* parent);
    void setResult(QList<CombatThread*>* results, float winPercentage, float drawPercentage, bool doesWin,
        float averageUnitLeft, int totalUnitsAtStart, float averageIPCLoss);
    void clearResults();

private slots:
    void detailedInformationToggeled(bool);

private:
    QLabel* winResult_;
    QLabel* drawResult_;
    QLabel* unitLeft_;
    QLabel* ipcLoss_;

    DetailedInformationWidget* detailedInformationWidget_;
};

class FactionWidget : public QGroupBox {
Q_OBJECT
public:
    FactionWidget(CombatWidget* parent, const QString& title, FactionSide side);

    QList<QPair<Unit*, int> > getUnits() const;
    void setUnits(const QList<QPair<Unit*, int> >& units);
    QString getFaction() const;
    void setFaction(const QString& faction);
    void clear();

    void setResults(QList<CombatThread*>* results, float winPercentage, float drawPercentage, bool doesWin,
        float averageUnitLeft, int totalUnitsAtStart, float averageIPCLoss);
    void clearResults();

private slots:
    void recreateUnits();

private:
    void initFactions();
    void createUnits();
    void createUnits(const QString& faction);
    void deleteUnits();

    QGridLayout* unitsLayout_;
    QComboBox* factionBox_;
    CombatWidget* parent_;
    FactionSide side_;
    InformationWidget* infoWidget_;
    
    QList<UnitWidget*> unitWidgets_;
};

#endif
