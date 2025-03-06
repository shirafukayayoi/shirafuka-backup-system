cmake_minimum_required(VERSION 3.16)
project(Shirafuka-Backup-System)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

# Qtのインストールパスを設定
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.2/msvc2019_64")

# Qtのモジュールを指定
find_package(Qt6 COMPONENTS Widgets REQUIRED)

# ソースファイルのリスト
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/backup/BackupEngine.cpp
    src/backup/BackupTask.cpp
    src/ui/BackupDialog.cpp
    src/ui/BackupCard.cpp
    src/ui/SettingsDialog.cpp
    src/ui/LogViewerDialog.cpp
    src/scheduler/BackupScheduler.cpp
    src/utils/FileSystem.cpp
    src/utils/Logger.cpp
    src/config/ConfigManager.cpp
    src/models/BackupConfig.cpp
)

# ヘッダーファイルのリスト
set(HEADERS
    src/MainWindow.h
    src/backup/BackupEngine.h
    src/backup/BackupTask.h
    src/ui/BackupDialog.h
    src/ui/FolderSelector.h
    src/ui/SettingsDialog.h
    src/utils/FileSystem.h
    src/utils/Logger.h
    src/config/ConfigManager.h
)

# 実行可能ファイルを作成
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Qtライブラリをリンク
target_link_libraries(${PROJECT_NAME} Qt6::Widgets)

# リソースファイルを追加
qt6_add_resources(RESOURCES resources/icons.qrc)