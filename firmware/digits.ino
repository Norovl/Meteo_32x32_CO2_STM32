// ==================== матрицы цифр ====================


// размер цифр: (Шир + Выс * 8). x = (код % 8). y = (код - х) / 8 (размер шрифта)
uint8_t szFont[7] = { 42, 43, 59, 75, 60, 76, 34 };

// 10 цифр 2х4 пикселей
uint8_t digit24[10] = { 0b11111111, 0b01010101, 0b11011011, 0b11010111, 0b11110101, 0b11100111, 0b11101111, 0b11010101, 0b11001111, 0b11110111 };
// 10 цифр 2х5 пикселей + ":" (0)
int16_t digit25[11] = { 0b1111111111, 0b0101010101, 0b1101111011, 0b1101110111, 0b1111110101, 0b1110110111, 0b1110111111, 0b1101010101, 0b1111001111, 0b1111110111, 0b0001000100 };
// 10 цифр 3х5 пикселей + (11)"%", (12)":", (13)"+", (14)"-", (15)"↑", (16)"↓"  (1)
int16_t digit35[16] = { 0b111101101101111, 0b010010010010010, 0b111001111100111, 0b111001111001111, 0b101101111001001, 0b111100111001111, 0b111100111101111, 0b111001010010010,
                        0b111101111101111, 0b111101111001111, 0b100001010100001, 0b000010000010000, 0b000010111010000, 0b000000111000000, 0b010111010010010, 0b010010010111010 };
// 10 цифр 3х7 пикселей + (11)"%" (2)
uint32_t digit37[11] = { 0b111101101101101101111, 0b010010010010010010010, 0b111001001111100100111, 0b111001001111001001111, 0b101101101111001001001, 0b111100100111001001111,
                         0b111100100111101101111, 0b111001001010010010010, 0b111101101111101101111, 0b111101101111001001111, 0b000100001010100001000 };
// 10 цифр 3х9 пикселей (4)
uint32_t digit39[10] = { 0b111101101101101101101101111, 0b010010010010010010010010010, 0b111001001001111100100100111, 0b111001001001111001001001111, 0b101101101101111001001001001,
                         0b111100100100111001001001111, 0b111100100100111101101101111, 0b111001001001001001001001001, 0b111101101101111101101101111, 0b111101101101111001001001111 };
// 10 цифр 4х7 пикселей + "%" (5)
uint32_t digit47[11] = { 0b1111100110011001100110011111, 0b0010011000100010001000100010, 0b1111000100011111100010001111, 0b1111000100011111000100011111,
                         0b1001100110011111000100010001, 0b1111100010001111000100011111, 0b1111100010001111100110011111, 0b1111000100010010001000100010,
                         0b1111100110011111100110011111, 0b1111100110011111000100011111, 0b0000000010010010010010010000 };
// 10 цифр 4х9 пикселей (7)
uint64_t digit49[10] = { 0b111110011001100110011001100110011111, 0b001000100010001000100010001000100010, 0b111100010001000111111000100010001111, 0b111100010001000111110001000100011111,
                         0b100110011001100111110001000100010001, 0b111110001000100011110001000100011111, 0b111110001000100011111001100110011111, 0b111100010001001000100100010001000100,
                         0b111110011001100111111001100110011111, 0b111110011001100111110001000100011111 };


// ==================== матрицы букв ====================

// 32 буквы 3х5 пикселей (нет Ё)
int16_t symb35[32] = { 0b010101101111101, 0b111100111101111, 0b111101110101111, 0b111100100100100, 0b010010010111101, 0b111100110100111, 0b101010111010101,
                       0b110001110001110, 0b101101111111101, 0b101101111111101, 0b101101110101101, 0b010101101101101, 0b101111111101101, 0b101101111101101,
                       0b010101101101010, 0b111101101101101, 0b111101111100100, 0b011100100100011, 0b111010010010010, 0b101101011001110, 0b010111111010010,
                       0b101101010101101, 0b101101101110001, 0b101101111001001, 0b101101111111111, 0b101111111110001, 0b110010011011011, 0b101101100110110,
                       0b100100111101111, 0b110001011001110, 0b101001111001101, 0b011101011101101 };
// 8 букв 5х5 пикселей (Д Ж М Ф Ш Щ Ы Ю)
uint32_t symb55[8] = { 0b0111001010010101111110001, 0b1010110101011101010110101, 0b1000111011101011000110001, 0b0010011111101011111100100,
                       0b1000110101101011010111111, 0b1000110101101011111000001, 0b1000110001111011010111101, 0b1011110101111011010110111 };
// 3 буквы 4х5 пикселей (И Й Ъ)
uint32_t symb45[3] = { 0b10011001101111011001, 0b10111000101111011001, 0b11000100011101010111 };

/* Коды символов:
--коды в Excel: А	192   Б	193   В	194   Г	195   Д	196   Е	197   Ж	198   З	199   И	200   Й	201   К	202   Л	203   М	204   Н	205   О	206   П	207   Р	208   С	209   Т	210   У	211   Ф	212   Х	213   Ц	214   Ч	215   Ш	216   Щ	217   Ъ	218   Ы	219   Ь	220   Э	221   Ю	222   Я	223
коды в Ардуино: А	144   Б	145   В	146   Г	147   Д	148   Е	149   Ж	150   З	151   И	152   Й	153   К	154   Л	155   М	156   Н	157   О	158   П	159   Р	160   С	161   Т	162   У	163   Ф	164   Х	165   Ц	166   Ч	167   Ш	168   Щ	169   Ъ	170   Ы	171   Ь	172   Э	173   Ю	174   Я	174   а 176
см. https://wiki.iarduino.ru/page/encoding-arduino/    в конце см. таблицу с кодировкой UTF-8
*/

// ----------Вывод символа размером w на h с битовой маской в sym----------
void symbSetx(int8_t x, int8_t y, uint64_t symb, int8_t w, int8_t h, int32_t color) {
  for (byte num = 0; num < (w * h); num++) {
    // жестко фиксируем высоту и ширину (32) - нет в шапке
    if ((x < 32) && (y < 32)) mSet(x + (num % w), y + (num / w), (symb & ((uint64_t)1 << ((w * h - 1) - num))) ? color : 0);
  }
}

// ----------Вывод буквы 3х5 (некоторые 4x5 и 5х5 при ext=true)----------
void symbSet(int8_t x, int8_t y, byte sym, int32_t color, boolean ext) {
  if ((ext) && ((sym == 148) || (sym == 150) || (sym == 156) || (sym == 164) || (sym == 168) || (sym == 169) || (sym == 171) || (sym == 174))) {  // 5x5
    switch (sym) {
      case 148:  //Д
        sym = 0;
        break;
      case 150:  //Ж
        sym = 1;
        break;
      case 156:  //М
        sym = 2;
        break;
      case 164:  //Ф
        sym = 3;
        break;
      case 168:  //Ш
        sym = 4;
        break;
      case 169:  //Щ
        sym = 5;
        break;
      case 171:  //Ы
        sym = 6;
        break;
      case 174:  //Ю
        sym = 7;
        break;
    }
    // 5x5
    symbSetx(x, y, symb55[sym], 5, 5, color);

  } else if ((ext) && ((sym == 152) || (sym == 153) || (sym == 170))) {  // 4х5
    switch (sym) {
      case 152:  //И
        sym = 0;
        break;
      case 153:  //Й
        sym = 1;
        break;
      case 170:  //Ъ
        sym = 2;
        break;
    }
    // 4x5
    symbSetx(x, y, symb45[sym], 4, 5, color);

  } else if (sym == 32) mSetFill(x, y, 0, 3, 5);                       //digSet35(x, y, 0, 0, 0, 0);              // пробел
  else if (sym == 37) symbSetx(x, y, 0b100001010100001, 3, 5, color);  //digSet35(x, y, 10, color, 10, 0);        // процент
  else if (sym == 46) symbSetx(x, y, 0b00001, 1, 5, color);            //digSet35(x - 1, y, 11, color, 11, 0);    // точка - смещаем на 1 пиксель влево
  else if (sym == 58) symbSetx(x, y, 0b01010, 1, 5, color);            //digSet35(x - 1, y, 11, color, 11, 0);    // двоеточие - смещаем на 1 пиксель влево
  else symbSetx(x, y, symb35[sym - 144], 3, 5, color);                 // 3x5
}

// ----------Вывод цифры w*h----------
void digSetwh(int8_t x, int8_t y, uint64_t dig, int32_t color, uint64_t digPrev, byte scr, byte w, byte h) {
  if (scr == 0) (digPrev = dig);
  if ((scr > h) || (dig == digPrev)) scr = h + 1;
  int32_t col = 0;

  // Проход по всем пикселям w*h
  for (byte num = 0; num < (w * h); num++) {

    byte colIdx = num % w;
    byte rowIdx = num / w;

    if ((h - scr) == rowIdx) col = 0;  // делаем пустую строку

    // Вывод пикселя старой цифры с учётом смещения scr
    if (rowIdx + scr < h) {
      if (digPrev & ((int64_t)1 << (w * h - 1 - (rowIdx + scr) * w - colIdx))) {
        col = color;
      } else {
        col = 0;
      }
    }

    // Вывод части новой цифры под старой
    if (rowIdx + scr >= (h + 1) && rowIdx + scr < (h * 2 + 1)) {
      if (dig & ((int64_t)1 << (w * h - 1 - (rowIdx + scr - (h + 1)) * w - colIdx))) {
        col = color;
      } else {
        col = 0;
      }
    }

    mSet(x + colIdx, y + rowIdx, col);
  }
}

// Вывод числа цифрами размером sz = 0 - 2х5; = 1 - 3х5; = 2 - 3х7; = 3 - 4х7; digits = количество цифр с отображением ведущих нулей, отрицательное - вместо ведущих нулей пробелы, 0 - без ведущих знаков
// digsPrev - предыдущее число для скролла, scroll - количество строк поднятия вверх старой цифры и отображения под ней новой, mbr - минимальная яркость канала (RGB) при наличии на нем цвета
void digPrintM(int8_t x, int8_t y, int digs, int32_t color, byte sz, byte digits, int digsPrev, byte scroll, uint8_t mbr) {
  // Serial.println(digs);
  uint8_t dig = 0;
  uint8_t digPrev = 0;
  if (scroll == 0) digsPrev = digs;
  for (byte d = 1; d <= abs(digits); d++) {  // Цикл по цифрам
    dig = abs(digs % 10);
    digPrev = abs(digsPrev % 10);  // предыдущее число для скролла
    switch (sz) {
      case 0:
        digSetwh(x + (abs(digits) - d) * 3, y, digit25[dig], color, digit25[digPrev], scroll, 2, 5);
        break;
      case 1:
        digSetwh(x + (abs(digits) - d) * 4, y, digit35[dig], color, digit35[digPrev], scroll, 3, 5);
        break;
      case 2:
        digSetwh(x + (abs(digits) - d) * 4, y, digit37[dig], color, digit37[digPrev], scroll, 3, 7);
        break;
      case 3:
        digSetwh(x + (abs(digits) - d) * 4, y, digit39[dig], color, digit39[digPrev], scroll, 3, 9);
        break;
      case 4:
        digSetwh(x + (abs(digits) - d) * 5, y, digit47[dig], color, digit47[digPrev], scroll, 4, 7);
        break;
      case 5:
        digSetwh(x + (abs(digits) - d) * 5, y, digit49[dig], color, digit49[digPrev], scroll, 4, 9);
        break;
      case 100:
        digSetwh(x + (abs(digits) - d) * 3, y, digit24[dig], color, digit24[digPrev], scroll, 2, 4);
        break;
    }
    digs = digs / 10;
    digsPrev = digsPrev / 10;
  }
}

// x, y, цифра, цвет, шрифт, кол-во отображаемых цифр, предыдущая цифра, скролл между предыдущей и текущей.
void digPrint(int8_t x, int8_t y, int digs, int32_t color, byte sz, byte digits, int digsPrev, byte scroll) {
  digPrintM(x, y, digs, color, sz, digits, digsPrev, scroll, 1);
}

// Вывод слова txt цветом color, размером 3х5 (при ext=true некоторые буквы 5х5)
void symPrint(int8_t x, int8_t y, const char* str, int32_t color, boolean ext) {
  byte plusx = 0;  // смещение координат Х для вывода следующей буквы
  //byte i = 0;
  while (*str > 0) {
    if (*str != 208) {  // если не второй код (юникод?) русской раскладки
      symbSet(x + plusx, y, *str, color, ext);
      plusx = plusx + 4;

      // если широкая буква и ext=true, то следующую смещаем на 1 или 2 позиции (И Й Ъ - на 1, Д Ж М Ф Ш Щ Ы Ю на 2)
      if ((ext) && (((*str) == 148) || ((*str) == 150) || ((*str) == 156) || ((*str) == 164) || ((*str) == 168) || ((*str) == 169) || ((*str) == 171) || ((*str) == 174))) plusx += 2;
      if ((ext) && (((*str) == 152) || ((*str) == 153) || ((*str) == 170))) plusx++;
    }
    str++;
  }
}
