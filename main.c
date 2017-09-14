#include "system.h"
#include <alt_types.h>
#include <io.h>
#include <unistd.h>


#define ADRR_REG_VIDEO_MEM_ADDR    0
#define ADRR_REG_VIDEO_MEM_SIZE    1
#define ADRR_REG_REFRESH_RATE      2
#define ADRR_REG_CTRL              3
#define ADRR_REG_CMD               4
#define ADRR_REG_DATA              5

#define CTRL_REG_RESET             (1<<0)
#define CTRL_REG_LCD_ON            (1<<1)
#define CTRL_REG_REFRESH_ON        (1<<2)

#define LCD_WRITE(addr,value) IOWR(LT24_CONTROLLER_0_BASE,addr,value)

#define  LCD_WR_REG(value)   IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_CMD,value)
#define  LCD_WR_DATA(value)  IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_DATA,value)

#define  Set_LCD_RST  IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_CTRL,CTRL_REG_RESET | CTRL_REG_LCD_ON)
#define  Clr_LCD_RST  IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_CTRL,CTRL_REG_LCD_ON)

#define LCD_SET_ADDR(addr)  IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_VIDEO_MEM_ADDR,addr)
#define LCD_SET_SIZE(size)  IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_VIDEO_MEM_SIZE,size)
#define LCD_SET_REFRESH_RATE(refresh_rate)  IOWR(LT24_CONTROLLER_0_BASE,ADRR_REG_REFRESH_RATE,refresh_rate)


void Delay_Ms(alt_u16 count_ms)
{
    while(count_ms--)
    {
        usleep(1000);
    }
}

void LCD_Init(alt_u16 value)
{
//	alt_u16 data1,data2;
//	alt_u16 data3,data4;
	Clr_LCD_RST;
	Delay_Ms(1);
	Set_LCD_RST;
	Delay_Ms(10);       // Delay 10ms // This delay time is necessary
	Clr_LCD_RST;
	Delay_Ms(120);       // Delay 120 ms
//	Clr_LCD_CS;



	LCD_WR_REG(0x0011); //Exit Sleep
	LCD_WR_REG(0x00CF);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0081);
		LCD_WR_DATA(0X00c0);

	LCD_WR_REG(0x00ED);
		LCD_WR_DATA(0x0064);
		LCD_WR_DATA(0x0003);
		LCD_WR_DATA(0X0012);
		LCD_WR_DATA(0X0081);

	LCD_WR_REG(0x00E8);
		LCD_WR_DATA(0x0085);
		LCD_WR_DATA(0x0001);
		LCD_WR_DATA(0x00798);

	LCD_WR_REG(0x00CB);
		LCD_WR_DATA(0x0039);
		LCD_WR_DATA(0x002C);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0034);
		LCD_WR_DATA(0x0002);

	LCD_WR_REG(0x00F7);
		LCD_WR_DATA(0x0020);

	LCD_WR_REG(0x00EA);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0x00B1);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x001b);

	LCD_WR_REG(0x00B6);
		LCD_WR_DATA(0x000A);
		LCD_WR_DATA(0x00A2);

	LCD_WR_REG(0x00C0);    //Power control
		LCD_WR_DATA(0x0005);   //VRH[5:0]

	LCD_WR_REG(0x00C1);    //Power control
		LCD_WR_DATA(0x0011);   //SAP[2:0];BT[3:0]

	LCD_WR_REG(0x00C5);    //VCM control
		LCD_WR_DATA(0x0045);       //3F
		LCD_WR_DATA(0x0045);       //3C

	 LCD_WR_REG(0x00C7);    //VCM control2
		 LCD_WR_DATA(0X00a2);

	LCD_WR_REG(0x0036);    // Memory Access Control
//		LCD_WR_DATA(0x0008);//48
		LCD_WR_DATA(value);//48

	LCD_WR_REG(0x00F2);    // 3Gamma Function Disable
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0x0026);    //Gamma curve selected
		LCD_WR_DATA(0x0001);

	LCD_WR_REG(0x00E0);    //Set Gamma
		LCD_WR_DATA(0x000F);
		LCD_WR_DATA(0x0026);
		LCD_WR_DATA(0x0024);
		LCD_WR_DATA(0x000b);
		LCD_WR_DATA(0x000E);
		LCD_WR_DATA(0x0008);
		LCD_WR_DATA(0x004b);
		LCD_WR_DATA(0X00a8);
		LCD_WR_DATA(0x003b);
		LCD_WR_DATA(0x000a);
		LCD_WR_DATA(0x0014);
		LCD_WR_DATA(0x0006);
		LCD_WR_DATA(0x0010);
		LCD_WR_DATA(0x0009);
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0X00E1);    //Set Gamma
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x001c);
		LCD_WR_DATA(0x0020);
		LCD_WR_DATA(0x0004);
		LCD_WR_DATA(0x0010);
		LCD_WR_DATA(0x0008);
		LCD_WR_DATA(0x0034);
		LCD_WR_DATA(0x0047);
		LCD_WR_DATA(0x0044);
		LCD_WR_DATA(0x0005);
		LCD_WR_DATA(0x000b);
		LCD_WR_DATA(0x0009);
		LCD_WR_DATA(0x002f);
		LCD_WR_DATA(0x0036);
		LCD_WR_DATA(0x000f);

	LCD_WR_REG(0x002B);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);///
		LCD_WR_DATA(0x00ef);

	 LCD_WR_REG(0x002A);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0000);
		LCD_WR_DATA(0x0001);/////
		LCD_WR_DATA(0x003f);

	LCD_WR_REG(0x003A);
		LCD_WR_DATA(0x0055);

	LCD_WR_REG(0x00f6);
		LCD_WR_DATA(0x0001);
		LCD_WR_DATA(0x0030);
		LCD_WR_DATA(0x0000);

	LCD_WR_REG(0x0029); //display on

	LCD_WR_REG(0x002c);    // 0x2C

}

void LCD_Init2()
{
//	alt_u16 data1,data2;
//	alt_u16 data3,data4;
	Clr_LCD_RST;
	Delay_Ms(1);
	Set_LCD_RST;
	Delay_Ms(10);       // Delay 10ms // This delay time is necessary
	Clr_LCD_RST;
	Delay_Ms(120);       // Delay 120 ms
//	Clr_LCD_CS;


}


void LCD_SetCursor(alt_u16 Xpos, alt_u16 Ypos)
{
	 LCD_WR_REG(0x002A);
		 LCD_WR_DATA(Xpos>>8);
		 LCD_WR_DATA(Xpos&0XFF);
	 LCD_WR_REG(0x002B);
		 LCD_WR_DATA(Ypos>>8);
		 LCD_WR_DATA(Ypos&0XFF);
	 LCD_WR_REG(0x002C);
}

void LCD_Clear(alt_u16 Color)
{
        alt_u32 index=0;
        LCD_SetCursor(0x00,0x0000);
        LCD_WR_REG(0x002C);
        for(index=0;index<76800;index++)
        {
        	LCD_WR_DATA(Color);
            usleep(100);
        }
/*
        for(index=0;index<76800;index++)
        {
        	LCD_WR_DATA(0x1F);
        }
*/
}

alt_u16 buff[128];
int main()
{
   int idx;
   LCD_Init(0x69);
   LCD_Clear(0XF800);
   LCD_Clear(0X001F);
   while(1);
/*
   for(idx=0;idx<1024;++idx)
	   buff[idx]=idx+1;

   LCD_SET_ADDR(buff);
   LCD_SET_SIZE((38400-1));
   LCD_SET_REFRESH_RATE(50000000);

   LCD_SetCursor(0x00,0x0000);
   LCD_WR_REG(0x002C);

   LCD_WRITE(ADRR_REG_CTRL,CTRL_REG_REFRESH_ON|CTRL_REG_LCD_ON);

   for(idx=0;idx<1024;++idx)
	   buff[idx]=0xFFFF;
*/
/*
   alt_u32 index=0;
   for(index=0;index<76800/2;index++)
   {
   	LCD_WR_DATA(0x001f);
   }
*/
   while(1);
}




