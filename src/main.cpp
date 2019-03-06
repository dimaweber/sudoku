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
#include <QElapsedTimer>

#include <QDialog>
#include <QBoxLayout>
#include <QPushButton>

#include <iostream>

int main(int argc, char *argv[])
{
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
        }
    }


    FieldGui fgui_before(array, &diag);
    layout.addWidget(&fgui_before);

    QElapsedTimer timer;
    timer.start();
    array.process();
    qint64 elaps = timer.elapsed();

    std::cout << qPrintable(filename) << "[" << plainTextInputFileLineNum << "] Done in " << elaps << " ms and is ";
    if (array.isResolved())
        std::cout << "resolved" << std::endl;
    else if (!array.isValid())
        std::cout << "is INVALID" << std::endl;
    else if (array.hasEmptyValues())
        std::cout << "NOT resolved" << std::endl;


    diag.setWindowTitle(QString("Sudoku [%1: %2]").arg(filename).arg(plainTextInputFileLineNum));
    FieldGui fgui_after(array, &diag);
    QPushButton arrow("-->", &diag);
    arrow.connect (&arrow, SIGNAL(pressed()), &fgui_after, SLOT(show()));
    layout.addWidget(&arrow);
    layout.addWidget(&fgui_after);
    fgui_after.hide();

    if (noGui)
    {
        array.print();
        return 0;
    }
    else
        diag.show();

    return app.exec();
}
