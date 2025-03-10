cmake_minimum_required(VERSION 3.16)
project(ShirafukaBackupSystem)

# 自動MOC、RCC、UICの設定
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON) # リソースの自動処理に必要
set(CMAKE_AUTOUIC ON) # UIファイル用
set(CMAKE_CXX_STANDARD 17)

# Qtのインストールパスを設定
set(CMAKE_PREFIX_PATH "C:/Qt/6.8.2/msvc2019_64")

# Qtのモジュールを指定
find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)

# デバッグ情報を表示
message(STATUS "Qt6_FOUND: ${Qt6_FOUND}")
message(STATUS "Qt6_VERSION: ${Qt6_VERSION}")

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
    resources.qrc # リソースファイルを直接ソースに追加
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

# Windowsではアイコンの設定
if(WIN32)
    set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
    list(APPEND SOURCES ${APP_ICON_RESOURCE_WINDOWS})
endif()

# 実行ファイル生成の設定
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# リンクするQt6モジュールを指定
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Core Qt6::Gui)