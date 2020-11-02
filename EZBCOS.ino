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
#define ver "0.0.1.2"
#define rev "Alpha"
byte ufgc = 7;
byte ubgc = 0;
FILE progFile;
FILE userFile[16];
//long progMemSize = 262144;
long progMemSize = 65536;
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
float    mathF32Reg = 0;
double   mathF64Reg = 0;
byte cClmn = 0;
byte cLine = 0;
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
  locate(1, 1);
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
    runProg(autoExecFileName, "", "/");
    EZSh();
  }
}

void loop() {
  hang();
  for (byte yp = 0; yp < 16; yp++) {
    for (byte xp = 0; xp < 16; xp++) {
      locate(xp + 1, yp + 1);
      //Serial.print("\u001b[1;1H");
      color(random(0, 256), random(0, 256));
      //Serial.print(char(yp * 16 + xp));
      Serial.print(char(random(32, 256)));
    }
  }
  Serial.println("");
} 

int EZSh() {
  const String EZShVer = "0.2";
  const String EZShRev = "Beta";
  String cwd = "/";
  String cmd = "";
  String arg = "";
  bool ezshExit = false;
  char cmdLine[2048];
  unsigned int cmdLinePos = 0;
  int inChar = 0;
  bool EOL = false;
  Serial.println("EZSh v" + EZShVer + " r" + EZShRev);
  byte tmpufgc = ufgc;
  while (!ezshExit) {
    shBegin:
    cmd = "";
    arg = "";
    for (int i = 0; i < 2048; i++) {
      cmdLine[i] = 0;
    }
    cmdLinePos = 0;
    Serial.print(cwd); Serial.print('>');
    while (inChar != 13) {
      inChar = Serial.read();
      if (inChar != -1) {
        switch (inChar) {
          case '\x08':
            if (cmdLinePos > 0) {
              cmdLinePos -= 1;
              cmdLine[cmdLinePos] = 0;
              getCursorLocation();
              if (cClmn == 1) {
                locate(80, cLine - 1);
                Serial.println(' ');
                locate(80, cLine - 1);
              } else {
                Serial.print("\x08"" ""\x08");
              }
            }
            break;
          case '\x03':
            Serial.println();
            goto shBegin;
            break;
          default:
            if (inChar > 31 && inChar < 127) {
              cmdLine[cmdLinePos] = char(inChar);
              cmdLinePos += 1;
              //locate(cClmn, cLine);
              getCursorLocation();
              if (cClmn == 80) {
                Serial.println(char(inChar));
              } else {
                Serial.print(char(inChar));
              }
              //getCursorLocation();
            }
            break;
        }
      }
    }
    int tmpScanPos = 0;
    char tmpInChar = cmdLine[tmpScanPos];
    //Serial.println(String(tmpInChar, DEC));
    while (true) {
      if (tmpInChar != ' ') {break;}
      tmpScanPos += 1;
      tmpInChar = cmdLine[tmpScanPos];
      //Serial.println();
      //Serial.print("1");
    }
    while (true) {
      if (tmpInChar == 0 || tmpInChar == ' ') {break;}
      cmd += tmpInChar;
      tmpScanPos += 1;
      tmpInChar = cmdLine[tmpScanPos];
      //Serial.println();
      //Serial.print("2");
    }
    tmpScanPos += 1;
    tmpInChar = cmdLine[tmpScanPos];
    while (true) {
      if (tmpInChar == 0) {break;}
      arg += tmpInChar;
      tmpScanPos += 1;
      tmpInChar = cmdLine[tmpScanPos];
      //Serial.println();
      //Serial.print("3");
    }
    Serial.println();
    cmd.trim();
    arg.trim();
    if (cmd != "") {
      if (cmd.charAt(0) == '&') {
        cmd = cmd.substring(1);
        cmd.toLowerCase();
        int err = -1;
        String errText = cmd + " is not a command";
        if (cmd == "exit") {
          err = 0;
          return 0;
        }
        if (cmd == "reset") {
          err = 0;
          ESP.restart();
        }
        if (cmd == "cls") {
          err = 0;
          cls();
        }
        if (cmd == "echo") {
          err = 0;
          if (arg.substring(0, 2) == "-n") {
            if (arg.substring(0, 3) == "-n ") {
              Serial.print(arg.substring(3));
            } else if (arg.substring(0, 3) == "-n" + '\x00') {
              Serial.print("");
            } else {
              Serial.println(arg);
            }
          } else {
            Serial.println(arg);
          }
        }
        if (err != 0) {
          printErr(errText + " (" + String(err, DEC) + ")");
        }
      } else {
        String tmpCwd = cwd;
        if (cmd.charAt(0) != '/') {
          cmd = cwd + cmd;
        }
        String tmpCmd = cmd;
        cwd = cmd.substring(0, cmd.lastIndexOf('/') + 1);
        cmd = cmd.substring(cmd.lastIndexOf('/') + 1);
        if (fs.exists(tmpCmd)) {
          File tmpPreChk = LITTLEFS.open(tmpCmd);
          if (tmpPreChk.isDirectory()) {
            printErr(tmpCmd + " is a directory.");
          } else {
            runProg(tmpCmd, arg, tmpCwd);
          }
        } else {
          if (cmd == "") {
            printErr("Could not find directory " + cwd);
          } else {
            printErr("Could not find " + cmd + " in " + cwd);
          }
        }
        cwd = tmpCwd;
      }
    }
    //Serial.println(cmd);
    //Serial.println("----------------");
    //Serial.println(arg);
    //Serial.println();
    //Serial.println(String(cClmn, DEC));
    //Serial.println(String(cLine, DEC));
    inChar = Serial.read();
  }
}

int runProg(String progFileName, String progArgs, String startDir) {
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

bool printErr(String errorText) {
  byte tfgc = ufgc;
  fgcolor(1);
  Serial.print("E: ");
  fgcolor(tfgc);
  Serial.println(errorText);
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
  locate(1, 25);
  Serial.print("                                                                                ");
  locate(1, 25);
  for (byte charSel = 0; charSel < statText.length(); charSel++) {
    char tempChar = statText.charAt(charSel);
    if (tempChar >= ' ' && tempChar <= 'ÿ' && tempChar != '\x7F') {Serial.print(tempChar);}
  }
  //locate(80, 24);
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
  Serial.print(y);
  Serial.print(";");
  Serial.print(x);
  Serial.print("H");
}

void getCursorLocation() {
  Serial.print("\u001b[6n");
  int tmpChar = 0;
  unsigned int idleCycles = 0;
  unsigned int idleCyclesMax = 65535;
  while (char(tmpChar) != '\x1b') {
    tmpChar = Serial.read();
    //Serial.print("1: "); Serial.println(String(tmpChar, DEC));
    idleCycles += 1;
    if (idleCycles >= idleCyclesMax) {
      return;
    }
  }
  idleCycles = 0;
  while (char(tmpChar) != '[') {
    tmpChar = Serial.read();
    //Serial.print("2: "); Serial.println(String(tmpChar, DEC));
    if (idleCycles >= idleCyclesMax) {
      return;
    }
  }
  String line = "";
  String clmn = "";
  idleCycles = 0;
  while (char(tmpChar) != ';') {
    tmpChar = Serial.read();
    if (char(tmpChar) != ';') {line += char(tmpChar);}
    if (idleCycles >= idleCyclesMax) {
      return;
    }
    //Serial.print("3: "); Serial.println(String(tmpChar, DEC));
  }
  idleCycles = 0;
  while (char(tmpChar) != 'R') {
    tmpChar = Serial.read();
    if (char(tmpChar) != 'R') {clmn += char(tmpChar);}
    if (idleCycles >= idleCyclesMax) {
      return;
    }
    //Serial.print("4: "); Serial.println(String(tmpChar, DEC));
  }
  cClmn = clmn.toInt();
  cLine = line.toInt();
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
    //locate(80, 25); if (tempc >= ' ' && tempc <= 'ÿ' && tempc != '\x7F') {Serial.print(tempc);}
  }
}
