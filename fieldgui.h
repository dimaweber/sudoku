#ifndef FIELDGUI_H
#define FIELDGUI_H

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif

class Field;
class QGridLayout;

class FieldGui : public QWidget
{
    Q_OBJECT
    QGridLayout* layout;
public:
    explicit FieldGui(Field& field, QWidget *parent = nullptr);

signals:

public slots:
};

#endif // FIELDGUI_H
