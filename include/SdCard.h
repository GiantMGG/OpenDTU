#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Hoymiles.h>
#include <time.h>
#include "Configuration.h"
#include <Arduino.h>

class SdCardClass{
    public:
        void init();
        void loop();



    private:
        void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
        void createDir(fs::FS &fs, const char * path);
        void removeDir(fs::FS &fs, const char * path);
        void readFile(fs::FS &fs, const char * path);
        void writeFile(fs::FS &fs, const char * path, const char * message);
        void appendFile(fs::FS &fs, const char * path, const char * message);
        void renameFile(fs::FS &fs, const char * path1, const char * path2);
        void deleteFile(fs::FS &fs, const char * path);
        void testFileIO(fs::FS &fs, const char * path);



        FieldId_t _publishFields[14] = {
            FLD_UDC,
            FLD_IDC,
            FLD_PDC,
            FLD_YD,
            FLD_YT,
            FLD_UAC,
            FLD_IAC,
            FLD_PAC,
            FLD_F,
            FLD_T,
            FLD_PF,
            FLD_EFF,
            FLD_IRR,
            FLD_Q
        };

        uint32_t _lastWrittenToSDCard[INV_MAX_COUNT];
        uint32_t _lastPublish;
        uint32_t  _lastSdLoop;
        uint32_t _lastSDWrite = 0;
        uint32_t _lastInvUpdateCheck = 0;
        uint32_t _newestInverterTimestamp = 0;
        uint32_t _lastWrittenDataTimeStamp = 0;
};

extern SdCardClass SdCard;
extern SPIClass spi;

