#include <QApplication>
#include <QSettings>
#include <QIcon> // アイコン用ヘッダーを追加
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // アプリケーション情報を設定
    QCoreApplication::setOrganizationName("ShirafukaSoft");
    QCoreApplication::setOrganizationDomain("shirafuka.example.com");
    QCoreApplication::setApplicationName("ShirafukaBackupSystem");

    // アプリケーションのアイコンを設定 - パスを修正
    app.setWindowIcon(QIcon(":/src/icons/app_icon.png"));

    // 設定情報を表示
    QSettings settings;
    qDebug() << "設定ファイルの場所: " << settings.fileName();

    // アプリケーションの終了動作を確実に制御 - 最後のウィンドウが閉じた時に終了するように設定
    // この設定を明示的に「true」にすることで、メインウィンドウが閉じたときのみアプリが終了します
    app.setQuitOnLastWindowClosed(true);

    // メインウィンドウを作成
    MainWindow mainWindow;

    // メインウィンドウを表示
    mainWindow.show();

    // メインウィンドウが閉じられた時のみアプリケーションを終了するよう接続
    QObject::connect(&mainWindow, &MainWindow::destroyed, &app, &QApplication::quit);

    return app.exec();
}