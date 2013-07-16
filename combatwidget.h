#ifndef BOCK_COMBATWIDGET_H
#define BOCK_COMBATWIDGET_H

#include <QWidget>

#include <QDomNode>
#include <QMap>
#include "combatthread.h"
#include "unit.h"

class ControlWidget;
class FactionWidget;
class QBoxLayout;

class CombatWidget : public QWidget {
Q_OBJECT
public:
    CombatWidget(const QString& directory, QWidget* parent = 0);
    ~CombatWidget();
    const QString& getDirectory() const;

    QStringList getFactions() const;
    QStringList getGroups() const;
    QStringList getFactionsForGroup(const QString& group) const;
    QList<Unit*> getUnits() const;
    QIcon getFlag(const QString& faction) const;
    QPixmap getUnitIcon(const QString& faction, int unitID) const;

    bool isLandBattle() const;
    bool isAmphibiousCombat() const;
    bool landUnitMustLive() const;
    OrderOfLoss orderOfLoss() const;

    QString getNameForID(int id) const;

private slots:
    void switchCombatSides();
    void startCombat();
    void clear();

private:
    struct CombatResult {
        float attackerWins;
        float defenderWins;
        float draw;
        float averageAttackerIPC;
        float averageAttackerUnit;
        float averageDefenderIPC;
        float averageDefenderUnit;
    };

    void initXML(const QString& xmlFile);
    QList<CombatThread*> startCombat(const QList<UnitLite>& attackerUnits, const QList<UnitLite>& defenderUnits);
    CombatResult computeCombatResults(const QList<CombatThread*>& results);

    FactionWidget* attackerWidget_;
    FactionWidget* defenderWidget_;
    ControlWidget* controlWidget_;
    
    QBoxLayout* attackerLayout_;
    QString directory_;
    QList<Unit*> units_;
    QMap<int, Unit*> idMap_;
    QMap<QString, QStringList> factionsDetail_;
    QStringList factions_;
    int ipcFactor_;
};

#endif
