#include <SdFat.h>
extern SdFs sd;
extern FsFile dir;
extern FsFile file;

void dumpFile(){
    Serial.println(F("\n\n---File info---\n\n"));
    while (file.available()){
        Serial.write(file.read());
    }
    file.close();
    Serial.println(F("\n\n---END---\n\n"));
}