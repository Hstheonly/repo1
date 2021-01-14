#ifndef __MB_CRC_H
#define __MB_CRC_H	

#define CRC_OK 1
#define CRC_ERROR 0	

u16 MB_CRC16( u8 * pucFrame, u8 usLen );
u8 CRC_TEST(u8 *buf,u8 len);
void ADD_CRC(u8 *buf,u8 len);
void IEEE754_F_H(float f,u8* buf);
void IEEE754_H_F(u8* buf,float* f);

#endif
