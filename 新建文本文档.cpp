#include<reg51.h>
#include<intrins.h>   
#define uchar unsigned char 
#define uint unsigned int
xdata uchar dled _at_ 0x8004;//段码
xdata uchar wled _at_ 0x8002;//位码
sbit DQ=P1^6;	  //温度输入口
sbit beep=P2^2;   //蜂鸣器
int temp1=0;	  
uint h;
uint temp;
uchar r;
uchar code ditab[16]={0x00,0x01,0x01,0x02,0x03,0x03,0x04,0x04,
                        0x05,0x06,0x06,0x07,0x08,0x08,0x09,0x09};	  //小数断码表
uchar code table_dm[12]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,
                            0x7d,0x07,0x7f,0x6f,0x00,0x40};	  //共阴LED段码表"0""1""2""3""4""5""6""7""8""9""不亮""-"
uchar code table_dm1[]={0xbf,0x86,0xdb,0xcf,0xe6,0xed,0xfd,0x87,0xff,0xef};	//个位带小数点的断码表
uchar data temp_data[2]={0x00,0x00};//读出温度暂放
uchar data display[5]={0x00,0x00,0x00,0x00,0x00};//显示单元数据，共四个数据和一个运算暂用
/***************11us延时函数***************/
void delay(uint t)
{
	for(;t>0;t--);
}
/**************数码管显示**************/
void xianshi()
{
	int j;
	for(j=0;j<4;j++)
	{
		switch(j)
		{
			case 0:
			dled=table_dm[display[0]];
			wled=0x01;
			delay(300);
			wled=0x00;break;						//小数位
			case 1:
			dled=table_dm1[display[1]];
			wled=0x02;
			delay(300);
			wled=0x00;break;						//个位
			case 2:
			dled=table_dm[display[2]];
			wled=0x04;
			delay(300);
			wled=0x00;break;					    //十位
			case 3:
			dled=table_dm[display[3]];
			wled=0x08;
			delay(300);
			wled=0x00;break;						//百位
		}
	}
}
/**************DS18B20初始化**************/
ow_reset(void)
{
	char presence=1;
	while(presence)
	{
		while(presence)
		{
			DQ=1;_nop_();_nop_();
			DQ=0;
			delay(50);
			DQ=1;
			delay(6);
			presence=DQ;
		}
		delay(45);
		presence=~DQ;
	}
	DQ=1;	
	return presence;
}
/****************写命令函数*****************/
void write_byte(uchar val)
{
	uchar i;
	for(i=8;i>0;i--)
	{
		DQ=1;_nop_();_nop_();
		DQ=0;_nop_();_nop_();_nop_();_nop_();
		DQ=val&0x01;
		delay(6);
		val=val>>1;		
	}
	DQ=1;
	delay(1);
}	
/****************读一个字节函数*******************/
uchar read_byte(void)
{
	uchar i;
	uchar value=0;
	for(i=8;i>0;i--)
	{
		DQ=1;_nop_();_nop_();
		value>>=1;
		DQ=0;_nop_();_nop_();_nop_();_nop_();
		DQ=1;_nop_();_nop_();_nop_();_nop_();
		if(DQ)value|=0x80;
		delay(6);
	}
	DQ=1;	
	return value; 
}
/***************读取温度函数***************/
read_temp()
{
	ow_reset();
	delay(200);
	write_byte(0xcc);
	write_byte(0x44);
	ow_reset();
	delay(1);
	write_byte(0xcc);
	write_byte(0xbe);
	temp_data[0]=read_byte();
	temp_data[1]=read_byte();
	temp=temp_data[1];
	temp<<=8;
	temp=temp|temp_data[0];
	return temp;
}
/***************温度数据处理函数***************/
work_temp(uint tem)
{
	uchar n=0;
	if(tem>6348)
	{
		tem=65536-tem;
		n=1;
	}
	display[4]=tem&0x0f;
	display[0]=ditab[display[4]];
	display[4]=tem>>4;
	display[3]=display[4]/100;
	display[1]=display[4]%100;			    
	display[2]=display[1]/10;
	display[1]=display[1]%10;
	r=display[1]+display[2]*10+display[3]*100;
	if(!display[3])
	{
		display[3]=0x0a;
		if(!display[2])	
		{
			display[2]=0x0a;
		}
	}
	if(n)
	{
		display[3]=0x0b;
	}
	return n;
}

/***************报警提示电路***************/
void BEEP()
{
	if((r>20))
	{
		beep=!beep;
	}
		else
	{
		beep=0;
	}}
/**************主函数*************/
void main()
{
	beep=0;
	for(h=0;h<4;h++)//初始化，显示000.0
	{
		display[h]=0;
	}
	ow_reset();
	write_byte(0xcc);
	write_byte(0x44);
	for(h=0;h<5;h++)
	{
		xianshi();
	}
	while(1)
	{
		if(temp1==0)
		{
		work_temp(read_temp());//转换读取温度值
		xianshi(); //数码管显示
		BEEP();//超过27度报警
		 		}
	}
}