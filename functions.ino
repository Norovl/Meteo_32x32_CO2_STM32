
//============ Обновление массивов для часовых графиков =========================================================================================================================================

void plotSensorsTmpTick(float datXIn, float datXOut, int8_t pos) {
  for (uint8_t i = 0; i < pos; i++) {
    tmp24Out[i] = tmp24Out[i + 1];
    tmp24In[i] = tmp24In[i + 1];
  }
  tmp24Out[pos] = datXOut;
  tmp24In[pos] = datXIn;
}

void plotSensorsMinTick() {  // обновление данных для минутного графика
  for (byte i = 0; i < 10; i++) {
    tempMinIn[i] = tempMinIn[i + 1];
    tempMinOut[i] = tempMinOut[i + 1];
    humMinIn[i] = humMinIn[i + 1];
    humMinOut[i] = humMinOut[i + 1];
    pressMinOut[i] = pressMinOut[i + 1];
    CO2Min[i] = CO2Min[i + 1];
  }
  tempMinIn[10] = dispTempIn;
  tempMinOut[10] = dispTempOut;
  humMinIn[10] = dispHumIn;
  humMinOut[10] = dispHumOut;
  pressMinOut[10] = dispPresOut;
  CO2Min[10] = dispCO2;
  updatedGraph |= (1 << 2);
}

void plotSensorsHourTick() {
  for (byte i = 0; i < 10; i++) {
    tempHourIn[i] = tempHourIn[i + 1];
    tempHourOut[i] = tempHourOut[i + 1];
    humHourIn[i] = humHourIn[i + 1];
    humHourOut[i] = humHourOut[i + 1];
    //pressHourIn[i] = pressHourIn[i + 1];
    pressHourOut[i] = pressHourOut[i + 1];
    CO2Hour[i] = CO2Hour[i + 1];
  }
  tempHourIn[10] = dispTempIn;
  tempHourOut[10] = dispTempOut;
  humHourIn[10] = dispHumIn;
  humHourOut[10] = dispHumOut;
  //pressHourIn[10] = dispPresIn;
  pressHourOut[10] = dispPresOut;
  CO2Hour[10] = dispCO2;
  updatedGraph |= (1 << 1);
}

void plotSensorsDayTick() {
  for (byte i = 0; i < 23; i++) {
    tempDayIn[i] = tempDayIn[i + 1];
    tempDayOut[i] = tempDayOut[i + 1];
    humDayIn[i] = humDayIn[i + 1];
    humDayOut[i] = humDayOut[i + 1];
    //pressDayIn[i] = pressDayIn[i + 1];
    pressDayOut[i] = pressDayOut[i + 1];
    CO2Day[i] = CO2Day[i + 1];
  }
  tempDayOut[23] = dispTempOut;
  tempDayIn[23] = dispTempIn;
  humDayOut[23] = dispHumOut;
  humDayIn[23] = dispHumIn;
  pressDayOut[23] = dispPresOut;
  CO2Day[23] = dispCO2;
  updatedGraph |= 1;
  /*// усредняем значения за час
  tempDayIn[23] = 0;
  tempDayOut[23] = 0;
  humDayIn[23] = 0;
  humDayOut[23] = 0;
  //pressDayIn[23] = 0;
  pressDayOut[23] = 0;
  CO2Day[23] = 0;

  for (byte i = 0; i < 11; i++) {
    tempDayIn[23] = tempDayIn[23] + tempHourIn[i];
    tempDayOut[23] = tempDayOut[23] + tempHourOut[i];
    humDayIn[23] = humDayIn[23] + humHourIn[i];
    humDayOut[23] = humDayOut[23] + humHourOut[i];
    //pressDayIn[23] = pressDayIn[23] + pressHourIn[i];
    pressDayOut[23] = pressDayOut[23] + pressHourOut[i];
    CO2Day[23] = CO2Day[23] + CO2Hour[i];
  }

  tempDayIn[23] = tempDayIn[23] / 11;
  tempDayOut[23] = tempDayOut[23] / 11;
  humDayIn[23] = humDayIn[23] / 11;
  humDayOut[23] = humDayOut[23] / 11;
  //pressDayIn[23] = pressDayIn[23] / 11;
  pressDayOut[23] = pressDayOut[23] / 11;
  CO2Day[23] = CO2Day[23] / 11;*/
}

// получение номера пикселя по координатам
int getPixNumber(int x, int y) {
  //y = M_HEIGHT - y;  // инвертируем Y для начала кординат с левого верхнего угля
  int thisX, thisY;
  switch (matrixConfig) {
    case 0:
      thisX = x;
      thisY = (M_HEIGHT - y - 1);
      break;
    case 4:
      thisX = (M_HEIGHT - y - 1);
      thisY = x;
      break;
    case 1:
      thisX = x;
      thisY = y;
      break;
    case 13:
      thisX = y;
      thisY = x;
      break;
    case 10:
      thisX = (M_WIDTH - x - 1);
      thisY = y;
      break;
    case 14:
      thisX = y;
      thisY = (M_WIDTH - x - 1);
      break;
    case 11:
      thisX = (M_WIDTH - x - 1);
      thisY = (M_HEIGHT - y - 1);
      break;
    case 7:
      thisX = (M_HEIGHT - y - 1);
      thisY = (M_WIDTH - x - 1);
      break;
  }
  if (matrixType || !(thisY % 2)) return (thisY * matrixW + thisX);  // если чётная строка
  else return (thisY * matrixW + matrixW - thisX - 1);               // если нечётная строка
}

// получение абсолютной влажности г/м3
float absHum(float temp, float hum, float press) {
  return (216.7 * (6.112 * exp(17.62 * temp / (temp + 243.12))) / (temp + 273.15)) * hum / 100 * press / 760;
}

//============Конвертация координат 32хX(8-16-24-32) в (32-64-96-128)х8 =================================================================================================================================================

// установка пикселя по координатам x, y, цветом color, при mbr > 0 - минимальное значение составляющего цвета RGB > 0
void mSetM(int8_t x, int8_t y, int32_t color, uint8_t mbr) {
  //  y = y + 8;  // тестовое смещение
  if ((x < 32) && (y < 32)) {  // ограничиваем вывод полем 32 х 32
#if (LEDDriver == 0)
    if ((x < M_WIDTH - ((y & 248) << 2)) && ((y & 7) < M_HEIGHT)) ledStrip.setPixel(getPixNumber((x + ((y & 248) << 2)), (y & 7)), brightRatio(color, bright, mbr));
#elif (LEDDriver == 1)
    if ((x < M_WIDTH - ((y & 248) << 2)) && ((y & 7) < M_HEIGHT)) leds[getPixNumber((x + ((y & 248) << 2)), (y & 7))] = brightRatio(color, bright, mbr);
#endif
  }
}

void mSet(int8_t x, int8_t y, int32_t color) {
  mSetM(x, y, color, 1);
}

// Заполнение цветом от левой верхней точки x, y на fillx пикселей в ширину и filly пикселей в высоту
void mSetFill(int8_t x, int8_t y, int32_t color, int8_t fillx, int8_t filly) {
  for (int8_t x1 = 0; x1 < fillx; x1++) {
    for (int8_t y1 = 0; y1 < filly; y1++) {
      mSet(x + x1, y + y1, color);
    }
  }
}

//============= график ==========================================================================================================================================================================

//drawPlot([x], [y], [ширина для вывода], [высота], [мин], [макс], [(int*)массив], [цвет], [пикселей на единицу [если 0, то от мин до макс]], [количество пропукаемых значений (смещение) для вывода только правой части графика]])
void drawPlot(byte x, byte y, byte width, byte height, float min_val, float max_val, float *plot_array, int32_t color, float stretch, int8_t skip) {

  float max_value = -32000;
  float min_value = 32000;
  height = height - 1;  // не учитываем нулевой уровень, т.к. его рисуем с полной яркостью

  //for (byte i = 0; i < 11; i++) {
  for (byte i = 0; i < width; i++) {
    //Serial.println (sizeof(plot_array) / sizeof(float) + 1);
    max_value = max(plot_array[i + skip], max_value);
    min_value = min(plot_array[i + skip], min_value);
  }
  if (min_value >= max_value) max_value = min_value + 0.1;
  // меняем пределы графиков на предельные/фактические значения (с)НР
  if (stretch >= 0) {  // если растягивать на диапазон,
    max_val = max_value;
    min_val = min_value;
    // если разница макс-мин меньше, чем шкала, то поднимаем макс до верха шкалы
    if (((float)height / stretch) > (max_val - min_val)) max_val = min_val + (float)height / stretch;
  }

  uint16_t infill, fract;  // кол-во полных строк, доля неполных (0-20)

  for (byte i = 0; i < width; i++) {  // каждый столбец параметров
    infill = (((plot_array[i + skip]) > min_val) ? ((float)(plot_array[i + skip] - min_val) * height * 20 / (max_val - min_val)) : 0);
    fract = infill % 20;  // количество оставшейся яркости все, что выше округления * 20 для тонкой настройки яркости
    infill = infill / 20;
    mSetFill((x + i), (y + height - infill), color, 1, infill + 1);                                            // заполняем полной яркостью
    if (infill < height - 1) mSetM((x + i), (y + height - infill - 1), brightRatio(color, fract * 10, 0), 0);  // заполняем дробные ячейки яркостью fract (0-20)
  }
}

// пропорционально уменьшаем яркость каждого цвет на br. Например, яркость красного 100, br=26 (из 255, т.е. 26/255), итоговая яркость красного будет 100 * 26 / 255 = 10
// col - базовый цвет, br - какая часть каждого цвета сохранится (из 255),
// mbr - минимальное значение присутствующего цвета из RGB (при mbr=1 если есть составляющая цвета RGB, то она будет не менее 1 (при R > 0, R будет не менее 1 и т.д.))
int32_t brightRatio(int32_t col, uint8_t br, uint8_t mbr) {
  return ((max((((((col >> 16) & 255) + 1) * br + 1) >> 8), ((col & 0xFF0000) && br) ? mbr : 0) << 16) + (max((((((col >> 8) & 255) + 1) * br + 1) >> 8), ((col & 0x00FF00) && br) ? mbr : 0) << 8) + max(((((col & 255) + 1) * br + 1) >> 8), ((col & 0x0000FF) && br) ? mbr : 0));
}

// вычитает яркость из каждого цвета в размере br. Например, яркость красного 100, br=26 (из 255), итоговая яркость красного будет 100 - 26 = 74
// col - базовый цвет, br - сколько вычесть из каждого цвета (из 255)
int32_t fadeAbs(int32_t col, byte br) {
  return ((max((((col >> 16) & 255) - br), 0) << 16) + (max((((col >> 8) & 255) - br), 0) << 8) + max(((col & 255) - br), 0));
}

// st 0 - рабочий режим (ничего не меняем), 1 - установка года, 2 - месяца, 3 - дня, 4 - часов, 5 - минут, 6 - секунд.
// corr 0 - ничего не делаем, 1 - прибавляем единицу, -1 - вычитаем единицу
void datetimeSet(byte st, int8_t corr) {
  //#if (TIMEON == 1)
  epochTime = rtc.getTime();
  switch (st) {
    years = date.year;
    months = date.month;
    days = date.day;
    case 1:        // годы
      days = 365;  // + ((corr < 0) ? 1 : 0);
      epochTime = epochTime + 86400 * days * corr;
      break;
    case 2:  // месяцы
      if ((months == 4) || (months == 6) || (months == 9) || (months == 11)) days = 30;
      else days = 31;
      if (months == 2) {                                            // если февраль
        if (((years - 1968) % 4) == 0 && years != 2100) days = 29;  // если високосный
        else days = 28;
      }
      epochTime = epochTime + 86400 * days * (int)corr;
      break;
    case 3:  // дни
      epochTime = epochTime + 86400 * (int)corr;
      break;
    case 4:  // часы
      epochTime = epochTime + 3600 * (int)corr;
      break;
    case 5:  // минуты
      epochTime = epochTime + 60 * (int)corr;
      break;
    case 6:  // секунды
      epochTime = epochTime + (int)corr;
      break;
    case 7:
      corhr += corr;
      if (corhr == 100) corhr = -99;
      if (corhr == -100) corhr = 99;
      break;
  }
  if (st < 8) {
    hulfTime = false;        // чтобы не мигало
    rtc.setTime(epochTime);  // excel =(СЕГОДНЯ()-ДАТА(1970;1;1))*86400+ВРЕМЯ(7;15;1)*86400 или =(ДАТА(2024;4;30)-ДАТА(1970;1;1))*86400+ВРЕМЯ(23;59;1)*86400
    rtc.epochToDateTime(epochTime, date, time);
  }
  /*#elif (TIMEON == 2)
  epochTime = rtclock.now();
  switch (st) {
    rtclock.breakTime(rtclock.now(), mtt);
    years = mtt.year + 1970;
    months = mtt.month;
    days = mtt.day;
    case 1:        // годы
      days = 365;  // + ((corr < 0) ? 1 : 0);
      epochTime = epochTime + 86400 * days * corr;
      break;
    case 2:  // месяцы
      if ((months == 4) || (months == 6) || (months == 9) || (months == 11)) days = 30;
      else days = 31;
      if (months == 2) {                                            // если февраль
        if (((years - 1968) % 4) == 0 && years != 2100) days = 29;  // если високосный
        else days = 28;
      }
      epochTime = epochTime + 86400 * days * (int)corr;
      break;
    case 3:  // дни
      epochTime = epochTime + 86400 * (int)corr;
      break;
    case 4:  // часы
      epochTime = epochTime + 3600 * (int)corr;
      break;
    case 5:  // минуты
      epochTime = epochTime + 60 * (int)corr;
      break;
    case 6:  // секунды
      epochTime = epochTime + (int)corr;
      break;
    case 10:
      corhr += corr;
      if (corhr == 100) corhr = -99;
      if (corhr == -100) corhr = 99;
      break;
  }
  hulfTime = false;            // чтобы не мигало
  rtclock.setTime(epochTime);  // excel =(СЕГОДНЯ()-ДАТА(1970;1;1))*86400+ВРЕМЯ(7;15;1)*86400 или =(ДАТА(2024;4;30)-ДАТА(1970;1;1))*86400+ВРЕМЯ(23;59;1)*86400
//#endif*/
}

//      tSet (для мигания при усановке) = 4 установка часов, 5 установка минут, 6 установка секунд
//      fnts - признак и шрифт вывода секунд
void printTime(int8_t x, int8_t y, byte fnt, byte tSet, int32_t color, byte fnts) {
  // часы
  digPrint(x, y, hours, (tSet == 4) ? color : mLime, fnt, 2, hoursPrev, ((tSet == 4) ? 0 : scroll));  //  x, y, цифра, цвет, шрифт, к-во цифр, предыдущая цифра, скролл

  // минуты
  digPrint(x + (szFontX(fnt) + 1) * 2 + 2, y, minutes, ((tSet == 5) ? color : mLime), fnt, 2, minutesPrev, ((tSet == 5) ? 0 : scroll));  // x, y, цифра, цвет, резмер, к-во цифр, предыдущая цифра, скролл

  uint8_t viewSet = 1;  // убрать и переработать
  // секунды
  if (viewSet <= 13 && fnts) digPrint(x + (szFontX(fnt) + 1) * 4 + 3, y + (szFontY(fnt) - szFontY(fnts)), seconds, ((tSet == 6) ? color : mLime), fnts, 2, secondsPrev, ((tSet == 6) ? 0 : scroll));  // x, y, цифра, цвет, резмер, к-во цифр, предыдущая цифра, скролл

  //  двоеточие
  if (hulfTime) {
    mSetFill(x + (szFontX(fnt) + 1) * 2,                                     // первая точка x
             y + ((szFontY(fnt) == 10) ? 3 : ((szFontY(fnt) > 5) ? 2 : 1)),  // первая точка y
             ((viewSet >= 8) ? RGBColor(130, 155, 80) : mLime),              // цвет
             ((viewSet < 8) ? 1 : 2), ((viewSet < 10) ? 1 : 2));             // заливка x и y
    mSetFill(x + (szFontX(fnt) + 1) * 2,                                     // вторая точка x
             y + ((szFontY(fnt) == 7) ? 4 : ((szFontY(fnt) > 7) ? 6 : 3)),   // вторая точка y
             ((viewSet >= 8) ? RGBColor(130, 155, 80) : mLime),              // цвет
             ((viewSet < 8) ? 1 : 2), ((viewSet < 10) ? 1 : 2));             // заливка x и y
  }
}

// Вывод цифр с десятичными значениями
// координаты x, y, значение, цвет, кол-во целых знаков, № шрифта, наличие дробной части (кол-во цифр после запятой, при отрицательных значениях дес. цифры вплотную к целым), № шрифта дробной части
void printFloat(int8_t x, int8_t y, float dat, int8_t digits, int32_t Col, int8_t Font, int8_t hasFloat, int8_t FontFloat) {
  digPrint(x, y, (hasFloat) ? abs(dat) : round(abs(dat)), Col, Font, digits, 0, 0);                                                                                                          // само значение
  if (hasFloat) {                                                                                                                                                                            // если указан вывод десятичных значений
    mSet(x + (szFontX(Font) + 1) * abs(digits) - ((hasFloat < 0) ? 1 : 0), y + szFontY(Font) - 1, (hasFloat < 0) ? mWhite : Col);                                                            // десятичная точка если цифры вплотную (hasFloat<0), тогда цвет белый, иначе под цвет цифр
    digPrint(x + (szFontX(Font) + 1) * abs(digits) + ((hasFloat < 0) ? 0 : 2), y + szFontY(Font) - szFontY(FontFloat), fractPart(abs(dat), hasFloat), Col, FontFloat, abs(hasFloat), 0, 0);  // доли значения
  }
}

// Вывод температуры. координата х с учетом знака минус при положителных значениях будет отступ
// координаты x, y, № шрифта, наличие дробной части (кол-во цифр после запятой, при отрицательных значениях дес. цифры вплотную к целым), № шрифта дробной части
void printTempAll(int8_t x, int8_t y, float dat, int8_t Font, int8_t hasFloat, int8_t FontFloat, int32_t colorTempOut) {
  if (hasFloat == 0) dat = round(dat);                            // если вывод без десятичных знаков, то округляем
  if (dat < 0) mSetFill(x, y + szFontY(Font) / 2, mWhite, 2, 1);  // минус
  printFloat(x + 1, y, dat, 2, colorTempOut, Font, hasFloat, FontFloat);
  mSet(x + (szFontX(Font) + 1) * 2 + ((szFontX(FontFloat) + 1) * abs(hasFloat)) + ((hasFloat > 0) ? 2 : 0), y, mWhite);  // градус
}

// Вывод температуры. координата х с учетом знака минус при положителных значениях будет отступ
void printTemp(int8_t x, int8_t y, float dat, int8_t Font, int8_t hasFloat, int8_t FontFloat) {
  int32_t colorTempOut = ((dat < 0) ? mBlue : mYellow);  // для отрицательной температуры - синий цвет, для положительной - желтый
  printTempAll(x, y, dat, Font, hasFloat, FontFloat, colorTempOut);
}

// Вывод температуры точки росы. координата х с учетом знака минус при положителных значениях будет отступ
void printTempDew(int8_t x, int8_t y, float dat, int8_t Font, int8_t hasFloat, int8_t FontFloat) {
  int32_t colorTempOut = ((dat < 0) ? 0x0088FF : 0xBBFF00);  // для отрицательной температуры - синий цвет, для положительной - желтый
  printTempAll(x, y, dat, Font, hasFloat, FontFloat, colorTempOut);
}

// Вывод важности (%)
void printHum(int8_t x, int8_t y, float dat, int8_t Font, int8_t hasFloat, int8_t FontFloat) {
  if (hasFloat == 0) dat = constrain(round(dat), 0, 99);  // если вывод без десятичных знаков, то максимально показываем 99, т.к. нужно двузначное число
  printFloat(x, y, dat, 2, mAqua, Font, hasFloat, FontFloat);
  symbSetx(x + (szFontX(Font) + 1) * 2 + ((hasFloat > 0) ? 1 : -1) + ((szFontX(FontFloat) + 1) * abs(hasFloat)), y + szFontY(Font) - ((hasFloat ? szFontY(FontFloat) : szFontY(Font)) - 5) / 2 - 5, 0b100001010100001, 3, 5, mWhite);  // процент 3x5
}

// Вывод абсолютной влажности (г/м3). Если dat отрицательное, то ведущие ули убираем
void printHumAbs(int8_t x, int8_t y, float dat, int8_t Font, int8_t hasFloat, int8_t FontFloat) {
  if (hasFloat == 0) dat = round(dat);  // если вывод без десятичных знаков, то округляем
  printFloat(x, y, abs(dat), (dat < 0) ? -2 : 2, 0x00FF44, Font, hasFloat, FontFloat);
  symbSetx(x + (szFontX(Font) + 1) * 2 + ((hasFloat < 0) ? 0 : 2) + ((szFontX(FontFloat) + 1) * abs(hasFloat)) - 1, y + szFontY(Font) - 3, 0b111010, 2, 3, mWhite);  // буква "г"
}

// Вывод давления
void printPress(int8_t x, int8_t y, float dat, int8_t Font, int8_t hasFloat, int8_t FontFloat) {
  if (hasFloat == 0) dat = round(dat);  // если вывод без десятичных знаков, то округляем
  printFloat(x, y, dat, 3, mMagenta, Font, hasFloat, FontFloat);
}

// ширина шрифта по номеру
uint8_t szFontX(uint8_t Fnt) {
  return szFont[((Fnt == 100) ? 17 : Fnt)] % 8;
}

// высота шрифта по номеру
uint8_t szFontY(uint8_t Fnt) {
  return ((szFont[((Fnt == 100) ? 17 : Fnt)] - (szFont[((Fnt == 100) ? 17 : Fnt)] % 8)) / 8);
}

// формирует число из дробной части
uint16_t fractPart(float flDec, int8_t numFloat) {
  numFloat = abs(numFloat);
  uint16_t mult = ((numFloat == 1) ? (10) : ((numFloat == 2) ? 100 : ((numFloat == 3) ? 1000 : 10000)));
  return uint32_t(flDec * mult) % mult;
  //return uint16_t((flDec - int32_t(flDec)) * ((numFloat == 1) ? (10) : ((numFloat == 2) ? 100 : ((numFloat == 3) ? 1000 : 10000))));
}

// Convert [hPa] to [mm Hg]
float pressureToMmHg1(float pressure) {
  return (float)(pressure * 0.750061683f);
}

// точка росы по температуре и влажности
float dispDewPoint(float Temp, float Hum) {

  float a, b, x, gamma;
  // Коэффициенты Магнуса
  if (Temp >= 0) {
    a = 17.62f;
    b = 243.12f;
  } else {
    a = 22.46f;
    b = 272.62f;
  }
  // Аппроксимация ln(RH / 100.0) третьей степени:
  x = Hum * 0.01f;
  // Формула Магнуса с приближением логарифма:
  gamma = (a * Temp) / (b + Temp) + (-3.17067f + 10.21044f * x - 12.52050f * x * x + 6.00238f * x * x * x);
  return (b * gamma) / (a - gamma);
}

int32_t RGBColor(int8_t r, int8_t g, int8_t b) {
  return (int32_t)(((int32_t)r << 16) + ((int32_t)g << 8) + (int32_t)b);
}

// "колесо" радужных цветов 0-255
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return ((((WheelPos * 3) & 0xFF) << 16) + (((255 - WheelPos * 3) & 0xFF) << 8) + 0);
  } else {
    if (WheelPos < 170) {
      WheelPos -= 85;
      return ((((255 - WheelPos * 3) & 0xFF) << 16) + 0 + ((WheelPos * 3) & 0xFF));
    } else {
      WheelPos -= 170;
      return (0 + (((255 - WheelPos * 3) & 0xFF) << 8) + (((255 - WheelPos * 3) & 0xFF) << 0));
    }
  }
}

// Преобразование HSV в RGB с минимальным использованием float
uint32_t hsvToRgb(uint16_t h, uint8_t s, uint8_t v) {
  uint8_t region = h / 43;
  uint8_t remainder = (h - (region * 43)) * 6; 
  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    uint8_t r, g, b;

  switch (region) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// Преобразование HSV в RGB с учётом насыщенности
/*uint32_t hsvToRgb(float h, float s, float v) {
  h = h / 360;
  s = s / 256;
  v = v / 256;
  uint32_t r, g, b;
  int i = int(h * 6);
  float f = (h * 6 - i) * 255;
  float p = (v * (1 - s)) * 255;
  float q = (v * (1 - f * s))*255;
  float t = (v * (1 - (1 - f) * s))*255;

  switch (i % 6) {
    case 0:
      r = v;
      g = t;
      b = p;
      break;
    case 1:
      r = q;
      g = v;
      b = p;
      break;
    case 2:
      r = p;
      g = v;
      b = t;
      break;
    case 3:
      r = p;
      g = q;
      b = v;
      break;
    case 4:
      r = t;
      g = p;
      b = v;
      break;
    case 5:
      r = v;
      g = p;
      b = q;
      break;
  }

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
*/