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
    for (int i=1; i<=qSqrt(Coord::maxRawIndex()); i++)
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
        Cell& cell = field.cell(coord);
        int row = coord.row();
        int col = coord.col();
        quint8 value = cell.value();
        if (cell.isResolved())
        {
            QLabel* label = new QLabel(this);
            QFont font = label->font();
            font.setPixelSize(FONT_SIZE * 2 / 3);
            font.setBold(true);
            label->setFont(font);
            QPalette pal = label->palette();
            if (cell.isInitialValue())
                pal.setColor(label->foregroundRole(), QColor("black"));
            else
                pal.setColor(label->foregroundRole(), QColor("blue"));
            if (coord.squareIdx() % 2)
                pal.setColor(QPalette::Window, QColor("pale green"));
            else
                pal.setColor(QPalette::Window, QColor("wheat"));
            label->setAutoFillBackground(true);

            label->setPalette(pal);
            QString text = QString("%1").arg(value);
            label->setText(text);
            label->setAlignment(Qt::AlignCenter);
            label->setMinimumSize(QSize(FONT_SIZE,FONT_SIZE));
            label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            label->setFrameStyle(QFrame::Box);
            layout->addWidget(label, row, col,Qt::AlignCenter);
        }
        else
        {
            QLabel* widget = new QLabel(this);
            QGridLayout* sublay = new QGridLayout(widget);
            sublay->setMargin(0);
            sublay->setSpacing(0);
            widget->setLayout(sublay);
            int s_n = static_cast<int>(qSqrt(cell.candidatesCapacity()));
            for (quint8 bit = 1; bit <= cell.candidatesCapacity(); bit++)
            {
                int subrow = (bit-1) / s_n;
                int subcol = (bit-1) % s_n;
                QString text;
                if (cell.hasCandidate(bit))
                {
                    text = QString("%1").arg(bit);
                }
                QLabel* label = new QLabel(widget);
                label->setText(text);
                label->setAlignment(Qt::AlignCenter);
                label->setMinimumSize(QSize(FONT_SIZE/3,FONT_SIZE/3));
                label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                QFont font = label->font();
                font.setPixelSize(FONT_SIZE * 2 / 3 / 2);
                font.setBold(true);
                label->setFont(font);
                sublay->addWidget(label, subrow, subcol,Qt::AlignCenter);
            }
            QPalette pal = widget->palette();
            pal.setColor(widget->foregroundRole(), QColor("blue"));
            if (coord.squareIdx() % 2)
                pal.setColor(QPalette::Window, QColor("pale green"));
            else
                pal.setColor(QPalette::Window, QColor("wheat"));
            widget->setAutoFillBackground(true);
            widget->setPalette(pal);
            widget->setMinimumSize(QSize(FONT_SIZE,FONT_SIZE));
            widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            widget->setFrameStyle(QFrame::Box);
            layout->addWidget(widget, row, col);
        }
    }
}
