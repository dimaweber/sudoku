/*
 Puzzles sources:
 - http://www.sudocue.net/download.php
 - http://opensudoku.p43.eu/
 Techniques descriptions:
 - http://www.sudokuwiki.org/Strategy_Families
*/

#include "field.h"

#include <QApplication>
#include <QElapsedTimer>
#include <QtMath>

#include <QDialog>
#include <QBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QFont>
#include <QColor>
#include <QPalette>

#include <iostream>

#define FONT_SIZE 72

int main(int argc, char *argv[])
{
    if (argc<2)
    {
        std::cout << "supply input file name as first parameter" << std::endl;
        return 1;
    }

    int plainTextInputFileLineNum = 1;

    QString filename = argv[1];

    if (argc > 2)
        plainTextInputFileLineNum = QString(argv[2]).toInt();

    Field array;
    array.readFromPlainTextFile(filename, plainTextInputFileLineNum);

    if (!array.isValid())
    {
        std::cout << "Invalid sudoku read" << std::endl;
        return 1;
    }

    QElapsedTimer timer;
    timer.start();
    array.process();
    qint64 elaps = timer.elapsed();

    if (array.isResolved())
        std::cout << "resolved in " << elaps << " ms" << std::endl;
    else if (!array.isValid())
        std::cout << "is INVALID" << std::endl;
    else if (array.hasEmptyValues())
        std::cout << "NOT resolved" << std::endl;


    QApplication app(argc, argv);
    QDialog diag;
    QGridLayout lay(&diag);
    lay.setMargin(0);
    lay.setSpacing(0);
    for (int i=1; i<=qSqrt(Coord::maxRawIndex()); i++)
    {
        QLabel* htitle = new QLabel(&diag);
        htitle->setText(QString("R%1").arg(i));

        QLabel* vtitle = new QLabel(&diag);
        vtitle->setText(QString("C%1").arg(i));

        vtitle->setAlignment(Qt::AlignCenter);
        lay.addWidget(vtitle, 0, i, Qt::AlignBottom | Qt::AlignHCenter);

        htitle->setAlignment(Qt::AlignCenter);
        lay.addWidget(htitle, i, 0, Qt::AlignRight | Qt::AlignVCenter);
    }
    for (Coord coord=Coord::first(); coord.isValid(); coord++)
    {
        Cell& cell = array.cell(coord);
        int row = coord.row();
        int col = coord.col();
        const int value = cell.value();
        if (value != 0)
        {
            QLabel* label = new QLabel(&diag);
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
            lay.addWidget(label, row, col,Qt::AlignCenter);
        }
        else
        {
            QLabel* widget = new QLabel(&diag);
            QGridLayout* sublay = new QGridLayout(widget);
            sublay->setMargin(0);
            sublay->setSpacing(0);
            widget->setLayout(sublay);
            int s_n = static_cast<int>(qSqrt(cell.candidatesCapacity()));
            for (int bit = 1; bit <= cell.candidatesCapacity(); bit++)
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
            lay.addWidget(widget, row, col);
        }
    }

    QFrame* line = new QFrame(&diag);
    line->setFrameShape(QFrame::VLine);
    line->setLineWidth(20);
    lay.addWidget(line, 0, 3, 9, 1, Qt::AlignCenter);
    diag.show();

    return app.exec();
}
