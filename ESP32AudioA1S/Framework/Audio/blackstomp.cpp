/*!
 *  @file       blackstomp.cpp
 *  Project     Blackstomp Arduino Library
 *  @brief      Blackstomp Library for the Arduino
 *  @author     Hasan Murod
 *  @date       19/11/2020
 *  @license    MIT - Copyright (c) 2020 Hasan Murod
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
  
#include "blackstomp.h"
#include "ac101.h"
#include "driver/i2s.h"
#include "esp_task_wdt.h"
#include "math.h"

//CONTROL INPUT
#define P1_PIN  39
#define P2_PIN  36
#define P3_PIN  34
#define P4_PIN  14
#define P5_PIN  13
#define P6_PIN  4

//ROTARY ENCODER
#define RE_BUTTON_PIN 19
#define RE_PHASE0_PIN 23
#define RE_PHASE1_PIN 18

//FOOT SW PIN SETUP
#define FS_PIN 5

//LED INDICATOR PIN SETUP
#define MAINLED_PIN 2
#define AUXLED_PIN 15

//OLED Display PIN SETUP
#define SCK_PIN 22
#define SDA_PIN 21

//AC101 AUDIO CODEC PIN SETUP
#define I2S_NUM         (0)
#define I2S_MCLK        (GPI_NUM_0)
#define I2S_BCK_IO      (GPIO_NUM_27)
#define I2S_WS_IO       (GPIO_NUM_26)
#define I2S_DO_IO       (GPIO_NUM_25)
#define I2S_DI_IO       (GPIO_NUM_35)
#define CODEC_SCK       (GPIO_NUM_32)
#define CODEC_SDA       (GPIO_NUM_33)

//audio processing frame length in samples (L+R) 64 samples (32R+32L) 256 Bytes
#define FRAMELENGTH    64
//sample count per channel for each frame (32)re
#define SAMPLECOUNT   FRAMELENGTH/2
//channel count inside a frame (always stereo = 2)
#define CHANNELCOUNT  2
//frame size in bytes
//audio processing priority
#define AUDIO_PROCESS_PRIORITY  10

//dma buffer length 32 bytes (8 samples: 4L+4R)
#define DMABUFFERLENGTH 32
//dma buffer count 20 (640 Bytes: 160 samples: 80L+80R) 
#define DMABUFFERCOUNT  20

//codec instance
static AC101 _codec;

#define BITSPERSAMPLE 32
#define tSample int32_t
#define FRAMESIZE   FRAMELENGTH*4

//effect module pointer
bool _codecIsReady = false;

//buffers

static tSample inbuffer[FRAMELENGTH];
static tSample outbuffer[FRAMELENGTH];
static float inleft[FRAMESIZE];
static float inright[FRAMESIZE];
static float outleft[FRAMESIZE];
static float outright[FRAMESIZE];

static unsigned int usedticks;
static unsigned int availableticks;
static unsigned int availableticks_start;
static unsigned int availableticks_end;
static unsigned int usedticks_start;
static unsigned int usedticks_end;
static volatile unsigned int processedframe;
static unsigned int audiofps;

bool DoReadLineIn=false;
void framecounter_task(void* arg)
{
  processedframe = 0;
  while(true)
  {
    audiofps = processedframe;
    processedframe = 0;
    vTaskDelay(1000);
  }
  vTaskDelete(NULL);
}
extern void process(float* inLeft, float* inRight, float* outLeft, float* outRight, int sampleCount); // fraai is anders...

void i2s_task(void* arg)
{
  size_t bytesread, byteswritten;

  //initialize all output buffer to zero
  for(int i= 0; i< FRAMELENGTH; i++)
    outbuffer[i] = 0;
    
  for(int i=0; i< SAMPLECOUNT; i++)
  {
    outleft[i]=0;
    outright[i]=0;
  }

  usedticks_start = xthal_get_ccount();
  availableticks_start = xthal_get_ccount();
  
  while(true)
  {
    availableticks_end = xthal_get_ccount();
    availableticks = availableticks_end - availableticks_start;
    availableticks_start = availableticks_end;
    
    if (DoReadLineIn)
      i2s_read((i2s_port_t)I2S_NUM,(void*) inbuffer, FRAMESIZE, &bytesread, 20);

    //used-tick counter starting point
    usedticks_start = xthal_get_ccount();

    if (DoReadLineIn)
      for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      //convert to 24 bit int then to float
      inleft[k] = (float) (inbuffer[i]>>8);
      inright[k] = (float) (inbuffer[i+1]>>8);

      //scale to 1.0
      inleft[k] = inleft[k]/8388608; // 8388608 = 0x800000
      inright[k]=inright[k]/8388608;
    }
  
    //process the signal by the effect module

    process(inleft, inright, outleft, outright, SAMPLECOUNT);
    processedframe++;
    
    //convert back float to int
    for(int i=0,k=0;k<SAMPLECOUNT;k++,i+=2)
    {
      //scale the left output to 24 bit range
      outleft[k] = outleft[k]*8388607;
      //saturate to signed 24bit range
      if(outleft[k]>8388607) outleft[k]=8388607;
      if(outleft[k]<-8388607) outleft[k]= -8388607;

      //scale the right output to 24 bit range
      outright[k]=outright[k]*8388607;
      //saturate to signed 24bit range
      if(outright[k]>8388607) outright[k]=8388607;
      if(outright[k]<-8388607) outright[k]= -8388607;
      
      //convert to 32 bit int
      outbuffer[i] = ((int32_t) outleft[k])<<8;
      outbuffer[i+1] = ((int32_t) outright[k])<<8;
    }

    //used-tick counter end point
    usedticks_end = xthal_get_ccount();
    usedticks = usedticks_end - usedticks_start;
    
    i2s_write((i2s_port_t)I2S_NUM,(void*) outbuffer, FRAMESIZE, &byteswritten, 20);
    esp_task_wdt_reset();
  }
  vTaskDelete(NULL);
}

void i2s_setup(bool receive)
{
  i2s_config_t i2s_config;
  i2s_config.mode =(i2s_mode_t) (I2S_MODE_MASTER | I2S_MODE_TX | (I2S_MODE_RX*receive));
  i2s_config.sample_rate = SAMPLE_RATE;
  i2s_config.bits_per_sample = (i2s_bits_per_sample_t) BITSPERSAMPLE;
  i2s_config.channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT; //both channel
  i2s_config.communication_format = (i2s_comm_format_t) I2S_COMM_FORMAT_I2S;
  
  i2s_config.dma_buf_count = DMABUFFERCOUNT;
  i2s_config.dma_buf_len = DMABUFFERLENGTH;

#ifdef USE_APLL_MCLK_6M
  i2s_config.use_apll = true;
  i2s_config.fixed_mclk = 6000000; 
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);
  WRITE_PERI_REG(PIN_CTRL, READ_PERI_REG(PIN_CTRL) & 0xFFFFFFF0);
#else
  i2s_config.use_apll = false;
  i2s_config.fixed_mclk = 0; 
#endif
  
  i2s_config.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1 ;
  i2s_driver_install((i2s_port_t)I2S_NUM, &i2s_config, 0, NULL);
  
  i2s_pin_config_t pin_config;
  pin_config.bck_io_num = I2S_BCK_IO;
  pin_config.ws_io_num = I2S_WS_IO;
  pin_config.data_out_num = I2S_DO_IO;
  pin_config.data_in_num = I2S_DI_IO;
  
  i2s_set_pin((i2s_port_t)I2S_NUM, &pin_config);
  i2s_set_clk((i2s_port_t)I2S_NUM, SAMPLE_RATE, (i2s_bits_per_sample_t) BITSPERSAMPLE, I2S_CHANNEL_STEREO);
}


void codecsetup()
{
  _codec.setup(CODEC_SDA, CODEC_SCK);
  _codecIsReady = true;
  _codec.LeftLineLeft(true);
  _codec.RightLineRight(true);
}

void blackstompSetup(bool doReadLineIn) 
{ 
  //assign the module pointer "_module" and init the module
  DoReadLineIn=doReadLineIn;
   codecsetup();
  i2s_setup(doReadLineIn);
  //the main audio task, dedicated on core 1
  xTaskCreatePinnedToCore(i2s_task, "i2s_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,0);

  //audio frame moitoring task
  xTaskCreatePinnedToCore(framecounter_task, "framecounter_task", 4096, NULL, AUDIO_PROCESS_PRIORITY, NULL,1);
  
}

void sysmon_task(void *arg)
{
	int* period = (int*)(arg);
	while(true)
	{
	  //System info
	  Serial.printf("\nSYSTEM INFO:\n");
	  Serial.printf("Internal Total heap %d, internal Free Heap %d\n",ESP.getHeapSize(),ESP.getFreeHeap());
	  Serial.printf("SPIRam Total heap %d, SPIRam Free Heap %d\n",ESP.getPsramSize(),ESP.getFreePsram());
	  Serial.printf("ChipRevision %d, Cpu Freq %d, SDK Version %s\n",ESP.getChipRevision(), ESP.getCpuFreqMHz(), ESP.getSdkVersion());
	  Serial.printf("Flash Size %d, Flash Speed %d\n",ESP.getFlashChipSize(), ESP.getFlashChipSpeed());
	  
	  //Blackstomp application info
	  Serial.printf("\nAPPLICATION INFO:\n");
	  Serial.printf("Audio frame per second: %d fps\n",getAudioFps());
	  Serial.printf("CPU ticks per frame period: %d\n",getTotalCpuTicks());
	  Serial.printf("Used CPU ticks: %d\n",getUsedCpuTicks());
	  Serial.printf("CPU Usage: %.2f %%\n", 100.0*getCpuUsage());
	  vTaskDelay(period[0]);
	}
	vTaskDelete(NULL);
}

int _updatePeriod;
void runSystemMonitor( int updatePeriod)
{
	_updatePeriod = updatePeriod;
	//run the performance monitoring task at 0 (idle) priority
	xTaskCreatePinnedToCore(sysmon_task, "sysmon_task", 4096, &_updatePeriod, 0, NULL,0);
}


int getTotalCpuTicks()
{
  return availableticks;
}

int getUsedCpuTicks()
{
  return usedticks;
}

float getCpuUsage()
{
  return (float)usedticks/(float)availableticks;
}

int getAudioFps()
{
  return audiofps;
}

void setMicGain(int gain)
{
  _codec.SetMicGain(gain);
}

int getMicGain()
{
  return _codec.GetMicGain();
}

void setOutVol(int vol)
{
  _codec.SetVolSpeaker(vol);
}

bool setOutMix(bool mixedLeft, bool mixedRight)
{
	return _codec.SetOutputMode(mixedLeft,mixedRight);
}

int getOutVol()
{
  return _codec.GetVolSpeaker();
}
