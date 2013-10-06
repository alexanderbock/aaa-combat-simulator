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

    FactionWidget* _attackerWidget;
    FactionWidget* _defenderWidget;
    ControlWidget* _controlWidget;
    
    QBoxLayout* _attackerLayout;
    QString _directory;
    QList<Unit*> _units;
    QMap<int, Unit*> _idMap;
    QMap<QString, QStringList> _factionsDetail;
    QStringList _factions;
    int _ipcFactor;
};

#endif
