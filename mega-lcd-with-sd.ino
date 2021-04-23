/***********************************************************************************
  @contributor NaAl
  To use the SD card as is (NO HW changes) an older version of the SD library is 
  needed, use the instuctions here:
  https://learn.adafruit.com/adafruit-data-logger-shield/for-the-mega-and-leonardo
  to install the older library.
  One the library is installed, format an SD Card (FAT) and copy some BMP files to the
  SD Card (code used 01.bmp etc) and upload this sketch to the MEGA and enjoy:)
  

  This program is a demo of how to display a bmp picture from SD card
  This demo was made for LCD modules with 8bit or 16bit data port.
  This program requires the the LCDKIWI library.

  File                : show_bmp_picture.ino
  Hardware Environment: Arduino MEGA with SOFT SPI (@contributor NaAl)
  Build Environment   : Arduino

  Set the pins to the correct ones for your development shield or breakout board.
  This demo use the BREAKOUT BOARD only and use these 8bit data lines to the LCD,
  pin usage as follow:
                   LCD_CS  LCD_CD  LCD_WR  LCD_RD  LCD_RST  SD_SS  SD_DI  SD_DO  SD_SCK
      Arduino Uno    A3      A2      A1      A0      A4      10     11     12      13

                   LCD_D0  LCD_D1  LCD_D2  LCD_D3  LCD_D4  LCD_D5  LCD_D6  LCD_D7
      Arduino Uno    8       9       2       3       4       5       6       7

  Remember to set the pins to suit your display module!

  @attention

  THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  TIME. AS A RESULT, QD electronic SHALL NOT BE HELD LIABLE FOR ANY
  DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  
**********************************************************************************/

#include <SD.h>

#include <LCDWIKI_GUI.h> //Core graphics library
#include <LCDWIKI_KBV.h> //Hardware-specific library

Sd2Card card;
SdVolume volume;
SdFile root;
//if the IC model is known or the modules is unreadable,you can use this constructed function
LCDWIKI_KBV my_lcd(ILI9486, A3, A2, A1, A0, A4); //model,cs,cd,wr,rd,reset
//if the IC model is not known and the modules is readable,you can use this constructed function
//LCDWIKI_KBV my_lcd(320,480,A3,A2,A1,A0,A4);//width,height,cs,cd,wr,rd,reset

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define FILE_NUMBER 4
#define FILE_NAME_SIZE_MAX 20
#define PORTRAIT  0
#define LANDSCAPE 1
#define USE_XPT2046   0
#define USE_LOCAL_KBV 1

uint32_t bmp_offset = 0;
uint16_t SCREEN_WIDTH;
uint16_t SCREEN_HEIGHT;
int16_t PIXEL_NUMBER;

char file_name[FILE_NUMBER][FILE_NAME_SIZE_MAX];

uint16_t read_16(SdFile &fp)
{
  uint8_t low;
  uint16_t high;
  low = fp.read();
  high = fp.read();
  return (high << 8) | low;
}

uint32_t read_32(SdFile &fp)
{
  uint16_t low;
  uint32_t high;
  low = read_16(fp);
  high = read_16(fp);
  return (high << 8) | low;
}

bool analysis_bpm_header(SdFile &fp)
{
  if (read_16(fp) != 0x4D42)
  {
    return false;
  }
  //get bpm size
  read_32(fp);
  //get creator information
  read_32(fp);
  //get offset information
  bmp_offset = read_32(fp);
  //get DIB infomation
  read_32(fp);
  //get width and heigh information
  uint32_t bpm_width = read_32(fp);
  uint32_t bpm_heigh = read_32(fp);
  if ((bpm_width != SCREEN_WIDTH) || (bpm_heigh != SCREEN_HEIGHT))
  {
    return false;
  }
  if (read_16(fp) != 1)
  {
    return false;
  }
  read_16(fp);
  if (read_32(fp) != 0)
  {
    return false;
  }
  return true;
}

void draw_bmp_picture(SdFile &fp)
{
  uint16_t i, j, k, l, m = 0;
  uint8_t bpm_data[PIXEL_NUMBER * 3] = {0};
  uint16_t bpm_color[PIXEL_NUMBER];

  fp.seekSet(bmp_offset);
  for (i = 0; i < SCREEN_HEIGHT; i++)
  {
    for (j = 0; j < SCREEN_WIDTH / PIXEL_NUMBER; j++)
    {
      m = 0;
      fp.read(bpm_data, PIXEL_NUMBER * 3);
      for (k = 0; k < PIXEL_NUMBER; k++)
      {
        bpm_color[k] = my_lcd.Color_To_565(bpm_data[m + 2], bpm_data[m + 1], bpm_data[m + 0]); //change to 565
        m += 3;
      }
      for (l = 0; l < PIXEL_NUMBER; l++)
      {
        my_lcd.Set_Draw_color(bpm_color[l]);
        my_lcd.Draw_Pixel(j * PIXEL_NUMBER + l, i);
      }
    }
  }
}

void setup()
{
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  my_lcd.Init_LCD();
  my_lcd.Set_Rotation(LANDSCAPE);
  Serial.print("LCD ID: ");
  Serial.println(my_lcd.Read_ID(), HEX);
  my_lcd.Fill_Screen(RED);
  my_lcd.Set_Text_colour(WHITE);
  my_lcd.Set_Text_Size(2);
  my_lcd.Print_String("Initializing SD CARD..." , 20, 1);
  /**
     init the SD card with soft SPI
  */
  while (!card.init(SPI_FULL_SPEED, 10, 11, 12, 13)) {
   
    Serial.println("initialization failed. Things to check:");
    my_lcd.Print_String("Initializing SD CARD... FAILED!" , 20, 1);

    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    my_lcd.Print_String("* is a card inserted?" , 20, 20);
    my_lcd.Print_String("* is your wiring correct?" , 20, 40);
    my_lcd.Print_String("* did you change the chipSelect pin" , 20, 60);
    my_lcd.Print_String("  to match your shield or module?" , 20, 80);

  }
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    my_lcd.Set_Text_colour(WHITE);
    my_lcd.Set_Text_Size(1);
    my_lcd.Print_String("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card!", 1, 10);
    while (1);
  }
  my_lcd.Print_String("Initializing SD CARD... Done!" , 20, 1);
  delay(2000);
  my_lcd.Fill_Screen(BLUE);
  my_lcd.Set_Rotation(PORTRAIT);
  
  SCREEN_WIDTH = my_lcd.Get_Display_Width();
  SCREEN_HEIGHT = my_lcd.Get_Display_Height();
  PIXEL_NUMBER = my_lcd.Get_Display_Width() / 4;
  if (PIXEL_NUMBER == 60) //240*320
  {
    strcpy(file_name[0], "tulip.bmp");
    strcpy(file_name[1], "game.bmp");
    strcpy(file_name[2], "tree.bmp");
    strcpy(file_name[3], "flower.bmp");
  }
  else //320*480
  {
    strcpy(file_name[0], "01.bmp");
    strcpy(file_name[1], "02.bmp");
    strcpy(file_name[2], "03.bmp");
    strcpy(file_name[3], "04.bmp");
  }


  root.openRoot(volume);
  root.ls(LS_R | LS_DATE | LS_SIZE);

}

void slideShow() {

  int i = 0;
  SdFile file;
  for (i = 0; i < FILE_NUMBER; i++){
    file.open(&root, file_name[i], O_RDONLY);
    if (!analysis_bpm_header(file)) {
      my_lcd.Set_Text_Back_colour(BLUE);
      my_lcd.Set_Text_colour(WHITE);
      my_lcd.Set_Text_Size(1);
      my_lcd.Print_String("bad bmp picture!", 0, 0);
      continue;
    }
    draw_bmp_picture(file);
    file.close();
    delay(2000);
  }
}
void loop()
{
  slideShow();

}
