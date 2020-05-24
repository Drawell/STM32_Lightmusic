#include "microphone_driver.h"

#define SPI_SCK_PIN GPIO_Pin_10
#define SPI_SCK_GPIO_PORT GPIOB
#define SPI_SCK_GPIO_CLK RCC_AHB1Periph_GPIOB
#define SPI_SCK_SOURCE GPIO_PinSource10
#define SPI_SCK_AF GPIO_AF_SPI2

#define SPI_MOSI_PIN GPIO_Pin_3
#define SPI_MOSI_GPIO_PORT GPIOC
#define SPI_MOSI_GPIO_CLK RCC_AHB1Periph_GPIOC
#define SPI_MOSI_SOURCE GPIO_PinSource3
#define SPI_MOSI_AF PIO_AF_SPI2

#define AUDIO_REC_SPI_IRQHANDLER SPI2_IRQHandler

#define MIC_FILTER_RESULT_LENGTH 16
#define PCM_OUT_SIZE MIC_FILTER_RESULT_LENGTH

#define INTERNAL_BUFF_SIZE 64

static uint16_t Mic_DMA_PDM_Buffer0[INTERNAL_BUFF_SIZE]; //buffer for RAW MIC data (filled by DMA)
static uint16_t Mic_DMA_PDM_Buffer1[INTERNAL_BUFF_SIZE]; //buffer for RAW MIC data (filled by DMA)

static void MP45DT02_I2S_init(void);
static void WaveRecorder_DMA_Init(void);
static callback_t callback_func = 0;

void init_microphone_driver(callback_t interrupt_handler)
{
    callback_func = interrupt_handler;
    MP45DT02_I2S_init();
}

void DMA1_Stream3_IRQHandler(void)
{
    if (DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3) != RESET)
    {
        DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
        if (callback_func == 0)
            return;

        if ((DMA1_Stream3->CR & DMA_SxCR_CT) == 0)
            callback_func(Mic_DMA_PDM_Buffer0, INTERNAL_BUFF_SIZE);  
        else
            callback_func(Mic_DMA_PDM_Buffer1, INTERNAL_BUFF_SIZE);              
    }
}

static void WaveRecorder_DMA_Init(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
    DMA_Cmd(DMA1_Stream3, DISABLE);
    DMA_DeInit(DMA1_Stream3);

    DMA_InitStructure.DMA_Channel = DMA_Channel_0;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&SPI2->DR;
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Mic_DMA_PDM_Buffer0;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
    DMA_InitStructure.DMA_BufferSize = (uint32_t)INTERNAL_BUFF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //16bit
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //<<<<<<<<<<<<
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA1_Stream3, &DMA_InitStructure);

    DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t)&Mic_DMA_PDM_Buffer1, DMA_Memory_0);

    DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);
    DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);

    DMA_Cmd(DMA1_Stream3, ENABLE);
}

static void MP45DT02_I2S_init(void)
{
    GPIO_InitTypeDef GPIO_ini_user;
    I2S_InitTypeDef i2s_struct_ini;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    RCC_PLLI2SCmd(ENABLE);

    //PB10 - SCK    PC3 - SD
    GPIO_ini_user.GPIO_Pin = GPIO_Pin_3;
    GPIO_ini_user.GPIO_Mode = GPIO_Mode_AF;
    GPIO_ini_user.GPIO_OType = GPIO_OType_PP;
    GPIO_ini_user.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_ini_user.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_ini_user);

    GPIO_ini_user.GPIO_Pin = GPIO_Pin_10;
    GPIO_ini_user.GPIO_Mode = GPIO_Mode_AF;
    GPIO_ini_user.GPIO_OType = GPIO_OType_PP;
    GPIO_ini_user.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_ini_user.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_ini_user);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);

    SPI_I2S_DeInit(SPI2);
    i2s_struct_ini.I2S_AudioFreq = I2S_AudioFreq_32k;
    i2s_struct_ini.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
    i2s_struct_ini.I2S_DataFormat = I2S_DataFormat_16b;
    i2s_struct_ini.I2S_Mode = I2S_Mode_MasterRx;
    i2s_struct_ini.I2S_Standard = I2S_Standard_Phillips;
    i2s_struct_ini.I2S_CPOL = I2S_CPOL_Low;

    I2S_Init(SPI2, &i2s_struct_ini);

    WaveRecorder_DMA_Init();

    I2S_Cmd(SPI2, ENABLE);
}