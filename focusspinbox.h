#ifndef BOCK_FOCUSSPINBOX_H
#define BOCK_FOCUSSPINBOX_H

#include <QSpinBox>

class FocusSpinBox : public QSpinBox {
Q_OBJECT
public:
    FocusSpinBox(QWidget* parent = 0);

signals:
    void editingFinished();
};

#endif
