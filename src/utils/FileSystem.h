#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QString>

namespace FileSystem
{
    bool copyFile(const QString &source, const QString &destination);
    // 他の関数宣言
}

#endif // FILESYSTEM_H