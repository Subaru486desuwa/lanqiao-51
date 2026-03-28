# include "STC15F2K60S2.H"


void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
void Da_Write(unsigned char dat);
void EEPROM_Read(unsigned char*EEPROM_String,unsigned char addr,unsigned char num);
void EEPROM_Write(unsigned char*EEPROM_String,unsigned char addr,unsigned char num);