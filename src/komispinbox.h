/*
 * komispinbox.h
 */

#ifndef KOMISPINBOX_H
#define KOMISPINBOX_H

#include <stdlib.h>

#include <qspinbox.h>

class KomiSpinBox : public QDoubleSpinBox
{
public:
    KomiSpinBox(QWidget *parent = 0) : QDoubleSpinBox(parent)
    {
        setRange(-500, 500);
        setValue(6.5);
        setSingleStep(1.0);
        setDecimals(1);
    }
};

#endif
