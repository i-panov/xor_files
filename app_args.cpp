#include "app_args.h"

#include <QCommandLineParser>

AppArgs parseAppArgs(const QCoreApplication& app)
{
    QCommandLineParser parser;
    parser.setApplicationDescription("XOR files processor");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption maskOption("mask", "source files mask", "mask", "*");
    QCommandLineOption removeSourcesOption("remove", "remove source files");
    QCommandLineOption inputPathOption("input", "input files path", "path");
    QCommandLineOption outputPathOption("output", "output files path", "path");
    QCommandLineOption modifyOutputNameOption("modify_output", "modify output file names on conflicts (default: overrite)");
    QCommandLineOption timerDurationOption("timer", "timer duration in seconds (default: single start)", "duration", "0");
    QCommandLineOption xorValueOption("value", "xor value number (0..255)", "number", "0");

    parser.addOption(maskOption);
    parser.addOption(removeSourcesOption);
    parser.addOption(inputPathOption);
    parser.addOption(outputPathOption);
    parser.addOption(modifyOutputNameOption);
    parser.addOption(timerDurationOption);
    parser.addOption(xorValueOption);

    parser.process(app);

    auto mask = parser.value(maskOption);
    auto inputPath = parser.value(inputPathOption);
    auto outputPath = parser.value(outputPathOption);

    AppArgs args;

    args.inputDir.setPath(inputPath);
    args.inputDir.setNameFilters(QStringList() << mask);
    args.inputDir.setFilter(QDir::Files);

    args.outputDir.setPath(outputPath);
    args.outputDir.setFilter(QDir::Files);

    args.removeSources = parser.isSet(removeSourcesOption);
    args.modifyOutput = parser.isSet(modifyOutputNameOption);
    args.timerDuration = parser.value(timerDurationOption).toInt();

    bool ok;
    int xorVal = parser.value(xorValueOption).toInt(&ok);
    args.xorValue = (ok && xorVal >= 0 && xorVal <= 255) ? static_cast<quint8>(xorVal) : 0;

    return args;
}
