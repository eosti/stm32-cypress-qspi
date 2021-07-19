/**
* @file cypressQSPI.h
* @brief driver library for communicating with FL- series QSPI flash memory
* @author Reid Sox-Harris
*/


/*
 * cypressQSPI.h
 *
 *  Created on: Jun 21, 2021
 *      Author: reid
 */

#ifndef INC_CYPRESSQSPI_H_
#define INC_CYPRESSQSPI_H_

#ifndef STM32H7xx_HAL_H
#include "stm32h7xx_hal.h"
#endif

/* Function defines */

HAL_StatusTypeDef Cypress_QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_WriteDisable(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_ErrorRecovery(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_WaitMemReady(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_WaitMemReady_IT(QSPI_HandleTypeDef *hqspi);

HAL_StatusTypeDef Cypress_QSPI_ReadSR1(QSPI_HandleTypeDef *hqspi, uint8_t* result);
HAL_StatusTypeDef Cypress_QSPI_ReadSR2(QSPI_HandleTypeDef *hqspi, uint8_t* result);
HAL_StatusTypeDef Cypress_QSPI_ReadCR(QSPI_HandleTypeDef *hqspi, uint8_t* result);
HAL_StatusTypeDef Cypress_QSPI_ClearSR(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_CheckForErrors(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_WriteCR(QSPI_HandleTypeDef *hqspi, uint8_t cReg);
HAL_StatusTypeDef Cypress_QSPI_WriteSR1(QSPI_HandleTypeDef *hqspi, uint8_t sReg);

HAL_StatusTypeDef Cypress_QSPI_SectorErase(QSPI_HandleTypeDef *hqspi, uint32_t address);
HAL_StatusTypeDef Cypress_QSPI_SectorErase_IT(QSPI_HandleTypeDef *hqspi, uint32_t address);
HAL_StatusTypeDef Cypress_QSPI_BulkErase(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_BulkErase_IT(QSPI_HandleTypeDef *hqspi);

HAL_StatusTypeDef Cypress_QSPI_Read(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *dest, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_Read_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *dest, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_Read_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *dest, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_ReadQuad(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *dest, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_ReadQuad_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *dest, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_ReadQuad_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *dest, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_Program(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *src, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_Program_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *src, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_Program_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *src, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_ProgramQuad(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *src, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_ProgramQuad_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *src, uint32_t count);
HAL_StatusTypeDef Cypress_QSPI_ProgramQuad_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint32_t *src, uint32_t count);

HAL_StatusTypeDef Cypress_QSPI_ModeBitReset(QSPI_HandleTypeDef *hqspi);
HAL_StatusTypeDef Cypress_QSPI_Reset(QSPI_HandleTypeDef *hqspi);

// Possible additions:
// MDMA mode
// Adding whatever the continuous write mode is
// Abort functions
// Memory mapping
// Getting ID_CFI info
// Make every function have an IT alternative (maybe something with RegisterCallback so that the function can be super-duper interrupt driven, even for the WRENs?)


/* S25FL512S Info */

#define QSPI_FLASH_SIZE                      29		// Bits to represent address
#define QSPI_PAGE_SIZE                       256
//#define FLASH_SIZE                            0x4000000 /* 512 MBits => 64MBytes */
//#define SECTOR_SIZE                           0x40000   /* 256 sectors of 256KBytes */
//#define PAGE_SIZE                             0x200     /* 131072 pages of 512 bytes */

#define BULK_ERASE_MAX_TIME                   460000
#define SECTOR_ERASE_MAX_TIME                 2600

/* S25FL512S Commands */
/* Reset Operations */
#define SOFTWARE_RESET_CMD                    0xF0
#define MODE_BIT_RESET_CMD                    0xFF

/* Identification Operations */
#define READ_ID_CMD                           0x90
#define READ_ID_CMD2                          0x9F
#define READ_ELECTRONIC_SIGNATURE             0xAB
#define READ_SERIAL_FLASH_DISCO_PARAM_CMD     0x5A

/* Register Operations */
#define READ_STATUS_REG1_CMD                  0x05
#define READ_STATUS_REG2_CMD                  0x07
#define READ_CONFIGURATION_REG1_CMD           0x35
#define WRITE_STATUS_CMD_REG_CMD              0x01
#define WRITE_DISABLE_CMD                     0x04
#define WRITE_ENABLE_CMD                      0x06
#define CLEAR_STATUS_REG1_CMD                 0x30
#define READ_AUTOBOOT_REG_CMD                 0x14
#define WRITE_AUTOBOOT_REG_CMD                0x15
#define READ_BANK_REG_CMD                     0x16
#define WRITE_BANK_REG_CMD                    0x17
#define ACCESS_BANK_REG_CMD                   0xB9
#define READ_DATA_LEARNING_PATTERN_CMD        0x41
#define PGM_NV_DATA_LEARNING_REG_CMD          0x43
#define WRITE_VOL_DATA_LEARNING_REG_CMD       0x4A

/* Read Operations */
#define READ_CMD                              0x03
#define READ_4_BYTE_ADDR_CMD                  0x13

#define FAST_READ_CMD                         0x0B
#define FAST_READ_4_BYTE_ADDR_CMD             0x0C
#define FAST_READ_DDR_CMD                     0x0D
#define FAST_READ__DDR_4_BYTE_ADDR_CMD        0x0E

#define DUAL_OUT_FAST_READ_CMD                0x3B
#define DUAL_OUT_FAST_READ_4_BYTE_ADDR_CMD    0x3C

#define QUAD_OUT_FAST_READ_CMD                0x6B
#define QUAD_OUT_FAST_READ_4_BYTE_ADDR_CMD    0x6C

#define DUAL_INOUT_FAST_READ_CMD              0xBB
#define DUAL_INOUT_FAST_READ_DTR_CMD          0xBD
#define DUAL_INOUT_FAST_READ_4_BYTE_ADDR_CMD  0xBC
#define DDR_DUAL_INOUT_READ_4_BYTE_ADDR_CMD   0xBE

#define QUAD_INOUT_FAST_READ_CMD              0xEB
#define QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD  0xEC
#define QUAD_INOUT_FAST_READ_DDR_CMD          0xED
#define QUAD_INOUT_READ_DDR_4_BYTE_ADDR_CMD   0xEE


/* Program Operations */
#define PAGE_PROG_CMD                         0x02
#define PAGE_PROG_4_BYTE_ADDR_CMD             0x12

#define QUAD_IN_FAST_PROG_CMD                 0x32
#define QUAD_IN_FAST_PROG_ALTERNATE_CMD       0x38
#define QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD     0x34

#define PROGRAM_SUSPEND_CMD                   0x85
#define PROGRAM_RESUME_CMD                    0x8A

/* Erase Operations */
#define SECTOR_ERASE_CMD                      0xD8
#define SECTOR_ERASE_4_BYTE_ADDR_CMD          0xDC

#define BULK_ERASE_CMD                        0x60
#define BULK_ERASE_ALTERNATE_CMD              0xC7

#define PROG_ERASE_SUSPEND_CMD                0x75
#define PROG_ERASE_RESUME_CMD                 0x7A

/* One-Time Programmable Operations */
#define PROG_OTP_ARRAY_CMD                    0x42
#define READ_OTP_ARRAY_CMD                    0x4B

/* Advanced Sector Protection Operations */
#define READ_DYB_CMD                          0xE0
#define WRITE_DYB_CMD                         0xE1

#define READ_PPB_CMD                          0xE2
#define PROGRAM_PPB_CMD                       0xE3
#define ERASE_PPB_CMD                         0xE4

#define READ_ASP_CMD                          0x2B
#define PROGRAM_ASP_CMD                       0x2F

#define READ_PPB_LOCKBIT_CMD                  0xA7
#define WRITE_PPB_LOCKBIT_CMD                 0xA6

#define READ_PASSWORD_CMD                     0xE7
#define PROGRAM_PASSWORD_CMD                  0xE8
#define UNLOCK_PASSWORD_CMD                   0xE9

/* S25FL512S Registers */
/* Status Register-1 */
#define SR1_WIP                               ((uint8_t)0x01)      /*!< Write in progress, device busy */
#define SR1_WREN                              ((uint8_t)0x02)      /*!< Write Registers, program or commands are accepted */
#define SR1_BP0                               ((uint8_t)0x04)      /*!< Sector0 protected from Program or Erase */
#define SR1_BP1                               ((uint8_t)0x08)      /*!< Sector1 protected from Program or Erase */
#define SR1_BP2                               ((uint8_t)0x10)      /*!< Sector2 protected from Program or Erase */
#define SR1_ERERR                             ((uint8_t)0x20)      /*!< Erase error */
#define SR1_PGERR                             ((uint8_t)0x40)      /*!< Program error */
#define SR1_SRWD                              ((uint8_t)0x80)      /*!< Status Register Write Disable */

/* Status Register-2 */
#define SR2_PS                                ((uint8_t)0x01)      /*!< Program in Suspend mode */
#define SR2_ES                                ((uint8_t)0x02)      /*!< Erase Suspend Mode */

/* Configuration Register CR1 */
#define CR1_FREEZE                            ((uint8_t)0x01)      /*!< Block protection and OTP locked */
#define CR1_QUAD                              ((uint8_t)0x02)      /*!< Quad mode enable */
#define CR1_BPNV                              ((uint8_t)0x08)      /*!< BP2-0 bits of Status Reg are volatile */
#define CR1_TBPROT                            ((uint8_t)0x20)      /*!< BPstarts at bottom */
#define CR1_LC_MASK                           ((uint8_t)0xC0)      /*!< Latency Code mask */
#define CR1_LC0                               ((uint8_t)0x00)      /*!< Latency Code = 0 */
#define CR1_LC1                               ((uint8_t)0x40)      /*!< Latency Code = 1 */
#define CR1_LC2                               ((uint8_t)0x80)      /*!< Latency Code = 2 */
#define CR1_LC3                               ((uint8_t)0xC0)      /*!< Latency Code = 3 */

/* AutoBoot Register */
#define AB_EN                                 ((uint32_t)0x00000001) /*!< AutoBoot Enabled     */
#define AB_SD_MASK                            ((uint32_t)0x000001FE) /*!< AutoBoot Start Delay mask */
#define AB_SA_MASK                            ((uint32_t)0xFFFFFE00) /*!< AutoBoot Start Address mask */

/* Bank Address Register */
#define BA_BA24                               ((uint8_t)0x01)      /*!< A24 for 512 Mb device */
#define BA_BA25                               ((uint8_t)0x02)      /*!< A25 for 512 Mb device */
#define BA_EXTADD                             ((uint8_t)0x80)      /*!< 4 bytes addressing required from command */

/* ASP Register */
#define ASP_PSTMLB                            ((uint16_t)0x0002)   /*!< Persistent protection mode not permanently enabled */
#define ASP_PWSMLB                            ((uint16_t)0x0003)   /*!< Password protection mode not permanently enabled */

/* PPB Lock Register */
#define PPBLOCK                               ((uint8_t)0x01)      /*!< PPB array may be programmed or erased */


/**
 * @name		Dummy cycle configuration
 * @brief	Dummy cycles for SDR, High Performance
 * @pre		Define QSPI_DUMMY_{50 | 80 | 90 |104} based on the QSPI peripheral clock speed
 * @retval	DUMMY_CLOCK_CYCLES_READ: dummy clock cycles for READ reads
 * @retval	DUMMY_CLOCK_CYCLES_FASTREAD: dummy clock cycles for FASTREAD reads
 * @retval	DUMMY_CLOCK_CYCLES_READ_DUAL: dummy clock cycles for DUAL reads
 * @retval	DUMMY_CLOCK_CYCLES_READ_DUALIO: dummy clock cycles for DUALIO reads
 * @retval	DUMMY_CLOCK_CYCLES_READ_QUAD: dummy clock cycles for QUAD reads
 * @retval	DUMMY_CLOCK_CYCLES_READ_QUADIO: dummy clock cycles for QUADIO reads
 * @retval	DUMMY_LC: Latency code for given speed
 * @post	User must set DUMMY_LC in CR1 for any read operations
 */

#if defined(QSPI_DUMMY_50)
// Freq <= 50MHz
#define DUMMY_CLOCK_CYCLES_READ				0
#define DUMMY_CLOCK_CYCLES_FASTREAD         0
#define DUMMY_CLOCK_CYCLES_READ_DUAL		0
#define DUMMY_CLOCK_CYCLES_READ_DUALIO		4
#define DUMMY_CLOCK_CYCLES_READ_QUAD		0
#define DUMMY_CLOCK_CYCLES_READ_QUADIO      1
#define DUMMY_LC							(CR1_LC3)

#elif defined(QSPI_DUMMY_90)
// Freq <= 90MHz
#define DUMMY_CLOCK_CYCLES_READ				NULL
#define DUMMY_CLOCK_CYCLES_FASTREAD         8
#define DUMMY_CLOCK_CYCLES_READ_DUAL		8
#define DUMMY_CLOCK_CYCLES_READ_DUALIO		8
#define DUMMY_CLOCK_CYCLES_READ_QUAD		5
#define DUMMY_CLOCK_CYCLES_READ_QUADIO      4
#define DUMMY_LC							(CR1_LC1)

#elif defined(QSPI_DUMMY_104)
// Freq <= 104MHz
#define DUMMY_CLOCK_CYCLES_READ				NULL
#define DUMMY_CLOCK_CYCLES_FASTREAD         8
#define DUMMY_CLOCK_CYCLES_READ_DUAL		8
#define DUMMY_CLOCK_CYCLES_READ_DUALIO		8
#define DUMMY_CLOCK_CYCLES_READ_QUAD		6
#define DUMMY_CLOCK_CYCLES_READ_QUADIO      5
#define DUMMY_LC							(CR1_LC2)

#else
// Freq <= 80MHz, default chip configuration
#define DUMMY_CLOCK_CYCLES_READ				NULL
#define DUMMY_CLOCK_CYCLES_FASTREAD         8
#define DUMMY_CLOCK_CYCLES_READ_DUAL		8
#define DUMMY_CLOCK_CYCLES_READ_DUALIO		8
#define DUMMY_CLOCK_CYCLES_READ_QUAD		4
#define DUMMY_CLOCK_CYCLES_READ_QUADIO      4
#define DUMMY_LC							(CR1_LC0)

#endif // End QSPI_DUMMY


#endif /* INC_CYPRESSQSPI_H_ */
