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

#include "combatthread.h"

OrderOfLoss _ool;

CombatThread::CombatThread(const Batallion& attacker, const Batallion& defender, bool isLandBattle, bool isAmphibiousCombat, bool landUnitMustLive, OrderOfLoss ool, int seedHelper)
    : QRunnable()
    , _attacker(attacker)
    , _defender(defender)
    , _isLandBattle(isLandBattle)
    , _isAmphibiousCombat(isAmphibiousCombat)
    , _landUnitMustLive(landUnitMustLive)
    , _seedHelper(seedHelper)
{
    _ool = ool;
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

    if (_ool == OrderOfLossValue) {
        if (p1.attackValue() < p2.attackValue())
            return true;
        else if (p1.attackValue() > p2.attackValue())
            return false;
        else
            return (p1.ipcValue() <= p2.ipcValue());
    }
    else {
        if (p1.ipcValue() < p2.ipcValue())
            return true;
        else if (p1.ipcValue() > p2.ipcValue())
            return false;
        else
            return (p1.attackValue() <= p2.attackValue());
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

    if (_ool == OrderOfLossValue) {
        if (p1.defenseValue() < p2.defenseValue())
            return true;
        else if (p1.defenseValue() > p2.defenseValue())
            return false;
        else
            return (p1.ipcValue() <= p2.ipcValue());
    }
    else {
        if (p1.ipcValue() < p2.ipcValue())
            return true;
        else if (p1.ipcValue() > p2.ipcValue())
            return false;
        else
            return (p1.defenseValue() <= p2.defenseValue());
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
    qsrand(_seedHelper);

    if (_isLandBattle)
        runLandBattle();
    else
        runSeaBattle();     
}

void CombatThread::runLandBattle() {
    // AA fire
    for (int i = 0; i < _defender.size(); ++i) {
        const UnitLite& uDefender = _defender[i];
        if (uDefender.isAA()) {
            for (int j = 0; j < _attacker.size(); ++j) {
                const UnitLite& uAttacker = _attacker[j];
                if (uAttacker.isAir()) {
                    for (int k = 0; k < uDefender.numRolls(); ++k) {
                        int aaRoll = getRoll();
                        if (aaRoll <= uDefender.defenseValue()) {
                            _attackerCasualities.append(uAttacker);
                            _attacker.removeAt(j--);
                        }
                    }
                }
            }
            _defender.removeAt(i--);
        }
    }

    // Bombardment
    for (int i = 0; i < _attacker.size(); ++i) {
        const UnitLite& uAtt = _attacker[i];

        if (uAtt.canBombard()) {
            for (int j = 0; j < uAtt.numRolls(); ++j) {
                int bombardRoll = getRoll();
                if (bombardRoll <= uAtt.bombardmentValue())
                    applyCasualtyLand(_defender, _defenderCasualities, 1, true, false);
                    // Bombarded -> remove it
            }
            _attacker.removeAt(i--);
        }
    }

    // regular battle
    while ((_attacker.size() > 0) && (_defender.size() > 0)) {
        int attackerHits = 0;
        int defenderHits = 0;
        int supporter = getNumberOfSupporters(_attacker);

        foreach (const UnitLite& unit, _attacker) {
            for (int i = 0; i < unit.numRolls(); ++i) {
                int roll = getRoll();
                if (unit.isArtillerySupportable() && (supporter > 0)) {
                    --roll;
                    --supporter;
                }
                if (_isAmphibiousCombat && unit.isMarine())
                    --roll;
                
                if (roll <= unit.attackValue())
                    ++attackerHits;
            }
        }

        foreach (const UnitLite& unit, _defender) {
            for (int i = 0; i < unit.numRolls(); ++i) {
                int roll = getRoll();
                if (roll <= unit.defenseValue())
                    ++defenderHits;
            }
        }

        applyCasualtyLand(_attacker, _attackerCasualities, defenderHits, false, _landUnitMustLive);
        applyCasualtyLand(_defender, _defenderCasualities, attackerHits, true, false);
    }

}

void CombatThread::runSeaBattle() {
    while ((_attacker.size() > 0) && (_defender.size() > 0)) {
        int attackerSubHits = 0;
        int defenderSubHits = 0;
        int attackerHits = 0;
        int defenderHits = 0;
        int supporter = getNumberOfSupporters(_attacker);

        // Sub Fire
        foreach (const UnitLite& unit, _attacker) {
            if (unit.isSub()) {
                for (int i = 0; i < unit.numRolls(); ++i) {
                    int roll = getRoll();
                    if (unit.isArtillerySupportable() && (supporter > 0)) {
                        --roll;
                        --supporter;
                    }
                    if (roll <= unit.attackValue())
                        ++attackerSubHits;
                }
            }
        }

        foreach (const UnitLite& unit, _defender) {
            if (unit.isSub()) {
                for (int i = 0; i < unit.numRolls(); ++i) {
                    int roll = getRoll();
                    if (roll <= unit.defenseValue())
                        ++defenderSubHits;
                }
            }
        }

        // if the attacker doesn't have destroyers, apply the casualties directly
        if (!hasDestroyer(_attacker)) {
            applyCasualtySub(_attacker, _attackerCasualities, defenderSubHits, false);
            defenderSubHits = 0;
        }

        // if the defender doesn't have destroyers, apply the casualites directly
        if (!hasDestroyer(_defender)) {
            applyCasualtySub(_defender, _defenderCasualities, attackerSubHits, true);
            attackerSubHits = 0;
        }

        foreach (const UnitLite& unit, _attacker) {
            if (!unit.isSub()) {
                for (int i = 0; i < unit.numRolls(); ++i) {
                    int roll = getRoll();
                    if (unit.isArtillerySupportable() && (supporter > 0)) {
                        --roll;
                        --supporter;
                    }

                    if (roll <= unit.attackValue())
                        ++attackerHits;
                }
            }
        }

        foreach (const UnitLite& unit, _defender) {
            if (!unit.isSub()) {
                for (int i = 0; i < unit.numRolls(); ++i) {
                    int roll = getRoll();
                    if (roll <= unit.defenseValue())
                        ++defenderHits;
                }
            }
        }

        applyCasualtySub(_attacker, _attackerCasualities, defenderSubHits, false);
        applyCasualtySea(_attacker, _attackerCasualities, defenderHits, false);
        applyCasualtySub(_defender, _defenderCasualities, attackerSubHits, true);
        applyCasualtySea(_defender, _defenderCasualities, attackerHits, true);
    }   
}

const Batallion& CombatThread::getAttacker() const {
    return _attacker;
}

const Batallion& CombatThread::getAttackerCasualities() const {
    return _attackerCasualities;
}

const Batallion& CombatThread::getDefender() const {
    return _defender;
}

const Batallion& CombatThread::getDefenderCasualities() const {
    return _defenderCasualities;
}
