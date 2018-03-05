#include "camera.h"

uint16_t H_Cnt=0;                     //��¼���ж���
uint32_t V_Cnt=0;                     //��¼���жϴ���

uint8_t  image = 0;                   //�궨��ż��
//imageΪ0, ��ʾ���ڴ���img1, DMA���մ洢��img2
//imageΪ1, ��ʾ���ڴ���img2, DMA���մ洢��img1
uint8_t  img1[CAMERA_ROW][CAMERA_COL];//�������洢λ��
uint8_t  img2[CAMERA_ROW][CAMERA_COL];//ż�����洢λ��
uint8_t *imgaddr;                     //��ǰ��������ʼ��ַ

uint8_t  l_line_index[CAMERA_ROW];    //���������к�
uint8_t  r_line_index[CAMERA_ROW];    //���������к�

//ÿһ�е�ɨ��ƫ����
const uint8_t offset[CAMERA_ROW]= {              
  40,    40,    40,    40,    40,    40,    40,    40,    40,    40,
  40,    40,    40,    40,    40,    38,    38,    38,    38,    38,
  35,    35,    35,    35,    35,    29,    28,    27,    25,    25,
  24,    24,    24,    24,    23,    23,    23,    23,    22,    22,
  22,    22,    21,    21,    21,    21,    21,    20,    20,    20,
};

void CAMERA_Init(void)
{ 
    // ����ͷ״̬�Ƴ�ʼ��
    GPIO_QuickInit(HW_GPIOE, 25, kGPIO_Mode_OPP);   
    // LED1��ʼ����PORTE_25��û��Ϊ�����������У���Ϊż����������
    GPIO_QuickInit(HW_GPIOC, 18, kGPIO_Mode_OPP);   
    // LED2��ʼ����PORTC_18������ʾ���ڴ���ǰͼ�񣬲�����ʾû�д���ͼ��

    //����ͷ���ݿڣ�����
    GPIO_QuickInit(HW_GPIOC, 8,  kGPIO_Mode_IPD); //��ʼ��PORT_C,  8�˿�
    GPIO_QuickInit(HW_GPIOC, 9,  kGPIO_Mode_IPD); //��ʼ��PORT_C,  9�˿�
    GPIO_QuickInit(HW_GPIOC, 10, kGPIO_Mode_IPD); //��ʼ��PORT_C, 10�˿�
    GPIO_QuickInit(HW_GPIOC, 11, kGPIO_Mode_IPD); //��ʼ��PORT_C, 11�˿�
    GPIO_QuickInit(HW_GPIOC, 12, kGPIO_Mode_IPD); //��ʼ��PORT_C, 12�˿�
    GPIO_QuickInit(HW_GPIOC, 13, kGPIO_Mode_IPD); //��ʼ��PORT_C, 13�˿�
    GPIO_QuickInit(HW_GPIOC, 14, kGPIO_Mode_IPD); //��ʼ��PORT_C, 14�˿�
    GPIO_QuickInit(HW_GPIOC, 15, kGPIO_Mode_IPD); //��ʼ��PORT_C, 15�˿�
    
    //���ж�
    GPIO_QuickInit(HW_GPIOC, 6, kGPIO_Mode_IPD);  //��ʼ��PORT_C,  6�˿�
    GPIO_CallbackInstall(HW_GPIOC, CAMERA_Interrupt_Handler);     //ע���жϺ���,  PORT_C�鴥��
    GPIO_ITDMAConfig(HW_GPIOC, 6, kGPIO_IT_RisingEdge,  false); //�����ش����ж�, ��ʱ�ر��ж�
    
    //���ж�
    GPIO_QuickInit(HW_GPIOC, 7, kGPIO_Mode_IPU);  //��ʼ��PORT_C,  7�˿�
    GPIO_CallbackInstall(HW_GPIOC, CAMERA_Interrupt_Handler);     //ע���жϺ���,  PORT_C�鴥��
    GPIO_ITDMAConfig(HW_GPIOC, 7, kGPIO_IT_RisingEdge,  false); //�����ش����ж�, ��ʱ�ر��ж�
    
    //���ص��ж�
    GPIO_QuickInit(HW_GPIOC, 2, kGPIO_Mode_IPU);
    GPIO_ITDMAConfig(HW_GPIOC, 2, kGPIO_DMA_RisingEdge, false); //�����ش���DMA,  ��ʱ�ر��ж�
    
    //DMA��ʼ��
    DMA_InitTypeDef DMA_InitStruct1   = {0};
    DMA_InitStruct1.chl               = HW_DMA_CH0;                   //DMAͨ����: 0
    DMA_InitStruct1.chlTriggerSource  = PORTC_DMAREQ;                 //DMA����Դѡ��: GPIO_PORTC
    DMA_InitStruct1.triggerSourceMode = kDMA_TriggerSource_Normal;    //DMA������ʽ: ��������
    DMA_InitStruct1.minorLoopByteCnt  = 1;                            //DMA�����ֽ���: 1�ֽ�
    DMA_InitStruct1.majorLoopCnt      = CAMERA_COL;                      //DMAѭ���ɼ���: 1������(152��)

    DMA_InitStruct1.sAddr             = (uint32_t) &(PTC->PDIR) + 1;  //DMA��Դ��ַ: PTC8~15(PORT_C��ʼ��ַƫ��һ���ֽ�)
    DMA_InitStruct1.sLastAddrAdj      = 0;                            //DMA��Դ��ַѭ����ƫ��: ÿ��ѭ�����ƫ����
    DMA_InitStruct1.sAddrOffset       = 0;                            //DMA��Դ��ַѭ����ƫ��: ÿ�����ں��ƫ����
    DMA_InitStruct1.sDataWidth        = kDMA_DataWidthBit_8;          //DMA��Դ���ݿ��: 8λ
    DMA_InitStruct1.sMod              = kDMA_ModuloDisable;           //DMA��Դ�ڴ�ռ�ģ��: ������Դ��ַ��ʼ���ڴ�ռ�ѭ����С��Χ, ���ڷ�ֹԽ��, ������            

    //DMA_InitStruct1.dAddr = (uint32_t)DestBuffer;                   //DMAĿ�ĵ�ַ: ��ʱ������, �ɳ��жϴ���������
    DMA_InitStruct1.dLastAddrAdj      = 0;                            //DMAĿ�ĵ�ַѭ����ƫ��: ÿ��ѭ�����ƫ����
    DMA_InitStruct1.dAddrOffset       = 1;                            //DMAĿ�ĵ�ַѭ����ƫ��: ÿ�����ں��ƫ����
    DMA_InitStruct1.dDataWidth        = kDMA_DataWidthBit_8;          //DMAĿ�����ݿ��: 8λ
    DMA_InitStruct1.dMod              = kDMA_ModuloDisable;           //DMAĿ���ڴ�ռ�ģ��: ����Ŀ�ĵ�ַ��ʼ���ڴ�ռ�ѭ����С��Χ, ���ڷ�ֹԽ��, ������
    
    DMA_Init(&DMA_InitStruct1);                                       //Ӧ��DMAͨ������
    DMA_DisableRequest(HW_DMA_CH0);                                   //�ȹر�DMA����, ���жϴ������д�����
    
    GPIO_ITDMAConfig(HW_GPIOC, 6, kGPIO_IT_RisingEdge,  true);        //�����ش����ж�, �����ж�
    GPIO_ITDMAConfig(HW_GPIOC, 7, kGPIO_IT_RisingEdge,  true);        //�����ش����ж�, �����ж�
    GPIO_ITDMAConfig(HW_GPIOC, 2, kGPIO_DMA_RisingEdge, true);        //�����ش���DMA, �����ж�

    //��������ͷ�Ĵ���
    uint8_t i = 0;
    GPIO_QuickInit(HW_GPIOC, 0, kGPIO_Mode_OPP);
    GPIO_QuickInit(HW_GPIOC, 3, kGPIO_Mode_OPP);
    while(i == 0)
      i += LPLD_SCCB_WriteReg(0x14, 0x24); //QVGA(320*120)
    while(i == 1)
      i += LPLD_SCCB_WriteReg(0x24, 0x20); //�����ɼ�ģʽ(320*240)
    while(i == 2)
      i += LPLD_SCCB_WriteReg(0x70, 0xc1); //������������һ��
    while(i == 3)
      i += LPLD_SCCB_WriteReg(0x06, 0xa0); //���ȿ���
}

void CAMERA_Processing(void)
{
    int16_t l1, l2;          //���������
    int16_t r1, r2;          //�ұ�������
    uint8_t cur_row = 0;     //ָ��ǰ�Ĵ������
    uint8_t pre_row = 0;     //ָ����һ���������
    uint8_t is_l_edge = 0;   //�Ƿ񿴵���߽�
    uint8_t is_r_edge = 0;   //�Ƿ񿴵��ұ߽�
    uint8_t l_black_cnt = 0; //��¼��߳��ּ����ڵ�
    uint8_t r_black_cnt = 0; //��¼�ұ߳��ּ����ڵ�
    uint8_t row_l_edge;      //��¼��ǰ�е���߽�
    uint8_t row_r_edge;      //��¼��ǰ�е��ұ߽�
    uint8_t *p;              //ͼ�������ָ��
    uint8_t i = 0, j = 0;    //�����±�����ʱ����

    PCout(18) = 1; // ��ʼ����LED����

    // ���ڵ�ǰͼ������Զ�����������ԭ��
    
    // ɨ����߽���
    if (l_line_index[0] != CAMERA_COL)     //ǰʮ�����зǿ��������
    {
        l1 = (l_line_index[0] > 1 + offset[0]) ? l_line_index[0] - offset[0] : 1;
        l2 = l1 + P_WIDTH;
    }
    else  //ǰһ�붼û���ҵ��ο�ֵ
    {
        if (r_line_index[0] != 0 && 0 < 15)
        {
            l1 = ((int16_t)((int16_t)r_line_index[0] + (int16_t)offset[0])>CAMERA_COL - P_WIDTH - 10) ? CAMERA_COL - P_WIDTH - 10 : r_line_index[0] + offset[0];
            l2 = l1 + P_WIDTH;
        }
        else
        {
            l1 = CAMERA_CENTER;
            l2 = l1 + P_WIDTH;
        }
    }

    // ɨ���ұ߽���
    if (r_line_index[0] != 0)
    {
        r1 = ((int16_t)((int16_t)r_line_index[0] + (int16_t)offset[0])<CAMERA_COL - P_WIDTH - 5) ? r_line_index[0] + offset[0] : CAMERA_COL - P_WIDTH - 5;
        r2 = r1 + P_WIDTH;
    }
    else  //���ǰ��һֱû���ҵ��ߣ���ô�������ҵ������ߵ�λ���Ʋ�l1,l2λ��
    {
        if (l_line_index[0] != CAMERA_COL&&0<15)
        {
            r1 = (l_line_index[0]>5 + offset[0]) ? l_line_index[0] - offset[0] : 5;
            r2 = r1 + P_WIDTH;
        }
        else
        {
            r1 = CAMERA_CENTER;
            r2 = r1 + P_WIDTH;
        }
    }

    // 
    for (cur_row = 0; cur_row < CAMERA_ROW; cur_row++)
    {
        p = imgaddr + cur_row * CAMERA_COL;                                                            //ָ��ǰ��

        if (cur_row>0)
        {
            //ȷ������ɨ�����  
            pre_row = cur_row - 1;                                                           //l_line_index[]=CAMERA_COL��ʾû���ҵ������
            
            while (pre_row>0 && cur_row - pre_row<5 && l_line_index[pre_row] == CAMERA_COL) pre_row--;
            
            if (l_line_index[pre_row] != CAMERA_COL)                                                   //�ڱ���ǰ�����ҵ���һ��������
            {
                if (l_line_index[pre_row]>offset[pre_row])
                    if (r_line_index[cur_row - 1] != 0 && l_line_index[pre_row] - offset[pre_row]<r_line_index[cur_row - 1] + 5)            //���ɨ�߿�ʼ������һ�����ߵ��ұ� ������һ�е�������Ϊɨ�߿�ʼ
                        l1 = r_line_index[cur_row - 1] + 5;
                    else
                        l1 = l_line_index[pre_row]-(offset[pre_row]/3);                                        //���޸�
                else
                    if (r_line_index[cur_row - 1] != 0 && r_line_index[cur_row - 1] + 5<CAMERA_COL - P_WIDTH)
                        l1 = r_line_index[cur_row - 1] + 5;
                    else
                        l1 = 1;
                l2 = l1 + P_WIDTH;
            }
            else                                             //ǰ��һֱ�޿���Ѱ�������� 
            {
                if (r_line_index[cur_row - 1] != 0)                  //��ô�����������������ȷ���ģ��������ߵ��Ƴ���ֵ
                {
                    l1 = ((int16_t)((int16_t)r_line_index[cur_row - 1] + 5)>CAMERA_COL - P_WIDTH - 10) ? CAMERA_COL - P_WIDTH - 10 : r_line_index[cur_row - 1] + 5;
                    l2 = l1 + P_WIDTH;
                }
                else
                {
                    l1 = CAMERA_CENTER;
                    l2 = l1 + P_WIDTH;
                }
            }
            
            //ȷ������ɨ�����
            pre_row = cur_row - 1;
            
            while (pre_row>0 && cur_row - pre_row<5 && r_line_index[pre_row] == 0) pre_row--;     //pre_row�м�¼��һ���ҵ�����Ч�����������������
                 
            if (r_line_index[pre_row] != 0)                                             //�ҵ���һ����������
            {
                if ((int16_t)((int16_t)r_line_index[pre_row] + (int16_t)offset[pre_row])<CAMERA_COL - P_WIDTH - 5)
                    if (l_line_index[cur_row - 1] != CAMERA_COL && (int16_t)(r_line_index[pre_row] + offset[pre_row])>(int16_t)(l_line_index[cur_row - 1] - P_WIDTH - 5))              //���ɨ�߿�ʼ������һ�����ߵ���� ������һ�е�������Ϊɨ�߿�ʼ
                        r1 = l_line_index[cur_row - 1] - P_WIDTH - 5;
                    else
                        r1 = r_line_index[pre_row];                                        //���޸�
                else
                    if (l_line_index[cur_row - 1] == CAMERA_COL)
                        r1 = CAMERA_COL - P_WIDTH - 5;
                    else if (l_line_index[cur_row - 1]>5 + P_WIDTH)
                        r1 = l_line_index[cur_row - 1] - P_WIDTH - 5;
                    else 
                        r1 = CAMERA_CENTER;
                r2 = r1 + P_WIDTH;
            }
            else //֮ǰһֱû���ҵ�����
            {
                if (l_line_index[cur_row - 1] != CAMERA_COL)//���ձ������ߵ�λ�� ȷ��l1 l2
                {
                    r1 = ((int16_t)(l_line_index[cur_row - 1]>5 + P_WIDTH + 5)) ? l_line_index[cur_row - 1] - P_WIDTH - 5 : 5;
                    r2 = r1 + P_WIDTH;
                }
                else
                { //��������û��
                    r1 = CAMERA_CENTER;
                    r2 = r1 + P_WIDTH;
                }
            }
            //������ɨ�߿�ʼ
        }

        if (r1 > 0) //�����ұ߽�
        {
            //------�����ұ߽�-------------------
            while (r1 > 0 && !is_r_edge)
            {
                if ((int16_t)(*(p + r2))>BW_DELTA + (int16_t)*(p + r1))     //�����з��ŵ����������
                {
                    while ((int16_t)(*(p + r2))>BW_DELTA + (int16_t)*(p + r1) && r1>0)
                    {
                        if ((int16_t)(*(p + r2))<255)
                            r_black_cnt++;
                        r1--;
                        r2--;
                        if (r_black_cnt >= LINE_EDGE)
                            break;
                    }
                    if (r_black_cnt >= LINE_EDGE) //�ж��ҵ�����
                    {
                        row_r_edge = r1 + LINE_EDGE;
                        r_black_cnt = 0;

                        j = 0;
                        for (i = 0; i < 10; i++)
                            if (*(p + r1 + LINE_EDGE + i) > THRESHOLD)
                        j++;
                        
                        if (j > 6)
                            is_r_edge = 1;
                        else
                            r_black_cnt = 0;
                    }
                    else
                        r_black_cnt = 0;
                }
                else
                {
                    r1--;
                    r2--;
                }
            }
        }
          
        if (l2 < CAMERA_COL)                    //�����ұ߽�,��������ɨ��
        {
                  //------������߽�-------------------
            while (l2 < CAMERA_COL && !is_l_edge)        //��l2û�е����е����ֵ����ɨ�裬��ɨ��ģʽ
            {
                if (((int16_t)(*(p + l1)))>BW_DELTA + (int16_t)*(p + l2))           // 
                {
                    while ((int16_t)(*(p + l1))>BW_DELTA + (int16_t)*(p + l2) && l2<CAMERA_COL)
                    {
                        if ((int16_t)(*(p + l1))<255)l_black_cnt++;
                        l1++;
                        l2++;
                        if (l_black_cnt >= LINE_EDGE)break;
                    }
                    
                    if (l_black_cnt >= LINE_EDGE)           //�ҵ���߽��˳�ѭ����l1��l2�����Ϊ1,                       
                    {
                        row_l_edge = l2 - LINE_EDGE;
                        l_black_cnt = 0;

                        j = 0;
                        for (j = 0; j < 10; j++)
                            if (*(p + l2 - LINE_EDGE - j)>THRESHOLD)
                                j++;

                        if (j>6)
                            is_l_edge = 1;
                        else
                            l_black_cnt = 0;
                    }
                    else                  //������㣬��������
                        l_black_cnt = 0;
                }
                else
                {
                    l1++;
                    l2++;
                }
            }
        }
        
        if (is_l_edge)
        {
            is_l_edge = 0;
            l_line_index[cur_row] = row_l_edge;
        }
        else l_line_index[cur_row] = CAMERA_COL;
        
        if (is_r_edge){
            is_r_edge = 0;
            r_line_index[cur_row] = row_r_edge;
        }
        else r_line_index[cur_row] = 0;
    }
    
    PCout(18) = 0; // ��������LED�ر�
}

void CAMERA_Display_Full(void)
{
    int i, j;
    // ���ֽ����д�벢ӳ�䵽OLED��
    for(i = 0; i < CAMERA_ROW; i++)
        for(j = 0; j < CAMERA_COL; j++)
            OLED_DrawPoint(j * OLED_COL / CAMERA_COL, i, img1[i][j] > THRESHOLD);
    // ˢ��OLED����ʾͼ��
    OLED_Refresh_Gram();
}

void CAMERA_Display_Edge(void)
{
    int i;
    // ���OLED��ʾ
    OLED_Fill(0, 0, 128, 50, 0);
    // ���ֽ����д�벢ӳ�䵽OLED��
    for(i = 0; i < CAMERA_ROW; i++)
    {
        if(l_line_index[i] > 0x00 && l_line_index[i] < CAMERA_COL)
            OLED_DrawPoint(l_line_index[i] * OLED_COL / CAMERA_COL, i, 1); // ��߽�
        if(r_line_index[i] > 0x00 && r_line_index[i] < CAMERA_COL)
            OLED_DrawPoint(r_line_index[i] * OLED_COL / CAMERA_COL, i, 1); // �ұ߽�
    }
    // ˢ��OLED����ʾͼ��
    OLED_Refresh_Gram();
}

void CAMERA_UART_TX_Full(const uint32_t instance)
{
    int i, j;

    // ����ɽ��๦�ܵ������֣���ʼ֡Ϊ0x01 0xFE
    UART_WriteByte(instance, 0x01);
    UART_WriteByte(instance, 0xFE);

    // ���ͼ���ֽ���������
    for(i = 0; i < CAMERA_ROW; i++)
        for(j = 0; j < CAMERA_COL; j++)
            UART_WriteByte(instance, img1[i][j]);

    // ����ɽ��๦�ܵ������֣�����֡Ϊ0xFE 0x01
    UART_WriteByte(instance, 0xFE);
    UART_WriteByte(instance, 0x01);
}

void CAMERA_UART_TX_Edge(const uint32_t instance)
{
    int i, j;

    // ����ɽ��๦�ܵ������֣���ʼ֡Ϊ0x01 0xFE
    UART_WriteByte(instance, 0x01);
    UART_WriteByte(instance, 0xFE);

    // ���ͼ���ֽ���������
    for(i = 0; i < CAMERA_ROW; i++)
        for(j = 0; j < CAMERA_COL; j++)
            if(j > 0x00 && j == l_line_index[i] || j < CAMERA_COL && j == r_line_index[i])
                UART_WriteByte(instance, 0xFF);
            else
                UART_WriteByte(instance, 0x00);

    // ����ɽ��๦�ܵ������֣�����֡Ϊ0xFE 0x01
    UART_WriteByte(instance, 0xFE);
    UART_WriteByte(instance, 0x01);
}
