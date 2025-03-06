#include <gtest/gtest.h>
#include "../src/backup/BackupEngine.h"

class BackupEngineTest : public ::testing::Test {
protected:
    BackupEngine* backupEngine;

    void SetUp() override {
        backupEngine = new BackupEngine();
    }

    void TearDown() override {
        delete backupEngine;
    }
};

TEST_F(BackupEngineTest, StartBackup) {
    EXPECT_NO_THROW(backupEngine->startBackup("source_folder", "destination_folder"));
}

TEST_F(BackupEngineTest, StopBackup) {
    backupEngine->startBackup("source_folder", "destination_folder");
    EXPECT_NO_THROW(backupEngine->stopBackup());
}

TEST_F(BackupEngineTest, GetProgress) {
    backupEngine->startBackup("source_folder", "destination_folder");
    EXPECT_GE(backupEngine->getProgress(), 0);
    EXPECT_LE(backupEngine->getProgress(), 100);
}