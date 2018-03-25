/*********************************************************
  @2017CameraDemo
  @�̼��⣺����V2.4
  @author��wgq & lq
  @2017.11.27
  @for seu2017 ����ͷ��
*********************************************************/

#include "isr.h"

//�жϷ���������Ҫ�ɼ�������(�У���)�������޸�
//�����ں����м���led�Ŀ���������Ƿ�����ж�
//���������жϷ�������ִ����ʱ�����ݴ���/�����շ�
void CAMERA_Interrupt_Handler(uint32_t mask)
{
    if(mask & (1 << 7)) //���ж�
    {
        if(H_Cnt % 2 && H_Cnt < 100)
            DMA_EnableRequest(HW_DMA_CH0); //ʹ��DMAͨ��0
        H_Cnt++; 
    }

    if(mask & (1 << 6)) //���ж�
    {
        H_Cnt = 0;
        //20��֮��ʼ�ɼ�
        if(V_Cnt < 20)
            V_Cnt++;
        else
        {
            image = 1 - image;  //��ż���л�
            PEout(25) = image;  //PORT_E25��ʾ��ǰ������������ż����
            DMA_SetDestAddress(HW_DMA_CH0, image ? (uint32_t) img1[0] : (uint32_t) img2[0]); //����DMAͨ����ַ
            imgaddr = image ? img2[0] : img1[0]; //imgaddr��ȡ��ǰû�б�DMAռ�õ�ͨ�����д���
        }
    }
}
