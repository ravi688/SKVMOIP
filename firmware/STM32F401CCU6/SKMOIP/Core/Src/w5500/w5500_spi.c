/*
 * w5500_spi.c
 *
 *  Created on: Dec 24, 2023
 *      Author: rp041
 */

#include "w5500_spi.h"
#include "wizchip_conf.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;

#define RESET_GPIO_PIN 			GPIO_PIN_3
#define CHIP_SELECT_GPIO_PIN 	GPIO_PIN_4

static void wizchip_select(void)
{
	/* Write Low, Low selects the slave chip */
	HAL_GPIO_WritePin(GPIOA, CHIP_SELECT_GPIO_PIN, GPIO_PIN_RESET);
}

static void wizchip_deselct(void)
{
	/* Write High, High deselects the slave chip */
	HAL_GPIO_WritePin(GPIOA, CHIP_SELECT_GPIO_PIN, GPIO_PIN_SET);
}

static void SPIWrite(uint8_t data)
{
	/* Wait till FIFO has a free slot */
	//while((hspi1.Instance->SR & SPI_FLAG_TXE) != SPI_FLAG_TXE);

	//*(__IO uint8_t*)&hspi1.Instance->DR = data;
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
	if(status > HAL_OK)
	{
		printf("Failed to Write\n");
		while(1);
	}
}

static uint8_t SPIRead()
{
	/* Now wait till data arrives */
	//while((hspi1.Instance->SR & SPI_FLAG_RXNE) != SPI_FLAG_RXNE);

	//return (*(__IO uint8_t*)&hspi1.Instance->DR);
	uint8_t data;
	HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, &data, 1, HAL_MAX_DELAY);
	if(status > HAL_OK)
	{
		printf("Failed to Read\n");
		while(1);
	}
	return data;
}

static inline uint8_t wizchip_read()
{
	return SPIRead();
}

static inline void wizchip_write(uint8_t wb)
{
	SPIWrite(wb);
}

static void wizchip_readburst(uint8_t* pBuf, uint16_t len)
{
	HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, pBuf, len, HAL_MAX_DELAY);
	if(status > HAL_OK)
	{
		printf("Failed to Read\n");
		while(1);
	}
}

static void wizchip_writeburst(uint8_t* pBuf, uint16_t len)
{
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, pBuf, len, HAL_MAX_DELAY);
	if(status > HAL_OK)
	{
		printf("Failed to Read\n");
		while(1);
	}
}

static void W5500_SPI_Init()
{
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	__HAL_RCC_GPIOA_CLK_ENABLE();

	GPIO_InitStruct.Pin = RESET_GPIO_PIN | CHIP_SELECT_GPIO_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void W5500_Device_Init()
{
	uint8_t memsize[2][8] = {
								{ 16, 0, 0, 0, 0, 0, 0, 0 },
								{ 16, 0, 0, 0, 0, 0, 0, 0 }
							};

	HAL_GPIO_WritePin(GPIOA, CHIP_SELECT_GPIO_PIN, GPIO_PIN_SET);

	HAL_GPIO_WritePin(GPIOA, RESET_GPIO_PIN, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(GPIOA, RESET_GPIO_PIN, GPIO_PIN_SET);
	HAL_Delay(1000);

	reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselct);
	reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
	reg_wizchip_spiburst_cbfunc(wizchip_readburst, wizchip_writeburst);

	/* wizchip initialize */
	if (ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1)
	{
		printf("WIZCHIP Initialized fail. \r\n");
		while(1);
	}

	printf("WIZCHIP Initialized success.\r\n");
}

void W5500_Init()
{
	W5500_SPI_Init();
	W5500_Device_Init();
}
