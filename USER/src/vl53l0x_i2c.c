#include "vl53l0x_i2c.h"
#include "vl53l0x_port_pin.h"

void VL_SDA_IN(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = VL53L0X_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(VL53L0X_SDA_PORT, &GPIO_InitStruct);
}

void VL_SDA_OUT(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = VL53L0X_SDA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(VL53L0X_SDA_PORT, &GPIO_InitStruct);
}

void VL_IIC_SCL(uint8_t Data) {
	if (Data == 1)
	{
		HAL_GPIO_WritePin(VL53L0X_SCL_PORT, VL53L0X_SCL_PIN, GPIO_PIN_SET);//PA11输出高	
	}
	else
	{
		HAL_GPIO_WritePin(VL53L0X_SCL_PORT, VL53L0X_SCL_PIN, GPIO_PIN_RESET);//PA11输出低	
	}
}

void VL_IIC_SDA(uint8_t Data) {
	if (Data == 1)
	{
		HAL_GPIO_WritePin(VL53L0X_SDA_PORT, VL53L0X_SDA_PIN, GPIO_PIN_SET);//PA12输出高	
	}
	else
	{
		HAL_GPIO_WritePin(VL53L0X_SDA_PORT, VL53L0X_SDA_PIN, GPIO_PIN_RESET);//PA12输出低	
	}
}

uint8_t VL_READ_SDA(void) {

	if (HAL_GPIO_ReadPin(VL53L0X_SDA_PORT, VL53L0X_SDA_PIN) == GPIO_PIN_SET)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

//us级等待
void delay_us(u32 nus) {
	int j;
	for (j = 0; j < nus; j++)
	{
		for (int i = 0; i < 20; i++)
		{
			__asm("NOP");
		}
	}
}

//VL53L0X I2C初始化
void VL53L0X_i2c_init(void) {
	GPIO_InitTypeDef GPIO_Initure;
	
#if CLK_GPIO == AAA
	__HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
#endif
#if CLK_GPIO == BBB
	__HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟
#endif
#if CLK_GPIO == CCC
	__HAL_RCC_GPIOC_CLK_ENABLE();           //开启GPIOC时钟
#endif
#if CLK_GPIO == DDD
	__HAL_RCC_GPIOD_CLK_ENABLE();           //开启GPIOD时钟
#endif
#if CLK_GPIO == EEE
	__HAL_RCC_GPIOE_CLK_ENABLE();           //开启GPIOE时钟
#endif
#if CLK_GPIO == FFF
	__HAL_RCC_GPIOF_CLK_ENABLE();           //开启GPIOF时钟
#endif
#if CLK_GPIO == GGG
	__HAL_RCC_GPIOG_CLK_ENABLE();           //开启GPIOG时钟
#endif

	GPIO_Initure.Pin = VL53L0X_SCL_PIN | VL53L0X_SDA_PIN; //PA11、PA12
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull = GPIO_PULLUP;          //上拉
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;    	//高速
	HAL_GPIO_WritePin(VL53L0X_SCL_PORT, VL53L0X_SCL_PIN, GPIO_PIN_SET);//输出高电平
	HAL_GPIO_Init(VL53L0X_SCL_PORT, &GPIO_Initure);


	GPIO_Initure.Pin = VL53L0X_XshutPin;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;  //推挽输出
	GPIO_Initure.Pull = GPIO_PULLUP;
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;    	//高速
	HAL_GPIO_WritePin(VL53L0X_XshutPort, VL53L0X_XshutPin, GPIO_PIN_SET);//输出高	
	HAL_GPIO_Init(VL53L0X_XshutPort, &GPIO_Initure);

}

//产生IIC起始信号
void VL_IIC_Start(void) {
	VL_SDA_OUT();//sda线输出
	VL_IIC_SDA(1);
	VL_IIC_SCL(1);
	delay_us(4);
	VL_IIC_SDA(0);//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	VL_IIC_SCL(0);//钳住I2C总线，准备发送或接收数据 
}

//产生IIC停止信号
void VL_IIC_Stop(void) {
	VL_SDA_OUT();//sda线输出
	VL_IIC_SCL(0);
	VL_IIC_SDA(0);//STOP:when CLK is high DATA change form low to high
	delay_us(4);
	VL_IIC_SCL(1);
	VL_IIC_SDA(1);//发送I2C总线结束信号
	delay_us(4);
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 VL_IIC_Wait_Ack(void) {
	u8 ucErrTime = 0;
	VL_SDA_IN();  //SDA设置为输入  
	VL_IIC_SDA(1); delay_us(1);
	VL_IIC_SCL(1); delay_us(1);
	while (VL_READ_SDA())
	{
		ucErrTime++;
		if (ucErrTime > 250)
		{
			VL_IIC_Stop();
			return 1;
		}
	}
	VL_IIC_SCL(0);//时钟输出0 	   
	return 0;
}

//产生ACK应答
void VL_IIC_Ack(void) {
	VL_IIC_SCL(0);
	VL_SDA_OUT();
	VL_IIC_SDA(0);
	delay_us(2);
	VL_IIC_SCL(1);
	delay_us(2);
	VL_IIC_SCL(0);
}

//不产生ACK应答		    
void VL_IIC_NAck(void) {
	VL_IIC_SCL(0);
	VL_SDA_OUT();
	VL_IIC_SDA(1);
	delay_us(2);
	VL_IIC_SCL(1);
	delay_us(2);
	VL_IIC_SCL(0);
}

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void VL_IIC_Send_Byte(u8 txd) {
	u8 t;
	VL_SDA_OUT();
	VL_IIC_SCL(0);//拉低时钟开始数据传输
	for (t = 0; t < 8; t++)
	{
		if ((txd & 0x80) >> 7)
			VL_IIC_SDA(1);
		else
			VL_IIC_SDA(0);
		txd <<= 1;
		delay_us(2);
		VL_IIC_SCL(1);
		delay_us(2);
		VL_IIC_SCL(0);
		delay_us(2);
	}
}

//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 VL_IIC_Read_Byte(unsigned char ack)
{
	unsigned char i, receive = 0;
	VL_SDA_IN();//SDA设置为输入
	for (i = 0; i < 8; i++)
	{
		VL_IIC_SCL(0);
		delay_us(4);
		VL_IIC_SCL(1);
		receive <<= 1;
		if (VL_READ_SDA())receive++;
		delay_us(4); //1
	}
	if (!ack)
		VL_IIC_NAck();//发送nACK
	else
		VL_IIC_Ack(); //发送ACK   
	return receive;
}

//IIC写一个字节数据
u8 VL_IIC_Write_1Byte(u8 SlaveAddress, u8 REG_Address, u8 REG_data)
{
	VL_IIC_Start();
	VL_IIC_Send_Byte(SlaveAddress);
	if (VL_IIC_Wait_Ack())
	{
		VL_IIC_Stop();//释放总线
		return 1;//没应答则退出

	}
	VL_IIC_Send_Byte(REG_Address);
	VL_IIC_Wait_Ack();
	VL_IIC_Send_Byte(REG_data);
	VL_IIC_Wait_Ack();
	VL_IIC_Stop();

	return 0;
}

//IIC读一个字节数据
u8 VL_IIC_Read_1Byte(u8 SlaveAddress, u8 REG_Address, u8 *REG_data)
{
	VL_IIC_Start();
	VL_IIC_Send_Byte(SlaveAddress);//发写命令
	if (VL_IIC_Wait_Ack())
	{
		VL_IIC_Stop();//释放总线
		return 1;//没应答则退出
	}
	VL_IIC_Send_Byte(REG_Address);
	VL_IIC_Wait_Ack();
	VL_IIC_Start();
	VL_IIC_Send_Byte(SlaveAddress | 0x01);//发读命令
	VL_IIC_Wait_Ack();
	*REG_data = VL_IIC_Read_Byte(0);
	VL_IIC_Stop();

	return 0;
}

//IIC写n字节数据
u8 VL_IIC_Write_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf)
{
	VL_IIC_Start();
	VL_IIC_Send_Byte(SlaveAddress);//发写命令
	if (VL_IIC_Wait_Ack())
	{
		VL_IIC_Stop();//释放总线
		return 1;//没应答则退出
	}
	VL_IIC_Send_Byte(REG_Address);
	VL_IIC_Wait_Ack();
	while (len--)
	{
		VL_IIC_Send_Byte(*buf++);//发送buff的数据
		VL_IIC_Wait_Ack();
	}
	VL_IIC_Stop();//释放总线

	return 0;

}

//IIC读n字节数据
u8 VL_IIC_Read_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf)
{
	VL_IIC_Start();
	VL_IIC_Send_Byte(SlaveAddress);//发写命令
	if (VL_IIC_Wait_Ack())
	{
		VL_IIC_Stop();//释放总线
		return 1;//没应答则退出
	}
	VL_IIC_Send_Byte(REG_Address);
	VL_IIC_Wait_Ack();

	VL_IIC_Start();
	VL_IIC_Send_Byte(SlaveAddress | 0x01);//发读命令
	VL_IIC_Wait_Ack();
	while (len)
	{
		if (len == 1)
		{
			*buf = VL_IIC_Read_Byte(0);
		}
		else
		{
			*buf = VL_IIC_Read_Byte(1);
		}
		buf++;
		len--;
	}
	VL_IIC_Stop();//释放总线

	return 0;

}

//VL53L0X 写多个数据
//address:地址
//index:偏移地址
//pdata:数据指针
//count:长度 最大65535
u8 VL53L0X_write_multi(u8 address, u8 index, u8 *pdata, u16 count)
{
	u8 status = STATUS_OK;

	if (VL_IIC_Write_nByte(address, index, count, pdata))
	{
		status = STATUS_FAIL;

	}

	return status;
}


//VL53L0X 读多个数据
//address:地址
//index:偏移地址
//pdata:数据指针
//count:长度 最大65535
u8 VL53L0X_read_multi(u8 address, u8 index, u8 *pdata, u16 count)
{
	u8 status = STATUS_OK;

	if (VL_IIC_Read_nByte(address, index, count, pdata))
	{
		status = STATUS_FAIL;
	}

	return status;
}

//VL53L0X 写1个数据(单字节)
//address:地址
//index:偏移地址
//data:数据(8位)
u8 VL53L0X_write_byte(u8 address, u8 index, u8 data)
{
	u8 status = STATUS_OK;

	status = VL53L0X_write_multi(address, index, &data, 1);

	return status;
}

//VL53L0X 写1个数据(双字节)
//address:地址
//index:偏移地址
//data:数据(16位)
u8 VL53L0X_write_word(u8 address, u8 index, u16 data)
{
	u8 status = STATUS_OK;

	u8 buffer[2];

	//将16位数据拆分成8位
	buffer[0] = (u8)(data >> 8);//高八位
	buffer[1] = (u8)(data & 0xff);//低八位

	if (index % 2 == 1)
	{
		//串行通信不能处理对非2字节对齐寄存器的字节
		status = VL53L0X_write_multi(address, index, &buffer[0], 1);
		status = VL53L0X_write_multi(address, index, &buffer[0], 1);
	}
	else
	{
		status = VL53L0X_write_multi(address, index, buffer, 2);
	}

	return status;
}

//VL53L0X 写1个数据(四字节)
//address:地址
//index:偏移地址
//data:数据(32位)
u8 VL53L0X_write_dword(u8 address, u8 index, u32 data)
{

	u8 status = STATUS_OK;

	u8 buffer[4];

	//将32位数据拆分成8位
	buffer[0] = (u8)(data >> 24);
	buffer[1] = (u8)((data & 0xff0000) >> 16);
	buffer[2] = (u8)((data & 0xff00) >> 8);
	buffer[3] = (u8)(data & 0xff);

	status = VL53L0X_write_multi(address, index, buffer, 4);

	return status;

}


//VL53L0X 读1个数据(单字节)
//address:地址
//index:偏移地址
//data:数据(8位)
u8 VL53L0X_read_byte(u8 address, u8 index, u8 *pdata)
{
	u8 status = STATUS_OK;

	status = VL53L0X_read_multi(address, index, pdata, 1);

	return status;

}

//VL53L0X 读个数据(双字节)
//address:地址
//index:偏移地址
//data:数据(16位)
u8 VL53L0X_read_word(u8 address, u8 index, u16 *pdata)
{
	u8 status = STATUS_OK;

	u8 buffer[2];

	status = VL53L0X_read_multi(address, index, buffer, 2);

	*pdata = ((u16)buffer[0] << 8) + (u16)buffer[1];

	return status;

}

//VL53L0X 读1个数据(四字节)
//address:地址
//index:偏移地址
//data:数据(32位)
u8 VL53L0X_read_dword(u8 address, u8 index, u32 *pdata)
{
	u8 status = STATUS_OK;

	u8 buffer[4];

	status = VL53L0X_read_multi(address, index, buffer, 4);

	*pdata = ((u32)buffer[0] << 24) + ((u32)buffer[1] << 16) + ((u32)buffer[2] << 8) + ((u32)buffer[3]);

	return status;

}
