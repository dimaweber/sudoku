#include "fieldgui.h"
#include "field.h"
#include <QtMath>

#include <QGridLayout>
#include <QLabel>
#include <QFont>
#include <QColor>
#include <QPalette>

#define FONT_SIZE 64

FieldGui::FieldGui(Field& field, QWidget* parent)
    :QWidget (parent)
{
    layout = new QGridLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    for (quint8 i=1; i<=field.getN(); i++)
    {
        QLabel* htitle = new QLabel(this);
        htitle->setText(QString("R%1").arg(i));

        QLabel* vtitle = new QLabel(this);
        vtitle->setText(QString("C%1").arg(i));

        vtitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(vtitle, 0, i, Qt::AlignBottom | Qt::AlignHCenter);

        htitle->setAlignment(Qt::AlignCenter);
        layout->addWidget(htitle, i, 0, Qt::AlignRight | Qt::AlignVCenter);
    }
    for (Coord coord=Coord::first(); coord.isValid(); coord++)
    {
        Cell::Ptr cell = field.cell(coord);
        CellGui* widget = new CellGui(cell, this);
        cellWidgets[cell] = widget;
        layout->addWidget(widget, coord.row(), coord.col(), Qt::AlignCenter);
        cell->setDelay(true);
    }
}

void FieldGui::highlightCellOn(Cell::Ptr pCell)
{
    cellWidgets[pCell]->highlightOn();
}

void FieldGui::highlightCellOff(Cell::Ptr pCell)
{
    cellWidgets[pCell]->highlightOff();
}

CellGui::CellGui(Cell::CPtr cell, QWidget* parent)
    :QLabel(parent), cell(cell), candidatesLayout(nullptr)
{
    QFont fnt = font();
    fnt.setPixelSize(FONT_SIZE * 2 / 3);
    fnt.setBold(true);
    setFont(fnt);

    QPalette pal = palette();
    if (cell->isInitialValue())
        pal.setColor(foregroundRole(), QColor("black"));
    else
        pal.setColor(foregroundRole(), QColor("blue"));
    if (cell->coord().squareIdx() % 2)
        pal.setColor(QPalette::Window, QColor("pale green"));
    else
        pal.setColor(QPalette::Window, QColor("wheat"));
    setPalette(pal);

    setAutoFillBackground(true);
    setAlignment(Qt::AlignCenter);
    setMinimumSize(QSize(FONT_SIZE,FONT_SIZE));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFrameStyle(QFrame::Box);

    backgroundBrush = pal.brush(QPalette::Background);
    hightlightBrush = QColor("tan");

    QGridLayout* sublay = new QGridLayout(this);
    sublay->setMargin(0);
    sublay->setSpacing(0);
    setLayout(sublay);
    int s_n = static_cast<int>(qSqrt(cell->candidatesCapacity()));
    for (int bit = 1; bit <= cell->candidatesCapacity(); bit++)
    {
        QLabel* label = new QLabel(this);
        QString text = QString("%1").arg(bit);
        label->setText(text);
        candidateLabel[bit-1] = label;
        int subrow = (bit-1) / s_n;
        int subcol = (bit-1) % s_n;
        sublay->addWidget(label, subrow, subcol, Qt::AlignCenter);

        label->setAlignment(Qt::AlignCenter);
        label->setMinimumSize(QSize(FONT_SIZE/3,FONT_SIZE/3));
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QFont font = label->font();
        font.setPixelSize(FONT_SIZE * 2 / 3 / 2);
        font.setBold(true);
        label->setFont(font);
    }
    candidatesLayout = sublay;

    if (cell->isResolved())
        setValue(cell->value());
    else
    {
        for (quint8 bit = 1; bit <= cell->candidatesCapacity(); bit++)
            if (!cell->hasCandidate(bit))
                removeCandidate(bit);
    }

    connect (cell, SIGNAL(valueSet(CellValue)),         this, SLOT(setValue(CellValue)), Qt::QueuedConnection);
    connect (cell, SIGNAL(candidateRemoved(CellValue)), this, SLOT(removeCandidate(CellValue)), Qt::QueuedConnection);
    connect (cell, &Cell::candidateAboutToBeRemoved, this, [this](CellValue v)
    {
        QLabel* label = this->candidateLabel[v-1];

        QPalette pal = label->palette();
        pal.setColor(label->foregroundRole(), QColor("red"));
        label->setPalette(pal);
    }, Qt::QueuedConnection);
    connect (cell, &Cell::candidatesRemoved, this, [this](QBitArray v)
    {
        for (int i=0; i<v.count(); i++)
        {
            if (v.testBit(i))
            {
                QLabel* label = this->candidateLabel[i];
                label->setText(" ");
            }
        }
    }, Qt::QueuedConnection);
    connect (cell, &Cell::candidatesAboutToBeRemoved, this, [this](QBitArray v)
    {
        for (int i=0; i<v.count(); i++)
        {
            if (v.testBit(i))
            {
                QLabel* label = this->candidateLabel[i];

                QPalette pal = label->palette();
                pal.setColor(label->foregroundRole(), QColor("red"));
                label->setPalette(pal);
            }
        }
    }, Qt::QueuedConnection);
    connect (cell, &Cell::valueAboutToBeSet, this, [this](CellValue)
    {
        QPalette pal = this->palette();
        pal.setBrush(QPalette::Window, hightlightBrush);
        this->setPalette(pal);
    }, Qt::QueuedConnection);
}

void CellGui::highlightOn()
{
    setFrameShadow(QFrame::Raised);
}

void CellGui::highlightOff()
{
    setFrameShadow(QFrame::Plain);
}

void CellGui::removeCandidate(CellValue bit)
{
    QLabel* label = candidateLabel[bit-1];
    label->setText(" ");
}

void CellGui::setValue(CellValue v)
{
    for(QLabel* candidate: candidateLabel)
        candidate->hide();
    QString text = QString("%1").arg(v);
    setText(text);

    QPalette pal = this->palette();
    pal.setBrush(QPalette::Window, backgroundBrush);
    this->setPalette(pal);
}
