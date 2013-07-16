#include "focusspinbox.h"

#include <QLineEdit>

FocusSpinBox::FocusSpinBox(QWidget* parent)
    : QSpinBox(parent)
{
    connect(lineEdit(), SIGNAL(editingFinished()), this, SIGNAL(editingFinished()));
}
