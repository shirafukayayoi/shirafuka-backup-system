#include <gtest/gtest.h>
#include "FileSystem.h"

class FileSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // テストの前に必要な初期化を行う
    }

    void TearDown() override {
        // テストの後に必要なクリーンアップを行う
    }
};

TEST_F(FileSystemTest, TestCopyFile) {
    // ファイルコピーのテストを実装
    FileSystem fs;
    // ここにファイルコピーのテストコードを書く
}

TEST_F(FileSystemTest, TestDeleteFile) {
    // ファイル削除のテストを実装
    FileSystem fs;
    // ここにファイル削除のテストコードを書く
}

TEST_F(FileSystemTest, TestCreateDirectory) {
    // ディレクトリ作成のテストを実装
    FileSystem fs;
    // ここにディレクトリ作成のテストコードを書く
}

TEST_F(FileSystemTest, TestDeleteDirectory) {
    // ディレクトリ削除のテストを実装
    FileSystem fs;
    // ここにディレクトリ削除のテストコードを書く
}