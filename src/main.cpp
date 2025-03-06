#include <QApplication>
#include <QSettings>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // アプリケーション情報を設定（これが重要！）
    QCoreApplication::setOrganizationName("ShirafukaSoft");
    QCoreApplication::setOrganizationDomain("shirafuka.example.com");
    QCoreApplication::setApplicationName("ShirafukaBackupSystem");

    // 設定情報を表示
    QSettings settings;
    qDebug() << "設定ファイルの場所: " << settings.fileName();

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}