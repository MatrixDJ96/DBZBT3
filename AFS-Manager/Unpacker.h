#ifndef UNPACKER_H
#define UNPACKER_H

#include <QThread>

#include "../Libraries/AFSCore/AFSCore.h"
#include "../Libraries/Dialog/Dialog.h"

class Unpacker : public QThread {
Q_OBJECT

public:
    Unpacker(AFS_File *afs, const QList<int> &list, const std::string &path);

    ~Unpacker();

    void resume();

    void skip();

    void skipAll();

private:
    virtual void run();

private:
    const AFS_File *afs;
    const QList<int> list;
    const std::string path;
    Progress *progress;
    bool isInterrupted;
    bool toSkip;
    bool toSkipAll;

signals:

    void abort();

    void done();

    void error(const QString &filename, bool multi);

    void next();
};

#endif // UNPACKER_H
