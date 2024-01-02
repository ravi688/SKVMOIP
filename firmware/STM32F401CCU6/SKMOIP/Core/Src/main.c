/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "usbd_hid.h"
#include "w5500_spi.h"
#include "wizchip_conf.h"
#include "w5500.h"
#include "socket.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* 10 Bytes */
typedef union NetworkPacket
{
    uint8_t deviceType: 1;
    
    /* Keyboard: 2 Bytes */
    struct
    {
        uint8_t : 1;
        uint8_t keyStatus : 1;
	    uint8_t usbHIDUsageID : 8;
	    uint8_t modifierKeys: 8;
    };

	/* Mouse: 10 Bytes */
	struct
	{
	    /* 2 Bytes */
	    uint8_t : 1;
		uint8_t middleMBPressed : 1;
		uint8_t middleMBReleased : 1;
		uint8_t leftMBPressed : 1;
		uint8_t leftMBReleased : 1;
		uint8_t rightMBPressed : 1;
		uint8_t rightMBReleased : 1;
		uint8_t bfMBPressed : 1;
		uint8_t bfMBReleased : 1;
		uint8_t bbMBPressed : 1;
		uint8_t bbMBReleased : 1;
		
	    /* 8 Bytes */
		int16_t mousePointX;
		int16_t mousePointY;
		int16_t mouseWheelX;
		int16_t mouseWheelY;
	};
} NetworkPacket;

#define RECEIVE_BUFFER_SIZE (128 * sizeof(NetworkPacket)) // must be greater than or equal to 3

typedef enum input_type_t
{
	INPUT_TYPE_KEYBOARD = 0x00,
	INPUT_TYPE_MOUSE = 0x01,
} input_type_t;

typedef enum keycode_t
{
	KEYCODE_A = 4,
	KEYCODE_B,
	KEYCODE_C,
	KEYCODE_D,
	KEYCODE_E,
	KEYCODE_F,
	KEYCODE_G,
	KEYCODE_H,
	KEYCODE_I,
	KEYCODE_J,
	KEYCODE_K,
	KEYCODE_L,
	KEYCODE_M,
	KEYCODE_N,
	KEYCODE_O,
	KEYCODE_P,
	KEYCODE_Q,
	KEYCODE_R,
	KEYCODE_S,
	KEYCODE_T,
	KEYCODE_U,
	KEYCODE_V,
	KEYCODE_W,
	KEYCODE_X,
	KEYCODE_Y,
	KEYCODE_Z,
	KEYCODE_1,
	KEYCODE_2,
	KEYCODE_3,
	KEYCODE_4,
	KEYCODE_5,
	KEYCODE_6,
	KEYCODE_7,
	KEYCODE_8,
	KEYCODE_9,
	KEYCODE_0,
	KEYCODE_RETURN,
	KEYCODE_ESCAPE,
	KEYCODE_DELETEDOT,
	KEYCODE_TAB,
	KEYCODE_SPACEBAR,
	KEYCODE_MINUS,
	KEYCODE_EQUAL,
	KEYCODE_BRACKETSTART,
	KEYCODE_BRACKETEND,
	KEYCODE_BACKSLASH,
	KEYCODE_TILDE,
	KEYCODE_SEMICOLON,
	KEYCODE_SINGLEQUOTE,
	KEYCODE_TILDE2,
	KEYCODE_COMMA,
	KEYCODE_FULLSTOP,
	KEYCODE_FORWARDSLASH,
	KEYCODE_CAPSLOCK,
	KEYCODE_F1,
	KEYCODE_F2,
	KEYCODE_F3,
	KEYCODE_F4,
	KEYCODE_F5,
	KEYCODE_F6,
	KEYCODE_F7,
	KEYCODE_F8,
	KEYCODE_F9,
	KEYCODE_F10,
	KEYCODE_F11,
	KEYCODE_F12,
	KEYCODE_PRINTSCREEN,
	KEYCODE_SCROLLLOCK,
	KEYCODE_PAUSE,
	KEYCODE_INSERT,
	KEYCODE_HOME,
	KEYCODE_PAGEUP,
	KEYCODE_DELETE,
	KEYCODE_END,
	KEYCODE_PAGEDOWN,
	KEYCODE_RIGHTARROW,
	KEYCODE_LEFTARROW,
	KEYCODE_UPARROW,
	KEYCODE_KEYPADNUMLOCK,
	KEYCODE_KEYPADFORWARDSLASH,
	KEYCODE_KEYPADPLUS,
	KEYCODE_KEYPADMINUS,
	KEYCODE_KEYPADENTER,
	KEYCODE_KEYPAD1END,
	KEYCODE_KEYPAD2DOWNARROW,
	KEYCODE_KEYPAD3PAGEDOWN,
	KEYCODE_KEYPAD4LEFTARROW,
	KEYCODE_KEYPAD5,
	KEYCODE_KEYPAD6RIGHTARROW,
	KEYCODE_KEYPAD7HOME,
	KEYCODE_KEYPAD8UPARROW,
	KEYCODE_KEYPAD9PAGEUP,
	KEYCODE_KEYPAD0INSERT,
	KEYCODE_KEYPADDOTDELETE,
	KEYCODE_BACKSLASHBAR,
	KEYCODE_APPLICATION,
	KEYCODE_POWER,
	KEYCODE_KEYPADEQUAL,
	KEYCODE_MAX
} keycode_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static wiz_NetInfo netInfo =
{
		.mac = { 0x0c, 0x29, 0xab, 0x7c, 0x00, 0x01 },
		.ip = { 192, 168, 1, 113 },
		.sn  = { 255, 255, 255, 0 },
		.gw = { 192, 168, 1, 1 },
		.dns = { 0, 0, 0, 0 },
		.dhcp = NETINFO_STATIC
};

static uint8_t getKeyCodeFromASCII(char ch)
{
	switch(ch)
	{
	case 'a': return KEYCODE_A;
	case 'b': return KEYCODE_B;
	case 'c': return KEYCODE_C;
	case 'd': return KEYCODE_D;
	case 'e': return KEYCODE_E;
	case 'f': return KEYCODE_F;
	case 'g': return KEYCODE_G;
	case 'h': return KEYCODE_H;
	case 'i': return KEYCODE_I;
	case 'j': return KEYCODE_J;
	case 'k': return KEYCODE_K;
	case 'l': return KEYCODE_L;
	case 'm': return KEYCODE_M;
	case 'n': return KEYCODE_N;
	case 'o': return KEYCODE_O;
	case 'p': return KEYCODE_P;
	case 'q': return KEYCODE_Q;
	case 'r': return KEYCODE_R;
	case 's': return KEYCODE_S;
	case 't': return KEYCODE_T;
	case 'u': return KEYCODE_U;
	case 'v': return KEYCODE_V;
	case 'w': return KEYCODE_W;
	case 'x': return KEYCODE_X;
	case 'y': return KEYCODE_Y;
	case 'z': return KEYCODE_Z;
	case '1': return KEYCODE_1;
	case '2': return KEYCODE_2;
	case '3': return KEYCODE_3;
	case '4': return KEYCODE_4;
	case '5': return KEYCODE_5;
	case '6': return KEYCODE_6;
	case '7': return KEYCODE_7;
	case '8': return KEYCODE_8;
	case '9': return KEYCODE_9;
	case '0': return KEYCODE_0;
	case '\n': return KEYCODE_RETURN;
	case '\r': return KEYCODE_RETURN;
	case '\b': return KEYCODE_DELETEDOT;
	case '\t': return KEYCODE_TAB;
	case ' ': return KEYCODE_SPACEBAR;
	default: return 0;
	}
	return 0;
}

static void SetupNetworkDevice()
{
	wizchip_setnetinfo(&netInfo);
	ctlnetwork(CN_SET_NETINFO, &netInfo);
}

static char getCode(int32_t size)
{
	switch(size)
	{
	case SOCKERR_SOCKSTATUS: return 'd';
	case SOCKERR_SOCKMODE: return 'e';
	case SOCKERR_SOCKNUM: return 'f';
	case SOCKERR_DATALEN: return 'g';
	case SOCK_BUSY: return 'h';
	default: return 'i';
	}
}

struct
{
	  uint8_t reportID;
	  uint8_t modifier;
	  uint8_t reserved;
	  uint8_t keycode0;
	  uint8_t keycode1;
	  uint8_t keycode2;
	  uint8_t keycode3;
	  uint8_t keycode4;
	  uint8_t keycode5;
} keyboardReport = { 0x01 };

struct
{
	  uint8_t reportID;
	  uint8_t button;
	  int8_t mouse_x;
	  int8_t mouse_y;
	  int8_t wheel;
} mouseReport = { 0x02 };

typedef enum key_status_t
{
	KEY_STATUS_RELEASED = 0,
	KEY_STATUS_PRESSED
} key_status_t;

static inline void SendKeyboardReportASCII(uint8_t keycode)
{
	keyboardReport.keycode0 = getKeyCodeFromASCII((char)keycode);
	USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&keyboardReport, sizeof(keyboardReport));
	keyboardReport.keycode0 = 0;
	HAL_Delay(10);
	USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&keyboardReport, sizeof(keyboardReport));
	HAL_Delay(20);
}

static inline void SendKeyboardReport(uint8_t hidUsageID, uint8_t modifierKeys, key_status_t status)
{
	keyboardReport.modifier = modifierKeys;
	switch(status)
	{
		case KEY_STATUS_RELEASED:
			{
				keyboardReport.keycode0 = 0;
				break;
			}
		case KEY_STATUS_PRESSED:
			{
				keyboardReport.keycode0 = hidUsageID;
				break;
			}
	}
	USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&keyboardReport, sizeof(keyboardReport));
}

static inline void SendMouseReport(uint8_t button, int8_t dx, int8_t dy, int8_t wheel)
{
	mouseReport.button = button;
	mouseReport.mouse_x = dx;
	mouseReport.mouse_y = dy;
	mouseReport.wheel = wheel;
	USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&mouseReport, sizeof(mouseReport));
	if(button != 0x0)
	{
		HAL_Delay(20);
		mouseReport.button = 0;
		mouseReport.mouse_x = 0;
		mouseReport.mouse_y = 0;
		mouseReport.wheel = 0;
		USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&mouseReport, sizeof(mouseReport));
		HAL_Delay(20);
		SendKeyboardReportASCII('a');
	}
}

static inline void Info(const char* str)
{
	uint8_t len = strlen(str);
	for(uint8_t i = 0; i < len; i++)
		 SendKeyboardReportASCII(str[i]);
}

static inline void Error(const char* str)
{
	Info("Error: ");
	Info(str);
	while(1);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
  W5500_Init();
  SetupNetworkDevice();
  SOCKET mySocket = socket(0, Sn_MR_TCP, 2000, 0);

  if(mySocket != 0)
  {
	  Error("failed to create socket 0\n");
  }

  if(sizeof(NetworkPacket) != 10)
  {
  	Error("sizeof network packet incorrect");
  }

  uint8_t recieveBuffer[RECEIVE_BUFFER_SIZE];

//  uint8_t previousKeyPressedID = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  if( listen(mySocket) != SOCK_OK)
	  {
		  Error("listen error\n");
	  }

	  while(getSn_SR(mySocket) != SOCK_ESTABLISHED)
	  {
		  Info("waiting for sock establish\n");
	  }

	  while(1)
	  {
		  int32_t size = recv(mySocket, recieveBuffer, RECEIVE_BUFFER_SIZE);
		  if(size <= 0)
		  {
			  if(size == SOCKERR_SOCKSTATUS)
			  {
				  if(close(mySocket) != SOCK_OK)
					  Error("failed to close socket\n");
				  HAL_Delay(100);
				  if(socket(mySocket, Sn_MR_TCP, 2000, 0) != mySocket)
				  {
				  	  Error("failed to create socket again\n");
				  }
				  break;
			  }
			  continue;
		  }

		  NetworkPacket* packets = (NetworkPacket*)(recieveBuffer);
		  NetworkPacket* end = (NetworkPacket*)(recieveBuffer + size);
		  for(; packets < end; ++packets)
		  {
			  NetworkPacket packet = *packets;
			  switch(packet.deviceType)
			  {
			  	  case 0 /* Keyboard */:
			  	  {
//			  		  if((previousKeyPressedID > 0) && (packet.keyStatus == KEY_STATUS_PRESSED))
//			  		  {
//			  			  SendKeyboardReport(previousKeyPressedID, KEY_STATUS_RELEASED);
//			  			  previousKeyPressedID = packet.usbHIDUsageID;
//			  			  HAL_Delay(20);
//			  		  }
			  		  SendKeyboardReport(packet.usbHIDUsageID, packet.modifierKeys, (key_status_t)packet.keyStatus);
			  		  HAL_Delay(10);
			  		  break;
			  	  }
			  	  case 1 /* Mouse */:
			  	  {
/*			  		  mouseReport.button = 0;
			  		  mouseReport.mouse_x = 0;
			  		  mouseReport.mouse_y = 0;
			  		  mouseReport.wheel = 0;
			  		  if(command & 0x02)
			  		  {
			  			  mouseReport.button = recieveBuffer[i++];
			  		  }
			  		  if(command & 0x04)
			  		  {
			  			  mouseReport.mouse_x = *((int8_t*)(recieveBuffer + (i++)));
			  			  mouseReport.mouse_y = *((int8_t*)(recieveBuffer + (i++)));
			  		  }
			  		  if(command & 0x08)
			  		  {
			  			  mouseReport.wheel = *((int8_t*)(recieveBuffer + (i++)));
			  		  }
			  		  uint8_t t = (mouseReport.mouse_x < 0) ? -mouseReport.mouse_x : mouseReport.mouse_x;
			  		  t *= 2;
			  		  while(t--)
			  		  {
			  			  USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t*)&mouseReport, sizeof(mouseReport));
			  			  HAL_Delay(1);
			  		  }*/
			  		  break;
			  	  }
			  	  default:
			  	  {
			  		  Info("invalid input type\n");
			  	  }
			  }
		  }
	  }
  }
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
  close(mySocket);
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 13;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
