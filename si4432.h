#ifndef __SI4432_H__

//#define __SI4432_H__

#define byte unsigned char
extern int SI4432_Sel;         // currently selected SI4432
void SI4432_Write_Byte(byte ADR, byte DATA );
byte SI4432_Read_Byte( byte ADR );

void SI4432_Init(void);
float SI4432_RSSI(uint32_t i, int s);
void SI4432_Set_Frequency ( long Freq );
float SI4432_SET_RBW(float WISH);
void PE4302_Write_Byte(unsigned char DATA );
void PE4302_init(void);
#endif //__SI4432_H__
