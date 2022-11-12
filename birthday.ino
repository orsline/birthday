#include <ThreeWire.h>
#include <RtcDS1302.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

volatile int ClockRSTPin;
volatile int ClockDATPin;
volatile int ClockClkPin;
String _LsTime;
String _LsDate;
volatile unsigned long _LsSec;
volatile int setYear;
volatile int setmonth;
volatile int setday;
volatile int sethour;
volatile int setminute;
volatile int secondChangeFlag;
volatile int setsecond;
volatile int Lastsetsecond;
volatile unsigned long TotalSecond;
volatile int currentSecond;
volatile int setweek;
volatile int currminisecond;
volatile int lastminisecond;
volatile int blink;
ThreeWire myWire(11, 10, 12);
RtcDS1302<ThreeWire> Rtc(myWire);
volatile int AdjustInput;
volatile int LastAdjustInput;
volatile int switchinputChangeFlg;
volatile int menuValue;
volatile int setinput;
volatile int LastsetInput;
volatile int setinputChangeFlg;
volatile int setFlag;
volatile int setdoneFlag;
volatile int AdjustPin;
volatile int setPin;
volatile int ledPin;
volatile int execFlag;
volatile int NotOpSecond;
String tmp;
String tmp2;
String tmp3;
volatile int tmp4;
String weekstring[] = {"", "Mon.", "Tue.", "Wed.", "Thu.", "Fri.", "Sat.", "Sun."};
char *monthstring[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "July", "Agu", "Sep", "Oct", "Nov", "Dec"};

int maxdays[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

LiquidCrystal_I2C mylcd(0x27, 16, 2);
String birthText1;
String birthText2;
volatile int temp;
volatile int humi;
String temperaturetext;
String humiditytext;
volatile int tempdisplaySec;
volatile int musicSetPin;
volatile int musicbusyPin;
volatile int music1Pin;
volatile int music2Pin;
volatile int music3Pin;
volatile int music4Pin;
DHT dhtA2(A2, 11);

void clock_init()
{
    Rtc.SetDateTime(RtcDateTime(__DATE__, __TIME__));
    get_current_time();
}

void LED_init()
{
    // digitalWrite(9,HIGH);
    mylcd.backlight();
}

void Music_init()
{
    delay(10);
    pinMode(musicSetPin, OUTPUT);
    digitalWrite(musicSetPin, LOW);
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    play_music(3);
}

void main_process()
{
    refresh_second();
    refresh_set_key();
    refresh_adjust_key();

    if (setFlag == 0)
    {
        main_display();
    }
    else
    {
        setproc_display();
    }

    ledbacklight_proc();
}

void ledbacklight_proc()
{
    if (secondChangeFlag == 1)
    {
        // 超过10秒不操作，关闭背光
        if (NotOpSecond < 10)
        {
            mylcd.backlight();
        }
        else
        {
            mylcd.noBacklight();
        }
    }
}

void setproc_display()
{
    currminisecond = millis();
    if (currminisecond - lastminisecond > 250)
    {
        lastminisecond = currminisecond;
        blink = (long)((blink + 1)) % (long)(2);
        pinMode(ledPin, OUTPUT);
        digitalWrite(ledPin, LOW);

        switch (setFlag)
        {
        case 0:
            break;
        case 1:
            display_num_blink(2, 2, setYear);
            break;
        case 2:
            display_num_noblink(2, 2, setYear);
            display_num_blink(2, 7, setmonth);
            break;
        case 3:
            display_num_noblink(2, 7, setmonth);
            display_num_blink(2, 10, setday);
            setdoneFlag = 1;
            break;
        case 4:

            display_num_noblink(2, 10, setday);
            if (setdoneFlag == 1)
            {
                // Serial.print(String("set: ") + String(setYear)+ String("-") + String(setmonth) + String("-") + String(setday) );
                // Serial.println(String(" ") + String(sethour)+ String(":") + String(setminute) + String(":") + String(setsecond));
                set_time_to_rtc(setYear, setmonth, setday, sethour, setminute, setsecond, setweek);
                mylcd.setCursor(0, 1);
                mylcd.print("Date Set OK");
                delay(1000);
                setFlag = 5;
            }

            delay(10);
            display_current_time3();
            // Serial.print(String("after: ") + String(setYear)+ String("-") + String(setmonth) + String("-") + String(setday) );
            // Serial.println(String(" ") + String(sethour)+ String(":") + String(setminute) + String(":") + String(setsecond));

            setdoneFlag = 0;
            break;
        case 5:
            display_num_blink(1, 5, sethour);
            break;
        case 6:
            display_num_noblink(1, 5, sethour);
            display_num_blink(1, 8, setminute);
            break;
        case 7:
            display_num_noblink(1, 8, setminute);
            display_num_blink(1, 11, setsecond);
            setdoneFlag = 1;
            break;
        case 8:
            if (setdoneFlag == 1)
            {
                display_num_noblink(1, 11, setsecond);
                set_time_to_rtc(setYear, setmonth, setday, sethour, setminute, setsecond, setweek);
                delay(10);
                mylcd.setCursor(0, 0);
                mylcd.print("Time Set OK");
                delay(1000);
                setFlag = 0;
            }
            display_current_time3();

            setdoneFlag = 0;
            break;
        default:
            break;
        }
    }
}

void display_num_blink(int x, int y, int z)
{
    if (z > 1000)
    { // year 与4个空格
        tmp3 = "    ";
    }
    else
    {
        tmp3 = "  "; //其他的都是2位
    }
    if (blink == 1)
    {
        tmp = adjust_string(z);
    }
    else
    {
        tmp = tmp3;
    }
    mylcd.setCursor(y - 1, x - 1);
    mylcd.print(tmp);
}

void display_num_noblink(int x, int y, int z)
{
    tmp = adjust_string(z);
    mylcd.setCursor(y - 1, x - 1);
    mylcd.print(tmp);
}
boolean mixly_digitalRead(uint8_t pin)
{
    pinMode(pin, INPUT);
    boolean _return = digitalRead(pin);
    pinMode(pin, OUTPUT);
    return _return;
}

String adjust_string(int x)
{
    tmp2 = "";
    if (x >= 0 && x <= 9)
    {
        tmp2 = String("0") + String(x);
    }
    else
    {
        tmp2 = x;
    }
    return tmp2;
}

void get_current_time()
{
    setYear = Rtc.GetDateTime().Year();
    setmonth = Rtc.GetDateTime().Month();
    setday = Rtc.GetDateTime().Day();
    sethour = Rtc.GetDateTime().Hour();
    setminute = Rtc.GetDateTime().Minute();
    setsecond = Rtc.GetDateTime().Second();
    setweek = Rtc.GetDateTime().DayOfWeek();
}

void refresh_second()
{
    currentSecond = Rtc.GetDateTime().Second();
    if (currentSecond != Lastsetsecond)
    {
        TotalSecond = TotalSecond + 1;
        Lastsetsecond = currentSecond;
        secondChangeFlag = 1;
        NotOpSecond = NotOpSecond + 1;
    }
    else
    {
        secondChangeFlag = 0;
    }
}
void refresh_set_key()
{
    setinput = mixly_digitalRead(setPin);
    if (setinput == 1)
    {
        NotOpSecond = 0;
        if (LastsetInput == 0)
        {
            //状态描述：
            // 0、非设置状态
            // 1、设置状态，设置年
            // 2、设置状态，设置月
            // 3、设置状态，设置日
            // 4、年月日生效
            // 5、设置状态，设置时
            // 6、设置状态，设置分
            // 7、设置状态，设置秒
            // 8、时间生效
            //

            setFlag = (long)((setFlag + 1)) % (long)(9);
            if (setFlag == 0)
            {
                mylcd.clear();
                get_current_time();
                menuValue = 0;
            }
            if (setFlag == 1 || setFlag == 5)
            {

                mylcd.clear();
                play_music(3);
                display_current_time3();
            }
        }
        else
        {
            //当前按键一直处于按下去的状态，什么都不处理
        }
    }
    LastsetInput = setinput;
}
void refresh_adjust_key()
{
    AdjustInput = mixly_digitalRead(AdjustPin);
    if (AdjustInput == 1)
    {
        NotOpSecond = 0;
        if (LastAdjustInput == 0)
        {
            // setFlag为0说明没有进入设置状态
            if (setFlag == 0)
            {
                refresh_main_list();
            }
            else
            {
                refresh_set_time_num();
            }
        }
        else
        {
            //当前按键一直处于按下去的状态，什么都不处理
        }
    }
    LastAdjustInput = AdjustInput;
}

void main_display()
{
    if (execFlag == 1)
    {
        execFlag = 0;
        mylcd.clear();
        switch (menuValue)
        {
        case 0:
            pinMode(ledPin, OUTPUT);
            digitalWrite(ledPin, LOW);
            play_music(3);
            display_current_time3();
            break;
        case 1:
            pinMode(ledPin, OUTPUT);
            digitalWrite(ledPin, LOW);
            play_music(3);
            display_temperature();
            break;
        case 2:
            pinMode(ledPin, OUTPUT);
            digitalWrite(ledPin, HIGH);
            display_birthday_string();
            play_music(1);
            break;
        case 3:
            pinMode(ledPin, OUTPUT);
            digitalWrite(ledPin, HIGH);
            display_birthday_string();
            play_music(2);
            break;
        default:
            pinMode(ledPin, OUTPUT);
            digitalWrite(ledPin, LOW);
            break;
        }
    }
    switch (menuValue)
    {
    case 0:
        display_current_time();
        break;
    case 1:
        display_temperature();
        break;
    }
}

void refresh_set_time_num()
{
    if (setFlag >= 1)
    {
        switch (setFlag)
        {
        case 1:
            setYear = setYear + 1;
            if (setYear > 2050)
            {
                setYear = 2000;
            }
            break;
        case 2:
            setmonth = setmonth + 1;
            if (setmonth > 12)
            {
                setmonth = 1;
            }
            break;
        case 3:
            setday = setday + 1;
            tmp4 = maxdays[setmonth];
            if ((long)(setYear) % (long)(4) == 0)
            {
                tmp4 = 29;
            }
            if (setday > tmp4)
            {
                setday = 1;
            }
            setdoneFlag = 1;
            break;
        case 4:
            // setdoneFlag = 1;
            break;
        case 5:
            sethour = (long)((sethour + 1)) % (long)(24);
            break;
        case 6:
            setminute = (long)((setminute + 1)) % (long)(60);
            break;
        case 7:
            setsecond = (long)((setsecond + 1)) % (long)(60);
            setdoneFlag = 1;
            break;
        case 8:
            // setdoneFlag = 1;
            break;
        }
    }
}

void refresh_main_list()
{
    //状态描述：
    // 0、播放生日歌音乐，闪灯
    // 1、播放祝福语，闪灯
    // 2、显示时间，不闪灯
    // 3、显示temp，不闪灯
    menuValue = (long)((menuValue + 1)) % (long)(4);
    execFlag = 1;
}

void play_music(int x)
{
    pinMode(musicSetPin, OUTPUT);
    digitalWrite(musicSetPin, LOW);
    switch (x)
    {
    case 1:
        pinMode(music1Pin, OUTPUT);
        digitalWrite(music1Pin, LOW);
        delay(30);
        pinMode(music1Pin, OUTPUT);
        digitalWrite(music1Pin, HIGH);

        break;
    case 2:
        pinMode(music2Pin, OUTPUT);
        digitalWrite(music2Pin, LOW);
        delay(30);
        pinMode(music2Pin, OUTPUT);
        digitalWrite(music2Pin, HIGH);
        break;
    case 3:
        pinMode(music3Pin, OUTPUT);
        digitalWrite(music3Pin, LOW);
        delay(30);
        pinMode(music3Pin, OUTPUT);
        digitalWrite(music3Pin, HIGH);
        break;
    }
}

void display_temperature()
{
    if (secondChangeFlag == 1)
    {
        temp = dhtA2.readTemperature();
        humi = dhtA2.readHumidity();
        mylcd.setCursor(0, 0);
        mylcd.print(String(temperaturetext) + String(temp));
        mylcd.setCursor(0, 1);
        mylcd.print(String(humiditytext) + String(humi));
    }
}

void display_birthday_string()
{
    mylcd.clear();
    birthText1 = "";
    birthText2 = "";
    birthText1 = String(birthText1) + String("Happy Birthday!");
    birthText2 = String(birthText2) + String("Kerry,10th.2022");
    mylcd.setCursor(0, 0);
    mylcd.print(birthText1);
    mylcd.setCursor(0, 1);
    mylcd.print(birthText2);
}

void display_current_time()
{
    if (secondChangeFlag == 1)
    {
        mylcd.clear();
        display_current_time3();
    }
}

void set_time_to_rtc(int year, int month, int day, int hour, int minute, int second, int week)
{
    char dateTmp[13];
    char timeTmp[10];
    sprintf(dateTmp, "%s/%02d/%02d", monthstring[month], day, year);
    sprintf(timeTmp, "%02d:%02d:%02d", hour, minute, second);
    RtcDateTime compiled = RtcDateTime(dateTmp, timeTmp);
    Rtc.SetDateTime(compiled);
}

void display_current_time3()
{
    get_current_time();
    display_set_time();
    display_set_date();
}

void display_set_date()
{
    _LsDate = String(setYear) + String("-");
    _LsDate = String(_LsDate) + String((adjust_string(setmonth)));
    _LsDate = String(_LsDate) + String("-");
    _LsDate = String(_LsDate) + String((adjust_string(setday)));
    _LsDate = String(_LsDate) + String(" ");
    _LsDate = String(_LsDate) + String(weekstring[Rtc.GetDateTime().DayOfWeek()]);
    mylcd.setCursor(2 - 1, 2 - 1);
    mylcd.print(_LsDate);
}

void display_set_time()
{
    _LsTime = String((adjust_string(sethour))) + String(":");
    _LsTime = String(_LsTime) + String((adjust_string(setminute)));
    _LsTime = String(_LsTime) + String(":");
    _LsTime = String(_LsTime) + String((adjust_string(setsecond)));
    _LsTime = String(_LsTime) + String(" ");
    mylcd.setCursor(5 - 1, 1 - 1);
    mylcd.print(_LsTime);
}

void setup()
{
    ClockRSTPin = 12;
    ClockDATPin = 11;
    ClockClkPin = 10;
    _LsTime = "";
    _LsDate = "";
    _LsSec = 0;
    setYear = 0;
    setmonth = 0;
    setday = 0;
    sethour = 0;
    setminute = 0;
    secondChangeFlag = 0;
    setsecond = 0;
    Lastsetsecond = 0;
    TotalSecond = 0;
    currentSecond = 0;
    setweek = 0;
    currminisecond = 0;
    lastminisecond = 0;
    blink = 0;
    Rtc.Begin();
    Rtc.SetIsRunning(true);
    AdjustInput = 0;
    LastAdjustInput = 1;
    switchinputChangeFlg = 0;
    menuValue = 0;
    setinput = 0;
    LastsetInput = 0;
    setinputChangeFlg = 0;
    setFlag = 0;
    setdoneFlag = 0;
    AdjustPin = 9;
    setPin = 8;
    ledPin = 7;
    execFlag = 1;
    NotOpSecond = 0;
    tmp = "0";
    tmp2 = "";
    tmp3 = "";
    tmp4 = "";
    LED_init();
    Music_init();
    clock_init();

    mylcd.init();
    mylcd.backlight();
    pinMode(9, OUTPUT);
    birthText1 = ' ';
    birthText2 = ' ';
    // Serial.begin(9600);
    temp = 0;
    humi = 0;
    temperaturetext = "temperature: ";
    humiditytext = "humidity:    ";
    tempdisplaySec = 0;
    musicSetPin = 0;
    musicbusyPin = 5;
    music1Pin = 1;
    music2Pin = 2;
    music3Pin = 3;
    music4Pin = 4;
    dhtA2.begin();
}

void loop()
{
    main_process();
}