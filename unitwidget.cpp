#include "unitwidget.h"

#include "factionwidget.h"
#include "unit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixMap>
#include <QSpinBox>

UnitWidget::UnitWidget(FactionWidget* parent, Unit* unit, const QPixmap& icon)
    : QWidget(parent)
    , iconLabel_(0)
    , icon_(icon)
    , amount_(0)
    , unit_(unit)
    , parent_(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);

    iconLabel_ = new QLabel;
    iconLabel_->setPixmap(icon_.scaledToWidth(40));
    iconLabel_->setMaximumSize(50, 50);
    layout->addWidget(iconLabel_);

    amount_ = new QSpinBox;
    amount_->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    layout->addWidget(amount_);

    setToolTip(unit->getDescription());
}

QPair<Unit*, int> UnitWidget::getUnits() const {
    return QPair<Unit*, int>(unit_, amount_->value());
}

Unit* UnitWidget::getWidgetUnit() const {
    return unit_;
}

void UnitWidget::setAmount(int amount) {
    amount_->setValue(amount);
}
