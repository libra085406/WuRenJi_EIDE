#include "Int_SI24R1.h"

uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x0A, 0x01, 0x07, 0x0E, 0x01}; // 定义一个静态发送地址

static uint8_t SPI_RW(uint8_t byte)
{
	uint8_t rx_byt;
	HAL_SPI_TransmitReceive(&hspi1, &byte, &rx_byt, 1, 1000);
	return rx_byt;
}

/********************************************************
函数功能：SI24R1引脚初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_SI24R1_Init(uint8_t mode)
{
	// 等待SI24R1上电完成
	while (1)
	{
		Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, MY_RF_CHANNEL);
		uint8_t res = Int_SI24R1_Read_Reg(RF_CH);
		if (res == MY_RF_CHANNEL)
		{
			break;
		}
	}
	if (mode == RX_MODE)
	{
		Int_SI24R1_RX_Mode(); // 初始化SI24R1为接收模式
		// 打印状态寄存器和FIFO寄存器的值
		uint8_t status = Int_SI24R1_Read_Reg(STATUS);
		uint8_t fifo = Int_SI24R1_Read_Reg(FIFO_STATUS);
		debug_printf("status:0x%02x,fifo:0x%02x\r\n", status, fifo);
	}
	else if (mode == TX_MODE)
	{
		Int_SI24R1_TX_Mode(); // 初始化SI24R1为发送模式
	}
}

/********************************************************
函数功能：写寄存器的值（单字节）
入口参数：reg:寄存器映射地址（格式：SI24R1_WRITE_REG｜reg）
					value:寄存器的值
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_SI24R1_Write_Reg(uint8_t reg, uint8_t value)
{
	SI24R1_NSS_LOW; // 拉低NSS引脚，片选SI24R1芯片
	uint8_t status = SPI_RW(reg);
	SPI_RW(value);
	SI24R1_NSS_HIGH; // 拉高NSS引脚，取消片选SI24R1芯片
	return (status);
}

/********************************************************
函数功能：写寄存器的值（多字节）
入口参数：reg:寄存器映射地址（格式：SI24R1_WRITE_REG｜reg）
					pBuf:写数据首地址
					bytes:写数据字节数
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_SI24R1_Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status, byte_ctr;
	SI24R1_NSS_LOW;
	status = SPI_RW(reg);
	for (byte_ctr = 0; byte_ctr < bytes; byte_ctr++)
	{
		SPI_RW(pBuf[byte_ctr]);
	}
	SI24R1_NSS_HIGH;
	return (status);
}

/********************************************************
函数功能：读取寄存器的值（单字节）
入口参数：reg:寄存器映射地址（格式：SI24R1_READ_REG｜reg）
返回  值：寄存器值
*********************************************************/
uint8_t Int_SI24R1_Read_Reg(uint8_t reg)
{
	SI24R1_NSS_LOW;
	SPI_RW(reg);
	uint8_t value = SPI_RW(0);
	SI24R1_NSS_HIGH;
	return (value);
}

/********************************************************
函数功能：读取寄存器的值（多字节）
入口参数：reg:寄存器映射地址（SI24R1_READ_REG｜reg）
					pBuf:接收缓冲区的首地址
					bytes:读取字节数
返回  值：状态寄存器的值
*********************************************************/
uint8_t Int_SI24R1_Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status, byte_ctr;
	SI24R1_NSS_LOW;
	status = SPI_RW(reg);
	for (byte_ctr = 0; byte_ctr < bytes; byte_ctr++)
	{
		pBuf[byte_ctr] = SPI_RW(0); // 读取数据，低字节在前
	}
	SI24R1_NSS_HIGH;
	return (status);
}

/********************************************************
函数功能：SI24R1接收模式初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_SI24R1_RX_Mode(void)
{
	SI24R1_CE_LOW;
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 接收设备接收通道0使用和发送设备相同的发送地址
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);						   // 使能接收通道0自动应答
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);					   // 使能接收通道0
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, MY_RF_CHANNEL);				   // 选择射频通道(信道)40
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RX_PW_P0, TX_PLOAD_WIDTH);			   // 接收通道0选择和发送通道相同有效数据宽度
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x0f);					   // 数据传输率2Mbps，发射功率7dBm
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0f);						   // CRC使能，16位CRC校验，上电，接收模式
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, 0xff);						   // 清除所有的中断标志位
	Int_SI24R1_Write_Reg(FLUSH_RX, 0xff);										   // 清除RX FIFO寄存器
	SI24R1_CE_HIGH;																   // 拉高CE启动接收设备
}

/********************************************************
函数功能：SI24R1发送模式初始化
入口参数：无
返回  值：无
*********************************************************/
void Int_SI24R1_TX_Mode(void)
{
	SI24R1_CE_LOW;
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);	   // 写入发送地址
	Int_SI24R1_Write_Buf(SI24R1_WRITE_REG + RX_ADDR_P0, TX_ADDRESS, TX_ADR_WIDTH); // 为了应答接收设备，接收通道0地址和发送地址相同

	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_AA, 0x01);		   // 使能接收通道0自动应答
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + EN_RXADDR, 0x01);	   // 使能接收通道0
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + SETUP_RETR, 0x0a);	   // 自动重发延时等待250us+86us，自动重发10次
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_CH, MY_RF_CHANNEL); // 选择射频通道(信道)40
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + RF_SETUP, 0x0f);	   // 数据传输率2Mbps，发射功率7dBm
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + CONFIG, 0x0e);		   // CRC使能，16位CRC校验，上电
																   // SI24R1_CE_HIGH;
}

/********************************************************
函数功能：读取接收数据
入口参数：rxbuf:接收数据存放首地址
返回  值：0:接收到数据
		  1:没有接收到数据
*********************************************************/
uint8_t Int_SI24R1_RxPacket(uint8_t *rxbuf)
{
	uint8_t state;
	state = Int_SI24R1_Read_Reg(STATUS);					// 读取状态寄存器的值
	Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state); // 清除RX_DR中断标志

	if (state & RX_DR) // 接收到数据
	{
		Int_SI24R1_Read_Buf(RD_RX_PLOAD, rxbuf, TX_PLOAD_WIDTH); // 读取数据
		Int_SI24R1_Write_Reg(FLUSH_RX, 0xff);					 // 清除RX FIFO寄存器
		return 0;
	}
	return 1; // 没收到任何数据
}

/********************************************************
函数功能：发送一个数据包
入口参数：txbuf:要发送的数据
返回  值：0x10:达到最大重发次数，发送失败
		  0x20:发送成功
		  0xff:发送失败
*********************************************************/
uint8_t Int_SI24R1_TxPacket(uint8_t *txbuf)
{
	uint8_t state;
	SI24R1_CE_LOW;											  // CE拉低，使能SI24R1配置
	Int_SI24R1_Write_Buf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH); // 写数据到TX FIFO,32个字节
	SI24R1_CE_HIGH;											  // CE置高，使能发送

	// 等待发送完成
	while (1)
	{
		state = Int_SI24R1_Read_Reg(STATUS);					// 读取状态寄存器的值
		Int_SI24R1_Write_Reg(SI24R1_WRITE_REG + STATUS, state); // 清除TX_DS或MAX_RT中断标志
		if (state & MAX_RT)										// 达到最大重发次数
		{
			Int_SI24R1_Write_Reg(FLUSH_TX, 0xff); // 清除TX FIFO寄存器
			return MAX_RT;
		}
		if (state & TX_DS) // 发送完成
		{
			return TX_DS;
		}
	}
}
