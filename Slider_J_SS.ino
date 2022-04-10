#define BLINKER_PRINT Serial
#define BLINKER_WIFI
#include <Blinker.h>
#include <Servo.h> //加载舵机库
Servo myservo1; //创建舵机名称
Servo myservo2;

/*********软件密钥、网络名称与密码，记得修改*******/
char auth[] = "2dbf9013d0df"; //点灯KEY
char ssid[] = "Rain";         //WiFi名称
char pswd[] = "13774215117";  //WiFi密码
/*********软件密钥、网络名称与密码，记得修改*******/

/******************IO定义**************************/
char IO_JQ8X00_BusyCheck = 2;        //忙检测IO引脚
char IO_JQ8X00_VPP = D5;         //单线串口通信IO引脚

/******************全局变量**************************/
char JQ8X00_BusyCheck = 0;     //忙检测，0表示不做忙检测，1表做忙检测

typedef enum {
  Play     = 0x11,         /*播放*/
  Pause   = 0x12,         /*暂停*/
  Stop    = 0x13,         /*停止*/
  LastSong    = 0x14,         /*上一曲*/
  NextSong  = 0x15,         /*下一曲*/
  LastList  = 0x16,         /*上一目录*/
  NextList  = 0x17,         /*下一目录*/
  ChooseSD  = 0x18,         /*选择SD卡*/
  ChooseUdisk = 0x19,         /*选择U盘*/
  ChooseFlash = 0x1a,         /*选择Flash*/
  SysSleep  = 0x1b,         /*系统睡眠*/
} LineByteSelect;

typedef enum {
  Track     = 0x0B,         /*曲目*/
  Volume    = 0x0C,         /*音量*/
  EQ      = 0x0D,         /*EQ*/
  LoopMode    = 0x0E,         /*循环模式*/
  Channel   = 0x0F,         /*通道*/
  CBTrack   = 0x10,         /*插播曲目*/
} LineModeSelect;

void OneLine_SendData(u8 DATA)
{
  u8 i;

  digitalWrite(IO_JQ8X00_VPP, HIGH);  
  delayMicroseconds(100);
  digitalWrite(IO_JQ8X00_VPP, LOW);;
  delay(3);

  for (i = 0; i < 8; i++)
  {
    digitalWrite(IO_JQ8X00_VPP, HIGH);
    if (DATA & 0x01)
    {
      delayMicroseconds(1500);
      digitalWrite(IO_JQ8X00_VPP, LOW);
      delayMicroseconds(500);
    }
    else
    {
      delayMicroseconds(500);
      digitalWrite(IO_JQ8X00_VPP, LOW);
      delayMicroseconds(1500);
    }
    DATA >>= 1;
  }

  digitalWrite(IO_JQ8X00_VPP, HIGH);
}

/************************************************************************
  功能描述： 单线字节控制
  入口参数：   Mode：功能
  返 回 值： none
  其他说明： 将需要的功能作为形参传入
**************************************************************************/
void OneLine_ByteControl(LineByteSelect Mode)
{
  if (JQ8X00_BusyCheck)
  {
    delay(10);
    while (digitalRead(IO_JQ8X00_BusyCheck) == HIGH);     
  }

  OneLine_SendData(Mode);
}

/************************************************************************x
  功能描述： 单线控制组合发送函数
  入口参数：   Nume：数字，Mode：功能
  返 回 值： none
  其他说明： 将需要发送的数据和需要的功能作为形参传入
**************************************************************************/
void OneLine_ZHControl(LineModeSelect Mode, u8 Nume)
{
  if (JQ8X00_BusyCheck)
  {
    delay(10);
    while (digitalRead(IO_JQ8X00_BusyCheck) == HIGH);   
  }

  OneLine_SendData(0x0a);
  if (Nume < 10)
  {
    OneLine_SendData(Nume);
  }
  else
  {
    OneLine_SendData(Nume / 10);
    OneLine_SendData(Nume % 10);
  }
  OneLine_SendData(Mode);
}

// 新建组件对象
BlinkerButton Button1("BT1");   //给Button1起名为BT1（手机端按钮命名匹配）
BlinkerSlider Slider1("SL1");
BlinkerButton Button2("BT2");
BlinkerSlider Slider2("SL2");
BlinkerJoystick JOY1("JO1");
int value1;
int value2;
void joystick1_callback(uint8_t xAxis, uint8_t yAxis)
{
  BLINKER_LOG("Joystick1 X axis: ", xAxis);//从摇杆读取到“xAxis”值（摇杆值为0-255）
  value1 = map(xAxis, 0, 255, 0, 180);    //value1等于通过MAP函数把读取到的“xAxis”值转化为0-180度
  myservo1.write(value1);                 //把舵机1角度旋转到“value1”值
  BLINKER_LOG("Joystick1 Y axis: ", yAxis);
  value2 = map(yAxis, 0, 255, 0, 180);
  myservo2.write(value2);
}

void button1_callback(const String & state)
{
  BLINKER_LOG("get button state: ", state);//从按键1读取到“state”值
  if (state == "on") {              //如果state为on
    OneLine_ZHControl(LoopMode, 1);//打开循环模式
    OneLine_ZHControl(Track, 1);   //定位1号音频播放
  }
  else {
    OneLine_ByteControl(Stop);
  }
}
void button2_callback(const String & state)
{
  BLINKER_LOG("get button state: ", state);
  if (state == "on") {
    OneLine_ZHControl(LoopMode, 1);
    OneLine_ZHControl(Track, 2);
  }
  else {
    OneLine_ByteControl(Stop);
  }
}
void slider1_callback(int32_t value)
{
  BLINKER_LOG("get slider value: ", value); //从滑块1读取到“value”值
  OneLine_ZHControl(Volume, value);         //把音量调整为“value”值
}
void slider2_callback(int32_t value)
{
  BLINKER_LOG("get slider value: ", value);//从滑块2读取到“value”值
  myservo1.write(value);                   //把舵机角度旋转到“value”值
}

void dataRead(const String & data)
{
  BLINKER_LOG("Blinker readString: ", data);
  Blinker.vibrate();
  uint32_t BlinkerTime = millis();
  Blinker.print("millis", BlinkerTime);
 }

void setup() {
  // 初始化串口
  Serial.begin(115200);
#if defined(BLINKER_PRINT)
  BLINKER_DEBUG.stream(BLINKER_PRINT);
#endif
  // 初始化blinker
  Blinker.begin(auth, ssid, pswd);
  Button1.attach(button1_callback);  //初始化按钮1
  Button2.attach(button2_callback);  //初始化按钮2
  Slider1.attach(slider1_callback);  //初始化滑块1
  Slider2.attach(slider2_callback);  //初始化滑块2
  JOY1.attach(joystick1_callback);  //初始化摇杆1
  Blinker.attachData(dataRead);
  if (JQ8X00_BusyCheck)
    pinMode(IO_JQ8X00_BusyCheck, INPUT);    
  pinMode(IO_JQ8X00_VPP, OUTPUT);      
  digitalWrite(IO_JQ8X00_VPP, HIGH);   
  OneLine_ZHControl(Volume, 20);      //默认音量(0-30)
  myservo1.attach(D8, 420, 2500);    //舵机引脚和参数
  myservo1.write(90);                //舵机初始角度
  myservo2.attach(D7, 420, 2500);
  myservo2.write(90);
}

void loop() {
  Blinker.run();
}
