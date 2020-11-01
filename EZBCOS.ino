#include <Arduino.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <esp32-hal-cpu.h>
#include <stdlib.h>
//#include <LittleFS.h>
/*struct FSInfo {
    size_t totalBytes;
    size_t usedBytes;
    size_t blockSize;
    size_t pageSize;
    size_t maxOpenFiles;
    size_t maxPathLength;
};*/
#define fs LITTLEFS
#define ver "0.0.1.0"
#define rev "Alpha"
byte ufgc = 7;
byte ubgc = 0;
FILE progFile;
FILE userFile[16];
long progMemSize = 262144;
unsigned int progMemChunkSize = 16384;
byte *progMem;
//byte progMem0[65536];
//long progMemSize = sizeof(progMem);
unsigned long progAddtPtr[256];
uint8_t  mathU08Reg = 0;
int8_t   mathS08Reg = 0;
uint16_t mathU16Reg = 0;
int16_t  mathS16Reg = 0;
uint32_t mathU32Reg = 0;
int32_t  mathS32Reg = 0;
uint64_t mathU64Reg = 0;
int64_t  mathS64Reg = 0;
void setup() {
  Serial.begin(115200);
  cls();
  color(7, 0);
  //Serial.print("PQWare EZBCOS v");
  //Serial.print(ver);
  //Serial.print(" r");
  //Serial.println(rev);
  delay(3000);
  //FSInfo fs_info;
  color(15, 4);
  cls();
  drwlogo();
  statBar("PQWare EZBCOS v" + String(ver) + " r" + String(rev), 0);
  beep(900, 150);
  delay(100);
  beep(900, 150);
  delay(2000);
  color(7, 0);
  cls();
  locate(0, 0);
  Serial.println("PQWare EZBCOS v" + String(ver) + " r" + String(rev));
  Serial.println("Initializing memory...");
  progMem = (byte *) malloc(progMemSize);
  Serial.print("Allocating memory in ");
  Serial.print(progMemChunkSize);
  Serial.println(" byte chunks...");
  recheckmem:
  if (progMem == NULL) {
    progMemSize = progMemSize - progMemChunkSize;
    if (progMemSize < 1) {
      Serial.println("Memory allocation failed.");
      hang();
    }
    progMem = (byte *) realloc(progMem, progMemSize);
    goto recheckmem;
  }
  for (long i = 0; i < progMemSize; i++) {
    progMem[i] = 0;
  }
  Serial.print("Program memory size: "); Serial.println(progMemSize);
  Serial.println("Initalizing filesystem...");
  bool fsFormatted = false;
  recheckfs:
  if (fs.begin()) {
    unsigned long fsTotalBytes = LITTLEFS.totalBytes();
    unsigned long fsUsedBytes = LITTLEFS.usedBytes();
    unsigned long fsFreeBytes = fsTotalBytes - fsUsedBytes;
    String fsVolLabel = "";
    Serial.println("Mounted filesystem.");
    Serial.print(char(195)); Serial.println(" Total space: " + String(fsTotalBytes / 1024, DEC) + "KB");
    Serial.print(char(195)); Serial.println(" Used space: " + String(fsUsedBytes / 1024, DEC) + "KB");
    Serial.print(char(192)); Serial.println(" Free space: " + String(fsFreeBytes / 1024, DEC) + "KB");
  } else {
    Serial.println("Filesystem mount failed.");
    if (!fsFormatted) {
      Serial.println("Filesystem mount failed. Formatting...");
      if (fs.format()) {
        goto recheckfs;
      } else {
        Serial.println("Filesystem format failed.");
        hang();
      }
    }
  }
  String autoExecFileName = "";
  if (fs.exists("/autostart.bcp")) {autoExecFileName = "/autostart.bcp";}
  if (fs.exists("/autorun.bcp")) {autoExecFileName = "/autorun.bcp";}
  if (fs.exists("/autoexec.bcp")) {autoExecFileName = "/autoexec.bcp";}
  if (autoExecFileName == "") {
    Serial.println("No autorun detected. Running EZSh...");
    EZSh();
  } else {
    File autorun = fs.open(autoExecFileName, "r");
    Serial.println("Running " + autoExecFileName + "...");
    runProg(autoExecFileName, "");
    EZSh();
  }
}

void loop() {
  hang();
  for (byte yp = 0; yp < 16; yp++) {
    for (byte xp = 0; xp < 16; xp++) {
      locate(xp, yp);
      //Serial.print("\u001b[1;1H");
      color(random(0, 256), random(0, 256));
      //Serial.print(char(yp * 16 + xp));
      Serial.print(char(random(32, 256)));
    }
  }
  Serial.println("");
} 

int EZSh() {
  String EZShVer = "0.1";
  String EZShRev = "Alpha";
  String cwd = "/";
  bool ezshExit = false;
  char pStr[2000];
  Serial.println("EZSh v" + EZShVer + " r" + EZShRev);
  byte tmpufgc = ufgc;
  fgcolor(1); Serial.print("E: "); fgcolor(tmpufgc); Serial.println("EZSh is not implemented yet. :(");
  while (!ezshExit) { 
    ezshExit = true;
  }
}

int runProg(String progFileName, String progArgs) {
  return 0;
}

void cls() {
  Serial.print("\u001b[2J\u001b[H");
}

void beep(unsigned int freq, unsigned int dur) {
  unsigned long tmpMillis = millis();
  freq *= 2.05;
  while (millis() - tmpMillis < dur) {
    delayMicroseconds(1000000 / freq);
    dacWrite(25, 255);
    dacWrite(26, 255);
    delayMicroseconds(1000000 / freq);
    dacWrite(25, 0);
    dacWrite(26, 0);
  }
}

void statBar(String statText, byte statType) {
  Serial.print("\u001b[s");
  switch (statType) {
    case 0:
      color(0, 7);
      break;
    case 1:
      color(0, 6);
      break;
    case 2:
      color(0, 3);
      break;
    case 3:
      color(0, 1);
      break;
  }
  locate(0, 24);
  Serial.print("                                                                                ");
  locate(0, 24);
  for (byte charSel = 0; charSel < statText.length(); charSel++) {
    char tempChar = statText.charAt(charSel);
    if (tempChar >= ' ' && tempChar <= 'ÿ' && tempChar != '\x7F') {Serial.print(tempChar);}
  }
  //locate(79, 23);
  Serial.print("\u001b[u");
}

void color(byte fgc, byte bgc) {
  ufgc = fgc;
  ubgc = bgc;
  Serial.print("\u001b[38;5;");
  Serial.print(fgc);
  Serial.print("m");
  Serial.print("\u001b[48;5;");
  Serial.print(bgc);
  Serial.print("m");
}

void fgcolor(byte fgc) {
  ufgc = fgc;
  Serial.print("\u001b[38;5;");
  Serial.print(fgc);
  Serial.print("m");
}

void bgcolor(byte bgc) {
  ubgc = bgc;
  Serial.print("\u001b[48;5;");
  Serial.print(bgc);
  Serial.print("m");
}

void locate(byte x, byte y) {
  Serial.print("\u001b[");
  Serial.print(y + 1);
  Serial.print(";");
  Serial.print(x + 1);
  Serial.print("H");
}

void drwlogo() {
  Serial.println("                                                                                ");
  Serial.println("                                                                                ");
  Serial.println("                                                                                ");
  Serial.println("                                                                                ");
  Serial.println("                                                                                ");
  Serial.println("                                                                                ");
  Serial.println("                                                                                ");
  Serial.println("                      EEEEE ZZZZZ BBBB   CCCC  OOO   SSS                        ");
  Serial.println("                      E        Z  B   B C     O   O S                           ");
  Serial.println("                      EEEE    Z   BBBB  C     O   O  SSS                        ");
  Serial.println("                      E      Z    B   B C     O   O     S                       ");
  Serial.println("                      EEEEE ZZZZZ BBBB   CCCC  OOO   SSS                        ");
  Serial.println("                                                                                ");
  Serial.println("                       ----- ___-- _-.-- _---- .-.-. _--_.                      ");
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                "); 
  Serial.println("                                                                                ");
  Serial.print("                                                                                ");
}

void hang() {
  while (true) {
    randomSeed( micros() + millis());
    long temp1 = random(0, 65536) + random(0, 65536);
    long temp2 = random(0, 65536) - random(0, 65536);
    long temp3 = random(0, 65536) * random(0, 65536);
    long temp4 = random(0, 65536) / random(0, 65536);
    long temp5 = temp1 + temp2 + temp3 + temp4;
    char tempc = char(temp5);
    //locate(79, 24); if (tempc >= ' ' && tempc <= 'ÿ' && tempc != '\x7F') {Serial.print(tempc);}
  }
}
