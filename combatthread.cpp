#include "combatthread.h"

OrderOfLoss ool_;

CombatThread::CombatThread(const Batallion& attacker, const Batallion& defender, bool isLandBattle, bool isAmphibiousCombat, bool landUnitMustLive, OrderOfLoss ool, int seedHelper)
    : QRunnable()
    , attacker_(attacker)
    , defender_(defender)
    , isLandBattle_(isLandBattle)
    , isAmphibiousCombat_(isAmphibiousCombat)
    , landUnitMustLive_(landUnitMustLive)
    , seedHelper_(seedHelper)
{
    ool_ = ool;
    setAutoDelete(false);
}

inline int getRoll() {
    return (qrand() % 6) + 1;
}

inline bool lessThanAttack(const UnitLite& p1, const UnitLite& p2) {
    if (p1.isTwoHit())
        return (!p1.isHit());
    
    if (p2.isTwoHit())
        return (p2.isHit());
    
    if (p1.hasTwoRolls() && !p2.hasTwoRolls())
        return true;

    if (p2.hasTwoRolls() && !p1.hasTwoRolls())
        return false;

    if (ool_ == OrderOfLossValue) {
        if (p1.getAttack() < p2.getAttack())
            return true;
        else if (p1.getAttack() > p2.getAttack())
            return false;
        else
            return (p1.getIPC() <= p2.getIPC());
    }
    else {
        if (p1.getIPC() < p2.getIPC())
            return true;
        else if (p1.getIPC() > p2.getIPC())
            return false;
        else
            return (p1.getAttack() <= p2.getAttack());
    }
}

inline bool lessThanDefense(const UnitLite& p1, const UnitLite& p2) {
    if (p1.isTwoHit())
        return (!p1.isHit());

    if (p2.isTwoHit())
        return (p2.isHit());
    
    if (p1.hasTwoRolls() && !p2.hasTwoRolls())
        return true;
    
    if (p2.hasTwoRolls() && !p1.hasTwoRolls())
        return false;

    if (ool_ == OrderOfLossValue) {
        if (p1.getDefense() < p2.getDefense())
            return true;
        else if (p1.getDefense() > p2.getDefense())
            return false;
        else
            return (p1.getIPC() <= p2.getIPC());
    }
    else {
        if (p1.getIPC() < p2.getIPC())
            return true;
        else if (p1.getIPC() > p2.getIPC())
            return false;
        else
            return (p1.getDefense() <= p2.getDefense());
    }
}

inline int getNumberOfSupporters(const Batallion& bat) {
    int result = 0;
    for (int i = 0; i < bat.size(); ++i) {
        if (bat[i].isArtillery()) {
            result += bat[i].getNumArtillery();
        }
    }
    return result;
}

inline bool hasOnlyOneLandUnit(Batallion& bat) {
    int count = 0;
    foreach (const UnitLite& unit, bat) {
        if (unit.isLand())
            count++;
        
        if (count > 1)
            return false;
    }
    return count == 1;
}

inline bool hasDestroyer(Batallion& bat) {
    foreach (const UnitLite& unit, bat) {
        if (unit.isDestroyer())
            return true;
    }
    return false;
}

void applyCasualtyLand(Batallion& bat, Batallion& casBat, int casualties, bool isDefender, bool landUnitMustLive, bool needsSorting) {
    if ((bat.size() == 0) || (casualties == 0))
        return;

    if (needsSorting) {
        if (isDefender)
            qSort(bat.begin(), bat.end(), lessThanDefense);
        else
            qSort(bat.begin(), bat.end(), lessThanAttack);
    }

    int index = 0;
    if (landUnitMustLive && bat[0].isLand() && (bat.count() > 1) && hasOnlyOneLandUnit(bat))
        index = 1;

    const UnitLite& unit = bat[index];

    if (unit.isTwoHit()) {
        if (unit.isHit()) {
            casBat.append(unit);
            bat.removeAt(index);
        }
        else {
            bat[index].setHit();
            applyCasualtyLand(bat, casBat, casualties - 1, isDefender, landUnitMustLive);
            return;
        }
    }
    else {
        casBat.append(unit);
        bat.removeAt(index);
    }
    applyCasualtyLand(bat, casBat, casualties - 1, isDefender, landUnitMustLive, false);
}

void applyCasualtySub(Batallion& bat, Batallion& casBat, int casualties, bool isDefender, bool needsSorting = true) {
    if ((bat.size() == 0) || (casualties == 0))
        return;
    
    if (needsSorting) {
        if (isDefender)
            qSort(bat.begin(), bat.end(), lessThanDefense);
        else
            qSort(bat.begin(), bat.end(), lessThanAttack);
    }
    
    for (int i = 0; i < bat.size(); ++i) {
        if (bat[i].isSea()) {
            if (bat[i].isTwoHit()) {
                if (bat[i].isHit()) {
                    const UnitLite& tmp = bat[i];
                    casBat.append(tmp);
                    bat.removeAt(i);
                    break;
                }
                else {
                    bat[i].setHit();
                    applyCasualtySub(bat, casBat, casualties - 1, isDefender);
                    return;
                }
            }
            else {
                const UnitLite& tmp = bat[i];
                casBat.append(tmp);
                bat.removeAt(i);
                break;
            }
        }
    }
    
    applyCasualtySub(bat, casBat, casualties - 1, isDefender, false);
}

void applyCasualtySea(Batallion& bat, Batallion& casBat, int casualties, bool isDefender, bool needsSorting) {
    if ((casualties == 0) || (bat.size() == 0))
        return;
    
    if (needsSorting) {
        if (isDefender)
            qSort(bat.begin(), bat.end(), lessThanDefense);
        else
            qSort(bat.begin(), bat.end(), lessThanAttack);
    }

    const UnitLite& unit = bat[0];
    
    if (unit.isTwoHit()) {
        if (unit.isHit()) {
            casBat.append(unit);
            bat.removeFirst();
        }
        else {
            bat[0].setHit();
            applyCasualtySea(bat, casBat, casualties - 1, isDefender);
            return;
        }
    }
    else {
        casBat.append(unit);
        bat.removeFirst();
    }
    applyCasualtySea(bat, casBat, casualties - 1, isDefender, false);    
}

void CombatThread::run() {
    qsrand(seedHelper_);

    if (isLandBattle_)
        runLandBattle();
    else
        runSeaBattle();     
}

void CombatThread::runLandBattle() {
    // AA fire
    for (int i = 0; i < defender_.size(); ++i) {
        const UnitLite& uDefender = defender_[i];
        if (uDefender.isAA()) {
            for (int j = 0; j < attacker_.size(); ++j) {
                const UnitLite& uAttacker = attacker_[j];
                if (uAttacker.isAir()) {
                    for (int k = 0; k < uDefender.getNumRolls(); ++k) {
                        int aaRoll = getRoll();
                        if (aaRoll <= uDefender.getDefense()) {
                            attackerCasualities_.append(uAttacker);
                            attacker_.removeAt(j--);
                        }
                    }
                }
            }
            defender_.removeAt(i--);
        }
    }

    // Bombardment
    for (int i = 0; i < attacker_.size(); ++i) {
        const UnitLite& uAtt = attacker_[i];

        if (uAtt.canBombard()) {
            for (int j = 0; j < uAtt.getNumRolls(); ++j) {
                int bombardRoll = getRoll();
                if (bombardRoll <= uAtt.getBombardmentValue())
                    applyCasualtyLand(defender_, defenderCasualities_, 1, true, false);
                    // Bombarded -> remove it
            }
            attacker_.removeAt(i--);
        }
    }

    // regular battle
    while ((attacker_.size() > 0) && (defender_.size() > 0)) {
        int attackerHits = 0;
        int defenderHits = 0;
        int supporter = getNumberOfSupporters(attacker_);

        foreach (const UnitLite& unit, attacker_) {
            for (int i = 0; i < unit.getNumRolls(); ++i) {
                int roll= getRoll();
                if (unit.isArtillerySupportable() && (supporter > 0)) {
                    roll--;
                    supporter--;
                }
                if (isAmphibiousCombat_ && unit.isMarine())
                    roll--;
                
                if (roll <= unit.getAttack())
                    attackerHits++;
            }
        }

        foreach (const UnitLite& unit, defender_) {
            for (int i = 0; i < unit.getNumRolls(); ++i) {
                int roll = getRoll();
                if (roll <= unit.getDefense())
                    defenderHits++;
            }
        }

        applyCasualtyLand(attacker_, attackerCasualities_, defenderHits, false, landUnitMustLive_);
        applyCasualtyLand(defender_, defenderCasualities_, attackerHits, true, false);
    }

}

void CombatThread::runSeaBattle() {
    while ((attacker_.size() > 0) && (defender_.size() > 0)) {
        int attackerSubHits = 0;
        int defenderSubHits = 0;
        int attackerHits = 0;
        int defenderHits = 0;
        int supporter = getNumberOfSupporters(attacker_);

        // Sub Fire
        foreach (const UnitLite& unit, attacker_) {
            if (unit.isSub()) {
                for (int i = 0; i < unit.getNumRolls(); ++i) {
                    int roll = getRoll();
                    if (unit.isArtillerySupportable() && (supporter > 0)) {
                        roll--;
                        supporter--;
                    }
                    if (roll <= unit.getAttack())
                        attackerSubHits++;
                }
            }
        }

        foreach (const UnitLite& unit, defender_) {
            if (unit.isSub()) {
                for (int i = 0; i < unit.getNumRolls(); ++i) {
                    int roll = getRoll();
                    if (roll <= unit.getDefense())
                        defenderSubHits++;
                }
            }
        }

        // if the attacker doesn't have destroyers, apply the casualties directly
        if (!hasDestroyer(attacker_)) {
            applyCasualtySub(attacker_, attackerCasualities_, defenderSubHits, false);
            defenderSubHits = 0;
        }

        // if the defender doesn't have destroyers, apply the casualites directly
        if (!hasDestroyer(defender_)) {
            applyCasualtySub(defender_, defenderCasualities_, attackerSubHits, true);
            attackerSubHits = 0;
        }

        foreach (const UnitLite& unit, attacker_) {
            if (!unit.isSub()) {
                for (int i = 0; i < unit.getNumRolls(); ++i) {
                    int roll = getRoll();
                    if (unit.isArtillerySupportable() && (supporter > 0)) {
                        roll--;
                        supporter--;
                    }

                    if (roll <= unit.getAttack())
                        attackerHits++;
                }
            }
        }

        foreach (const UnitLite& unit, defender_) {
            if (!unit.isSub()) {
                for (int i = 0; i < unit.getNumRolls(); ++i) {
                    int roll = getRoll();
                    if (roll <= unit.getDefense())
                        defenderHits++;
                }
            }
        }

        applyCasualtySub(attacker_, attackerCasualities_, defenderSubHits, false);
        applyCasualtySea(attacker_, attackerCasualities_, defenderHits, false);
        applyCasualtySub(defender_, defenderCasualities_, attackerSubHits, true);
        applyCasualtySea(defender_, defenderCasualities_, attackerHits, true);
    }   
}

const Batallion& CombatThread::getAttacker() const {
    return attacker_;
}

const Batallion& CombatThread::getAttackerCasualities() const {
    return attackerCasualities_;
}

const Batallion& CombatThread::getDefender() const {
    return defender_;
}

const Batallion& CombatThread::getDefenderCasualities() const {
    return defenderCasualities_;
}
