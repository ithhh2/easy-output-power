#include "mid_oled.h"
#include "mid_oledfont.h"
#include "i2c.h"

static uint8_t OLED_GRAM[144][8] = {0};


/*
 * Function Content: oled write byte
 * Function Parameter: unsigned char dat,unsigned char cmd
 * Return Value: null
 */
void OLED_WR_Byte(unsigned char dat,unsigned char cmd)
{
	if(cmd){
		HAL_I2C_Mem_Write( &OLED_COM_PORT,OLED_ADDR,0x40,I2C_MEMADD_SIZE_8BIT, &dat,1,100);
	}
	else{
		HAL_I2C_Mem_Write( &OLED_COM_PORT,OLED_ADDR,0x00,I2C_MEMADD_SIZE_8BIT, &dat,1,100);
	}
}

/*
 * Function Content: inverse display function
 * Function Parameter: uint8_t i
 * Return Value: null
 */
void OLED_ColorTurn(uint8_t i)
{
	if(i==0)
	{
		OLED_WR_Byte(0xA6,OLED_CMD);//normal display
	}
	if(i==1)
	{
		OLED_WR_Byte(0xA7,OLED_CMD);//inverse display
	}
}

/*
 * Function Content: rotate the screen by 180 degress
 * Function Parameter: uint8_t i
 * Return Value: null
 */
void OLED_DisplayTurn(uint8_t i)
{
	if(i==0)
		{
			OLED_WR_Byte(0xC8,OLED_CMD);//normal display
			OLED_WR_Byte(0xA1,OLED_CMD);
		}
	if(i==1)
		{
			OLED_WR_Byte(0xC0,OLED_CMD);//rotate display
			OLED_WR_Byte(0xA0,OLED_CMD);
		}
}

/*
 * Function Content: open oled display
 * Function Parameter: null
 * Return Value: null
 */
void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//charge pump enable
	OLED_WR_Byte(0x14,OLED_CMD);//open charge pump
	OLED_WR_Byte(0xAF,OLED_CMD);//turn on screen
}

/*
 * Function Content: close oled display
 * Function Parameter: null
 * Return Value: null
 */
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//charge pump enable
	OLED_WR_Byte(0x10,OLED_CMD);//close charge pump
	OLED_WR_Byte(0xAE,OLED_CMD);//turn off screen
}

/*
 * Function Content: updata the graphics memory to oled
 * Function Parameter: null
 * Return Value: null
 */
void OLED_Refresh(void)
{
	uint8_t i = 0,n = 0;
	uint16_t t = 0;
	uint8_t sendData[128] = {0};
	for(i=0;i<8;i++)
	{
		OLED_WR_Byte(0xb0+i,OLED_CMD); //set the starting address of the row
		OLED_WR_Byte(0x00,OLED_CMD);   //set the low column starting address
		OLED_WR_Byte(0x10,OLED_CMD);   //set the high column starting address
		t = 0;
		for(n=0;n<128;n++)
		{
			sendData[t++] = OLED_GRAM[n][i];
		}
		HAL_I2C_Mem_Write(&OLED_COM_PORT,OLED_ADDR,0x40,I2C_MEMADD_SIZE_8BIT, &sendData[0],128,1000);
  }
}

/*
 * Function Content: clear screen function
 * Function Parameter: null
 * Return Value: null
 */
void OLED_Clear(void)
{
	uint8_t i = 0,n = 0;
	for(i=0;i<8;i++)
	{
	  for(n=0;n<128;n++)
		{
			OLED_GRAM[n][i] = 0;//clear all data
		}
  }
	OLED_Refresh();//refresh display
}

/*
 * Function Content: draw point
 * Function Parameter: x:0~127	y:0~63	t:1 fill 0 empty
 * Return Value: null
 */
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
	uint8_t i = 0,m = 0,n = 0;
	i = y / 8;
	m = y % 8;
	n = 1 << m;
	if(t){
		OLED_GRAM[x][i]|=n;
	}
	else
	{
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
		OLED_GRAM[x][i]|=n;
		OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	}
}

/*
 * Function Content: draw line
 * Function Parameter: x1,y1: starting point coordinates
 * 						x2,y2:	end point coordinates
 * 						mode:1 fill 0 empty
 * Return Value: null
 */
void OLED_DrawLine(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t mode)
{
	uint16_t t = 0;
	int xerr = 0,yerr = 0,delta_x = 0,delta_y = 0,distance = 0;
	int incx = 0,incy = 0,uRow = 0,uCol = 0;
	delta_x = x2-x1; //calculate coordinate increments
	delta_y = y2-y1;
	uRow = x1;	//row starting point coordinates
	uCol = y1;	//col starting point coordinates
	if(delta_x > 0){
		incx = 1; //set the single-step direction
	}
	else if(delta_x == 0){
		incx = 0;	//plumb line
	}
	else{
		incx = -1;
		delta_x = -delta_x;
	}
	if(delta_y > 0){
		incy = 1;
	}
	else if(delta_y == 0){
		incy = 0;	//horizontal line
	}
	else{
		incy = -1;delta_y = -delta_x;
	}
	if(delta_x > delta_y){
		distance = delta_x; //select the basic increment coordinate axis
	}
	else{
		distance = delta_y;
	}
	for(t=0;t<distance+1;t++)
	{
		OLED_DrawPoint(uRow,uCol,mode);//draw point
		xerr+=delta_x;
		yerr+=delta_y;
		if(xerr>distance)
		{
			xerr-=distance;
			uRow+=incx;
		}
		if(yerr>distance)
		{
			yerr-=distance;
			uCol+=incy;
		}
	}
}

/*
 * Function Content: draw circle
 * Function Parameter: x,y: center coordinate r: radius
 * Return Value: null
 */
void OLED_DrawCircle(uint8_t x,uint8_t y,uint8_t r)
{
	int a = 0, b = 0,num = 0;
	a = 0;
	b = r;
	while(2 * b * b >= r * r)
	{
		OLED_DrawPoint(x + a, y - b,1);
		OLED_DrawPoint(x - a, y - b,1);
		OLED_DrawPoint(x - a, y + b,1);
		OLED_DrawPoint(x + a, y + b,1);

		OLED_DrawPoint(x + b, y + a,1);
		OLED_DrawPoint(x + b, y - a,1);
		OLED_DrawPoint(x - b, y - a,1);
		OLED_DrawPoint(x - b, y + a,1);

		a++;
		num = (a * a + b * b) - r*r;//calculate the distance of the point from the center of the circle
		if(num > 0)
		{
				b--;
				a--;
		}
	}
}

/*
 * Function Content: show char
 * Function Parameter: x: 0~127, y: 0~63;
 * 						size1: 6x8/6x12/8x16/12x24;
 * 						mode: 0--inverse display,1--normal display
 * Return Value: null
 */
void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1,uint8_t mode)
{
	uint8_t i = 0,m = 0,temp = 0,size2 = 0,chr1 = 0;
	uint8_t x0 = x,y0 = y;
	if(size1 == 8){
		size2 = 6;
	}
	else{
		//Obtain the number of bytes occupied by the font
		size2 = (size1/8+((size1%8)?1:0))*(size1/2);
	}
	chr1 = chr-' ';  //Calculate the value after the offset
	for(i=0;i<size2;i++)
	{
		if(size1 == 8){
			temp = asc2_0806[chr1][i];
		} //Call the 0806 font
		else if(size1 == 12){
			temp = asc2_1206[chr1][i];
		} //Call the 1206 font
		else if(size1 == 16){
			temp = asc2_1608[chr1][i];
		} //Call the 1608 font
		else if(size1 == 24){
			temp = asc2_2412[chr1][i];
		} //Call the 2412 font
		else{
			return;
		}
		for(m=0;m<8;m++)
		{
			if(temp&0x01){
				OLED_DrawPoint(x,y,mode);
			}
			else{
				OLED_DrawPoint(x,y,!mode);
			}
			temp >>= 1;
			y++;
		}
		x++;
		if((size1!=8) && ((x-x0) == size1/2)){
			x = x0;
			y0 = y0+8;
		}
		y = y0;
  }
}

/*
 * Function Content:  show string
 * Function Parameter: 	x,y:start point coordinates
 * 						size: font size
 * 						*chr: string starting address
 * 						mode: 0--inverse display 1--normal display
 * Return Value: null
 */
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1,uint8_t mode)
{
	while((*chr >= ' ') && (*chr <= '~'))//are there any illegal characters
	{
		OLED_ShowChar(x,y,*chr,size1,mode);
		if(size1==8){
			x+=6;
		}
		else{
			x+=size1/2;
		}
		chr++;
  }
}

/*
 * Function Content:  calculate m^n
 * Function Parameter: 	null
 * Return Value: null
 */
uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
	uint32_t result = 1;
	while(n--)
	{
	  result *= m;
	}
	return result;
}

/*
 * Function Content: show num
 * Function Parameter: 	x,y:start point coordinates
 * 						num : show num
 * 						len: num of digits
 * 						size: font size
 * 						mode: 0--inverse display 1--normal display
 * Return Value: null
 */
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1,uint8_t mode)
{
	uint8_t t = 0,temp = 0,m = 0;
	if(size1 == 8){
		m=2;
	}
	for(t=0;t<len;t++)
	{
		temp=(num / OLED_Pow(10,len-t-1)) % 10;
		if(temp==0)
		{
			OLED_ShowChar(x+(size1/2+m)*t,y,'0',size1,mode);
		}
		else
		{
			OLED_ShowChar(x+(size1/2+m)*t,y,temp+'0',size1,mode);
		}
  }
}

/*
 * Function Content: oled display init
 * Function Parameter: null
 * Return Value: null
 */
void OLED_Init(void)
{
	HAL_Delay(200);
	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//
	OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	OLED_Clear();
	OLED_WR_Byte(0xAF,OLED_CMD);
}
