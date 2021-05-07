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
    QBrush backgroundBrush;
    QBrush hightlightBrush;
public:
    CellGui(Cell::CPtr cell, QWidget* parent = nullptr);

    void highlightOn();
    void highlightOff();

    void resetCandidates();
    
public slots:
    void setValue(CellValue );
    void removeCandidate(CellValue bit);
    void onCellReset();
};

class FieldGui : public QWidget
{
    Q_OBJECT
    QGridLayout* layout;

    QMap<Cell::Ptr, CellGui*> cellWidgets;
public:
    explicit FieldGui(Field& field, QWidget *parent = nullptr);
public slots:
    void highlightCellOn(Cell::Ptr );
    void highlightCellOff(Cell::Ptr );
signals:
private:
    void setCellWidgetValue(const Cell& cell);
};

#endif // FIELDGUI_H
