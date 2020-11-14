#include <Arduino.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>
#include <esp32-hal-cpu.h>
#include <stdlib.h>
#define lfs LITTLEFS
#define ver "0.0.1.3"
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
String cwd = "/";
String cmd = "";
String arg = "";

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
  if (lfs.begin(false)) {
    unsigned long fsTotalBytes = lfs.totalBytes();
    unsigned long fsUsedBytes = lfs.usedBytes();
    unsigned long fsFreeBytes = fsTotalBytes - fsUsedBytes;
    String fsVolLabel = "";
    Serial.println("Mounted filesystem.");
    Serial.print(char(195)); Serial.println(" Total space: " + String(fsTotalBytes / 1024, DEC) + "KB");
    Serial.print(char(195)); Serial.println(" Used space: " + String(fsUsedBytes / 1024, DEC) + "KB");
    Serial.print(char(192)); Serial.println(" Free space: " + String(fsFreeBytes / 1024, DEC) + "KB");
  } else {
    Serial.println("Filesystem mount failed.");
    if (!fsFormatted) {
      fsFormatted = true;
      char tmpChr = prompt("Format (this will erase the flash memory)? ", 0).charAt(0);
      Serial.println();
      if (tmpChr == 'y' || tmpChr == 'Y') {
        Serial.println("Formatting filesystem...");
        if (lfs.format()) {
          Serial.println("Filesystem format succeeded.");
          goto recheckfs;
        } else {
          //Serial.println("Filesystem format failed.");
          char tmpChr2 = prompt("Filesystem format failed. Continue? ", 0).charAt(0);
          if (!(tmpChr == 'y' || tmpChr == 'Y')) {
            hang();
          }
        }
      }
    }
  }
  String autoExecFileName = "";
  if (lfs.exists("/autostart.bcp")) {autoExecFileName = "/autostart.bcp";}
  if (lfs.exists("/autorun.bcp")) {autoExecFileName = "/autorun.bcp";}
  if (lfs.exists("/autoexec.bcp")) {autoExecFileName = "/autoexec.bcp";}
  if (autoExecFileName == "") {
    Serial.println("No autorun detected. Running EZSh...");
    EZSh();
  } else {
    File autorun = lfs.open(autoExecFileName, "r");
    Serial.println("Running " + autoExecFileName + "...");
    runProg(autoExecFileName, "", "/");
    autorun.close();
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
  const String EZShVer = "0.8";
  const String EZShRev = "Beta";
  bool ezshExit = false;
  bool EOL = false;
  Serial.println("EZMicroShell v" + EZShVer + " r" + EZShRev);
  Serial.println("NOTE: This shell is meant for starting bytecode programs and has very little built in commands.");
  Serial.println("Internal commands start with ':'.");
  byte tmpufgc = ufgc;
  int tmpScanPos = 0;
  while (!ezshExit) {
    shBegin:
    String tmpInCmd = prompt(cwd + ">", 0);
    tmpInCmd.trim();
    cmd = "";
    arg = "";
    int tmpScanPos = 0;
    char tmpInChar = tmpInCmd.charAt(tmpScanPos);
    //Serial.print(String(tmpInChar, HEX) + " ");
    /*while (true) {
      if (tmpInChar != ' ') {break;}
      tmpScanPos += 1;
      tmpInChar = cmdLine[tmpScanPos];
      //Serial.println();
      //Serial.print("1");
    }*/
    while (true) {
      if (tmpInChar == 0 || tmpInChar == ' ') {break;}
      cmd += tmpInChar;
      tmpScanPos += 1;
      tmpInChar = tmpInCmd.charAt(tmpScanPos);
      //Serial.println();
      //Serial.print("2");
    }
    tmpScanPos += 1;
    tmpInChar = tmpInCmd.charAt(tmpScanPos);
    while (true) {
      if (tmpInChar == 0) {break;}
      arg += tmpInChar;
      tmpScanPos += 1;
      tmpInChar = tmpInCmd.charAt(tmpScanPos);
      //Serial.println();
      //Serial.print("3");
    }
    Serial.println();
    cmd.trim();
    arg.trim();
    if (cmd != "") {
      if (cmd.charAt(0) == ':') {
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
        if (cmd == "ls") {
          err = 0;
          //String tmpStr = "";
          File dir = lfs.open(cwd);
          File file = dir.openNextFile();
          while(file){
            Serial.print(file.name());
            Serial.print(" - ");
            Serial.print(file.size());
            Serial.println(" bytes");
            file = dir.openNextFile();
          }
          file.close();
          dir.close();
        }
        /*if (cmd == "cd") {
          cwd = arg;
          //int tmpCharPos2 = arg.
          rechecknum00:
          int tmpChrPos = arg.lastIndexOf('/');
          if (tmpChrPos == arg.length() - 1) {
            
          } else if
        }*/
        if (cmd == "beep") {
          err = 0;
          beep(900, 150);
        }
        if (cmd == "echo") {
          err = 0;
          if (arg.substring(0, 2) == "-n") {
            if (arg.charAt(2) == ' ') {
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
        if (cmd == "mkdir" || cmd == "md") {
          err = 0;
          if (arg != "") {
            String tmpStr = getAbsPath(arg);
            if (isValidPath(tmpStr)) {
              lfs.mkdir(tmpStr);
              File tmpFile = lfs.open(tmpStr);
              if (lfs.exists(tmpStr)) {
                if (!tmpFile.isDirectory()) {
                  Serial.println("Could not make dir.");
                }
              } else {
                Serial.println("Could not make dir.");
              }
              tmpFile.close();
            } else {
              Serial.println("Invalid file/directory name.");
            }
          } else {
            Serial.println("Arguments cannot be blank.");
          }
        }
        if (cmd == "make" || cmd == "mk") {
          err = 0;
          if (arg != "") {
            String tmpStr = getAbsPath(arg);
            if (isValidPath(tmpStr)) {
              if (lfs.exists(tmpStr)) {
                Serial.println("File already exists.");
              } else {
                File tmpFile = lfs.open(tmpStr, "w");
                tmpFile.close();
                if (!lfs.exists(tmpStr)) {
                  Serial.println("Could not make file.");
                }
              }
            } else {
              Serial.println("Invalid file/directory name.");
            }
          } else {
            Serial.println("Arguments cannot be blank.");
          }
        }
        if (cmd == "chdir" || cmd == "cd") {
          err = 0;
          if (arg != "") {
            //tPause(true);
            String tmpStr = getAbsPath(arg);
            //tPause(true);
            //Serial.println(tmpStr);
            //tPause(true);
            if (arg == "..") {
              if (cwd != "/") {cwd = cwd.substring(0, cwd.lastIndexOf('/', cwd.length() - 2) + 1);}
            } else if (arg == ".") {} else if (lfs.exists(tmpStr)) {
              File tmpFile = lfs.open(tmpStr);
              //tPause(true);
              //Serial.println(tmpFile.name());
              cwd = tmpFile.name();
              //tPause(true);
              tmpFile.close();
              //tPause(true);
            } else {
              Serial.println("Directory does not exist.");
            }
          } else {
            Serial.println("Arguments cannot be blank.");
          }
        }
        if (cmd == "rmdir" || cmd == "rd") {
          err = 0;
          if (arg != "") {
            String tmpStr = getAbsPath(arg);
            File tmpFile = lfs.open(tmpStr);
            if (tmpFile.isDirectory()) {
              tmpFile.close();
              lfs.rmdir(tmpStr);
              if (lfs.exists(tmpStr)) {
                Serial.println("Could not remove dir.");
              }
            } else {
              tmpFile.close();
              Serial.println("Not a directory.");
            }
          } else {
            Serial.println("Arguments cannot be blank.");
          }
        }
        if (cmd == "remove" || cmd == "rm") {
          err = 0;
          if (arg != "") {
            String tmpStr = getAbsPath(arg);
            File tmpFile = lfs.open(tmpStr);
            if (!tmpFile.isDirectory()) {
              tmpFile.close();
              lfs.remove(tmpStr);
              if (lfs.exists(tmpStr)) {
                Serial.println("Could not remove file.");
              }
            } else {
              tmpFile.close();
              Serial.println("Not a file.");
            }
          } else {
            Serial.println("Arguments cannot be blank.");
          }
        }
        if (cmd == "htfile") {
          err = 0;
          Serial.println("htfile hex encoded file transfer tool");
          gethtfileName:
          String htfileName = getAbsPath(prompt("File name: ", 2));
          Serial.println();
          if (htfileName.charAt(htfileName.length() - 1) == '/') {
            Serial.println("Path cannot be a directory.");
            goto gethtfileName;
          }
          bool htfail = false;
          if (lfs.exists(htfileName)) {
            Serial.println("File already exists.");
            String tmpStr = prompt("Overwrite?: ", 0);
            if (tmpStr.charAt(0) == 'y' || tmpStr.charAt(0) == 'Y') {
              htfail = false;
            } else {
              htfail = true;
            }
            Serial.println();
          }
          if (!htfail) {
            Serial.print("Press ENTER when the file has been sent");
            File htfile = lfs.open(htfileName, "w");
            byte hexVal = 0;
            byte hexNum = 0;
            while (true) {
              int tmpChr = Serial.read();
              if (tmpChr == 3) {
                htfile.close();
                lfs.remove(htfileName);
                break;
              } else if (tmpChr == 13) {
                htfile.close();
                break;
              } else if (tmpChr != -1) {
                bool addHexChar = false;
                switch (char(tmpChr)) {
                  case '0':
                    hexVal += 0;
                    addHexChar = true;
                    break;
                  case '1':
                    hexVal += 1;
                    addHexChar = true;
                    break;
                  case '2':
                    hexVal += 2;
                    addHexChar = true;
                    break;
                  case '3':
                    hexVal += 3;
                    addHexChar = true;
                    break;
                  case '4':
                    hexVal += 4;
                    addHexChar = true;
                    break;
                  case '5':
                    hexVal += 5;
                    addHexChar = true;
                    break;
                  case '6':
                    hexVal += 6;
                    addHexChar = true;
                    break;
                  case '7':
                    hexVal += 7;
                    addHexChar = true;
                    break;
                  case '8':
                    hexVal += 8;
                    addHexChar = true;
                    break;
                  case '9':
                    hexVal += 9;
                    addHexChar = true;
                    break;
                  case 'a':
                  case 'A':
                    hexVal += 10;
                    addHexChar = true;
                    break;
                  case 'b':
                  case 'B':
                    hexVal += 11;
                    addHexChar = true;
                    break;
                  case 'c':
                  case 'C':
                    hexVal += 12;
                    addHexChar = true;
                    break;
                  case 'd':
                  case 'D':
                    hexVal += 13;
                    addHexChar = true;
                    break;
                  case 'e':
                  case 'E':
                    hexVal += 14;
                    addHexChar = true;
                    break;
                  case 'f':
                  case 'F':
                    hexVal += 15;
                    addHexChar = true;
                    break;
                  default:
                    break;
                }
                if (addHexChar) {
                  hexNum += 1;
                  if (hexNum > 1) {
                    hexNum = 0;
                    htfile.write(hexVal);
                    hexVal = 0;
                  } else {
                    hexVal = hexVal << 4;
                  }
                }
              }
            }
            Serial.println();
          }
        }
        if (cmd == "listfile" || cmd == "lf") {
          err = 0;
          String tmpStr = getAbsPath(arg);
          if (arg != "") {
            if (isValidPath(tmpStr)) {
              if (lfs.exists(tmpStr)) {
                File lfile = lfs.open(tmpStr, "r");
                if (lfile.isDirectory()) {
                  Serial.print("Must be file.");
                } else {
                  while (lfile.available() && Serial.read() != 3) {
                    char tmpChar = lfile.read();
                    if (tmpChar == 13) {
                      char tmpChar2 = lfile.read();
                      tmpChar = 0;
                      if (tmpChar2 != 10) {
                        Serial.println();
                        tmpChar = tmpChar2;
                      }
                    }
                    if (tmpChar == 10) {
                      char tmpChar2 = lfile.read();
                      tmpChar = 0;
                      if (tmpChar2 != 13) {
                        Serial.println();
                        tmpChar = tmpChar2;
                      }
                    }
                    Serial.write(tmpChar);
                  }
                }
                lfile.close();
              } else {
                Serial.print("File does not exist.");
              }
              Serial.println();
            } else {
              Serial.println("Invalid file/directory name.");
            }
          } else {
            Serial.println("Arguments cannot be blank.");
          }
        }
        if (cmd == "lfs_format") {
          err = 0;
          char tmpChr = prompt("Are you sure (this will erase the flash memory)? ", 0).charAt(0);
          Serial.println();
          if (tmpChr == 'y' || tmpChr == 'Y') {
            Serial.println("Formatting filesystem...");
            if (lfs.format()) {
              Serial.println("Filesystem format succeeded.");
            } else {
              Serial.println("Filesystem format failed.");
            }
          }
          cwd = "/";
        }
        if (cmd == "lfs_info") {
          err = 0;
          unsigned long fsTotalBytes = lfs.totalBytes();
          unsigned long fsUsedBytes = lfs.usedBytes();
          unsigned long fsFreeBytes = fsTotalBytes - fsUsedBytes;
          Serial.println("Total space: " + String(fsTotalBytes / 1024, DEC) + "KB");
          Serial.println("Used space: " + String(fsUsedBytes / 1024, DEC) + "KB");
          Serial.println("Free space: " + String(fsFreeBytes / 1024, DEC) + "KB");
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
        if (lfs.exists(tmpCmd)) { 
          File tmpPreChk = lfs.open(tmpCmd);
          if (tmpPreChk.isDirectory()) {
            tmpPreChk.close();
            printErr(tmpCmd + " is a directory.");
          } else {
            tmpPreChk.close();
            Serial.print("Program returned ");
            Serial.println(String(runProg(tmpCmd, arg, tmpCwd), DEC));
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
    //inChar = Serial.read();
  }
}

int runProg(String progFileName, String progArgs, String startDir) {
  // Placeholder for bytecode interpreter
  byte inst;
  switch (inst) {
    case 0:
      break;
    default:
      break;
  }
  return 0;
}

String getAbsPath(String filePath) {
  if (filePath.charAt(0) != '/') {
    filePath = cwd + filePath;
  }
  File tmpFile = lfs.open(filePath);
  if (tmpFile.isDirectory() && filePath.charAt(filePath.length() - 1) != '/') {
    filePath += "/";
  }
  tmpFile.close();
  return filePath;
}

bool isValidPath(String filePath) {
  int charCount = 0;
  charCount += filePath.indexOf(60);
  charCount += filePath.indexOf(62);
  charCount += filePath.indexOf(58);
  charCount += filePath.indexOf(34);
  charCount += filePath.indexOf(92);
  charCount += filePath.indexOf(124);
  charCount += filePath.indexOf(63);
  charCount += filePath.indexOf(42);
  return (charCount < -7);
}

void tPause(bool eraseChar) {
  while (Serial.peek() < 1) {}
  if (eraseChar) {Serial.read();}
}

String prompt(String promptString, byte promptType) {
  char cmdLine[1024];
  unsigned int cmdLinePos = 0;
  int totalChars = 0;
  bool sawPeriod = false;
  int inChar = 0;
  for (int i = 0; i < 1024; i++) {
    cmdLine[i] = 0;
  }
  Serial.print(promptString);
  while (inChar != 13) {
    inChar = Serial.read();
    if (inChar != -1) {
      switch (inChar) {
        case '\x08':
          if (cmdLinePos > 0 && totalChars > 0) {
            cmdLinePos -= 1;
            if (cmdLine[cmdLinePos] == 46) {sawPeriod = false;}
            cmdLine[cmdLinePos] = 0;
            getCursorLocation();
            if (cClmn == 1) {
              locate(80, cLine - 1);
              Serial.println(' ');
              locate(80, cLine - 1);
            } else {
              Serial.print("\x08"" ""\x08");
            }
            totalChars -= 1;
          }
          break;
        case '\x03':
          return "";
        default:
          if ((promptType == 0 && (inChar > 31 && inChar < 127)) || (promptType == 1 && ((inChar > 47 && inChar < 58) || (inChar == 46 && !sawPeriod))) || (promptType == 2 && (inChar > 31 && inChar < 127) && inChar != 60 && inChar != 62 && inChar != 58 && inChar != 34 && inChar != 92 && inChar != 124 && inChar != 63 && inChar != 42) && totalChars < 1023) {
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
            totalChars += 1;
            if (inChar == 46) {sawPeriod = true;}
          }
          break;
      }
    }
  }
  return String(cmdLine);
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
