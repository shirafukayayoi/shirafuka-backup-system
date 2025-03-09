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

    // アプリケーションの終了動作を管理
    app.setQuitOnLastWindowClosed(false);

    // メインウィンドウを作成
    MainWindow mainWindow;

    // メインウィンドウを表示
    mainWindow.show();

    // メインウィンドウが閉じられた時のみアプリケーションを終了するよう接続
    QObject::connect(&mainWindow, &MainWindow::destroyed, &app, &QApplication::quit);

    return app.exec();
}