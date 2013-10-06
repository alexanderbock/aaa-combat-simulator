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

    const Batallion& attacker() const;
    const Batallion& attackerCasualities() const;
    const Batallion& defender() const;
    const Batallion& defenderCasualities() const;

private:
    void runLandBattle();
    void runSeaBattle();

    Batallion _attacker;
    Batallion _attackerCasualities;
    Batallion _defender;
    Batallion _defenderCasualities;
    bool _isLandBattle;
    bool _isAmphibiousCombat;
    bool _landUnitMustLive;
    int _seedHelper;
};

#endif
