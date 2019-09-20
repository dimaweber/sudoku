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
#include <QCommandLineParser>
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
    QCoreApplication::setApplicationName("QSudokuSolver");
    QCoreApplication::setApplicationVersion("1.1");

    QCommandLineParser parser;
    parser.setApplicationDescription("sudoku 3x3, 4x4, 5x5 puzzles solver");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", "The file to open");
    parser.addPositionalArgument("puzzle", "The line  number with puzzle to solve");

    QCommandLineOption noGuiOption (QStringList() << "n" << "no-gui", "Use text interface");
    parser.addOption(noGuiOption);

    parser.addOptions({
                       {"no-hidden-single", "Disable Hidden Single technique"},
                       {"no-naked-group", "Disable Naked Group technique"},
                       {"no-hidden-group", "Disable Hidden Group technique"},
                       {"no-intersections", "Disable Intersections technique"},
                       {"no-bi-lication-coloring", "Disable Bi-Location Coloring technique"},
                       {"no-xwing", "Disable Hidden X-Wing technique"},
                       {"no-ywing", "Disable Hidden Y-Wing technique"},
                       {"no-xyzwing", "Disable Hidden XYZ-Wing technique"},
                       {"unique-rectangle", "Disable Unique Rectangle technique"},
                     });

    parser.process(app);

    int plainTextInputFileLineNum = 1;
    bool noGui = false;
    noGui = parser.isSet(noGuiOption);
    QStringList args = parser.positionalArguments();
    if (args.size() != 2)
    {
        std::cerr << "filename or linenumber is missing. Exiting";
        parser.showHelp(1);
        Q_UNREACHABLE();
    }

    QString filename = args.at(0);
    plainTextInputFileLineNum = args.at(1).toInt();

    Field array;
    array.readFromPlainTextFile(filename, plainTextInputFileLineNum);

    if (!array.isValid())
    {
        std::cout << "Invalid sudoku read" << std::endl;
        return 1;
    }

    Resolver resolver(array);
    resolver.registerTechnique<NakedSingleTechnique>();
    resolver.registerTechnique<HiddenSingleTechnique>()->setEnabled(!parser.isSet("no-hidden-single"));
    resolver.registerTechnique<NakedGroupTechnique>()->setEnabled(!parser.isSet("no-naked-group"));
    resolver.registerTechnique<HiddenGroupTechnique>()->setEnabled(!parser.isSet("no-hidden-group"));
    resolver.registerTechnique<IntersectionsTechnique>()->setEnabled(!parser.isSet("no-intersections"));
    resolver.registerTechnique<BiLocationColoringTechnique>()->setEnabled(!parser.isSet("no-bi-lication-coloring"));
    resolver.registerTechnique<XWingTechnique>()->setEnabled(!parser.isSet("no-xwing"));
    resolver.registerTechnique<YWingTechnique>()->setEnabled(!parser.isSet("no-ywing"));
    resolver.registerTechnique<XYZWingTechnique>()->setEnabled(!parser.isSet("no-xyzwing"));
    resolver.registerTechnique<UniqueRectangle>()->setEnabled(!parser.isSet("unique-rectangle"));

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
