#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include "tab_temp.h"

const uint16_t temp_tab[] PROGMEM = {941,939,938,936,935,933,932,930,928,927,925,924,922,921,919,918,//0 473  // ADC -> 950 - 450
                916,914,913,911,910,908,907,905,904,902,901,899,898,897,895,894, //16
                892,891,889,888,886,885,883,882,881,879,878,876,875,873,872,871, //32
                869,868,866,865,864,862,861,860,858,857,855,854,853,851,850,849, //48
                847,846,845,843,842,841,839,838,837,835,834,833,831,830,829,828, //64
                826,825,824,822,821,820,819,817,816,815,813,812,811,810,808,807, //80
                806,805,803,802,801,800,798,797,796,795,793,792,791,790,789,787, //96
                786,785,784,782,781,780,779,778,776,775,774,773,772,770,769,768, //112
                767,766,765,763,762,761,760,759,757,756,755,754,753,752,750,749, //128
                748,747,746,745,743,742,741,740,739,738,736,735,734,733,732,731, //144
                729,728,727,726,725,724,723,721,720,719,718,717,716,714,713,712, //160
                711,710,709,708,706,705,704,703,702,701,700,698,697,696,695,694, //176
                693,692,690,689,688,687,686,685,684,682,681,680,679,678,677,676, //192
                674,673,672,671,670,669,667,666,665,664,663,662,660,659,658,657, //208
                656,655,654,652,651,650,649,648,646,645,644,643,642,641,639,638, //224
                637,636,635,634,632,631,630,629,628,626,625,624,623,622,620,619, //240
                618,617,616,614,613,612,611,609,608,607,606,604,603,602,601,600, //256
                598,597,596,595,593,592,591,590,588,587,586,584,583,582,581,579, //272
                578,577,576,574,573,572,570,569,568,566,565,564,563,561,560,559, //288
                557,556,555,553,552,551,549,548,546,545,544,542,541,540,538,537, //304
                536,534,533,531,530,529,527,526,524,523,522,520,519,517,516,514, //320
                513,511,510,509,507,506,504,503,501,500,498,497,495,494,492,491, //336
                489,488,486,485,483,482,480,479,477,476,474,472,471,469,468,466, //352
                465,463,461,460,458,457,455,453,452,450,448,447,445,444,442,440, //368
                439,437,435,434,432,430,428,427,425,423,422,420,418,416,415,413, //384
                411,409,408,406,404,402,401,399,397,395,393,392,390,388,386,384, //400
                382,381,379,377,375,373,371,369,367,366,364,362,360,358,356,354, //416
                352,350,348,346,344,342,340,338,336,334,332,330,328,326,324,322, //432
                320,318,316,314,312,310,308,306,303,301,299,297,295,293,291,288, //448
                286,284,282,280,278,275,273,271,269,266,264,262,260,257,255,253, //464
                251,248,246,244,241,239,237,234,232,230,227,225,223,220,218,215, //480
                213,211,208,206,203};

#define ADC_INF   0
#define ADC_SUP   1023
#define ADC_BUFF  8

uint16_t TempGet(){

  uint16_t data = 0, valor;

  // valor =  2000;
  valor = analogRead(A0);

  if(valor < ADC_INF){
    data = pgm_read_word(&temp[0]);
  }else if(valor > ADC_SUP){
    data = pgm_read_word(&temp[ADC_SUP - ADC_INF]);
  }else{
    data = pgm_read_word(&temp[valor - ADC_INF]);
  }
  Serial.println(analogRead(A0));
  return data;
}

//------------------------------------------------------------------------------
LiquidCrystal_I2C lcd(0x3f, 16, 2); 
//------------------------------------------------------------------------------
MAX6675 thermocouple;

int thermoSO_pin = 8;
int thermoCS_pin = 9;
int thermoCLK_pin = 10;

int mosfet_pin = 11;

double temp_1_signal, temp_2_signal, erro_signal, ref_signal, pwm_signal;
#define BUFFER_SIZE 8
int buffer_signal_index;
double buffer_signal[BUFFER_SIZE];
const double fs_signal = 32;
const double Ts_signal = 1/fs_signal;

void setup() {
	//=====================================
	Serial.begin(9600);
	//=====================================
	// initialize the lcd 
	lcd.init();                      
	// Print a message to the LCD.
	lcd.backlight();
	lcd.setCursor(0,0);
	lcd.print("START!");
	//=====================================
	thermocouple.begin(thermoCLK_pin, thermoCS_pin, thermoSO_pin);
	//=====================================
	pinMode(mosfet_pin, OUTPUT);
  for(int i=0; i<BUFFER_SIZE; i++)
  {
    buffer_signal[i] = 0;
  }
}

void loop() 
{
	ref_signal = analogRead(A1)>>2;

	temp_1_signal = double(TempGet());

  if(buffer_signal_index<BUFFER_SIZE)
    buffer_signal_index++;
  else
    buffer_signal_index=0;
  
  buffer_signal[buffer_signal_index] = temp_1_signal;

  temp_1_signal = 0;
  for(int i=0; i<BUFFER_SIZE; i++)
  {
    temp_1_signal = temp_1_signal + buffer_signal[i];
  }
  temp_1_signal = temp_1_signal/BUFFER_SIZE;

	erro_signal = ref_signal - temp_1_signal;
	pwm_signal = 10*erro_signal;

	if(pwm_signal>255)
		pwm_signal = 255;
	if(pwm_signal<0)
		pwm_signal = 0;


	analogWrite(mosfet_pin, uint8_t(pwm_signal));
	// basic readout test, just print the current temp
	lcd.clear();
	lcd.setCursor(0, 0);
  lcd.print("temp: ");
	lcd.print(temp_1_signal);
	// go to line #1
	lcd.setCursor(0,1);
  lcd.print("ref: ");
	lcd.print(ref_signal);

	delay(1000*Ts_signal);
}
