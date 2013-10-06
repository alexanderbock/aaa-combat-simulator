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
    QLabel* _winResult;
    QLabel* _drawResult;
    QLabel* _unitLeft;
    QLabel* _ipcLoss;

    DetailedInformationWidget* _detailedInformationWidget;
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

    QGridLayout* _unitsLayout;
    QComboBox* _factionBox;
    CombatWidget* _parent;
    FactionSide _side;
    InformationWidget* _infoWidget;
    
    QList<UnitWidget*> _unitWidgets;
};

#endif
