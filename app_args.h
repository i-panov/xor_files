#ifndef APP_ARGS_H
#define APP_ARGS_H

#include <QCoreApplication>
#include <QDir>

struct AppArgs {
    QDir inputDir, outputDir;
    bool removeSources, modifyOutput;
    int timerDuration;
    quint8 xorValue;

    void print() {
        qDebug() << "inputDir: " << inputDir;
        qDebug() << "outputDir: " << outputDir;
        qDebug() << "removeSources: " << removeSources;
        qDebug() << "modifyOutput: " << modifyOutput;
        qDebug() << "timerDuration: " << timerDuration;
        qDebug() << "xorValue: " << xorValue;
    }
};

AppArgs parseAppArgs(const QCoreApplication& app);

#endif // APP_ARGS_H
