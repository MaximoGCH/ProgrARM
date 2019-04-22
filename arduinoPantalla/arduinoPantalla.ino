
//pantalla de 240x320

#include <Elegoo_GFX.h>    // Core graphics library
#include <Elegoo_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>
#include <SPI.h>
//#include <SD.h>
#include <SdFat.h>           // Use the SdFat library
#include <math.h>

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 22
//   D1 connects to digital pin 23
//   D2 connects to digital pin 24
//   D3 connects to digital pin 25
//   D4 connects to digital pin 26
//   D5 connects to digital pin 27
//   D6 connects to digital pin 28
//   D7 connects to digital pin 29

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin
/*
  #define TS_MINX 50
  #define TS_MAXX 920

  #define TS_MINY 100
  #define TS_MAXY 940
*/
//Touch For New ILI9341 TP
#define TS_MINX 120
#define TS_MAXX 900

#define TS_MINY 70
#define TS_MAXY 920

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAYL   0xC638
#define GRAY    0x8C71
#define GRAYD   0x528A
#define COLOR_BACKGROUND        0x10A2
#define COLOR_BUTTON_BACK       0x6B4B
#define COLOR_BUTTON_AC_BACK    0x2124
#define COLOR_BUTTON_TEXT       0xFFFF


Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 40
#define PENRADIUS 3
int oldcolor, currentcolor;

int global_button_continue;
typedef struct buttons_t {
  int actionId;
  int x;
  int y;
  int w;
  int h;
  char str[50];
  void (*onclick)(int);
  int state;
  struct buttons_t *next;
};

typedef struct keypad_t {
  struct buttons_t *buttons;
  char *returnString;
  int r;
};

typedef struct projectInfo_t {
  char str[100];
  struct projectInfo_t *next;
  struct projectInfo_t *prev;
};

//variables globales
SdFat SD;
buttons_t *buttons_files;
buttons_t *buttons_ui;
keypad_t keypadVars;
int mouseX;
int mouseY;
int mouseDown;
int mouseUp;
int mouseIs;
int mouseUpdateCount;
projectInfo_t *projectInfo;
projectInfo_t *actualProjectInfo;

int pointInRectangle(int x, int y, int i, int j, int w, int h) {
  return (x >= i && y >= j && x <= i + w && y <= j + h);
}

void refreshProjectInfo() {
  projectInfo_t *aux;
  projectInfo_t *auxNext;
  projectInfo_t *newInfo;
  File dir;
  File entry;
  int i;
  char filename[100];

  aux = projectInfo;
  //borrar toda la informacion que habia antes
  while (aux != NULL) {
    auxNext = aux->next;
    free(aux);
    aux = auxNext;
  }
  projectInfo = NULL;
  actualProjectInfo = NULL;

  dir = SD.open("/");
  //saltandose el directorio del propio volumen
  entry =  dir.openNextFile();
  if (entry)
    while (true) {
      entry =  dir.openNextFile();
      if (! entry) {
        // no more files
        break;
      }
      if (!entry.isDirectory()) {
        entry.getName(filename, 100);
        newInfo = malloc(sizeof(projectInfo_t));
        strcpy(newInfo->str, filename);
        if (projectInfo == NULL) {
          newInfo->next = NULL;
          newInfo->prev = NULL;
          projectInfo = newInfo;
          aux = newInfo;
          actualProjectInfo = newInfo;
        } else {
          newInfo->prev = aux;
          newInfo->next = NULL;
          aux->next = newInfo;
          aux = newInfo;
        }
      }
      entry.close();
    }

}

void addButton(buttons_t **arr, int x, int y, int w, int h, char *str, void (*onclick)(int), int actionId) {

  buttons_t *newButton;
  newButton = malloc(sizeof(buttons_t));

  newButton->state = -1;
  newButton->actionId = actionId;
  newButton->x = x;
  newButton->y = y;
  newButton->w = w;
  newButton->h = h;
  strcpy(newButton->str, str);
  newButton->onclick = onclick;

  newButton->next = (*arr);
  (*arr) = newButton;

}

void removeButtons(buttons_t **arrP) {

  buttons_t *before;
  buttons_t *arr = *arrP;
  global_button_continue = 0;

  if (arr == NULL) {
    return;
  }

  while (arr->next != NULL) {
    before = arr;
    arr = arr->next;
    free(before);
  }
  free(arr);
  (*arrP) = NULL;

}

void updateButtons(buttons_t *arr) {
  global_button_continue = 1;
  while (arr != NULL) {
    updateButton(arr);
    if (global_button_continue == 0)
      break;
    if (arr != NULL)
      arr = arr->next;
  }
}

void updateButton(buttons_t *button) {

  int clickBool = pointInRectangle(mouseX, mouseY, button->x + 1, button->y + 1, button->w - 1, button->h - 1) && mouseIs;

  if (clickBool != button->state) {
    if (clickBool == 0) {
      drawButton(button, COLOR_BUTTON_BACK, COLOR_BUTTON_TEXT);
      if (button->onclick != NULL && button->state != -1) {
        (*(button->onclick))(button->actionId);
      }
      button->state = clickBool;
      if (global_button_continue == 0)
        return;
    } else {
      drawButton(button, COLOR_BUTTON_AC_BACK, COLOR_BUTTON_TEXT);
      button->state = clickBool;
    }
  }

}

void createButtons(buttons_t *arr) {

  while (arr != NULL) {
    drawButton(arr, COLOR_BUTTON_BACK, COLOR_BUTTON_TEXT);
    arr = arr->next;
  }

}

void drawButton(buttons_t *button, int color, int textColor) {
  tft.fillRoundRect(button->x, button->y, button->w, button->h, 10, color);
  tft.setCursor(button->x + (button->w / 2) - (6 * strlen(button->str)), button->y + (button->h / 2) - 6);
  tft.setTextColor(textColor);
  tft.setTextSize(2);
  tft.println(button->str);
}

/*
   struct buttons_t *buttons;
   char *returnString;
*/
char *keypad_getString(char* title) {
  keypadVars.buttons = NULL;
  tft.fillScreen(COLOR_BACKGROUND);
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println(title);
  keypadVars.returnString = (char *)malloc(sizeof(char));
  keypadVars.returnString[0] = '\0';
  keypadVars.r = 0;;
  tft.fillRect(20, 40, 200, 70, WHITE);
  addKeypad(0);
  while (keypadVars.r != 1) {
    updateScreen();
    updateButtons(keypadVars.buttons);
    delay(1);
  }
  removeButtons(&(keypadVars.buttons));
  keypadVars.buttons = NULL;
  return keypadVars.returnString;
}

void addKeypad(int mode) {
  char auxStr[2];
  removeButtons(&(keypadVars.buttons));
  auxStr[1] = '\0';
  char teclas1Fila[4][10] = { {'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'}, {'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'}, {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'}, {'~', '`', '|', ' ', ' ', ' ', ' ', ' ', ' ', ' '} };
  char teclas2Fila[4][10] = { {'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ñ'}, {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ñ'}, {'@', '#', '€', '_', '&', '-', '+', '(', ')', '/'}, {' ', ' ', '$', ' ', '^', ' ', '=', '{', '}', '\\'} };
  char teclas3Fila[4][7] = { {'z', 'x', 'c', 'v', 'b', 'n', 'm'}, {'Z', 'X', 'C', 'V', 'B', 'N', 'M'}, {'*', '\"', '\'', ':', ';', '!', '?'}, {'%', '<', '>', ' ', ' ', '[', ']'} };
  int i = 0;
  for (i = 0; i < 10; i++) {
    auxStr[0] = teclas1Fila[mode][i];
    addButton(&(keypadVars.buttons), 24 * i, 160, 24, 40, auxStr, &buttonHanddlerKeyPad, (int)teclas1Fila[mode][i]);
  }
  for (i = 0; i < 10; i++) {
    auxStr[0] = teclas2Fila[mode][i];
    addButton(&(keypadVars.buttons), 24 * i, 200, 24, 40, auxStr, &buttonHanddlerKeyPad, (int)teclas2Fila[mode][i]);
  }
  for (i = 0; i < 7; i++) {
    auxStr[0] = teclas3Fila[mode][i];
    addButton(&(keypadVars.buttons), 36 + 24 * i, 240, 24, 40, auxStr, &buttonHanddlerKeyPad, (int)teclas3Fila[mode][i]);
  }
  addButton(&(keypadVars.buttons), 36, 280, 24, 40, ",", &buttonHanddlerKeyPad, (int)',');
  addButton(&(keypadVars.buttons), 180, 280, 24, 40, ".", &buttonHanddlerKeyPad, (int)'.');
  addButton(&(keypadVars.buttons), 60, 280, 120, 40, "Espacio", &buttonHanddlerKeyPad, (int)' ');
  addButton(&(keypadVars.buttons), 204, 240, 36, 40, "Del", keyPad_backspace, 0);
  addButton(&(keypadVars.buttons), 204, 280, 36, 40, "Ok", &keyPad_end, 1);
  if (mode == 0) {
    addButton(&(keypadVars.buttons), 0, 240, 36, 40, "Up", &addKeypad, 1);
    addButton(&(keypadVars.buttons), 0, 280, 36, 40, "?12", &addKeypad, 2);
  } else if (mode == 1) {
    addButton(&(keypadVars.buttons), 0, 240, 36, 40, "Low", &addKeypad, 0);
    addButton(&(keypadVars.buttons), 0, 280, 36, 40, "?12", &addKeypad, 2);
  } else if (mode == 2) {
    addButton(&(keypadVars.buttons), 0, 240, 36, 40, "=/", &addKeypad, 3);
    addButton(&(keypadVars.buttons), 0, 280, 36, 40, "abc", &addKeypad, 0);
  } else if (mode == 3) {
    addButton(&(keypadVars.buttons), 0, 240, 36, 40, "?12", &addKeypad, 2);
    addButton(&(keypadVars.buttons), 0, 280, 36, 40, "abc", &addKeypad, 0);
  }

  createButtons(keypadVars.buttons);
}

void buttonHanddlerKeyPad(int id) {
  int len;
  int x;
  int y;

  len = strlen(keypadVars.returnString);
  keypadVars.returnString = (char *)realloc( keypadVars.returnString, sizeof(char) * (len + 1 + 1) );
  keypadVars.returnString[len] = (char)id;
  keypadVars.returnString[len + 1] = '\0';

  x = 12 * (len);
  y = 42 + (floor(x / 180)) * 17;
  x = 22 + (x % 180);

  tft.setCursor(x, y);
  tft.setTextColor(BLACK);
  tft.setTextSize(2);
  tft.println(keypadVars.returnString[len]);
}

void keyPad_backspace(int id) {
  int len;
  int x;
  int y;

  len = strlen(keypadVars.returnString);

  x = 12 * (len - 1);
  y = 42 + (floor(x / 180)) * 17;
  x = 22 + (x % 180);

  tft.setCursor(x, y);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println(keypadVars.returnString[len - 1]);

  keypadVars.returnString = (char *)realloc( keypadVars.returnString, sizeof(char) * (len) );
  keypadVars.returnString[len - 1] = '\0';
}

void keyPad_end(int id) {
  keypadVars.r = true;
}

#define MINPRESSURE 1
#define MAXPRESSURE 1000

void updateScreen(void) {

  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  mouseUp = 0;
  mouseDown = 0;
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = (tft.height() - map(p.y, TS_MINY, TS_MAXY, tft.height(), 0));

    mouseX = p.x;
    mouseY = p.y;
    mouseUpdateCount = 0;
    if (mouseIs == 0) {
      mouseIs = 1;
      mouseDown = 1;
    }
  } else {
    mouseUpdateCount ++;
    if (mouseUpdateCount > 50) {
      mouseUpdateCount = 50;
      if (mouseIs == 1) {
        mouseIs = 0;
        mouseUp = 1;
      }
    }
  }

}

void initProyectScreen(char *wifiName, char *ipName) {
  tft.drawRect(18, 98, 204, 204, WHITE);
  tft.drawRect(18, 18, 204, 74, WHITE);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 30);
  tft.println(wifiName);
  tft.setCursor(20, 60);
  tft.println(ipName);

  createButtons(buttons_ui);
  refreshProjectsButons(0);
}

void wifiConnection(int n) {
  Serial.println("Conectando a wifi...");
  char *netName;
  char *netPasw;
  char readChar;
  int wait, count, i;
  char *netIP;
  int netIPSize;
  
  netName = keypad_getString("NOMBRE de la red");
  netPasw = keypad_getString("CONTRASENA");
  tft.fillScreen(COLOR_BACKGROUND);
  Serial3.print(netName);
  Serial3.print('\n');
  Serial3.print(netPasw);
  count = 0;
  wait = 1;
  i = 0;
  while (wait == 1) {
    i ++;
    updateScreen();
    tft.setTextSize(2);
    if (i % 70 == 0) {
      count ++;
      if (count > 4) {
        count = 1;
      }
      if (count == 1) {
        tft.setTextColor(COLOR_BACKGROUND);
        tft.setCursor(120 - (6 * 13), 160 - 6);
        tft.println("Conectando...");
        tft.setTextColor(WHITE);
        tft.setCursor(120 - (6 * 10), 160 - 6);
        tft.println("Conectando");
      }
      if (count == 2) {
        tft.setTextColor(COLOR_BACKGROUND);
        tft.setCursor(120 - (6 * 10), 160 - 6);
        tft.println("Conectando");
        tft.setTextColor(WHITE);
        tft.setCursor(120 - (6 * 11), 160 - 6);
        tft.println("Conectando.");
      }
      if (count == 3) {
        tft.setTextColor(COLOR_BACKGROUND);
        tft.setCursor(120 - (6 * 11), 160 - 6);
        tft.println("Conectando.");
        tft.setTextColor(WHITE);
        tft.setCursor(120 - (6 * 12), 160 - 6);
        tft.println("Conectando..");
      }
      if (count == 4) {
        tft.setTextColor(COLOR_BACKGROUND);
        tft.setCursor(120 - (6 * 12), 160 - 6);
        tft.println("Conectando..");
        tft.setTextColor(WHITE);
        tft.setCursor(120 - (6 * 13), 160 - 6);
        tft.println("Conectando...");
      }
    }
    while (Serial3.available() > 0) {
      readChar = Serial3.read();
      if (readChar == '\n') {
        wait = 0;
        break;
      } else {
        Serial.print(readChar);
      }
    }
    delay(1);
  }
  Serial.println();
  while (Serial3.available() == 0) {
    delay(1);
  }
  wait = 1;
  netIPSize = 4;
  netIP = (char*) malloc(sizeof(char)*netIPSize);
  netIP[0] = 'I';
  netIP[1] = 'P';
  netIP[2] = ' ';
  netIP[3] = '\0';
  while (wait == 1) {
    readChar = Serial3.read();
    if (readChar == '\n') {
      wait = 0;
    } else {
      netIPSize ++;
      netIP = (char *)realloc(netIP, netIPSize * sizeof(char));
      netIP[netIPSize - 2] = readChar;
      netIP[netIPSize - 1] = '\0';
    }
  }
  Serial.print("IP: ");
  Serial.print(netIP);
  Serial.print('\n');
  Serial.print("netName ");
  Serial.println(netName);
  Serial.print("netPasw ");
  Serial.println(netPasw);
  initProyectScreen(netName, netIP);
}

char * searchDataName(int id) {
  int i;
  projectInfo_t *aux;
  aux = actualProjectInfo;
  for(i = 0; i < id; i++){
      aux = aux->next;
  }
  return aux->str;
}

void sendDataToArm(int id) {
  File file;
  char chr;
  String str;
  char *filename;
  
  filename = searchDataName(id);
  Serial.print("Loading ");
  Serial.println(filename);
  file = SD.open(filename);
  while (file.available()) {
    chr = file.read();
    str += chr;
  }
  Serial2.print(str);
  Serial.print(str);
  file.close();

}

void deleteData(int id) {
  char *filename;
  
  filename = searchDataName(id);
  Serial.print("Delete ");
  Serial.println(filename);
  SD.remove(filename);
  refreshProjectInfo();
  refreshProjectsButons(0);
}

void refreshProjectsButons(int dir) {
  int i;
  projectInfo_t *aux;
  char str[14];

  removeButtons(&buttons_files);
  buttons_files = NULL;
  if (actualProjectInfo != NULL) {
    if (dir > 0) {
      if (actualProjectInfo->next != NULL && actualProjectInfo->next->next != NULL && actualProjectInfo->next->next->next != NULL && actualProjectInfo->next->next->next->next != NULL)
        actualProjectInfo = actualProjectInfo->next->next->next->next;
    } else if (dir < 0) {
      if (actualProjectInfo->prev != NULL && actualProjectInfo->prev->prev != NULL && actualProjectInfo->prev->prev->prev != NULL && actualProjectInfo->prev->prev->prev->prev != NULL)
        actualProjectInfo = actualProjectInfo->prev->prev->prev->prev;
    }
    aux = actualProjectInfo;
    for (i = 0; i < 4; i++) {
      if (aux != NULL) {
        if (strlen(aux->str) > 9) {
          strncpy(str, aux->str, 8);
          str[8] = '.';
          str[9] = '.';
          str[10] = '.';
          str[11] = '\0';
        } else {
          strcpy(str, aux->str);
        }
        addButton(&buttons_files, 20, 100 + 40 * i, 170, 38, str, &sendDataToArm, i);
        addButton(&buttons_files, 190, 100 + 40 * i, 30, 38, "X", &deleteData, i);
        aux = aux->next;
      } else {
        tft.fillRect(20, 100 + 40 * i, 200, 38, BLACK);
      }
    }
  }
  createButtons(buttons_files);
}

void setup(void) {
  delay(1000);
  Serial.begin(9600);
  Serial3.begin(115200);
  Serial2.begin(9600);
  Serial.println("Start!");

  pinMode(53, OUTPUT);
  digitalWrite(53, HIGH);
  if (!SD.begin(53)) {
    Serial.println("SD initialization failed!");
    while (1);
  }

  tft.reset();

  uint16_t identifier = tft.readID();
  if (identifier == 0x9325) {
    Serial.println("Found ILI9325 LCD driver");
  } else if (identifier == 0x9328) {
    Serial.println("Found ILI9328 LCD driver");
  } else if (identifier == 0x4535) {
    Serial.println("Found LGDP4535 LCD driver");
  } else if (identifier == 0x7575) {
    Serial.println("Found HX8347G LCD driver");
  } else if (identifier == 0x9341) {
    Serial.println("Found ILI9341 LCD driver");
  } else if (identifier == 0x8357) {
    Serial.println("Found HX8357D LCD driver");
  } else if (identifier == 0x0101)
  {
    identifier = 0x9341;
    Serial.println("Found 0x9341 LCD driver");
  } else {
    Serial.print("Unknown LCD driver chip: ");
    Serial.println(identifier, HEX);

  }

  tft.begin(identifier);
  tft.setRotation(2);

  tft.fillScreen(COLOR_BACKGROUND);
  pinMode(13, OUTPUT);

  //refrescar info
  refreshProjectInfo();

  //crearTeclado(0);

  mouseX = 0;
  mouseY = 0;
  mouseDown = 0;
  mouseUp = 0;
  mouseIs = 0;
  mouseUpdateCount = 0;

  buttons_files = NULL;
  buttons_ui = NULL;

  addButton(&buttons_ui, 20, 260, 50, 40, "<", &refreshProjectsButons, -1);
  addButton(&buttons_ui, 170, 260, 50, 40, ">", &refreshProjectsButons, 1);
  addButton(&buttons_ui, 120, 20, 100, 30, "Conectar", &wifiConnection, 0);

  initProyectScreen("Red wifi","ninguna");
}

void saveFile() {
  char *filename;
  char *body;
  char readChar;
  int nombre, continueB, longFilename, longBody;
  File file;

  filename = (char *)malloc(sizeof(char));
  filename[0] = '\0';
  body = (char *)malloc(sizeof(char));;
  body[0] = '\0';
  nombre = 1;
  longFilename = 1;
  longBody = 1;

  while (Serial3.available() > 0) {
    Serial.println("Guardando archivo...");
    readChar = Serial3.read();
    if (readChar == '\n') {
      if(nombre == 1){
        nombre = 0;
      }else{
        break;
      }
    } else if (nombre == 1) {
      longFilename ++;
      filename = (char *)realloc(filename, longFilename * sizeof(char));
      filename[longFilename - 2] = readChar;
      filename[longFilename - 1] = '\0';
    } else {
      longBody ++;
      body = (char *)realloc(body, longBody * sizeof(char));
      body[longBody - 2] = readChar;
      body[longBody - 1] = '\0';
    }
  }

  Serial.print("Escribiendo... ");
  Serial.print(filename);
  Serial.println(" <- ");
  Serial.print(body);
  file = SD.open(filename, FILE_WRITE);
  file.println(body);
  file.close();
  Serial.print("Archivo guardado");  
  refreshProjectInfo();
  refreshProjectsButons(0);
}

void loop() {
  updateScreen();
  updateButtons(buttons_files);
  updateButtons(buttons_ui);
  if (Serial3.available() && Serial3.read() == 'S') {
    if (Serial3.available() && Serial3.read() == 'v')
      if (Serial3.available())
        saveFile();
  }
}
