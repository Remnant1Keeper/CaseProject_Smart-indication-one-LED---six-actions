// Определение пинов для кнопок и светодиодов
int buttons[] = {2, 3, 4};  // Кнопки подключены к цифровым пинам 2,3,4
int leds[] = {5, 6, 7};     // Светодиоды подключены к цифровым пинам 5,6,7

// Переменные для отслеживания состояний кнопок
bool btnNow[3] = {HIGH, HIGH, HIGH};    // Текущее состояние кнопок
bool btnOld[3] = {HIGH, HIGH, HIGH};    // Предыдущее состояние кнопок
unsigned long btnTimer[3] = {0, 0, 0};  // Время начала нажатия
int clickStage[3] = {0, 0, 0};          // Стадия нажатия: 0-ожидание, 1-первое нажатие
unsigned long firstClickTime[3] = {0, 0, 0}; // Время первого нажатия
bool needIndication[3] = {false, false, false}; // Флаг необходимости индикации
int indicationType[3] = {0, 0, 0};      // Тип индикации для каждого светодиода
bool isHolding[3] = {false, false, false}; // Флаг длительного удержания

void setup() 
{
  // Инициализация пинов
  for (int i = 0; i < 3; i++) 
  {
    pinMode(buttons[i], INPUT_PULLUP);  // Кнопки с подтяжкой к питанию
    pinMode(leds[i], OUTPUT);           // Светодиоды как выходы
    digitalWrite(leds[i], LOW);         // Выключить светодиоды при старте
  }
}

void loop() 
{
  unsigned long time = millis();  // Получить текущее время
  
  // Обработка каждой кнопки
  for (int i = 0; i < 3; i++) 
  {
    btnOld[i] = btnNow[i];  // Сохранить предыдущее состояние
    btnNow[i] = digitalRead(buttons[i]);  // Прочитать текущее состояние
    
    // Обнаружение начала нажатия
    if (btnNow[i] == LOW && btnOld[i] == HIGH) 
    {
      btnTimer[i] = time;  // Запомнить время начала нажатия
      isHolding[i] = false;
    }
    
    // Если кнопка нажата и это может быть длинное нажатие
    if (btnNow[i] == LOW) 
    {
      unsigned long pressLength = time - btnTimer[i];
      
      // Если нажатие дольше 1000 мс - зажигаем светодиод
      if (pressLength > 1000 && !isHolding[i]) 
      {
        digitalWrite(leds[i], HIGH);  // Горит пока нажата
        isHolding[i] = true;
      }
    }
    
    // Обнаружение отпускания кнопки
    if (btnNow[i] == HIGH && btnOld[i] == LOW) 
    {
      unsigned long pressLength = time - btnTimer[i];  // Вычислить длительность нажатия
      
      // Всегда гасим светодиод при отпускании
      digitalWrite(leds[i], LOW);
      isHolding[i] = false;
      
      // Если это первое нажатие в последовательности
      if (clickStage[i] == 0) 
      {
        if (pressLength > 2000) 
        {
          // Зажатие - моментальное выключение (без индикации)
        }
        else if (pressLength > 1000) 
        {
          // Длительное нажатие
          needIndication[i] = true;
          indicationType[i] = 2; // Длительное - 800 мс
        }
        else 
        {
          // Короткое нажатие - начинаем ожидание второго нажатия
          clickStage[i] = 1;
          firstClickTime[i] = time;
          // НЕ индицируем сразу, ждем возможного второго нажатия
        }
      }
      // Если ожидаем второе нажатие
      else if (clickStage[i] == 1) 
      {
        // Второе нажатие всегда сбрасывает ожидание
        clickStage[i] = 0;
        
        if (pressLength > 2000) 
        {
          // Короткое + зажатие - моментальное выключение (без индикации)
        }
        else if (pressLength > 1000) 
        {
          // Короткое + длинное - показываем комбинированную индикацию
          needIndication[i] = true;
          indicationType[i] = 4; // Комбо короткое+длинное
        }
        else 
        {
          // Двойное короткое
          needIndication[i] = true;
          indicationType[i] = 6; // Двойное - мигание 2 раза
        }
      }
    }
    
    // Проверка таймаута для ожидания второго нажатия (500 мс)
    if (clickStage[i] == 1 && (time - firstClickTime[i] > 500)) 
    {
      // Если время вышло и второго нажатия не было - это одиночное короткое
      needIndication[i] = true;
      indicationType[i] = 1; // Короткое - 150 мс
      clickStage[i] = 0;
    }
  }
  
  // Выполнение индикации ПОСЛЕ обработки всех кнопок
  for (int i = 0; i < 3; i++) 
  {
    if (needIndication[i]) 
    {
      executeIndication(i, indicationType[i]);
      needIndication[i] = false;
      indicationType[i] = 0;
    }
  }
  
  checkMultiButton(time);  // Проверить множественное нажатие
}

// Выполнение индикации
void executeIndication(int ledNum, int type) 
{
  switch(type) 
  {
    case 1: // Короткое нажатие
      digitalWrite(leds[ledNum], HIGH);
      delay(150);
      digitalWrite(leds[ledNum], LOW);
      break;
      
    case 2: // Длительное нажатие
      digitalWrite(leds[ledNum], HIGH);
      delay(800);
      digitalWrite(leds[ledNum], LOW);
      break;
      
    case 4: // Короткое + длинное (комбинированная индикация)
      // Сначала короткое
      digitalWrite(leds[ledNum], HIGH);
      delay(150);
      digitalWrite(leds[ledNum], LOW);
      delay(100);
      // Потом длинное
      digitalWrite(leds[ledNum], HIGH);
      delay(800);
      digitalWrite(leds[ledNum], LOW);
      break;
      
    case 6: // Двойное короткое
      for (int j = 0; j < 2; j++) 
      {
        digitalWrite(leds[ledNum], HIGH);
        delay(200);
        digitalWrite(leds[ledNum], LOW);
        if (j < 1) delay(100);
      }
      break;
  }
}

// Проверка одновременного нажатия нескольких кнопок
void checkMultiButton(unsigned long time) 
{
  static unsigned long multiTimer = 0;  // Таймер для множественного нажатия
  static bool multiActive = false;      // Флаг активного множественного нажатия
  static bool needMultiIndication = false; // Флаг необходимости множественной индикации
  
  int count = 0;
  // Подсчет нажатых кнопок
  for (int i = 0; i < 3; i++) 
  {
    if (btnNow[i] == LOW) 
    {
      count++;
    }
  }
  
  // Обнаружение начала множественного нажатия
  if (count >= 2 && !multiActive) 
  {
    multiTimer = time;
    multiActive = true;
  }
  
  // Обработка завершения множественного нажатия
  if (count == 0 && multiActive && (time - multiTimer > 50)) 
  {
    needMultiIndication = true;
    multiActive = false;
  }
  
  // Выполнение множественной индикации
  if (needMultiIndication) 
  {
    // Мигание всеми светодиодами 3 раза
    for (int j = 0; j < 3; j++) 
    {
      for (int i = 0; i < 3; i++) 
      {
        digitalWrite(leds[i], HIGH);
      }
      delay(150);  // Длительность свечения при мигании
      for (int i = 0; i < 3; i++) 
      {
        digitalWrite(leds[i], LOW);
      }
      if (j < 2) delay(100);  // Пауза между миганиями
    }
    needMultiIndication = false;
  }
}