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
#include <QGroupBox>
#include <QCheckBox>
#include <iostream>


int main(int argc, char *argv[])
{
qRegisterMetaType<CellValue>("CellValue");
    QApplication app(argc, argv);

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
        qint64 elaps;
        QElapsedTimer timer;
        timer.start();
        array.process();
        elaps = timer.elapsed();

        array.print();

        std::cout << qPrintable(filename) << "[" << plainTextInputFileLineNum << "] Done in " << elaps << " ms and is ";
        if (array.isResolved())
            std::cout << "resolved" << std::endl;
        else if (!array.isValid())
            std::cout << "is INVALID" << std::endl;
        else if (array.hasEmptyValues())
            std::cout << "NOT resolved" << std::endl;

        return 0;
    }
    else
    {
        QDialog diag;
        FieldGui fgui_before(array, &diag);
        QPushButton goButton("Go", &diag);
        QString windowTitle = QString("Sudoku [%1: %2]").arg(filename).arg(plainTextInputFileLineNum);
        QGroupBox box;
        QCheckBox* pNakedSingleCheck = new QCheckBox("Naked Single", &box);
        QCheckBox* pHiddenSingleCheck = new QCheckBox("Hidden Single", &box);
        QCheckBox* pNakedGroupCheck = new QCheckBox("Naked Group", &box);
        QCheckBox* pHiddenGroupCheck = new QCheckBox("Hidden Group", &box);
        QCheckBox* pIntersectionCheck = new QCheckBox("Intersection", &box);
        QCheckBox* pXWingCheck = new QCheckBox("X-Wing", &box);
        QCheckBox* pYWingCheck = new QCheckBox("Y-Wing", &box);

        QHBoxLayout layout(&diag);
        QVBoxLayout controlsLayout(&diag);
        QVBoxLayout boxLayout(&box);

        layout.addWidget(&fgui_before);
        layout.addLayout(&controlsLayout);
        controlsLayout.addWidget(&box);
        controlsLayout.addWidget(&goButton);
        controlsLayout.addStretch();

        boxLayout.addWidget(pNakedSingleCheck);
        boxLayout.addWidget(pHiddenSingleCheck);
        boxLayout.addWidget(pNakedGroupCheck);
        boxLayout.addWidget(pHiddenGroupCheck);
        boxLayout.addWidget(pIntersectionCheck);
        boxLayout.addWidget(pXWingCheck);
        boxLayout.addWidget(pYWingCheck);

        diag.setWindowTitle(windowTitle);
        box.setLayout(&boxLayout);

        box.setTitle("Techniques");

        pNakedSingleCheck->setChecked( array.isTechEnabled(Field::NakedSingle));
        pHiddenSingleCheck->setChecked( array.isTechEnabled(Field::HiddenSingle));
        pNakedGroupCheck->setChecked( array.isTechEnabled(Field::NakedGroup));
        pHiddenGroupCheck->setChecked( array.isTechEnabled(Field::HiddenGroup));
        pIntersectionCheck->setChecked(array.isTechEnabled(Field::Intersections));
        pXWingCheck->setChecked(array.isTechEnabled(Field::XWing));
        pYWingCheck->setChecked(array.isTechEnabled(Field::YWing));

        pNakedSingleCheck->setEnabled(false);

        app.connect(pNakedSingleCheck, &QCheckBox::clicked, [&array, pNakedSingleCheck]()
        {
            array.enableTechnique(Field::NakedSingle, pNakedSingleCheck->isChecked());
        });
        app.connect(pHiddenSingleCheck, &QCheckBox::clicked, [&array, pHiddenSingleCheck]()
        {
            array.enableTechnique(Field::HiddenSingle, pHiddenSingleCheck->isChecked());
        });
        app.connect(pNakedGroupCheck, &QCheckBox::clicked, [&array, pNakedGroupCheck]()
        {
            array.enableTechnique(Field::NakedGroup, pNakedGroupCheck->isChecked());
        });
        app.connect(pHiddenGroupCheck, &QCheckBox::clicked, [&array, pHiddenGroupCheck]()
        {
            array.enableTechnique(Field::HiddenGroup, pHiddenGroupCheck->isChecked());
        });
        app.connect(pIntersectionCheck, &QCheckBox::clicked, [&array, pIntersectionCheck]()
        {
           array.enableTechnique(Field::Intersections, pIntersectionCheck->isChecked());
        });
        app.connect(pXWingCheck, &QCheckBox::clicked, [&array, pXWingCheck]()
        {
            array.enableTechnique(Field::XWing, pXWingCheck->isChecked());
        });
        app.connect(pYWingCheck, &QCheckBox::clicked, [&array, pYWingCheck]()
        {
            array.enableTechnique(Field::YWing, pYWingCheck->isChecked());
        });

        goButton.connect(&goButton, SIGNAL(pressed()), &resolver, SLOT(start()));
        app.connect(&resolver, &Resolver::done, [&windowTitle, &diag, &resolver]()
        {
            diag.setWindowTitle(QString("%1 resolved in %2 ms").arg(windowTitle).arg(resolver.resolveTime()));
        });


        diag.show();
        app.exec();
    }

    return 0;
}
