/*
 Puzzles sources:
 - http://www.sudocue.net/download.php
 - http://opensudoku.p43.eu/
 Techniques descriptions:
 - http://www.sudokuwiki.org/Strategy_Families
*/

#include "field.h"
#include "fieldgui.h"

#include <QApplication>

#include <QDialog>
#include <QBoxLayout>
#include <QPushButton>

#include <iostream>


int main(int argc, char *argv[])
{
qRegisterMetaType<CellValue>("CellValue");
    QApplication app(argc, argv);
    QDialog diag;
    QHBoxLayout layout(&diag);

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

    bool noGui = false;
    if (argc > 3)
    {
        for(int i=3; i< argc; i++)
        {
            QString arg = QString(argv[i]);
            if (arg == "-no-gui")
                noGui = true;
            if (arg == "-gui")
                noGui = false;
            if (arg == "-no-hidden-single")
                array.enableTechnique(Field::HiddenSingle, false);
            if (arg == "-no-naked-group")
                array.enableTechnique(Field::NakedGroup, false);
            if (arg == "-no-hidden-group")
                array.enableTechnique(Field::HiddenGroup, false);
            if (arg == "-no-intersections")
                array.enableTechnique(Field::Intersections, false);
            if (arg == "-no-xwing")
                array.enableTechnique(Field::XWing, false);
            if (arg == "-no-bi-location-coloring")
                array.enableTechnique(Field::BiLocationColoring, false);
            if (arg == "-no-ywing")
                array.enableTechnique(Field::YWing, false);
            if (arg == "-no-xyzwing")
                array.enableTechnique(Field::XYZWing, false);
        }
    }


    Resolver resolver(array);
    if (noGui)
    {
        quint64 elaps;
        QElapsedTimer timer;
        timer.start();
        array.process();
        elaps = timer.elapsed();

        array.print();

        return 0;
    }
    else
    {
        FieldGui fgui_before(array, &diag);
        layout.addWidget(&fgui_before);

        diag.setWindowTitle(QString("Sudoku [%1: %2]").arg(filename).arg(plainTextInputFileLineNum));
        QPushButton arrow("-->", &diag);
        arrow.connect(&arrow, SIGNAL(pressed()), &resolver, SLOT(start()));
        layout.addWidget(&arrow);

        diag.show();
        app.exec();
    }

    return 0;
}
