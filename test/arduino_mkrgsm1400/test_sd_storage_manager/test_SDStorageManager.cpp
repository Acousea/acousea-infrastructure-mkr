#include <Arduino.h>
#include <unity.h>
#include "StorageManager/SDStorageManager/SDStorageManager.h"
#include "StorageManager/SDStorageManager/SDPath/SDPath.hpp"

SDStorageManager storage(SDCARD_SS_PIN);



void test_begin_and_delete()
{
    // delay(1000);
    const char *path = SD_PATH("/test_del");
    TEST_ASSERT_TRUE(storage.createEmptyFile(path));
    char content[16] = "temporary";
    TEST_ASSERT_TRUE(storage.overwriteBytesToFile(path, (uint8_t *)content, strlen(content)));
    TEST_ASSERT_TRUE(storage.deleteFile(path));
}

void test_write_and_read_text()
{
    // delay(1000);
    const char *path = SD_PATH("/test_text");
    char content[32] = "Hello";
    char moreContent[16] = " World!";

    TEST_ASSERT_TRUE(storage.createEmptyFile(path));

    TEST_ASSERT_TRUE(storage.overwriteBytesToFile(path, (uint8_t *)content, strlen(content)));
    TEST_ASSERT_TRUE(storage.appendBytesToFile(path, (uint8_t *)moreContent, strlen(moreContent)));

    uint8_t buf[64] = {};
    size_t n = storage.readFileBytes(path, buf, sizeof(buf));

    TEST_ASSERT_EQUAL_STRING("Hello World!", (const char *)buf);
    TEST_ASSERT_EQUAL_UINT32(12, n);
    // TEST_ASSERT_TRUE(storage.deleteFile(path));
}

void test_write_and_read_binary()
{
    // delay(1000);
    const char *path = SD_PATH("/test_bin");

    TEST_ASSERT_TRUE(storage.createEmptyFile(path));

    uint8_t data[4] = {1, 2, 3, 4};
    TEST_ASSERT_TRUE(storage.writeFileBytes(path, data, sizeof(data)));

    uint8_t read[4] = {};
    size_t n = storage.readFileBytes(path, read, sizeof(read));
    TEST_ASSERT_EQUAL_UINT32(sizeof(data), n);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, read, sizeof(data));
    // TEST_ASSERT_TRUE(storage.deleteFile(path));
}

void test_read_region()
{
    // delay(1000);
    const char *path = SD_PATH("/test_reg");
    TEST_ASSERT_TRUE(storage.createEmptyFile(path));

    uint8_t data[6] = {10, 20, 30, 40, 50, 60};
    storage.writeFileBytes(path, data, sizeof(data));

    uint8_t region[3] = {};
    size_t n = storage.readFileRegionBytes(path, 2, region, 3);
    TEST_ASSERT_EQUAL_UINT32(3, n);
    uint8_t expectedRegion[3] = {30, 40, 50};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedRegion, region, 3);
    // TEST_ASSERT_TRUE(storage.deleteFile(path));
}

void test_truncate()
{
    // delay(1000);
    const char *path = SD_PATH("/test_trun");
    TEST_ASSERT_TRUE(storage.createEmptyFile(path));


    uint8_t data[6] = {1, 2, 3, 4, 5, 6};
    storage.writeFileBytes(path, data, sizeof(data));

    TEST_ASSERT_TRUE(storage.truncateFileFromOffset(path, 2));

    uint8_t read[6] = {};
    size_t n = storage.readFileBytes(path, read, sizeof(read));
    TEST_ASSERT_EQUAL_UINT32(4, n);
    uint8_t expectedRegion[4] = {3, 4, 5, 6};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedRegion, read, 4);
    // TEST_ASSERT_TRUE(storage.deleteFile(path));
}

void test_truncate_from_offset()
{
    // delay(1000);
    const char *path = SD_PATH("/test_trun");
    TEST_ASSERT_TRUE(storage.createEmptyFile(path));

    uint8_t data[6] = {1, 2, 3, 4, 5, 6};
    storage.writeFileBytes(path, data, sizeof(data));

    TEST_ASSERT_TRUE(storage.truncateFileFromOffset(path, 2)); // Debería truncar dejando solo 3, 4, 5, 6

    uint8_t read[6] = {};
    size_t n = storage.readFileBytes(path, read, sizeof(read));
    TEST_ASSERT_EQUAL_UINT32(4, n); // Después del truncado, solo deben quedar 4 bytes
    uint8_t expected[4] = {3, 4, 5, 6};
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expected, read, 4);

    TEST_ASSERT_TRUE(storage.deleteFile(path));
}


void test_exists()
{
    // delay(1000);
    const char *path = SD_PATH("/test_exist");
    TEST_ASSERT_TRUE(storage.createEmptyFile(path));

    uint8_t data[4] = {10, 20, 30, 40};
    TEST_ASSERT_TRUE(storage.writeFileBytes(path, data, sizeof(data)));

    TEST_ASSERT_TRUE(storage.fileExists(path)); // El archivo debe existir después de ser creado

    TEST_ASSERT_TRUE(storage.deleteFile(path));
    TEST_ASSERT_FALSE(storage.fileExists(path)); // El archivo no debe existir después de ser eliminado
}


void test_rename_file()
{
    // delay(1000);
    const char *pathOld = SD_PATH("/test_old");
    const char *pathNew = SD_PATH("/test_new");

    TEST_ASSERT_TRUE(storage.createEmptyFile(pathOld));

    uint8_t data[4] = {1, 2, 3, 4};
    TEST_ASSERT_TRUE(storage.writeFileBytes(pathOld, data, sizeof(data)));

    TEST_ASSERT_TRUE(storage.renameFile(pathOld, pathNew)); // Debe renombrarse correctamente

    // Verificar que el archivo original ya no existe
    TEST_ASSERT_FALSE(storage.fileExists(pathOld));
    // Verificar que el archivo renombrado existe
    TEST_ASSERT_TRUE(storage.fileExists(pathNew));

    TEST_ASSERT_TRUE(storage.deleteFile(pathNew));
}


void test_create_and_clear_directory()
{
    // delay(1000);

    const char *dirPath = SD_PATH("/test_dir");    

    TEST_ASSERT_TRUE(storage.createDirectory(dirPath));

    TEST_ASSERT_TRUE(storage.existsDirectory(dirPath));

    TEST_ASSERT_TRUE(storage.clearDirectory(dirPath));
    
    TEST_ASSERT_TRUE(storage.deleteDirectory(dirPath));
}


void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(10);
    }
    const bool beginOk = storage.begin();
    if (!beginOk)
    {
        Serial.println("Failed to initialize SD storage");
        while (1)
        {
            delay(1000);
        }
    }

    // Remove any existing test files/directories before starting tests


    UNITY_BEGIN();

    RUN_TEST(test_create_and_clear_directory);  // Nuevo test
    RUN_TEST(test_begin_and_delete);
    RUN_TEST(test_write_and_read_text);
    RUN_TEST(test_write_and_read_binary);
    RUN_TEST(test_read_region);  // Nuevo test
    RUN_TEST(test_truncate);
    RUN_TEST(test_truncate_from_offset);  // Nuevo test
    RUN_TEST(test_exists);  // Nuevo test
    RUN_TEST(test_rename_file);  // Nuevo test

    UNITY_END();
}

void loop() {}
