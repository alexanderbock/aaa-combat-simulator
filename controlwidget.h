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

#ifndef BOCK_CONTROLWIDGET_H
#define BOCK_CONTROLWIDGET_H

#include <QWidget>
#include "combatthread.h"

class QCheckBox;
class QComboBox;

class ControlWidget : public QWidget {
Q_OBJECT
public:
    ControlWidget(QWidget* parent = 0);

    bool isLandBattle() const;
    bool isAmphibiousCombat() const;
    bool landUnitMustLive() const;
    OrderOfLoss orderOfLoss() const;
    
signals:
    void landBattleCheckboxDidChange();
    void switchSides();
    void startCombat();
    void clear();

private slots:
    void landBattleCheckboxChanged(int state);

private:
    QCheckBox* _landBattle;
    QCheckBox* _oneLandUnitMustSurvive;
    QCheckBox* _amphibiousCombat;
    QComboBox* _oolType;

    bool _oneLandUnitOldValue;
    bool _amphibiousOldValue;
};

#endif
