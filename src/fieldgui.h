#ifndef FIELDGUI_H
#define FIELDGUI_H

#include <QtCore/qglobal.h>
#if QT_VERSION >= 0x050000
#include <QtWidgets/QWidget>
#else
#include <QtGui/QWidget>
#endif
#include <QMap>
#include <QLabel>
#include "cell.h"

class QWidget;
class Field;
class QGridLayout;

class CellGui : public QLabel
{
    Q_OBJECT
    Cell::CPtr cell;
    QLayout* candidatesLayout;
    QMap<int, QLabel*> candidateLabel;
public:
    CellGui(Cell::CPtr cell, QWidget* parent = nullptr);
public slots:
    void setValue(CellValue );
    void removeCandidate(CellValue bit);

};

class FieldGui : public QWidget
{
    Q_OBJECT
    QGridLayout* layout;

    QMap<Cell*, QWidget*> cellWidgets;
public:
    explicit FieldGui(Field& field, QWidget *parent = nullptr);

signals:
private:
    void setCellWidgetValue(const Cell& cell);
};

#endif // FIELDGUI_H
