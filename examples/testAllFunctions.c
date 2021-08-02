/* USER CODE BEGIN Header */
/**
 * @example testAllFunctions.c
 * @author  Reid Sox-Harris (@eosti)
 * Tests every function to ensure functionality of the library
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Cypress_FLS_QSPI_Driver.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

QSPI_HandleTypeDef hqspi;
MDMA_HandleTypeDef hmdma_quadspi_fifo_th;

/* USER CODE BEGIN PV */
// Volatile flags for callbacks
__IO uint8_t CmdCplt, RxCplt, TxCplt, StatusMatch, TimeOut;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_MDMA_Init(void);
static void MX_QUADSPI_Init(void);
/* USER CODE BEGIN PFP */
static void CPU_CACHE_Enable(void);
void Assert_Error(void);
uint8_t compareBuffers(uint8_t buf1[], uint8_t buf2[], uint8_t len);
void initBuffer(uint8_t buf[], uint8_t len);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
    CPU_CACHE_Enable();
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
  MX_MDMA_Init();
  MX_QUADSPI_Init();
  /* USER CODE BEGIN 2 */

  // Ensure that we can write to the memory
  // Once QUAD is set, WP is ignored, but we must pull WP high to do that
  Cypress_QSPI_DisableWP(QUADSPI_WRITEPROT_GPIO_Port, QUADSPI_WRITEPROT_Pin);

  /* Test ReadCR, WriteCR, latency macros */
  // Read CR
  uint8_t configRegister;
  if (Cypress_QSPI_ReadCR(&hqspi, &configRegister) != HAL_OK) {
      Error_Handler();
  }

  // Set QUAD, LC bits
  MODIFY_REG(configRegister, 0x02, 0x02);
  MODIFY_REG(configRegister, 0xC0, CYPRESS_DUMMY_LC);

  if (Cypress_QSPI_WriteCR(&hqspi, configRegister) != HAL_OK) {
      Error_Handler();
  }

  // Get new state of config register
  uint8_t configRegisterNew;
  if (Cypress_QSPI_ReadCR(&hqspi, &configRegisterNew) != HAL_OK) {
      Error_Handler();
  }

  // Verify that proper bits set
  if (configRegisterNew != configRegister) {
      Assert_Error();
  }

  // Now that QUAD is set, WP is disabled, so we can restore the WP pin
  Cypress_QSPI_ResetWP(QUADSPI_WRITEPROT_GPIO_Port, QUADSPI_WRITEPROT_Pin);

     /* Test ReadSR1, ReadSR2, WriteEnable, WriteDisable */
    // Read status register
    uint8_t statusRegister;
    if (Cypress_QSPI_ReadSR1(&hqspi, &statusRegister) != HAL_OK) {
        Error_Handler();
    }

    // Enable write
    if (Cypress_QSPI_WriteEnable(&hqspi) != HAL_OK) {
        Error_Handler();
    }

    // Get new state of status register
    uint8_t statusRegisterWREN;
    if (Cypress_QSPI_ReadSR1(&hqspi, &statusRegisterWREN) != HAL_OK) {
        Error_Handler();
    }

    Cypress_QSPI_WaitWriteReady(&hqspi, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    // Verify WREN bit was set
    if ((statusRegister | SR1_WREN) != statusRegisterWREN) {
        Assert_Error();
    }

    // Disable write
    if (Cypress_QSPI_WriteDisable(&hqspi) != HAL_OK) {
        Error_Handler();
    }

    // Get new state of status register
    uint8_t statusRegisterWDIS;
    if (Cypress_QSPI_ReadSR1(&hqspi, &statusRegisterWDIS) != HAL_OK) {
        Error_Handler();
    }

    // Verify SR1 is restored
    if (statusRegister != statusRegisterWDIS) {
        Assert_Error();
    }

    // Read SR2
    uint8_t statusRegister2;
    if (Cypress_QSPI_ReadSR1(&hqspi, &statusRegister2) != HAL_OK) {
        Error_Handler();
    }

    /* Test all program and read operations */
    uint8_t programString[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
    uint8_t programStringLen = (COUNTOF(programString) - 1);
    uint32_t address = 0;
    uint8_t receptionBuffer[programStringLen];

    if (Cypress_QSPI_SectorErase(&hqspi, address) != HAL_OK) {
    	Error_Handler();
    }

    // Single, blocking functions
    initBuffer(receptionBuffer, programStringLen);
    if (Cypress_QSPI_Program(&hqspi, address, programString, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    if (Cypress_QSPI_WaitMemReady(&hqspi, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    	Error_Handler();
    }

    if (Cypress_QSPI_Read(&hqspi, address, receptionBuffer, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    if (!compareBuffers(programString, receptionBuffer, programStringLen)) {
        Assert_Error();
    }

    // Single, interrupt functions
    address += 8 * QSPI_PAGE_SIZE;
    initBuffer(receptionBuffer, programStringLen);
    TxCplt = 0;

    if (Cypress_QSPI_Program_IT(&hqspi, address, programString, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (TxCplt == 0) {
    	// Wait for command to complete
    }

    TxCplt = 0;
    StatusMatch = 0;

    if (Cypress_QSPI_WaitMemReady_IT(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    while (StatusMatch == 0) {
    	// Wait for command to complete
    }

    StatusMatch = 0;
    RxCplt = 0;

    if (Cypress_QSPI_Read_IT(&hqspi, address, receptionBuffer, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (RxCplt == 0) {
    	// Wait for command to complete
    }

    RxCplt = 0;

    if (Cypress_QSPI_CheckForErrors(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    if (!compareBuffers(programString, receptionBuffer, programStringLen)) {
        Assert_Error();
    }

    // Single, DMA functions
    address += 8 * QSPI_PAGE_SIZE;
    initBuffer(receptionBuffer, programStringLen);
    TxCplt = 0;

    if (Cypress_QSPI_Program_DMA(&hqspi, address, programString, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (TxCplt == 0) {
    	// Wait for command to complete
    }

    TxCplt = 0;
    StatusMatch = 0;

    if (Cypress_QSPI_WaitMemReady_IT(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    while (StatusMatch == 0) {
    	// Wait for command to complete
    }

    StatusMatch = 0;
    RxCplt = 0;

    if (Cypress_QSPI_Read_DMA(&hqspi, address, receptionBuffer, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (RxCplt == 0) {
    	// Wait for command to complete
    }

    RxCplt = 0;

    if (Cypress_QSPI_CheckForErrors(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    if (!compareBuffers(programString, receptionBuffer, programStringLen)) {
        Assert_Error();
    }

    // Quad, blocking functions
    address += 8 * QSPI_PAGE_SIZE;
    initBuffer(receptionBuffer, programStringLen);

    if (Cypress_QSPI_ProgramQuad(&hqspi, address, programString, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    if (Cypress_QSPI_WaitMemReady(&hqspi, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
    	Error_Handler();
    }

    // Get new state of status register
    uint8_t statusRegisteasdf;
    if (Cypress_QSPI_ReadSR1(&hqspi, &statusRegisteasdf) != HAL_OK) {
        Error_Handler();
    }
    // Get new state of config register
    uint8_t configRegisterNewa;
    if (Cypress_QSPI_ReadCR(&hqspi, &configRegisterNewa) != HAL_OK) {
        Error_Handler();
    }


    if (Cypress_QSPI_ReadQuadAlt(&hqspi, address, receptionBuffer, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    if (!compareBuffers(programString, receptionBuffer, programStringLen)) {
        Assert_Error();
    }

    // Quad, interrupt functions
    address += 8 * QSPI_PAGE_SIZE;
    initBuffer(receptionBuffer, programStringLen);
    TxCplt = 0;

    if (Cypress_QSPI_ProgramQuad_IT(&hqspi, address, programString, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (TxCplt == 0) {
    	// Wait for command to complete
    }

    TxCplt = 0;
    StatusMatch = 0;

    if (Cypress_QSPI_WaitMemReady_IT(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    while (StatusMatch == 0) {
    	// Wait for command to complete
    }

    StatusMatch = 0;
    RxCplt = 0;

    if (Cypress_QSPI_ReadQuad_IT(&hqspi, address, receptionBuffer, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (RxCplt == 0) {
    	// Wait for command to complete
    }

    RxCplt = 0;

    if (Cypress_QSPI_CheckForErrors(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    if (!compareBuffers(programString, receptionBuffer, programStringLen)) {
        Assert_Error();
    }

    // Quad, DMA functions
    address += 8 * QSPI_PAGE_SIZE;
    initBuffer(receptionBuffer, programStringLen);
    TxCplt = 0;

    if (Cypress_QSPI_ProgramQuad_DMA(&hqspi, address, programString, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (TxCplt == 0) {
    	// Wait for command to complete
    }

    TxCplt = 0;
    StatusMatch = 0;

    if (Cypress_QSPI_WaitMemReady_IT(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    while (StatusMatch == 0) {
    	// Wait for command to complete
    }

    StatusMatch = 0;
    RxCplt = 0;

    if (Cypress_QSPI_ReadQuad_DMA(&hqspi, address, receptionBuffer, programStringLen) != HAL_OK) {
        Error_Handler();
    }

    while (RxCplt == 0) {
    	// Wait for command to complete
    }

    RxCplt = 0;

    if (Cypress_QSPI_CheckForErrors(&hqspi) != HAL_OK) {
    	Error_Handler();
    }

    if (!compareBuffers(programString, receptionBuffer, programStringLen)) {
        Assert_Error();
    }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 19;
  RCC_OscInitStruct.PLL.PLLP = 38;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOMEDIUM;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief QUADSPI Initialization Function
  * @param None
  * @retval None
  */
static void MX_QUADSPI_Init(void)
{

  /* USER CODE BEGIN QUADSPI_Init 0 */

  /* USER CODE END QUADSPI_Init 0 */

  /* USER CODE BEGIN QUADSPI_Init 1 */

  /* USER CODE END QUADSPI_Init 1 */
  /* QUADSPI parameter configuration*/
  hqspi.Instance = QUADSPI;
  hqspi.Init.ClockPrescaler = 16;
  hqspi.Init.FifoThreshold = 1;
  hqspi.Init.SampleShifting = QSPI_SAMPLE_SHIFTING_NONE;
  hqspi.Init.FlashSize = 29;
  hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_2_CYCLE;
  hqspi.Init.ClockMode = QSPI_CLOCK_MODE_0;
  hqspi.Init.FlashID = QSPI_FLASH_ID_2;
  hqspi.Init.DualFlash = QSPI_DUALFLASH_DISABLE;
  if (HAL_QSPI_Init(&hqspi) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN QUADSPI_Init 2 */

  /* USER CODE END QUADSPI_Init 2 */

}

/**
  * Enable MDMA controller clock
  */
static void MX_MDMA_Init(void)
{

  /* MDMA controller clock enable */
  __HAL_RCC_MDMA_CLK_ENABLE();
  /* Local variables */

  /* MDMA interrupt initialization */
  /* MDMA_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MDMA_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(MDMA_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_OTG_FS_PWR_EN_GPIO_Port, USB_OTG_FS_PWR_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PF3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PC1 PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA1 PA2 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : STLINK_RX_Pin STLINK_TX_Pin */
  GPIO_InitStruct.Pin = STLINK_RX_Pin|STLINK_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS_PWR_EN_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_PWR_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_OTG_FS_PWR_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OTG_FS_OVCR_Pin */
  GPIO_InitStruct.Pin = USB_OTG_FS_OVCR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OTG_FS_OVCR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA11 PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG1_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PG11 PG13 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/**
 * @brief  Command completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_CmdCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    CmdCplt++;
}

/**
 * @brief  Rx Transfer completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_RxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    RxCplt++;
}

/**
 * @brief  Tx Transfer completed callbacks.
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_TxCpltCallback(QSPI_HandleTypeDef *hqspi)
{
    TxCplt++;
}

/**
 * @brief  Status Match callbacks
 * @param  hqspi: QSPI handle
 * @retval None
 */
void HAL_QSPI_StatusMatchCallback(QSPI_HandleTypeDef *hqspi)
{
    StatusMatch++;
}

/**
* @brief Clears a buffer
*/
void initBuffer(uint8_t buf[], uint8_t len) {
    for(uint8_t i = 0; i < len; i++) {
        buf[i] = 0;
    }
}

/**
* @brief Compares two buffers
*/
uint8_t compareBuffers(uint8_t buf1[], uint8_t buf2[], uint8_t len) {
    for(uint8_t i = 0; i < len; i++) {
        if(buf1[i] != buf2[i]) {
            return 0;
        }
    }
    return 1;
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

void Assert_Error(void) {
    HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);
    __disable_irq();
    while(1) {
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    // Get SR1
    uint8_t sr, cr;
    Cypress_QSPI_ReadSR1(&hqspi, &sr);
    Cypress_QSPI_ReadCR(&hqspi, &cr);
    HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
-
