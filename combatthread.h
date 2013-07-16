#ifndef BOCK_COMBATTHREAD_H
#define BOCK_COMBATTHREAD_H

#include <QRunnable>

#include "unit.h"
#include <QList>
#include <QPair>

enum OrderOfLoss {
    OrderOfLossIPC,
    OrderOfLossValue
};

typedef QList<UnitLite> Batallion;

void applyCasualtyLand(Batallion& bat, Batallion& casBat, int casualties, bool isDefender, bool landUnitMustLive, bool needsSorting = true);
void applyCasualtySea(Batallion& bat, Batallion& casBat, int casualties, bool isDefender, bool needsSorting = true);

class CombatThread : public QRunnable {
public:
    CombatThread(const Batallion& attacker, const Batallion& defender, bool isLandBattle, bool isAmphibiousCombat, bool landUnitMustLive, OrderOfLoss ool, int seedHelper);

    void run();

    const Batallion& getAttacker() const;
    const Batallion& getAttackerCasualities() const;
    const Batallion& getDefender() const;
    const Batallion& getDefenderCasualities() const;

private:
    void runLandBattle();
    void runSeaBattle();

    Batallion attacker_;
    Batallion attackerCasualities_;
    Batallion defender_;
    Batallion defenderCasualities_;
    bool isLandBattle_;
    bool isAmphibiousCombat_;
    bool landUnitMustLive_;
    int seedHelper_;
};

#endif
