// ============= НАСТРОЙКИ =============

#define LEDDriver 0  // 0 - встроенная самописная библиотека управления лентой (первый пиксель не мигает), 1 - универсльная (первый пиксель мигает)
#define BMEOUTON 2   // 0 - отключает (отладка), 1 - включает BME280I2C bmeOut, 2 - включает BlueDot_BME280 bmeOut, 3 - включает GyverBME280 bmeOut
#define BMEINON 2    // 0 - отключает (отладка), 1 - включает BME280I2C bmeIn, 2 - включает BlueDot_BME280 bmeIn, 3 - включает GyverBME280 bmeIn

//#define F_CPU 72000000

// ---------- МАТРИЦА ---------
#define DATA_PIN PA7        // пин ленты
#define BRIGHTNESS 20       // стандартная максимальная яркость (0-255)
#define minBRIGHTNESS 2     // минимальная яркость (1-BRIGHTNESS)
#define CURRENT_LIMIT 1000  // лимит по току в миллиамперах, автоматически управляет яркостью (пожалей свой блок питания!) 0 - выключить лимит

// ============= ДЛЯ РАЗРАБОТЧИКОВ =============
#define BTNMain_PIN PA4  // пин основной кнопки
#define BTNTime_PIN PA5  // пин кнопки установки времени
#define PHOTO PA3        // фотодатчик для настройки яркости

#define M_WIDTH 128                    // ширина матрицы
#define M_HEIGHT 8                     // высота матрицы
#define NUM_LEDS (M_WIDTH * M_HEIGHT)  // для удобства запомним и количество ледов

// ---------------- БИБЛИОТЕКИ -----------------
#if (LEDDriver == 0)
#include "WS2812Controller.h"  // светодиодная панель (самописная, сокращенная)
#elif (LEDDriver == 1)
#include <FastLED.h>  // светодиодная панель (универсльная библиотека)
#endif
#include <stm32f1_rtc.h>     // Дата, время
#include <BME280I2C.h>       // 1 вид подключения
#include <BlueDot_BME280.h>  // 2 вид подключения
#include <GyverBME280.h>     // 3 вид подключения
#include <Wire.h>
#include <GyverButton.h>
#include <EEPROM.h>
#include <MHZ19.h>  // датчик СО2


#define BME280_ADDRESS_OUT 0x77
#define BME280_ADDRESS_IN 0x76
#if (BMEOUTON == 1)
BME280I2C bmeOut;
#elif (BMEOUTON == 2)
BlueDot_BME280 bmeOut;
#elif (BMEOUTON == 3)
GyverBME280 bmeOut;
#endif
#if (BMEINON == 1)
BME280I2C bmeIn;  // Создание обьекта bme
#elif (BMEINON == 2)
BlueDot_BME280 bmeIn;
#elif (BMEINON == 3)
GyverBME280 bmeIn;
#endif


// инициализация у матрицы такая же, как у ленты, но добавляются параметры в (скобках)
#if (LEDDriver == 0)
static WS2812Controller ledStrip(NUM_LEDS, DATA_PIN);
#elif (LEDDriver == 1)
CRGB leds[NUM_LEDS];
#endif

// вид подключения
enum m_Type { ZIGZAG,
              PARALLEL };

enum m_Connection { LEFT_BOTTOM,
                    LEFT_TOP,
                    RIGHT_TOP,
                    RIGHT_BOTTOM };

enum m_Dir { DIR_RIGHT,
             DIR_UP,
             DIR_LEFT,
             DIR_DOWN };

// угол подключения: 0 LEFT_BOTTOM - левый нижний, 1 LEFT_TOP - левый верхний, 2 RIGHT_TOP - правый верхний, 3 RIGHT_BOTTOM - правый нижний
// направление ленты из угла подключения: 0 DIR_RIGHT - вправо, 1 DIR_UP - вверх, 2 DIR_LEFT - влево, 3 DIR_DOWN - вниз
//microLED<NUM_LEDS, M_PIN, MLED_NO_CLOCK, LED_WS2812, ORDER_GRB, CLI_AVER> matrix(M_WIDTH, M_HEIGHT, ZIGZAG, RIGHT_TOP, DIR_DOWN);
//matrixConfig( (uint8_t)УголПодключения | ((uint8_t)направление << 2))
// тип матрицы: 0 ZIGZAG - зигзаг, 1 PARALLEL - параллельная
int8_t matrixType = ZIGZAG;
int8_t matrixConfig = ((RIGHT_BOTTOM) | (DIR_UP << 2));
int8_t matrixW;

static const char *weekdayName[] = { "ВС", "ПН", "ВТ", "СР", "ЧТ", "ПТ", "СБ" };
static const char *monthName[] = { "ЯНВ", "ФЕВ", "МАР", "АПР", "МАЙ", "ИЮН", "ИЮЛ", "АВГ", "СЕН", "ОКТ", "НОЯ", "ДЕК" };

// переменные для времени
uint32_t epochTime, epochTimePrev, startepochTime;
byte hoursPrev = 0, minutesPrev = 0, secondsPrev = 0;
int16_t years = 0, months = 0, days = 0;
byte hours = 0, minutes = 0, seconds = 0;
DateVar date;
TimeVar time;
STM32F1_RTC rtc;
int8_t corhr = 0;  // корректировка точности хода часов за 10 дней, секунды (+/-)

MHZ19 myMHZ19;

// ------------------- ТИПЫ --------------------
//GButton button(BTN_PIN, LOW_PULL, NORM_OPEN); // для физической кнопки
GButton buttonMain(BTNMain_PIN, HIGH_PULL, NORM_OPEN);  // для основной кнопки
GButton buttonTime(BTNTime_PIN, HIGH_PULL, NORM_OPEN);  // для кнопки времени

// ----------------- ПЕРЕМЕННЫЕ ------------------

// цвета
const int32_t mWhite = 0xFFFFFF,  // белый
  mSilver = 0xC0C0C0,             // серебро
  mGray = 0x808080,               // серый
  mBlack = 0x000000,              // чёрный
  mRed = 0xFF0000,                // красный
  mMaroon = 0x800000,             // бордовый
  mOrange = 0xFF3000,             // оранжевый
  mYellow = 0xFFFF00,             // жёлтый 0xFF8000,
  mOlive = 0x808000,              // олива
  mLime = 0x00FF00,               // лайм
  mGreen = 0x008000,              // зелёный
  mAqua = 0x00FFFF,               // аква
  mTeal = 0x008080,               // цвет головы утки чирка
  mBlue = 0x0000FF,               // синий
  mNavy = 0x000080,               // тёмно-синий
  mMagenta = 0xFF00FF,            // розовый
  mPurple = 0x800080;             // пурпурный

// ----------------- переменные для вывода -----------------
float dispTempOut, dispTempIn;
float dispHumOut, dispHumIn;
float dispPresOut, dispPresIn;
float dispCO2 = 400;
float tmp;  // временная переменная - для содержания влаги в воздухе г/м3 при просмотре влажности с долями

int32_t colorTempIn, colorTempOut;  // цвета температуры для дома и улицы

// ----------------- массивы графиков -----------------
float tempDayIn[24], tempDayOut[24], tempHourIn[11], tempHourOut[11], tempMinIn[11], tempMinOut[11];
float humDayIn[24], humDayOut[24], humHourIn[11], humHourOut[11], humMinIn[11], humMinOut[11];
float pressDayOut[24], pressHourOut[11], pressMinOut[11];
float CO2Day[24], CO2Hour[11], CO2Min[11];
float tmp24In[24], tmp24Out[24];
int8_t hourInDay;          // указатель на час сутки назад (-24 + hourInDay) в суточных графиках для отображения параметра температуры, влажности, давления на указанный час (включается двойное нажатие с удержанием первой кнопки)
uint8_t updatedGraph = 7;  // признак обновления основных графиков для обновления временных (tmp) графиков. Бит 1 = 1 - обновился суточный график, Бит 2 = 1 - обновился часовой график, Бит 3 = 1 - обновился минутный график, Бит 8 = 1 - было нажатие кнопки Main
uint8_t newSec = 255;      // при обновлении секунды присваивается 255 для битового сравнения

// Массив указателей на переменные
//float *arrHourOut[4] = { tempHourOut, humHourOut, pressHourOut, tmp24Out };
//float *arrHourIn[4] = { tempHourIn, humHourIn, CO2Hour, tmp24In };
//const int32_t *arrColor[4] = { &colorTempOut, &mAqua, &mMagenta, &mBlue };

// ----------------- вид экрана -----------------
byte timeSet = 0;                        // 0 - рабочий режим, 1 - установка года, 2 - месяца, 3 - дня, 4 - часов, 5 - минут, 6 - секунд.
byte currentMode = 0;                    // основной режим отображения (0 - температура, влажность, давление; текщее: 1 - температура; 2 - влажность, 3 - абс.влажность; 4 - давление/CO2; 5 - точка росы; за последний час: 10-14... за сутки 20-24...)
byte graphModeIn = 0, graphModeOut = 0;  // отображение графика In/Out  (0 - температура, 1 - влажность, 2 - давление)
byte graphMode = 8;                      // вид табло в правой части в основном режиме (0 - давлениеOut-CO2, 1 - графикOut-давлениеOut-графикIn, 2 - графикOut-CO2, 3 - графикOut1-графикOut2-CO2, 4 - давлениеOut-графикIn, 5 - давлениеOut-графикIn1-графикIn2, 6 - графикOut1-графикOut2-графикIn1-графикIn2, 7 - графикOut-графикIn, 8  - графикOut-давлениеOut-CO2)
byte x, y, w, h, z;                      // для графиков координаты x, y, ширина, высота и кол-во значений на пиксель (глубина/масштаб)
int8_t numBME = 0;                       // для чередования чтения датчиков BME
byte scroll = 0;                         // строки скролла при изменении цифр
byte bright = 1;                         // яркость
int16_t getbright = 4000;                // яркость, полученная от датчика
boolean hulfTime;                        // полусекундие :)

//----------------- таймеры -----------------
unsigned long sensorsTimer = 150, sensorsTimerD = 0;  // время обновления показаний сенсоров в памяти, миллисекунд !!!250 * (231 / 360) - отставание скорости тиков millis()
unsigned long redrawTimer = 40, redrawTimerD = 0;     // время обновления показаний экрана, миллисекунд
unsigned long holdTimer = 700, holdTimerD = 0;        // время удержания кнопки для смены формата графиков, миллисекунд
unsigned long holdSetTime = 150, holdSetTimeD = 0;    // время удержания кнопки при установке даты/времени, миллисекунд
unsigned long CO2Timer = 5000, CO2TimerD = 0;         // время обновления показаний CO2 в памяти, миллисекунд


// ====================== Настройка ===============================================================================================================================================
void setup() {

  buttonMain.setStepTimeout(100);
  buttonMain.setClickTimeout(500);
  buttonTime.setStepTimeout(100);
  buttonTime.setClickTimeout(500);

  Serial.begin(9600);
  Serial3.begin(9600);
  myMHZ19.begin(Serial3);
  myMHZ19.autoCalibration(false);
  dispCO2 = myMHZ19.getCO2();  // первое считывание пустое

#if (LEDDriver == 1)
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  //NEOPIXEL  WS2812B
#endif

  // https://github.com/ZulNs/STM32F1_RTC

  if (!rtc.begin()) Serial.println("RTS not started!");
  else Serial.println("RTS started");

  epochTime = rtc.getTime();
  if (epochTime < 1000) {     // если дата и время не сохранено, ставим дефолтное
    rtc.setTime(1723852741);  // excel =(СЕГОДНЯ()-ДАТА(1970;1;1))*86400+ВРЕМЯ(7;15;1)*86400 или =(ДАТА(2024;4;30)-ДАТА(1970;1;1))*86400+ВРЕМЯ(23;59;1)*86400

    //описание функций __TIME__ и __DATE__
    //https://stackoverflow.com/questions/11697820/how-to-use-date-and-time-predefined-macros-in-as-two-integers-then-stri
    epochTime = rtc.getTime();
  }
  startepochTime = epochTime;
  rtc.epochToDate(epochTime, date);
  rtc.epochToTime(epochTime, time);

  // Радужная шахматка:////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  for (uint16_t x = 0; x < 64; x++) {
    for (uint16_t i = 0; i < 256; i++) {  //NUM_LEDS
#if (LEDDriver == 0)
      ledStrip.setPixel(i * 2, brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel((i * 2 + 256), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel((i * 2 + 512), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel((i * 2 + 768), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel(255 - (i * 2), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel(511 - (i * 2), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel(767 - (i * 2), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
      ledStrip.setPixel(1023 - (i * 2), brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0));
#elif (LEDDriver == 1)
      leds[i * 2] = brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0);
      leds[i * 2 + 256] = brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0);
      leds[i * 2 + 512] = brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0);
      leds[511 - (i * 2)] = brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0);
      leds[767 - (i * 2)] = brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0);
      leds[1023 - (i * 2)] = brightRatio(Wheel((i * 2 + x * 8) & 255), 19 - abs(map(x, 0, 63, -19, 19)), 0);
#endif
    }
#if (LEDDriver == 0)
    ledStrip.show();
#elif (LEDDriver == 1)
    FastLED.show();
#endif
  }
  // ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  EEPROM.init();

  if (((int8_t)(EEPROM.read(0)) >= 100) || ((int8_t)(EEPROM.read(0)) <= -100)) EEPROM.write(0, corhr);  // если мусор, то по умолчанию
  corhr = (int8_t)EEPROM.read(0);
  //if ((((EEPROM.read(4))) > 18) || (((EEPROM.read(4))) < 0)) EEPROM.write(4, viewTime);  // если мусор, то по умолчанию
  //viewTime = (EEPROM.read(4));
  if ((((EEPROM.read(8))) > 3) || (((EEPROM.read(8))) < 0)) EEPROM.write(8, currentMode);  // если мусор, то по умолчанию
  currentMode = (EEPROM.read(8));
  if ((((EEPROM.read(12))) > 2) || (((EEPROM.read(12))) < 0)) EEPROM.write(12, graphModeIn);  // если мусор, то по умолчанию
  graphModeIn = (EEPROM.read(12));
  if ((((EEPROM.read(16))) > 2) || (((EEPROM.read(16))) < 0)) EEPROM.write(16, graphModeOut);  // если мусор, то по умолчанию
  graphModeOut = (EEPROM.read(16));
  if ((((EEPROM.read(20))) > 8) || (((EEPROM.read(20))) < 0)) EEPROM.write(20, graphMode);  // если мусор, то по умолчанию
  graphMode = (EEPROM.read(20));


  //RTC_SECONDS_INTERRUPT An occured event every time the RTC counter register incremented.
  //secondsTick - The function to call when the interrupt occurs.
  //rtc.attachInterrupt(RTC_SECONDS_INTERRUPT, secondsTick);
  //можно вместо прерывания выше использовать контрукцию в LOOP:
  //if (rtc.isCounterUpdated()) {rtc.clearSecondFlag();}

  pinMode(PHOTO, INPUT_PULLUP);
  pinMode(PB6, INPUT_PULLUP);  // подтяжка пина BME (работает и без этого)
  pinMode(PB7, INPUT_PULLUP);  // подтяжка пина BME (работает и без этого)
                               //pinMode(PA7, INPUT_PULLUP);  // подтяжка пина LED (работает и без этого)

  Wire.begin();
  Wire.setClock(20000);  // снижаем частоту шины до 20 кГц (со стандартных 100кГц) для упрощения прохождения сигнала по длинному проводу BME280 (при 10кГц анимация экрана подтормаживает (смена секунд))
  //Serial.println("Wire started");

// Настройка адреса и инициализация первого сенсора
#if (BMEOUTON == 1)
  BME280I2C::Settings settingsOut;
  settingsOut.bme280Addr = BME280I2C::I2CAddr_0x77;  // поменять на 77!!!
  bmeOut.setSettings(settingsOut);
  if (!bmeOut.begin()) Serial.println("Out Could not find BME280 sensor!");
  else Serial.println("Out BME280 sensor started");
#elif (BMEOUTON == 2)
  bmeOut.parameter.communication = 0;           //I2C communication for Sensor 1
  bmeOut.parameter.I2CAddress = 0x77;           //Set the I2C address of your breakout board
  bmeOut.parameter.sensorMode = 0b11;           //Setup Sensor mode for Sensor 1
  bmeOut.parameter.IIRfilter = 0b100;           //IIR Filter for Sensor 1
  bmeOut.parameter.humidOversampling = 0b101;   //Humidity Oversampling for Sensor 1
  bmeOut.parameter.tempOversampling = 0b101;    //Temperature Oversampling for Sensor 1
  bmeOut.parameter.pressOversampling = 0b101;   //Pressure Oversampling for Sensor 1
  bmeOut.parameter.pressureSeaLevel = 1013.25;  //default value of 1013.25 hPa (Sensor 1)
  bmeOut.parameter.tempOutsideCelsius = 15;     //default value of 15°C
  bmeOut.parameter.tempOutsideFahrenheit = 59;  //default value of 59°F
  bmeOut.init();
#elif (BMEOUTON == 3)
  //bmeOut.setStandbyTime(STANDBY_10MS);
  bmeOut.setMode(NORMAL_MODE);
  bmeOut.begin(BME280_ADDRESS_OUT);
#endif

// Настройка адреса и инициализация второго сенсора
#if (BMEINON == 1)
  BME280I2C::Settings settingsIn;
  settingsIn.bme280Addr = BME280I2C::I2CAddr_0x76;
  bmeIn.setSettings(settingsIn);
  if (!bmeIn.begin()) Serial.println("In Could not find BME280 sensor!");
  else Serial.println("In BME280 sensor started");
#elif (BMEINON == 2)
  bmeIn.parameter.communication = 0;           //I2C communication for Sensor 2
  bmeIn.parameter.I2CAddress = 0x76;           //Set the I2C address of your breakout board
  bmeIn.parameter.sensorMode = 0b11;           //Setup Sensor mode for Sensor 2
  bmeIn.parameter.IIRfilter = 0b100;           //IIR Filter for Sensor 2
  bmeIn.parameter.humidOversampling = 0b101;   //Humidity Oversampling for Sensor 2
  bmeIn.parameter.tempOversampling = 0b101;    //Temperature Oversampling for Sensor 2
  bmeIn.parameter.pressOversampling = 0b101;   //Pressure Oversampling for Sensor 2
  bmeIn.parameter.pressureSeaLevel = 1013.25;  //default value of 1013.25 hPa (Sensor 2)
  bmeIn.parameter.tempOutsideCelsius = 15;     //default value of 15°C
  bmeIn.parameter.tempOutsideFahrenheit = 59;  //default value of 59°F
  bmeIn.init();                                //if (bme2.init() != 0x60) bme2Detected = 0;
#elif (BMEINON == 3)
  //bmeIn.setStandbyTime(STANDBY_10MS);
  bmeIn.setMode(NORMAL_MODE);
  bmeIn.begin(BME280_ADDRESS_IN);
#endif

  readSensors(0);
  delay(250);
  readSensors(0);

  if (matrixConfig == 4 || matrixConfig == 13 || matrixConfig == 14 || matrixConfig == 7) matrixW = M_HEIGHT;
  else matrixW = M_WIDTH;

  for (byte i = 0; i < 24; i++) {  // счётчик от 0 до 10
    if (i < 11) {
      tempHourIn[i] = dispTempIn;
      tempHourOut[i] = dispTempOut;
      humHourIn[i] = dispHumIn;
      humHourOut[i] = dispHumOut;
      pressHourOut[i] = dispPresOut;
      CO2Hour[i] = 400;
      tempMinIn[i] = dispTempIn;
      tempMinOut[i] = dispTempOut;
      humMinIn[i] = dispHumIn;
      humMinOut[i] = dispHumOut;
      pressMinOut[i] = dispPresOut;
      CO2Min[i] = 400;
    }
    tempDayIn[i] = dispTempIn;
    tempDayOut[i] = dispTempOut;
    humDayIn[i] = dispHumIn;
    humDayOut[i] = dispHumOut;
    pressDayOut[i] = dispPresOut;
    CO2Day[i] = 400;  //dispCO2;
  }
}


// ==================== Читаем показания датчиков с усреднением avr, чтобы не было резких скачков (с)НР - "скользящее среднее" ====================================================
void readSensors(byte avr) {
  float temp, hum, pres;
  if (numBME) {  //считывание датчиков по очереди, чтобы не мешали друг другу
#if (BMEINON == 1)
    bmeIn.read(pres, temp, hum);
    pres = pressureToMmHg1(pres);
    if ((pres > 800) || (pres < 500)) bmeIn.begin();  // если сбой в работе датчика (т.е. давление нереальное), то переинициализируем датчик
#elif (BMEINON == 2)
    temp = bmeIn.readTempC();
    hum = bmeIn.readHumidity();
    pres = pressureToMmHg1(bmeIn.readPressure());
    if ((pres > 800) || (pres < 500)) bmeIn.init();  // если сбой в работе датчика (т.е. давление нереальное), то переинициализируем датчик
#elif (BMEINON == 3)
    while (bmeIn.isMeasuring())
      ;
    temp = bmeIn.readTemperature();
    hum = bmeIn.readHumidity();
    pres = pressureToMmHg(bmeIn.readPressure());
    if ((pres > 800) || (pres < 500)) bmeIn.begin(BME280_ADDRESS_IN);  // если сбой в работе датчика (т.е. давление нереальное), то переинициализируем датчик
#endif
#if (BMEINON != 0)
    dispTempIn = ((float)dispTempIn * avr + temp - 1) / (avr + 1);  // "-1" - поправка на нагрев(?) датчика
    dispHumIn = ((float)dispHumIn * avr + hum) / (float)(avr + 1);
    dispPresIn = ((float)dispPresIn * avr + pres) / (avr + 1);
#endif
  } else {
#if (BMEOUTON == 1)
    bmeOut.read(pres, temp, hum);
    pres = pressureToMmHg1(pres);
    if ((pres > 800) || (pres < 500)) bmeOut.begin();  // если сбой в работе датчика (т.е. давление нереальное), то переинициализируем датчик
#elif (BMEOUTON == 2)
    temp = bmeOut.readTempC();
    hum = bmeOut.readHumidity();
    pres = pressureToMmHg1(bmeOut.readPressure());
    if ((pres > 800) || (pres < 500)) bmeOut.init();  // если сбой в работе датчика (т.е. давление нереальное), то переинициализируем датчик
#elif (BMEOUTON == 3)
    while (bmeOut.isMeasuring())
      ;
    temp = bmeOut.readTemperature();
    hum = bmeOut.readHumidity();
    pres = pressureToMmHg(bmeOut.readPressure());
    if ((pres > 800) || (pres < 500)) bmeOut.begin(BME280_ADDRESS_OUT);  // если сбой в работе датчика (т.е. давление нереальное), то переинициализируем датчик
#endif
#if (BMEOUTON != 0)
    dispTempOut = ((float)dispTempOut * avr + temp - 1) / (avr + 1);  // "-1" - поправка на нагрев(?) датчика
    dispHumOut = ((float)dispHumOut * avr + hum) / (float)(avr + 1);
    dispPresOut = ((float)dispPresOut * avr + pres) / (avr + 1);
#endif
  }
  numBME = !numBME;  // поочередное считывание показаний, чтобы датчики не мешали друг другу
}

// ====================== Перерисовываем экран (с)НР ==============================================================================================================================
void drawScreen() {

#if (LEDDriver == 0)
  ledStrip.clear();
#elif (LEDDriver == 1)
  FastLED.clearData();
#endif

  int32_t colorTime = (hulfTime) ? mYellow : mLime;  // для мигания
  unsigned long clkTmr;
  int32_t colorTempIn = ((dispTempIn < 0) ? mBlue : mYellow);    // для отрицательной температуры - синий цвет, для положительной - желтый
  int32_t colorTempOut = ((dispTempOut < 0) ? mBlue : mYellow);  // для отрицательной температуры - синий цвет, для положительной - желтый

  // ====================================== Время, дата, день недели. + настройка времени ======================================
  switch (timeSet) {
    case 0:  // стандартный режим - нет установки даты/времени
      // ====================================== Температура, давление, влажность ======================================
      printTime((currentMode < 10) ? 1 : 0, 0, (currentMode > 19) ? 2 : 4, timeSet, colorTime, (currentMode < 10) ? 1 : 0);  // Вывод времени

      tempMinOut[10] = dispTempOut;  // обновляем последнее значение минутных графиков текущим значенем
      humMinOut[10] = dispHumOut;
      pressMinOut[10] = dispPresOut;
      tempMinIn[10] = dispTempIn;
      humMinIn[10] = dispHumIn;
      CO2Min[10] = dispCO2;
      tempHourOut[10] = dispTempOut;  // обновляем последнее значение часовых графиков текущим значенем
      humHourOut[10] = dispHumOut;
      pressHourOut[10] = dispPresOut;
      tempHourIn[10] = dispTempIn;
      humHourIn[10] = dispHumIn;
      CO2Hour[10] = dispCO2;
      /*tempDayIn[23] = dispTempIn;  // обновляем последнее значение суточных графиков текущим значенем
      tempDayOut[23] = dispTempOut;
      humDayIn[23] = dispHumIn;
      humDayOut[23] = dispHumOut;
      pressDayOut[23] = dispPresOut;
      CO2Day[23] = dispCO2;*/

      switch (currentMode) {
        case 0:  // Главный экран (температура, влажность и давление)

          digPrint(0, 9, date.day, 0xFFFF80, 1, 2, 0, 0);                                                                          // число
          symPrint(9, 9, monthName[date.month - 1], 0xFFFF80, true);                                                               // месяц
          symPrint(25, 9, weekdayName[date.weekday], (((date.weekday == 0) || (date.weekday == 6)) ? 0xFF0000 : 0xFF8060), true);  // день недели

          printTemp(0, 16, dispTempOut, 2, 0, 0);  // Вывод температуры -----------------------------------------------------------------------------------------------------------------------------------------------------------
          printTemp(0, 25, dispTempIn, 2, 0, 0);
          printHum(10, 16, dispHumOut, 2, 0, 0);  // Вывод влажности --------------------------------------------------------------------------------------------------------------------------------------------------------------
          printHum(10, 25, dispHumIn, 2, 0, 0);

          // Вывод CO2 (давления) ---------------------------------------------------------------------------------------------------------------------------------------------------------------
          // In ----------
          if ((graphMode == 0) || (graphMode == 2)) {
            digPrint(21, 8 + 1 + 16, dispCO2 / 10, 0xFFFF88, 2, 3, 0, 0);  // крупное CO2
            mSet(24, 14 + 1 + 16, mBlue);                                  // точка
          }
          if ((graphMode == 3) || (graphMode == 8)) {
            digPrint(21, 0 + 11 + 16, dispCO2 / 10, 0xFFFF88, 1, 3, 0, 0);  // мелкое CO2 внизу
            mSet(24, 14 + 1 + 16, mBlue);                                   // точка
          }

          // Out ----------
          if ((graphMode == 0) || (graphMode == 4)) printPress(21, 16, dispPresOut, 2, 0, 0);  // крупное давлениеOut
          if ((graphMode == 1) || (graphMode == 8)) printPress(21, 21, dispPresOut, 1, 0, 0);  // мелкое давлениеOut в середине
          if (graphMode == 5) printPress(21, 16, dispPresOut, 1, 0, 0);                        // мелкое давлениеOut вверху

          // Вывод графиков ---------------------------------------------------------------------------------------------------------------------------------------------------------------
          w = 11;
          // In ----------
          if ((graphMode == 1) || (graphMode == 4) || (graphMode == 5) || (graphMode == 6) || (graphMode == 7)) {
            x = 21;
            //  w = 11;
            y = 11;
            h = 5;
            z = 2;  // 2 пикселя на градус - 0,5 градуса на пикс
            if (graphMode == 5) y = 6;
            if ((graphMode == 4) || (graphMode == 7)) {
              y = 9;
              h = 7;
            }
            if (graphMode == 6) {
              y = 8;
              h = 4;
              z = 3;  // 3 пикселя на градус - 0,33 градуса на пикс
            }
            switch (graphModeIn) {
              //drawPlot([x], [y], [ширина], [высота], [мин], [макс], [(int*)массив], [цвет], [пикселей на единицу [если 0, то от мин до макс]])
              case 0:
                drawPlot(x, y + 16, w, h, 22, 30, (float *)tempHourIn, colorTempIn, z, 0);
                break;
              case 1:
                drawPlot(x, y + 16, w, h, 20, 80, (float *)humHourIn, mAqua, 5, 0);
                break;
              case 2:
                drawPlot(x, y + 16, w, h, 400, 2000, (float *)CO2Hour, 0xFFFF88, 0.005, 0);
                break;
            }
          }
          if ((graphMode == 5) || (graphMode == 6)) {  // второй график
            y = 11;
            if (graphMode == 6) y = 12;
            switch (graphModeIn) {  // смещаем на один график, чтобы выводился следующий
              //drawPlot([x], [y], [ширина], [высота], [мин], [макс], [(int*)массив], [цвет], [пикселей на единицу [если 0, то от мин до макс]])
              case 0:
                drawPlot(x, y + 16, w, h, 20, 80, (float *)humHourIn, mAqua, 5, 0);
                break;
              case 1:
                drawPlot(x, y + 16, w, h, 400, 2000, (float *)CO2Hour, 0xFFFF88, 0.005, 0);
                break;
              case 2:
                drawPlot(x, y + 16, w, h, 22, 30, (float *)tempHourIn, colorTempIn, z, 0);
                break;
            }
          }

          // Out ----------
          if ((graphMode == 1) || (graphMode == 2) || (graphMode == 3) || (graphMode == 6) || (graphMode == 7) || (graphMode == 8)) {
            x = 21;
            //    w = 11;
            y = 0;
            h = 5;
            z = 3;  // 3 пикселя на градус - 0,33 градуса на пикс
            if ((graphMode == 2) || (graphMode == 7)) {
              h = 7;
              z = 2;  // 2 пикселя на градус - 0,5 градуса на пикс
            }
            if (graphMode == 6) {
              h = 4;  // 4 пикселя на градус - 0,25 градуса на пикс
            }
            switch (graphModeOut) {
              case 0:
                drawPlot(x, y + 16 - 1, w, h, -30, 30, (float *)tempHourOut, colorTempOut, z, 0);
                break;
              case 1:
                drawPlot(x, y + 16 - 1, w, h, 20, 80, (float *)humHourOut, mAqua, 5, 0);
                break;
              case 2:
                drawPlot(x, y + 16 - 1, w, h, 730, 750, (float *)pressHourOut, mMagenta, 5, 0);
                break;
            }
            if ((graphMode == 3) || (graphMode == 6)) {  // второй график
              y = 5;
              if (graphMode == 6) y = 4;

              switch (graphModeOut) {  // смещаем на один график, чтобы выводился следующий
                case 0:
                  drawPlot(x, y + 16 - 1, w, h, 20, 80, (float *)humHourOut, mAqua, 5, 0);
                  break;
                case 1:
                  drawPlot(x, y + 16 - 1, w, h, 730, 750, (float *)pressHourOut, mMagenta, 5, 0);
                  break;
                case 2:
                  drawPlot(x, y + 16 - 1, w, h, -30, 30, (float *)tempHourOut, colorTempOut, z, 0);
                  break;
              }
            }
          }
          break;

        case 1:  // Вывод температуры с долями -----------------------------------------------------------------------

          symPrint(0, 9, "ТЕМП.\0", mWhite, 1);
          symPrint(21, 9, "ТЕК\0", mWhite, 1);
          printTemp(0, 16, dispTempOut, 2, -3, 1);
          printTemp(0, 25, dispTempIn, 2, -3, 1);
          drawPlot(21, 16, 11, 7, -10, 30, (float *)tempMinOut, colorTempOut, 200, 0);  // график температуры
          drawPlot(21, 25, 11, 7, -10, 30, (float *)tempMinIn, colorTempIn, 200, 0);    // график температуры
          break;
        case 2:  // Вывод относительной влажности с долями -----------------------------------------------------------------------

          symPrint(0, 9, "ВЛАЖ.\0", mWhite, 1);
          symPrint(21, 9, "ОТН\0", mWhite, 1);
          printHum(0, 25, dispHumIn, 2, -2, 1);                              // In ----------
          printHum(0, 16, dispHumOut, 2, -2, 1);                             // Out ----------
          drawPlot(21, 16, 11, 7, 0, 70, (float *)humMinOut, mAqua, 25, 0);  // график отн. влажности
          drawPlot(21, 25, 11, 7, 0, 70, (float *)humMinIn, mAqua, 25, 0);   // график отн. влажности
          break;
        case 3:  // Вывод абсолютной влажности с долями -----------------------------------------------------------------------

          symPrint(0, 9, "ВЛАЖ.\0", mWhite, 1);
          symPrint(21, 9, "АБС\0", mWhite, 1);
          if (updatedGraph & (1 << 7)) {  // установлен бит переключения
            for (byte i = 0; i < 11; i++) {
              tmp24Out[i] = absHum(tempMinOut[i], humMinOut[i], pressMinOut[i]);
              tmp24In[i] = absHum(tempMinIn[i], humMinIn[i], pressMinOut[i]);
            }
            updatedGraph &= ~(1 << 7);  // обнуление бита
          }
          if (updatedGraph & (1 << 2)) {  // установлен бит обновления минутного графика
            plotSensorsTmpTick(absHum(dispTempIn, dispHumIn, dispPresOut), absHum(dispTempOut, dispHumOut, dispPresOut), 10);
            updatedGraph &= ~(1 << 2);  // обнуление бита
          }
          tmp24In[10] = absHum(dispTempIn, dispHumIn, dispPresOut);             // постоянное обновление последнего значения графика
          tmp24Out[10] = absHum(dispTempOut, dispHumOut, dispPresOut);          // постоянное обновление последнего значения графика
          printHumAbs(0, 25, tmp24In[10], 2, -2, 1);                            // In ----------
          printHumAbs(0, 16, tmp24Out[10], 2, -2, 1);                           // Out ----------
          drawPlot(21, 16, 11, 7, 0, 70, (float *)tmp24Out, 0x00FF44, 100, 0);  // график абс. влажности
          drawPlot(21, 25, 11, 7, 0, 70, (float *)tmp24In, 0x00FF44, 100, 0);   // график абс. влажности
          break;
        case 4:  // Вывод давления/CO2 с долями -----------------------------------------------------------------------

          symPrint(0, 9, "ДАВЛ.\0", mWhite, 1);
          symPrint(21, 9, "СО\0", mWhite, 1);
          digPrint(29, 9, 2, mWhite, 1, 1, 0, 0);
          printPress(0, 16, dispPresOut, 2, -2, 1);        // давление
          digPrint(0, 25, dispCO2, 0xFFFF88, 2, 4, 0, 0);  // основное значение CO2
          //symPrint(18, 9 + 16 + 2, "РРМ\0", mWhite, true);     // надпись ppm
          drawPlot(21, 16, 11, 7, 0, 70, (float *)pressMinOut, mMagenta, 100, 0);  // график давление
          drawPlot(21, 25, 11, 7, 0, 70, (float *)CO2Min, 0xFFFF88, 0.005, 0);     // график СО2
          break;
        case 5:  // Вывод точки росы -----------------------------------------------------------------------

          symbSetx(0, 9, 0b111011011010011011010011011010011001010011001, 9, 5, mWhite);  // символы "ТОЧ"
          symPrint(10, 9, "КА\0", mWhite, 1);
          symbSetx(19, 9, 0b1101100101011011010010110110100111001101001110011001011, 11, 5, mWhite);  // символы "РОСЫ"
          mSetFill(31, 9, mWhite, 1, 5);                                                              // хвостик от буквы Ы
          if (updatedGraph & (1 << 7)) {                                                              // установлен бит переключения
            for (byte i = 0; i < 11; i++) {
              tmp24Out[i] = dispDewPoint(tempMinOut[i], humMinOut[i]);
              tmp24In[i] = dispDewPoint(tempMinIn[i], humMinIn[i]);
            }
            updatedGraph &= ~((1 << 2) | (1 << 7));  // обнуление битов (обоих, т.к. значение для графика обновлены)
          }
          if (updatedGraph & (1 << 2)) {  // установлен бит обновления минутного графика)
            plotSensorsTmpTick(dispDewPoint(dispTempIn, dispHumIn), dispDewPoint(dispTempOut, dispHumOut), 10);
            updatedGraph &= ~(1 << 2);  // обнуление бита обновления минутного графика
          }
          tmp24In[10] = dispDewPoint(dispTempIn, dispHumIn);     // постоянное обновление последнего значения графика
          tmp24Out[10] = dispDewPoint(dispTempOut, dispHumOut);  // постоянное обновление последнего значения графика
          printTempDew(0, 25, tmp24In[10], 2, -2, 1);            // In ----------
          printTempDew(0, 16, tmp24Out[10], 2, -2, 1);           // Out ----------

          drawPlot(21, 16, 11, 7, 0, 70, (float *)tmp24Out, ((tmp24Out[10] < 0) ? 0x0088FF : 0xBBFF00), 50, 0);  // график точки росы
          drawPlot(21, 25, 11, 7, 0, 70, (float *)tmp24In, ((tmp24In[10] < 0) ? 0x0088FF : 0xBBFF00), 50, 0);    // график точки росы
          break;
        case 6:  // переход на 10
          currentMode = 10;
          break;
        case 10:  // отображаем температуру за час (начало и конец) Out и In -----------------------------------------------------------------------

          symPrint(0, 9, "ТЕМП.\0", mWhite, 1);
          symPrint(21, 9, "ЧАС\0", mWhite, 1);
          printTemp(0, 16, tempHourOut[0], 2, -2, 0);  // вывод температуры час назад
          printTemp(17, 16, dispTempOut, 2, -2, 0);    // вывод текущей температуры
          mSetFill(15, 16 + 3, mGray, 2, 1);           // тире (-)
          printTemp(0, 25, tempHourIn[0], 2, -2, 0);   // вывод температуры час назад
          printTemp(17, 25, dispTempIn, 2, -2, 0);     // вывод текущей температуры
          mSetFill(15, 25 + 3, mGray, 2, 1);           // тире (-)
          //drawPlot(22, 0, 10, 4, 0, 1, (float *)arrHourOut[0], *arrColor[0], 2, 1);  // график
          //drawPlot(22, 4, 10, 4, 0, 1, (float *)arrHourIn[0], *arrColor[0], 2, 1);   // график
          drawPlot(22, 0, 10, 4, 0, 1, (float *)tempHourOut, colorTempOut, 2, 1);  // график
          drawPlot(22, 4, 10, 4, 0, 1, (float *)tempHourIn, colorTempIn, 2, 1);    // график
          break;
        case 11:  // отображаем относительную влажность за час (начало и конец) Out и In -----------------------------------------------------------------------

          symPrint(0, 9, "ВЛАЖ.\0", mWhite, 1);
          symPrint(21, 9, "ОТН\0", mWhite, 1);
          printHum(0, 16, humHourOut[0], 2, -1, 1);                        // вывод отн.влажности час назад
          printHum(18, 16, dispHumOut, 2, -1, 1);                          // вывод текущей отн.влажности
          printHum(0, 25, humHourIn[0], 2, -1, 1);                         // вывод отн.влажности час назад
          printHum(18, 25, dispHumIn, 2, -1, 1);                           // вывод текущей отн.влажности
          mSetFill(15, 16 + 3, mGray, 2, 1);                               // тире (-)
          mSetFill(15, 25 + 3, mGray, 2, 1);                               // тире (-)
          drawPlot(22, 0, 10, 4, 0, 1, (float *)humHourOut, mAqua, 2, 1);  // график
          drawPlot(22, 4, 10, 4, 0, 1, (float *)humHourIn, mAqua, 2, 1);   // график
          break;
        case 12:  // отображаем абсолютную влажность за час (начало и конец) Out и In -----------------------------------------------------------------------

          symPrint(0, 9, "ВЛАЖ.\0", mWhite, 1);
          symPrint(21, 9, "АБС\0", mWhite, 1);
          if ((updatedGraph & (1 << 1)) || (updatedGraph & (1 << 7))) {  // установлен бит обновления часового графика
            for (byte i = 0; i < 11; i++) {
              tmp24Out[i] = absHum(tempHourOut[i], humHourOut[i], pressHourOut[i]);
              tmp24In[i] = absHum(tempHourIn[i], humHourIn[i], pressHourOut[i]);
            }
            updatedGraph &= ~(1 << 1);  // обнуление бита
            updatedGraph &= ~(1 << 7);  // обнуление бита
          }
          tmp24Out[10] = absHum(dispTempOut, dispHumOut, dispPresOut);      // постоянное обновление текущего значения
          tmp24In[10] = absHum(dispTempIn, dispHumIn, dispPresOut);         // постоянное обновление текущего значения
          printHumAbs(0, 16, tmp24Out[0], 2, -1, 1);                        // вывод абс.влажности час назад
          printHumAbs(18, 16, tmp24Out[10], 2, -1, 1);                      // вывод текущей абс.влажности
          mSetFill(15, 16 + 3, mGray, 2, 1);                                // тире (-)
          printHumAbs(0, 25, tmp24In[0], 2, -1, 1);                         // вывод абс.влажности час назад
          printHumAbs(18, 25, tmp24In[10], 2, -1, 1);                       // вывод текущей абс.влажности
          mSetFill(15, 25 + 3, mGray, 2, 1);                                // тире (-)
          drawPlot(22, 0, 10, 4, 0, 1, (float *)tmp24Out, 0x00FF44, 2, 1);  // график
          drawPlot(22, 4, 10, 4, 0, 1, (float *)tmp24In, 0x00FF44, 2, 1);   // график
          break;
        case 13:  // отображаем давление/CO2 за час (начало и конец) -----------------------------------------------------------------------

          symPrint(0, 9, "ДАВЛ.\0", mWhite, 1);
          symPrint(21, 9, "СО\0", mWhite, 1);
          digPrint(29, 9, 2, mWhite, 1, 1, 0, 0);
          printPress(0, 16, pressHourOut[0], 2, -1, 0);                         // вывод давления час назад
          printPress(18, 16, dispPresOut, 2, -1, 0);                            // вывод текущего давления
          digPrint(0, 25, CO2Hour[0], 0xFFFF88, 2, 4, 0, 0);                    // основное значение CO2 час назад
          digPrint(17, 25, dispCO2, 0xFFFF88, 2, 4, 0, 0);                      // основное значение CO2
          mSetFill(15, 16 + 3, mGray, 2, 1);                                    // тире (-)
          mSetFill(15, 25 + 3, mGray, 2, 1);                                    // тире (-)
          drawPlot(22, 0, 10, 4, 0, 1, (float *)pressHourOut, mMagenta, 1, 1);  // график
          drawPlot(22, 4, 10, 4, 0, 1, (float *)CO2Hour, 0xFFFF88, 0.005, 1);   // график
          break;
        case 14:  // отображаем точку росы за час (начало и конец) -----------------------------------------------------------------------

          symbSetx(0, 9, 0b111011011010011011010011011010011001010011001, 9, 5, mWhite);  // символы "ТОЧ"
          symPrint(10, 9, "КА\0", mWhite, 1);
          symbSetx(19, 9, 0b1101100101011011010010110110100111001101001110011001011, 11, 5, mWhite);  // символы "РОСЫ"
          mSetFill(31, 9, mWhite, 1, 5);                                                              // хвостик от буквы Ы
          if (updatedGraph & (1 << 7)) {                                                              // если было переключение, то заполняем текущим значением
            for (byte i = 0; i < 11; i++) {
              tmp24Out[i] = dispDewPoint(tempHourOut[i], humHourOut[i]);
              tmp24In[i] = dispDewPoint(tempHourIn[i], humHourIn[i]);
            }
            updatedGraph &= ~((1 << 1) | (1 << 7));  // обнуление битов (обоих, т.к. значение для графика обновлены)
          }
          if (updatedGraph & (1 << 1)) {  // установлен бит обновления часового графика)
            plotSensorsTmpTick(dispDewPoint(dispTempIn, dispHumIn), dispDewPoint(dispTempOut, dispHumOut), 10);
            updatedGraph &= ~(1 << 1);  // обнуление бита обновления часового графика
          }
          tmp24In[10] = dispDewPoint(dispTempIn, dispHumIn);                                                  // постоянное обновление последнего значения графика
          tmp24Out[10] = dispDewPoint(dispTempOut, dispHumOut);                                               // постоянное обновление последнего значения графика
          printTempDew(0, 16, tmp24Out[0], 2, -2, 0);                                                         // вывод точки росы час назад
          printTempDew(17, 16, tmp24Out[10], 2, -2, 0);                                                       // вывод текущей точки росы
          mSetFill(15, 16 + 3, mGray, 2, 1);                                                                  // тире (-)
          printTempDew(0, 25, tmp24In[0], 2, -2, 0);                                                          // вывод точки росы час назад
          printTempDew(17, 25, tmp24In[10], 2, -2, 0);                                                        // вывод текущей точки росы
          mSetFill(15, 25 + 3, mGray, 2, 1);                                                                  // тире (-)
          drawPlot(22, 0, 10, 4, 0, 1, (float *)tmp24Out, ((tmp24Out[10] < 0) ? 0x0088FF : 0xBBFF00), 2, 1);  // график
          drawPlot(22, 4, 10, 4, 0, 1, (float *)tmp24In, ((tmp24In[10] < 0) ? 0x0088FF : 0xBBFF00), 2, 1);    // график
          break;
        case 15:  // переход на 20
          currentMode = 20;
          break;
        case 20:  // температура почасово за последние сутки + суточный график Out -----------------------------------------------------------------------

          symPrint(0, 9, "ТЕМП.\0", mWhite, 1);
          symPrint(21, 9, "СУТ\0", mWhite, 1);
          printTemp(24, 16, tempDayOut[hourInDay], 2, 0, 0);
          printTemp(24, 25, tempDayIn[hourInDay], 2, 0, 0);
          drawPlot(0, 16, 24, 7, -30, 30, (float *)tempDayOut, colorTempOut, 2, 0);  // график
          drawPlot(0, 25, 24, 7, 22, 30, (float *)tempDayIn, colorTempIn, 2, 0);     // график
          break;
        case 21:  // влажность относительная почасово за последние сутки + суточный график Out -----------------------------------------------------------------------

          symPrint(0, 9, "ВЛАЖ.\0", mWhite, 1);
          symPrint(21, 9, "ОТН\0", mWhite, 1);
          printHum(25, 16, humDayOut[hourInDay], 2, 0, 0);
          printHum(25, 25, humDayIn[hourInDay], 2, 0, 0);
          drawPlot(0, 16, 24, 7, 0, 60, (float *)humDayOut, mAqua, 2, 0);  // график
          drawPlot(0, 25, 24, 7, 0, 60, (float *)humDayIn, mAqua, 2, 0);   // график
          break;
        case 22:  // влажность абсолютная почасово за последние сутки + суточный график Out -----------------------------------------------------------------------

          symPrint(0, 9, "ВЛАЖ.\0", mWhite, 1);
          symPrint(21, 9, "АБС\0", mWhite, 1);
          if ((updatedGraph & (1)) || (updatedGraph & (1 << 7))) {  // установлен бит обновления суточного графика или установлен бит переключения
            for (byte i = 0; i < 24; i++) {
              tmp24Out[i] = absHum(tempDayOut[i], humDayOut[i], pressDayOut[i]);  // количество влаги в воздухе Out почасово г/м3
              tmp24In[i] = absHum(tempDayIn[i], humDayIn[i], pressDayOut[i]);     // количество влаги в воздухе In почасово г/м3
            }
            updatedGraph &= ~((1 << 1) | (1 << 7));  // обнуление битов (обоих, т.к. значение для графика обновлены)
          }
          tmp24Out[23] = absHum(dispTempOut, dispHumOut, dispPresOut);  // постоянное обновление текущего значения
          tmp24In[23] = absHum(dispTempIn, dispHumIn, dispPresOut);     // постоянное обновление текущего значения
          printHumAbs(25, 16, tmp24Out[hourInDay], 2, 0, 0);
          printHumAbs(25, 25, tmp24In[hourInDay], 2, 0, 0);
          drawPlot(0, 16, 24, 7, 1, 10, (float *)tmp24Out, 0x00FF44, 1, 0);  // график
          drawPlot(0, 25, 24, 7, 1, 10, (float *)tmp24In, 0x00FF44, 1, 0);   // график
          break;
        case 23:  // Давление и СО2 почасово за последние сутки + суточный график -----------------------------------------------------------------------

          symPrint(0, 9, "ДАВЛ.\0", mWhite, 1);
          symPrint(21, 9, "СО\0", mWhite, 1);
          digPrint(29, 9, 2, mWhite, 1, 1, 0, 0);
          //printPress(24, 16 + 2, pressDayOut[hourInDay], 0, 0, 0);                  // цифровое значение показателя
          digPrint(24, 16 + 2, round(pressDayOut[hourInDay]), mRed, 0, 3, 0, 0);
          digPrint(24, 25 + 2, round(CO2Day[hourInDay] / 10), mWhite, 0, 3, 0, 0);  // CO2 почасово - цифровое значение показателя
          mSet(26, 25 + 6, mBlue);                                                  // десятичная точка
          drawPlot(0, 16, 24, 7, 730, 750, (float *)pressDayOut, mMagenta, 1, 0);   // график
          drawPlot(0, 25, 24, 7, 400, 2000, (float *)CO2Day, 0xFFFF88, 0.005, 0);   // график
          break;
        case 24:  // точка росы почасово за последние сутки + суточный график -----------------------------------------------------------------------

          symbSetx(0, 9, 0b111011011010011011010011011010011001010011001, 9, 5, mWhite);  // символы "ТОЧ"
          symPrint(10, 9, "КА\0", mWhite, 1);
          symbSetx(19, 9, 0b1101100101011011010010110110100111001101001110011001011, 11, 5, mWhite);  // символы "РОСЫ"
          mSetFill(31, 9, mWhite, 1, 5);                                                              // хвостик от буквы Ы
          if ((updatedGraph & (1)) || (updatedGraph & (1 << 7))) {                                    // установлен бит обновления суточного графика
            for (byte i = 0; i < 24; i++) {
              tmp24Out[i] = dispDewPoint(tempDayOut[i], humDayOut[i]);  // точка росы Out почасово
              tmp24In[i] = dispDewPoint(tempDayIn[i], humDayIn[i]);     // точка росы In почасово
            }
            updatedGraph &= ~((1 << 1) | (1 << 7));  // обнуление битов (обоих, т.к. значение для графика обновлены)
          }
          tmp24Out[23] = dispDewPoint(dispTempOut, dispHumOut);  // постоянное обновление текущего значения
          tmp24In[23] = dispDewPoint(dispTempIn, dispHumIn);     // постоянное обновление текущего значения
          printTempDew(24, 16, tmp24Out[hourInDay], 2, 0, 0);
          printTempDew(24, 25, tmp24In[hourInDay], 2, 0, 0);
          drawPlot(0, 16, 24, 7, -3, 3, (float *)tmp24Out, (tmp24Out[hourInDay] < 0) ? 0x0088FF : 0xBBFF00, 2, 0);  // график
          drawPlot(0, 25, 24, 7, -3, 3, (float *)tmp24In, (tmp24In[hourInDay] < 0) ? 0x0088FF : 0xBBFF00, 2, 0);    // график
          break;
        case 25:  // переход на 0
          currentMode = 0;
          break;
      }
      if (currentMode > 19) {                                              // маркер смещения по суточному графику
        digPrint(19, 2, (hours + hourInDay + 1) % 24, mGray, 0, 2, 0, 0);  // Вывод часа показаний для суточного графика
        digPrint(27, 2, 0, mGray, 0, 2, 0, 0);
        symbSetx(25, 3, 0b101, 1, 3, mGray);        // двоеточие
        mSet(hourInDay, 22, mRed);                  // Красный указатель на час в суточном графике, с которого показывается значение
        mSet(hourInDay, 31, mRed);                  // Красный указатель на час в суточном графике, с которого показывается значение
        if (((abs(hulfTime) + 1) << 6) & newSec) {  // каждые полсекунды сдвигаем маркер
          hourInDay = (hourInDay == 23) ? 0 : (hourInDay + 1);
          newSec = newSec & (255 - ((abs(hulfTime) + 1) << 6));  // обнуляем биты для исключения повторного смещения внутри полусекундии :)
        }
      }
      break;
    case 1:  // установка года -----------------------------------------------------------------------
      symPrint(0, 1, "ГОД:\0", mWhite, true);
      digPrint(16, 0, date.year, colorTime, 2, 4, 0, 0);
      break;
    case 2:  // установка месяца -----------------------------------------------------------------------
      symPrint(0, 1, "МЕСЯЦ:\0", mWhite, true);
      digPrint(24, 0, date.month, colorTime, 2, 2, 0, 0);
      break;
    case 3:  // установка дня -----------------------------------------------------------------------
      symPrint(0, 1, "ЧИСЛО:\0", mWhite, true);
      digPrint(24, 0, date.day, colorTime, 2, 2, 0, 0);
      break;
    case 4:  // установка часов -----------------------------------------------------------------------
    case 5:  // установка минут -----------------------------------------------------------------------
    case 6:  // установка секунд -----------------------------------------------------------------------

      printTime(0, 0, 4, timeSet, colorTime, 1);  // Вывод времени
      break;

    case 7:  // настройка корректировки времени -----------------------------------------------------------------------
      symPrint(0, 1, "КОРР:\0", mWhite, true);
      digPrint(21, 1, abs(corhr), colorTime, 1, 2, 0, 0);
      mSet(24, 5, mWhite);                              // десятичная точка
      if (corhr < 0) mSetFill(18, 3, colorTime, 2, 1);  // если отрицательное - показываем минус
      break;
    case 8:  // выходим из настройки времени -----------------------------------------------------------------------
      timeSet = 0;
      break;
    case 11:  // отладка - показываем яркость экрана и данные датчика света, миллисекунды, аптайм по миллисекундам -----------------------------------------------------------------------

      clkTmr = epochTime - startepochTime;
      digPrint(0, 0, bright, mLime, 1, 3, 0, 0);
      digPrint(12, 0, analogRead(PHOTO), mYellow, 1, 4, 0, 0);
      // RGB+белый с яркостью 1, 2 и 3 - отладка

#if (LEDDriver == 0)
      ledStrip.setPixel(getPixNumber((29 + ((0 & 248) << 2)), (0 & 7)), 0x010000);
      ledStrip.setPixel(getPixNumber((29 + ((1 & 248) << 2)), (1 & 7)), 0x000100);
      ledStrip.setPixel(getPixNumber((29 + ((2 & 248) << 2)), (2 & 7)), 0x000001);
      ledStrip.setPixel(getPixNumber((29 + ((4 & 248) << 2)), (4 & 7)), 0x010101);

      ledStrip.setPixel(getPixNumber((30 + ((0 & 248) << 2)), (0 & 7)), 0x020000);
      ledStrip.setPixel(getPixNumber((30 + ((1 & 248) << 2)), (1 & 7)), 0x000200);
      ledStrip.setPixel(getPixNumber((30 + ((2 & 248) << 2)), (2 & 7)), 0x000002);
      ledStrip.setPixel(getPixNumber((30 + ((4 & 248) << 2)), (4 & 7)), 0x020202);

      ledStrip.setPixel(getPixNumber((31 + ((0 & 248) << 2)), (0 & 7)), 0x030000);
      ledStrip.setPixel(getPixNumber((31 + ((1 & 248) << 2)), (1 & 7)), 0x000300);
      ledStrip.setPixel(getPixNumber((31 + ((2 & 248) << 2)), (2 & 7)), 0x000003);
      ledStrip.setPixel(getPixNumber((31 + ((4 & 248) << 2)), (4 & 7)), 0x030303);

#elif (LEDDriver == 1)
      leds[getPixNumber((29 + ((0 & 248) << 2)), (0 & 7))] = 0x010000;
      leds[getPixNumber((29 + ((1 & 248) << 2)), (1 & 7))] = 0x000100;
      leds[getPixNumber((29 + ((2 & 248) << 2)), (2 & 7))] = 0x000001;
      leds[getPixNumber((29 + ((4 & 248) << 2)), (4 & 7))] = 0x010101;

      leds[getPixNumber((30 + ((0 & 248) << 2)), (0 & 7))] = 0x020000;
      leds[getPixNumber((30 + ((1 & 248) << 2)), (1 & 7))] = 0x000200;
      leds[getPixNumber((30 + ((2 & 248) << 2)), (2 & 7))] = 0x000002;
      leds[getPixNumber((30 + ((4 & 248) << 2)), (4 & 7))] = 0x020202;

      leds[getPixNumber((31 + ((0 & 248) << 2)), (0 & 7))] = 0x030000;
      leds[getPixNumber((31 + ((1 & 248) << 2)), (1 & 7))] = 0x000300;
      leds[getPixNumber((31 + ((2 & 248) << 2)), (2 & 7))] = 0x000003;
      leds[getPixNumber((31 + ((4 & 248) << 2)), (4 & 7))] = 0x030303;
#endif

      mSetM(28, 0, 0xFF0000, 1);  // текущая яркость
      mSetM(28, 1, 0x00FF00, 1);
      mSetM(28, 2, 0x0000FF, 1);
      mSetM(28, 4, 0xFFFFFF, 1);
      digPrint(0, 6, millis(), mGray, 0, 11, 0, 0);  // миллисекунды
      // дни, часы, минуты, секунды по часам реального времени как разница между текущим временем и временем запуска
      digPrint(0, 11, clkTmr / 86400, 0xFF6060, 0, 3, 0, 0);            // дни
      digPrint(11, 11, (clkTmr % 86400) / 3600, 0xFF6060, 0, 2, 0, 0);  // часы
      digPrint(19, 11, (clkTmr % 3600) / 60, 0xFF6060, 0, 2, 0, 0);     // минуты
      digPrint(27, 11, (clkTmr % 60), 0xFF6060, 0, 2, 0, 0);            // секунды
      // двоеточия
      symbSetx(9, 12, 0b101, 1, 3, 0xFF6060);
      symbSetx(17, 12, 0b101, 1, 3, 0xFF6060);
      symbSetx(25, 12, 0b101, 1, 3, 0xFF6060);

      /*    uint8_t r, g, b;
      uint32_t rr, gg;
      //for (byte y = 0; y < 8; y++) {
      for (byte x = 0; x < 8; x++) {
        r = 255 - (x << 5);
        g = r;
        b = r;
        rr=(r << 16);
        gg=(g << 8);
#if (LEDDriver == 0)

        mSetM(x*2, 25, rr, 1);
        mSetM(x*2, 26, rr | ((255 - g) << 8), 1);
        mSetM(x*2, 27, rr | (255 - b), 1);
        mSetM(x*2, 28, rr | ((255 - g) << 8) | b, 1);
        mSetM(x*2, 29, ((255 - r) << 16) | ((255 - b) << 8) | b, 1);
        mSetM(x*2, 30, ((255 - r) << 16) | gg | b, 1);
        mSetM(x*2, 31, rr | (g << 8) | b, 1);
        
        mSetM(x*2 + 16, 25, gg, 1);
        mSetM(x*2 + 16, 26, gg | (255 - b), 1);
        mSetM(x*2 + 16, 27, b, 1);
        mSetM(x*2 + 16, 28, ((255 - r) << 16) | gg | (255 - b), 1);
        mSetM(x*2 + 16, 29, rr | gg | (255 - b), 1);
        mSetM(x*2 + 16, 31, ((255 - r) << 16) | ((255 - g) << 8) | (255 - b), 1);
        mSetM(x*2 + 16, 30, rr | ((255 - g) << 8) | (255 - b), 1);

        mSetM(x*2+1, 25, rr, 0);
        mSetM(x*2+1, 26, rr | ((255 - g) << 8), 0);
        mSetM(x*2+1, 27, rr | (255 - b), 0);
        mSetM(x*2+1, 28, rr | ((255 - g) << 8) | b, 0);
        mSetM(x*2+1, 29, ((255 - r) << 16) | ((255 - b) << 8) | b, 0);
        mSetM(x*2+1, 30, ((255 - r) << 16) | gg | b, 0);
        mSetM(x*2+1, 31, rr | (g << 8) | b, 0);
        
        mSetM(x*2 + 17, 25, gg, 0);
        mSetM(x*2 + 17, 26, gg | (255 - b), 0);
        mSetM(x*2 + 17, 27, b, 0);
        mSetM(x*2 + 17, 28, ((255 - r) << 16) | gg | (255 - b), 0);
        mSetM(x*2 + 17, 29, rr | (g << 8) | (255 - b), 0);
        mSetM(x*2 + 17, 31, ((255 - r) << 16) | ((255 - g) << 8) | (255 - b), 0);
        mSetM(x*2 + 17, 30, rr | ((255 - g) << 8) | (255 - b), 0);
       mSetM(x, 25, (r << 16), 0);
        mSetM(x, 26, (r << 16) | ((255 - g) << 8), 0);
        mSetM(x, 27, (r << 16) | (255 - b), 0);
        mSetM(x, 28, (r << 16) | ((255 - g) << 8) | b, 0);
        mSetM(x, 29, ((255 - r) << 16) | ((255 - b) << 8) | b, 0);
        mSetM(x, 30, ((255 - r) << 16) | (g << 8) | b, 0);
        mSetM(x, 31, (r << 16) | (g << 8) | b, 0);
        
        mSetM(x + 16, 25, (g << 8), 0);
        mSetM(x + 16, 26, (g << 8) | (255 - b), 0);
        mSetM(x + 16, 27, b, 0);
        mSetM(x + 16, 28, ((255 - r) << 16) | (g << 8) | (255 - b), 0);
        mSetM(x + 16, 29, (r << 16) | (g << 8) | (255 - b), 0);
        mSetM(x + 16, 31, ((255 - r) << 16) | ((255 - g) << 8) | (255 - b), 0);
        mSetM(x + 16, 30, (r << 16) | ((255 - g) << 8) | (255 - b), 0);


#elif (LEDDriver == 1)
#endif
      }*/
      //}
      break;
    case 12:        // выход из расширенного режима
      timeSet = 0;  // выходим из расширенного режима
      break;
  }

  // Усреднение последних 5 значений яркости с учетом петли гистерезиса (+/- 100)
  getbright = (getbright * 5 + ((getbright >= analogRead(PHOTO)) ? min(analogRead(PHOTO) + 100, getbright) : max(analogRead(PHOTO) - 100, getbright))) / 6;
  //bright = map(constrain(getbright, 900, 1800), 900, 1800, BRIGHTNESS, 1);
  //bright = max(map(constrain(getbright, 800, 2400), 800, 2400, BRIGHTNESS, 1), minBRIGHTNESS);
  bright = map(constrain(getbright, 800, 2400), 800, 2400, BRIGHTNESS, minBRIGHTNESS);
#if (LEDDriver == 0)
  ledStrip.show();
#elif (LEDDriver == 1)
  FastLED.show();
#endif
}

// ====================== Таймеры =================================================================================================================================================
boolean testTimer(unsigned long &dataTimer, unsigned long setTimer) {  // Проверка таймеров (с)НР
  if (millis() - dataTimer >= setTimer) {
    dataTimer = millis();
    return true;
  } else return false;
}

// ====================== Тики кнопки =============================================================================================================================================
void modesTick() {
  buttonMain.tick();
  buttonTime.tick();

  //----------кнопка настройки времени--------------------------
  if (buttonTime.isSingle()) {
    if (timeSet == 0) {
      graphMode++;
      if (graphMode == 9) graphMode = 0;
    } else {
      if ((timeSet == 7) && (corhr != (int8_t)EEPROM.read(0))) EEPROM.write(0, corhr);  // если изменили корректировку скорости, хода то сохраняем ее
      timeSet = (timeSet == 7) ? 0 : (timeSet + 1);                                     // если прошлись по всем параметрам установки времени, то выход из режима установки
    }
  }

  if (buttonTime.isHolded()) {  // удержание кн.установки времени - стандартный режим <--> установка времени
    holdTimerD = millis();
    if (buttonTime.getHoldClicks() == 0) {
      if ((timeSet == 7) && (corhr != (int8_t)EEPROM.read(0))) EEPROM.write(0, corhr);  // если изменили корректировку скорости, хода то сохраняем ее
      timeSet = 0;
    }
  }

  if (buttonTime.isDouble()) {
    if (timeSet == 0) timeSet++;
  }

  if (buttonTime.isHold()) {
    if (testTimer(holdTimerD, holdTimer)) {
      if (timeSet == 0) {
        graphMode++;
        if (graphMode == 9) graphMode = 0;
      }
    }
  }

  if (buttonTime.hasClicks()) {
    if (buttonTime.getClicks() == 5) {  // сохранение текущего вида настроек при пятикратном нажатии кнопки
      //if (viewTime != (int16_t)EEPROM.read(4)) EEPROM.write(4, (int16_t)viewTime);
      if (currentMode != EEPROM.read(8)) EEPROM.write(8, currentMode);
      if (graphModeIn != EEPROM.read(12)) EEPROM.write(12, graphModeIn);
      if (graphModeOut != EEPROM.read(16)) EEPROM.write(16, graphModeOut);
      if (graphMode != EEPROM.read(20)) EEPROM.write(20, graphMode);
#if (LEDDriver == 0)
      ledStrip.clear();
#elif (LEDDriver == 1)
      FastLED.clearData();
#endif
      symPrint(0, 6, "ТЕКУЩИЙ\0", mWhite, true);
      symPrint(8, 12, "ВИД\0", mWhite, true);
      symPrint(0, 18, "СОХРАНЕН\0", mWhite, true);
#if (LEDDriver == 0)
      ledStrip.show();
#elif (LEDDriver == 1)
      FastLED.show();
#endif
      delay(1500);
    }
  }

  if (buttonTime.isTriple()) {
    if (timeSet == 0) timeSet = 11;  // отладочный экран
  }

  //-------------------главная кнопка--------------------------
  if (buttonMain.isSingle()) {
    if (timeSet == 0) {  // если НЕ режим установки времени
      currentMode++;
      updatedGraph |= (1 << 7);  // установка бита переключения
    } else if (timeSet < 10) datetimeSet(timeSet, 1);
  }

  if (buttonMain.isDouble()) {
    if ((timeSet != 0) && (timeSet < 10)) datetimeSet(timeSet, -1);                                              // если режим установки времени
    else if (currentMode == 0) graphModeOut = (graphModeOut == 2) ? 0 : (graphModeOut + 1);  // если основной экран переключаем виды графиков Out на главном экране
    else if (currentMode != 0) currentMode = (currentMode < 10) ? 10 : 20;                   // прыгаем на -> часовые -> суточные показания
    //else if (currentMode != 0) currentMode = (currentMode < 10) ? 10 : ((currentMode < 20) ? 20 : currentMode + 1);  // прыгаем на -> часовые -> суточные показания
  }

  if (buttonMain.isTriple()) {
    if (timeSet == 0) {
      if (currentMode == 0) graphModeIn = (graphModeIn == 2) ? 0 : (graphModeIn + 1);  // если НЕ режим установки времени переключаем виды графиков In на главном экране
      else currentMode = (currentMode < 20) ? 1 : 10;                                  // прыгаем обратно на -> часовые - > текущие показания;
      //else currentMode = (currentMode < 10) ? 1 : ((currentMode < 20) ? 1 : 10);      // прыгаем обратно на -> часовые - > текущие показания;
    }
  }

  if (buttonMain.isHolded()) {
    if ((timeSet == 0) || (timeSet > 10)) {  // если НЕ режим установки времени
      holdTimerD = millis();                 // сбрасываем таймер для ожидания, если были не на главном экране
      currentMode = 0;
    }
  }

  if (buttonMain.isHold()) {
    if (timeSet == 0) {  // если НЕ режим установки времени
      if (testTimer(holdTimerD, holdTimer)) {
        switch (buttonMain.getHoldClicks()) {
          case 0:
            currentMode++;
            updatedGraph |= (1 << 7);  // установка бита переключения
            break;
          case 1:
            if (currentMode == 0) {
              graphModeOut++;
              if (graphModeOut == 3) graphModeOut = 0;
            }
            break;
          case 2:
            if (currentMode == 0) {
              graphModeIn++;
              if (graphModeIn == 3) graphModeIn = 0;
            }
            break;
        }
      }
    } else {
      if (testTimer(holdSetTimeD, holdSetTime)) {
        datetimeSet(timeSet, ((buttonMain.getHoldClicks()) ? (-1) : (1)));
      }
    }
  }
}

// ====================== Основной цикл ===========================================================================================================================================
void loop() {

  // обновление экрана
  if ((testTimer(redrawTimerD, redrawTimer))) {
    if (scroll > 0) scroll++;
    if (scroll > 12) scroll = 0;
    hulfTime = (rtc.getMilliseconds() < 500);
    if (epochTime != rtc.getTime()) {
      //  if (rtc.isCounterUpdated()) {       // !!!закомментил, т.к. с этой строкой первый пиксель может мигать. Но работает и так
      //  rtc.clearSecondFlag();            // !!!закомментил, т.к. с этой строкой первый пиксель может мигать. Но работает и так
      epochTime = rtc.getTime();
      rtc.epochToDateTime(epochTime, date, time);
      if ((corhr != 0) && (corhr != 127)) {
        if (epochTime % ((int32_t)(86400 / ((float)abs(corhr) / 10))) == 0) {
          rtc.setTime(epochTime + ((corhr > 0) ? 1 : (-1)));  // корректировка хода часов
          corhr = 127;                                        // чтобы не было повторной корректировки на следующей секунде
        }
      } else {
        if (corhr == 127) corhr = (int8_t)EEPROM.read(0);  // возвращаем значение коэффициента после корректировки времени
      }
    }
    if (epochTime != epochTimePrev) {
      epochTimePrev = epochTime;
      hoursPrev = hours;
      minutesPrev = minutes;
      secondsPrev = seconds;
      hours = time.hours;
      minutes = time.minutes;
      seconds = time.seconds;
      newSec = 255;
      if (seconds == 0 && minutes == 0) plotSensorsDayTick();             // если новый час, то обновляем суточные графики
      if (((minutes % 6) == 0) && (seconds == 0)) plotSensorsHourTick();  // обновление графиков 1 раз в 6 минут (10 показаний в час)
      if ((seconds % 6) == 0) plotSensorsMinTick();                       // обновление графиков 1 раз в 6 секунд (10 показаний в минуту)
      scroll = 1;
    }
    drawScreen();  // перерисовываем экран
    // обновление датчиков
    if ((testTimer(sensorsTimerD, sensorsTimer))) readSensors(2);      // читаем показания датчиков температуры, влажности, давления
    if ((testTimer(CO2TimerD, CO2Timer))) dispCO2 = myMHZ19.getCO2();  // читаем показание датчика CO2
  }

  modesTick();  // тики кнопки
}