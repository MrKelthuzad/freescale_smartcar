#ifndef __CAMERA_H__
#define __CAMERA_H__

#include "isr.h"
#include "oled.h"
#include "sccbext.h"
#include "chlib_k.h"

#define CAMERA_ROW    50                           //����ͷ�ɼ�����
#define CAMERA_COL    152                          //����ͷ�ɼ�����
#define CAMERA_CENTER 80                           //����ͷ�ɼ�����ֵ

#define P_WIDTH       8                            //lp1����lp2ָ���֮����
#define BW_DELTA      50                           //
#define THRESHOLD     110                          //����ͷ��ֵ
#define LINE_EDGE     2                            //
#define BLOCK_LEN     20                           //

extern uint8_t  image;  
extern uint16_t H_Cnt;                             //��¼���ж���
extern uint32_t V_Cnt;                             //��¼���жϴ���

extern uint8_t  image;                       //�궨��ż��
//imageΪ0, ��ʾ���ڴ���img1, DMA���մ洢��img2
//imageΪ1, ��ʾ���ڴ���img2, DMA���մ洢��img1
extern uint8_t  img1[CAMERA_ROW][CAMERA_COL];//�������洢λ��
extern uint8_t  img2[CAMERA_ROW][CAMERA_COL];//ż�����洢λ��
extern uint8_t *imgaddr;                     //��ǰ��������ʼ��ַ

extern uint8_t  l_line_index[CAMERA_ROW];    //���������к�
extern uint8_t  r_line_index[CAMERA_ROW];    //���������к�

extern const uint8_t offset[];                     //ÿһ�е�lp1,lp2ɨ��ƫ����

void CAMERA_Init(void);                            //ͼ���ʼ��
void CAMERA_Processing(void);                      //ͼ��ʶ������
void CAMERA_Display_Full(void);                    //��ʾ����ͼ��OLED
void CAMERA_Display_Edge(void);                    //��ʾ�߽�ͼ��OLED
void CAMERA_UART_TX_Full(const uint32_t instance); //����ͼ��Ҷȴ��ڷ�s��
void CAMERA_UART_TX_Edge(const uint32_t instance); //�߽�ͼ��Ҷȴ��ڷ���

#endif
