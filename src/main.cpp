/*
 Puzzles sources:
 - http://www.sudocue.net/download.php
 - http://opensudoku.p43.eu/
 Techniques descriptions:
 - http://www.sudokuwiki.org/Strategy_Families
*/

#include "field.h"
#include "fieldgui.h"
#include "resolver.h"

#include <QApplication>
#include <QElapsedTimer>

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
            /*
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
            */
        }
    }

    Resolver resolver(array);
    resolver.registerTechnique<NakedSingleTechnique>();
    resolver.registerTechnique<HiddenSingleTechnique>();
    resolver.registerTechnique<NakedGroupTechnique>();
    resolver.registerTechnique<HiddenGroupTechnique>();
    resolver.registerTechnique<IntersectionsTechnique>();
    resolver.registerTechnique<BiLocationColoringTechnique>();
    resolver.registerTechnique<XWingTechnique>();
    resolver.registerTechnique<YWingTechnique>();
    resolver.registerTechnique<XYZWingTechnique>();
    resolver.registerTechnique<UniqueRectangle>();

    if (noGui)
    {
        qint64 elaps;
        QElapsedTimer timer;
        timer.start();
        resolver.process();
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
        QPushButton reloadButton("Reload", &diag);
        QString windowTitle = QString("Sudoku [%1: %2]").arg(filename).arg(plainTextInputFileLineNum);
        QGroupBox box;

        QHBoxLayout layout;
        diag.setLayout(&layout);
        QVBoxLayout controlsLayout;
        QVBoxLayout boxLayout(&box);

        for (Technique* tech: resolver.techniques)
        {
            QCheckBox* pCheck = new QCheckBox(tech->name(), &box);
            boxLayout.addWidget(pCheck);
            pCheck->setChecked(tech->isEnabled());
            pCheck->setEnabled(tech->canBeDisabled());
            QApplication::connect(pCheck, &QCheckBox::clicked, pCheck, [tech, pCheck]()
            {
                tech->setEnabled( pCheck->isChecked());
            }, Qt::QueuedConnection);
            QApplication::connect(tech, &Technique::started, pCheck, [pCheck]()
            {
                QFont font = pCheck->font();
                font.setBold(true);
                pCheck->setFont(font);
            }, Qt::QueuedConnection);
            QApplication::connect(tech, &Technique::done, pCheck, [pCheck]()
            {
                QFont font = pCheck->font();
                font.setBold(false);
                pCheck->setFont(font);

                QPalette pal = pCheck->palette();
                pal.setColor(pCheck->foregroundRole(), QColor("red"));
                pCheck->setPalette(pal);
            }, Qt::QueuedConnection);
            QApplication::connect(tech, &Technique::applied, pCheck, [pCheck]()
            {
                QFont font = pCheck->font();
                font.setBold(false);
                pCheck->setFont(font);

                QPalette pal = pCheck->palette();
                pal.setColor(pCheck->foregroundRole(), QColor("green"));
                pCheck->setPalette(pal);
            }, Qt::QueuedConnection);
            QApplication::connect(&resolver, &Resolver::newIteration, pCheck, [pCheck]()
            {
                QPalette pal = pCheck->palette();
                pal.setColor(pCheck->foregroundRole(), QColor("black"));
                pCheck->setPalette(pal);
            }, Qt::QueuedConnection);

            QApplication::connect(tech, &Technique::cellAnalyzeStarted, &fgui_before, &FieldGui::highlightCellOn, Qt::QueuedConnection);
            QApplication::connect(tech, &Technique::cellAnalyzeFinished, &fgui_before, &FieldGui::highlightCellOff, Qt::QueuedConnection);
        }

        layout.addWidget(&fgui_before);
        layout.addLayout(&controlsLayout);
        controlsLayout.addWidget(&box);
        controlsLayout.addWidget(&goButton);
        controlsLayout.addWidget(&reloadButton);
        controlsLayout.addStretch();

        diag.setWindowTitle(windowTitle);
        box.setLayout(&boxLayout);

        box.setTitle("Techniques");

        QPushButton::connect(&goButton, SIGNAL(pressed()), &resolver, SLOT(start()));
        QApplication::connect(&resolver, &Resolver::done, &diag, [&windowTitle, &diag](quint64 e)
        {
            diag.setWindowTitle(QString("%1 resolved in %2 ms").arg(windowTitle).arg(e));
        }, Qt::QueuedConnection);
        QApplication::connect(&resolver, &Resolver::started, &app, [&goButton, &reloadButton](){goButton.setEnabled(false);reloadButton.setEnabled(false);}, Qt::QueuedConnection);
        QApplication::connect(&resolver, &Resolver::done, &app, [&goButton, &reloadButton](){reloadButton.setEnabled(true);goButton.setEnabled(true);}, Qt::QueuedConnection);
        QApplication::connect(&reloadButton, &QPushButton::pressed, [filename, plainTextInputFileLineNum, &array]()
        {
//            array.readFromPlainTextFile(filename, plainTextInputFileLineNum);
        });
        QApplication::connect(&app, &QApplication::aboutToQuit, &resolver, &Resolver::stop);

        diag.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        diag.show();
        QApplication::exec();
    }

    return 0;
}
