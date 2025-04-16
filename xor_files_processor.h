#ifndef XOR_FILES_PROCESSOR_H
#define XOR_FILES_PROCESSOR_H

#include "app_args.h"

#include <QObject>

class XorFilesProcessor : public QObject
{
    Q_OBJECT
public:
    explicit XorFilesProcessor(const AppArgs& args, QObject *parent = nullptr);

    void run();

signals:
    void finished();

private:
    AppArgs args;
};

#endif // XOR_FILES_PROCESSOR_H
