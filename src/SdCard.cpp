#include "SdCard.h"

SdCardClass SdCard;


void SdCardClass::init()
{
    return;
}

void SdCardClass::loop()
{
    // see: https://github.com/me-no-dev/ESPAsyncWebServer#limiting-the-number-of-web-socket-clients
    if (millis() - _lastSdLoop < 1000) {
        return;
    }

    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }
        int	tm_sec = timeinfo.tm_sec;
        int	tm_min = timeinfo.tm_min;
        int	tm_hour = timeinfo.tm_hour;
        int	tm_mday = timeinfo.tm_mday;
        int	tm_mon = timeinfo.tm_mon;
        int	tm_year = timeinfo.tm_year;


    _lastSdLoop = millis();
    _lastInvUpdateCheck = millis();

    
    uint32_t maxTimeStamp = 0;
    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);

        if (inv->Statistics()->getLastUpdate() > maxTimeStamp) {
            maxTimeStamp = inv->Statistics()->getLastUpdate();
        }
    }
    Serial.printf("SD Checkpoint 2\n");
    // Write new Data to SD Card every 10 seconds and only if new Data from inverter has been fetched
    if (millis() - _lastSDWrite > (10 * 1000) || (maxTimeStamp - _lastWrittenDataTimeStamp > 0)) {
        char dataToWrite[512];
        Serial.printf("SD Checkpoint 3\n");
        Serial.printf("Current time is: %i:%i:%i", tm_hour,tm_min,tm_sec);
        snprintf(dataToWrite, sizeof(dataToWrite), "%02d:%02d:%02d;", tm_hour, tm_min, tm_sec);
        // Loop all inverters
        for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
            auto inv = Hoymiles.getInverterByPos(i);

            uint32_t lastUpdate = inv->Statistics()->getLastUpdate();
            // if (lastUpdate > 0 && lastUpdate != _lastWrittenToSDCard[i]) {
            _lastWrittenToSDCard[i] = lastUpdate;

            // Loop all channels
            for (auto& t : inv->Statistics()->getChannelTypes()) {
                for (auto& c : inv->Statistics()->getChannelsByType(t)) {
                    INVERTER_CONFIG_T* inv_cfg = Configuration.getInverterConfig(inv->serial());
                    if (inv_cfg != nullptr) {
                        strcat(dataToWrite, "CHNr:");
                        char intStr[32];
                        snprintf(intStr, sizeof(intStr), "%d", c); // Convert the integer to a string
                        strcat(dataToWrite, intStr);
                        strcat(dataToWrite, ";");
                        for (uint8_t f = 0; f < sizeof(_publishFields) / sizeof(FieldId_t); f++) { 
                            float fieldValue = 0;
                            fieldValue = inv->Statistics()->getChannelFieldValue(t, c, _publishFields[f]);
                            // uint8_t fieldDigits = 0;
                            // fieldDigits = static_cast<unsigned int>(inv->Statistics()->getChannelFieldDigits(t, c, _publishFields[f]));
                            snprintf(intStr, sizeof(intStr), "%.2f", fieldValue); // Convert the integer to a string
                            strcat(dataToWrite, intStr);

                            // snprintf(intStr, sizeof(intStr), "%d", fieldDigits);
                            strcat(dataToWrite, ";");
                        }
                    }
                }
            }
        }
        try {
            strcat(dataToWrite, "\n");
            char fileName[37];
            snprintf(fileName, sizeof(fileName), "/%04d_%02d_%02d", tm_year+1900, tm_mon+1, tm_mday);
            SdCard.appendFile(SD, fileName, dataToWrite);
        } catch (...) {
            Serial.printf("Writing to SD Card did not work.");
        }
        _lastSDWrite = millis();
        _lastWrittenDataTimeStamp = maxTimeStamp;
    }
}


void SdCardClass::listDir(fs::FS& fs, const char* dirname, uint8_t levels)
{
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels) {
                listDir(fs, file.name(), levels - 1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void SdCardClass::createDir(fs::FS& fs, const char* path)
{
    Serial.printf("Creating Dir: %s\n", path);
    if (fs.mkdir(path)) {
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void SdCardClass::removeDir(fs::FS& fs, const char* path)
{
    Serial.printf("Removing Dir: %s\n", path);
    if (fs.rmdir(path)) {
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void SdCardClass::readFile(fs::FS& fs, const char* path)
{
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if (!file) {
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while (file.available()) {
        Serial.write(file.read());
    }
    file.close();
}

void SdCardClass::writeFile(fs::FS& fs, const char* path, const char* message)
{
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void SdCardClass::appendFile(fs::FS& fs, const char* path, const char* message)
{
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void SdCardClass::renameFile(fs::FS& fs, const char* path1, const char* path2)
{
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void SdCardClass::deleteFile(fs::FS& fs, const char* path)
{
    Serial.printf("Deleting file: %s\n", path);
    if (fs.remove(path)) {
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void SdCardClass::testFileIO(fs::FS& fs, const char* path)
{
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if (file) {
        len = file.size();
        size_t flen = len;
        start = millis();
        while (len) {
            size_t toRead = len;
            if (toRead > 512) {
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }

    file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for (i = 0; i < 2048; i++) {
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}