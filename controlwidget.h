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
    QCheckBox* landBattle_;
    QCheckBox* oneLandUnitMustSurvive_;
    QCheckBox* amphibiousCombat_;
    QComboBox* oolType_;

    bool oneLandUnitOldValue_;
    bool amphibiousOldValue_;
};

#endif
