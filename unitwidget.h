#ifndef BOCK_UNITWIDGET_H
#define BOCK_UNITWIDGET_H

#include <QWidget>

#include <QPixmap>
#include "unit.h"

class FactionWidget;
class QLabel;
class QSpinBox;

class UnitWidget : public QWidget {
Q_OBJECT
public:
    UnitWidget(FactionWidget* parent, Unit* unit, const QPixmap& icon);

    Unit* getWidgetUnit() const;
    QPair<Unit*, int> getUnits() const;
    void setAmount(int amount);

private:
    QLabel* iconLabel_;
    QPixmap icon_;
    QSpinBox* amount_;

    Unit* unit_;
    FactionWidget* parent_;
};

#endif
