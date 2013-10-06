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

#include "unitwidget.h"

#include "factionwidget.h"
#include "unit.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPixMap>
#include <QSpinBox>

UnitWidget::UnitWidget(FactionWidget* parent, Unit* unit, const QPixmap& icon)
    : QWidget(parent)
    , _iconLabel(0)
    , _icon(icon)
    , _amount(0)
    , _unit(unit)
    , _parent(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setSpacing(0);

    _iconLabel = new QLabel;
    _iconLabel->setPixmap(_icon.scaledToWidth(40));
    _iconLabel->setMaximumSize(50, 50);
    layout->addWidget(_iconLabel);

    _amount = new QSpinBox;
    _amount->setButtonSymbols(QAbstractSpinBox::PlusMinus);
    layout->addWidget(_amount);

    setToolTip(unit->description());
}

QPair<Unit*, int> UnitWidget::getUnits() const {
    return QPair<Unit*, int>(_unit, _amount->value());
}

Unit* UnitWidget::getWidgetUnit() const {
    return _unit;
}

void UnitWidget::setAmount(int amount) {
    _amount->setValue(amount);
}
