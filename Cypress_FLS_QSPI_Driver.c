/**
* @file Cypress_FLS_QSPI_Driver.c
* @brief driver library for communicating with FL-S series QSPI flash memory
* @author Reid Sox-Harris
* @defgroup functions Public Functions
* @{
*/

/*
* cypressQSPI.c
*
*  Created on: Jun 21, 2021
*      Author: reid
*
*      all reads/writes assume 4 byte addresses, so ExtAdd is irrelevant
*      For most functions, there is no postfix, IT, or DMA modes
*      No postfix -> polling and therefore blocking (preferable for single operations)
*      IT -> Interrupt driven, will interrupt frequently (preferable for smaller writes, since no DMA overhead at start)
*      DMA -> Interrupt driven, but will access memory directly so only one interrupt (preferable for large writes)
*/

#include "Cypress_FLS_QSPI_Driver.h"

/**
* @brief    Enable write operations and wait until effective (blocking)
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WriteEnable(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;

    // Send WREN
    sCommand.Instruction        = WRITE_ENABLE_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;


    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Disable write operations, terminating any current operation
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WriteDisable(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = WRITE_DISABLE_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Check for program/erase error, and then recover by clearing the SR and disabling the write
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ErrorRecovery(QSPI_HandleTypeDef *hqspi)
{
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_ERROR)
    {
        // If no failure, then no error to recover from
        return HAL_ERROR;
    }

    // To recover, we clear the SR and disable the write
    if  (Cypress_QSPI_ClearSR(hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if  (Cypress_QSPI_WriteDisable(hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Polls the SR until the WIP bit is unset (blocking)
* @param    hqspi: QSPI handle
* @param    timeout: Time to wait before erroring out
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WaitMemReady(QSPI_HandleTypeDef *hqspi, uint32_t timeout)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    // Read SR1
    sCommand.Instruction        = READ_STATUS_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }


    // Look for bit zero, Write In Progress, to become 0 -> device ready
    sConfig.Match               = 0x00;
    sConfig.Mask                = SR1_WIP;
    sConfig.Interval            = 0x10;
    sConfig.StatusBytesSize     = 1;
    sConfig.MatchMode           = QSPI_MATCH_MODE_AND;
    sConfig.AutomaticStop       = QSPI_AUTOMATIC_STOP_ENABLE;

    // Keep checking until bit set
    if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, timeout) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Polls the SR until the WIP bit is unset (non-blocking, requires interrupts)
* @param    hqspi: QSPI handle
* @return   HAL status
* @remark   Calls HAL_QSPI_StatusMatchCallback when complete as an interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_WaitMemReady_IT(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    // Read SR1
    sCommand.Instruction        = READ_STATUS_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }


    // Look for bit zero, Write In Progress, to become 0 -> device ready
    sConfig.Match               = 0x00;
    sConfig.Mask                = SR1_WIP;
    sConfig.MatchMode           = QSPI_MATCH_MODE_AND;
    sConfig.StatusBytesSize     = 1;
    sConfig.Interval            = 0x10;
    sConfig.AutomaticStop       = QSPI_AUTOMATIC_STOP_ENABLE;

    // This will call HAL_QSPI_StatusMatchCallback when complete
    if (HAL_QSPI_AutoPolling_IT(hqspi, &sCommand, &sConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Polls the SR until the WREN bit is set (blocking)
* @param    hqspi: QSPI handle
* @param    timeout: Time to wait before erroring out
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WaitWriteReady(QSPI_HandleTypeDef *hqspi, uint32_t timeout)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    // Read SR1
    sCommand.Instruction        = READ_STATUS_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }


    // Look for bit one, Write Enable, to become 1 -> device ready for write
    sConfig.Match               = 0x02;
    sConfig.Mask                = SR1_WREN;
    sConfig.Interval            = 0x10;
    sConfig.StatusBytesSize     = 1;
    sConfig.MatchMode           = QSPI_MATCH_MODE_AND;
    sConfig.AutomaticStop       = QSPI_AUTOMATIC_STOP_ENABLE;

    // Keep checking until bit set
    if (HAL_QSPI_AutoPolling(hqspi, &sCommand, &sConfig, timeout) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Polls the SR until the WREN bit is set (non-blocking, requires interrupts)
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WaitWriteReady_IT(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;
    QSPI_AutoPollingTypeDef sConfig;

    // Read SR1
    sCommand.Instruction        = READ_STATUS_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }


    // Look for bit one, Write Enable, to become 1 -> device ready for write
    sConfig.Match               = 0x02;
    sConfig.Mask                = SR1_WREN;
    sConfig.Interval            = 0x10;
    sConfig.StatusBytesSize     = 1;
    sConfig.MatchMode           = QSPI_MATCH_MODE_AND;
    sConfig.AutomaticStop       = QSPI_AUTOMATIC_STOP_ENABLE;

    // This will call HAL_QSPI_StatusMatchCallback when complete
    if (HAL_QSPI_AutoPolling_IT(hqspi, &sCommand, &sConfig) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads SR1 to a variable
* @param    hqspi: QSPI handle
* @param    result: Location to store SR1
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ReadSR1(QSPI_HandleTypeDef *hqspi, uint8_t *result)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = READ_STATUS_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 1;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(hqspi, result, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads SR2 to a variable
* @param    hqspi: QSPI handle
* @param    result: Location to store SR2
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ReadSR2(QSPI_HandleTypeDef *hqspi, uint8_t *result)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = READ_STATUS_REG2_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 1;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(hqspi, result, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads the CR to a variable
* @param    hqspi: QSPI handle
* @param    result: Location to store the CR
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ReadCR(QSPI_HandleTypeDef *hqspi, uint8_t *result)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = READ_CONFIGURATION_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 1;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(hqspi, result, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Clears the Erase Fail and Program Fail flags
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ClearSR(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = CLEAR_STATUS_REG1_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Checks SR1 for program/erase errors
* @param    hqspi: QSPI handle
* @return   HAL status
* @remark   Returns HAL_OK if no errors, HAL_ERROR if errors
* @post If error, call \ref Cypress_QSPI_ErrorRecovery
*/

HAL_StatusTypeDef Cypress_QSPI_CheckForErrors(QSPI_HandleTypeDef *hqspi)
{
    uint8_t statusRegister;
    if  (Cypress_QSPI_ReadSR1(hqspi, &statusRegister) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if  ((statusRegister & SR1_ERERR) || (statusRegister & SR1_PGERR))
    {
        // Either there was a program error or erase error, so return error!
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Writes to SR1
* @param    hqspi: QSPI handle
* @param    sReg: status to write
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WriteSR1(QSPI_HandleTypeDef *hqspi, uint8_t sReg)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = WRITE_STATUS_CMD_REG_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 1;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Transmit(hqspi, &sReg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Verify no errors occurred during the write
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Writes to CR
* @param    hqspi: QSPI handle
* @param    cReg: configuration to write
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_WriteCR(QSPI_HandleTypeDef *hqspi, uint8_t cReg)
{
    // Unfortunately, the only way to write the CR is to write SR1 first
    // So we first get the contents of SR1 as to not overwrite anything
    uint8_t sReg;
    Cypress_QSPI_ReadSR1(hqspi, &sReg);

    // Create the payload
    uint8_t payload[] = { sReg, cReg };

    Cypress_QSPI_WriteEnable(hqspi);

    // Send configuration
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = WRITE_STATUS_CMD_REG_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 2;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Transmit(hqspi, payload, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Verify no errors occured during the write
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Sets all bits in a sector to 1 (blocking)
* @param    hqspi: QSPI handle
* @param    address: address within the sector to erase
* @return   HAL status
* @post If error, call Cypress_QSPI_ErrorRecovery to clear errors
*/

HAL_StatusTypeDef Cypress_QSPI_SectorErase(QSPI_HandleTypeDef *hqspi, uint32_t address)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = SECTOR_ERASE_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Wait for the erase to complete
    if  (Cypress_QSPI_WaitMemReady(hqspi, SECTOR_ERASE_MAX_TIME) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Verify the erase worked properly
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_OK)
    {
        // If failed, SR1 should be cleared to recover
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Sets all bits in a sector to 1 (non-blocking, requires interrupts)
* @param    hqspi: QSPI handle
* @param    address: address within the sector to erase
* @return   HAL status
* @post User should verify that no errors occurred after erase
* @remark   Calls HAL_QSPI_StatusMatchCallback when complete via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_SectorErase_IT(QSPI_HandleTypeDef *hqspi, uint32_t address)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = SECTOR_ERASE_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_StatusMatchCallback when complete
    if  (Cypress_QSPI_WaitMemReady_IT(hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // User should verify that no errors were raised after the callback occurs
    return HAL_OK;
}

/**
* @brief    Sets *all* bits in the flash memory to 1 (blocking)
* @param    hqspi: QSPI handle
* @return   HAL status
* @post If error, call Cypress_QSPI_ErrorRecovery to clear errors
*/

HAL_StatusTypeDef Cypress_QSPI_BulkErase(QSPI_HandleTypeDef *hqspi)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = BULK_ERASE_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if  (Cypress_QSPI_WaitMemReady(hqspi, BULK_ERASE_MAX_TIME))

    // If any block protection bits are set, this will fail!
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_OK)
    {
        // If failed, SR1 should be cleared to recover
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Sets *all* bits in the flash memory to 1 (non-blocking, requires interrupts)
* @param    hqspi: QSPI handle
* @return   HAL status
* @post User should verify that no errors occurred after erase
* @remark   Calls HAL_QSPI_StatusMatchCallback when complete via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_BulkErase_IT(QSPI_HandleTypeDef *hqspi)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = BULK_ERASE_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_StatusMatchCallback when complete
    if  (Cypress_QSPI_WaitMemReady_IT(hqspi) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // User should verify that no errors were raised after the callback occurs
    // e.g. if any block protection is set, this will fail
    return HAL_OK;
}

/**
* @brief    Reads data into memory in SPI mode (blocking)
* @param    hqspi: QSPI handle
* @param    address: starting address to read
* @param    dest: pointer to memory destination
* @param    count: bytes to read
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_Read(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *dest, uint32_t count)
{
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = READ_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(hqspi, dest, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads data into memory in SPI mode (non-blocking, requires callbacks)
* @param    hqspi: QSPI handle
* @param    address: starting address to read
* @param    dest: pointer to memory destination
* @param    count: bytes to read
* @return   HAL status
* @remark   Calls HAL_QSPI_RxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_Read_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *dest, uint32_t count)
{
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = READ_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Will call HAL_QSPI_RxCpltCallback on completion
    if (HAL_QSPI_Receive_IT(hqspi, dest) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads data directly into memory in SPI mode (non-blocking, requires callbacks)
* @param    hqspi: QSPI handle
* @param    address: starting address to read
* @param    dest: pointer to memory destination
* @param    count: bytes to read
* @return   HAL status
* @remark   Calls HAL_QSPI_RxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_Read_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *dest, uint32_t count)
{
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = READ_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Will call HAL_QSPI_RxCpltCallback on completion
    if (HAL_QSPI_Receive_DMA(hqspi, dest) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads data into memory using QSPI (blocking)
* @pre  CR1 must have CR1_QUAD set (0x02) to enable quad mode
* @pre      The latency codes must be set appropriately, \ref QSPI_DUMMY
* @param    hqspi: QSPI handle
* @param    address: starting address to read
* @param    dest: pointer to memory destination
* @param    count: bytes to read
* @return   HAL status
* @note     this took me like 3 days to make work: it sends a mode byte as an alternate byte that is super-duper required
*/

HAL_StatusTypeDef Cypress_QSPI_ReadQuad(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *dest, uint32_t count)
{
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = CYPRESS_DUMMY_CLOCK_CYCLES_READ_QUADIO;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_4_LINES;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.DataMode           = QSPI_DATA_4_LINES;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Receive(hqspi, dest, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads data into memory using QSPI (nonblocking, requires callbacks)
* @pre  CR1 must have CR1_QUAD set (0x02) to enable quad mode
* @param    hqspi: QSPI handle
* @param    address: starting address to read
* @param    dest: pointer to memory destination
* @param    count: bytes to read
* @return   HAL status
* @remark   Calls HAL_QSPI_RxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_ReadQuad_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *dest, uint32_t count)
{
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = CYPRESS_DUMMY_CLOCK_CYCLES_READ_QUADIO;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_4_LINES;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.DataMode           = QSPI_DATA_4_LINES;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_RxCpltCallback when complete
    if (HAL_QSPI_Receive_IT(hqspi, dest) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Reads data directly into memory using QSPI (nonblocking, requires callbacks)
* @pre  CR1 must have CR1_QUAD set (0x02) to enable quad mode
* @param    hqspi: QSPI handle
* @param    address: starting address to read
* @param    dest: pointer to memory destination
* @param    count: bytes to read
* @return   HAL status
* @remark   Calls HAL_QSPI_RxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_ReadQuad_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *dest, uint32_t count)
{
    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = QUAD_INOUT_FAST_READ_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = CYPRESS_DUMMY_CLOCK_CYCLES_READ_QUADIO;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_4_LINES;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_4_LINES;
    sCommand.DataMode           = QSPI_DATA_4_LINES;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_RxCpltCallback when complete
    if (HAL_QSPI_Receive_DMA(hqspi, dest) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Writes data into a page in SPI mode (blocking)
* @param    hqspi: QSPI handle
* @param    address: page to write
* @param    src: pointer to data to write
* @param    count: bytes to write
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_Program(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *src, uint32_t count)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = PAGE_PROG_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Transmit(hqspi, src, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Verify no errors occured during the write
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Writes data into a page in SPI mode (nonblocking, requires callbacks)
* @param    hqspi: QSPI handle
* @param    address: page to write
* @param    src: pointer to data to write
* @param    count: bytes to write
* @return   HAL status
* @post     User should verify that no errors were raised after the write
* @remark   Calls HAL_QSPI_TxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_Program_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *src, uint32_t count)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = PAGE_PROG_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_TxCpltCallback when complete
    if (HAL_QSPI_Transmit_IT(hqspi, src) != HAL_OK)
    {
        return HAL_ERROR;
    }
    // User should verify after the callback that no errors were raised

    return HAL_OK;
}

/**
* @brief    Writes data into a page in SPI mode (nonblocking, requires callbacks)
* @param    hqspi: QSPI handle
* @param    address: page to write
* @param    src: pointer to data to write
* @param    count: bytes to write
* @return   HAL status
* @post     User should verify that no errors were raised after the write
* @remark   Calls HAL_QSPI_TxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_Program_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *src, uint32_t count)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = PAGE_PROG_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_TxCpltCallback when complete
    if (HAL_QSPI_Transmit_DMA(hqspi, src) != HAL_OK)
    {
        return HAL_ERROR;
    }
    // User should verify after the callback that no errors were raised

    return HAL_OK;
}

/**
* @brief    Writes data into a page using QSPI (blocking)
* @pre  CR1 must have CR1_QUAD set (0x02) to enable quad mode
* @param    hqspi: QSPI handle
* @param    address: page to write
* @param    src: pointer to data to write
* @param    count: bytes to write
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ProgramQuad(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *src, uint32_t count)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_4_LINES;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Transmit(hqspi, src, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // Verify no errors occured during the write
    if  (Cypress_QSPI_CheckForErrors(hqspi) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Writes data into a page using QSPI (non-blocking, requires callbacks)
* @pre  CR1 must have CR1_QUAD set (0x02) to enable quad mode
* @param    hqspi: QSPI handle
* @param    address: page to write
* @param    src: pointer to data to write
* @param    count: bytes to write
* @return   HAL status
* @post     User should verify that no errors were raised after the write
* @remark   Calls HAL_QSPI_TxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_ProgramQuad_IT(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *src, uint32_t count)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_4_LINES;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_TxCpltCallback when complete
    if (HAL_QSPI_Transmit_IT(hqspi, src) != HAL_OK)
    {
        return HAL_ERROR;
    }
    // User should verify after the callback that no errors were raised

    return HAL_OK;
}

/**
* @brief    Writes data into a page using QSPI (non-blocking, requires callbacks)
* @pre  CR1 must have CR1_QUAD set (0x02) to enable quad mode
* @param    hqspi: QSPI handle
* @param    address: page to write
* @param    src: pointer to data to write
* @param    count: bytes to write
* @return   HAL status
* @post     User should verify that no errors were raised after the write
* @remark   Calls HAL_QSPI_TxCpltCallback on completion via interrupt
*/

HAL_StatusTypeDef Cypress_QSPI_ProgramQuad_DMA(QSPI_HandleTypeDef *hqspi, uint32_t address, uint8_t *src, uint32_t count)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;

    sCommand.Instruction        = QUAD_IN_FAST_PROG_4_BYTE_ADDR_CMD;
    sCommand.Address            = address;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_1_LINE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_4_LINES;
    sCommand.NbData             = count;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if  (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    // This will call HAL_QSPI_TxCpltCallback when complete
    if (HAL_QSPI_Transmit_DMA(hqspi, src) != HAL_OK)
    {
        return HAL_ERROR;
    }
    // User should verify after the callback that no errors were raised

    return HAL_OK;
}

/**
* @brief    Resets device to power-up state
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_Reset(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = SOFTWARE_RESET_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Forces device into normal mode after continuous high performance read mode
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ModeBitReset(QSPI_HandleTypeDef *hqspi)
{
    QSPI_CommandTypeDef     sCommand;

    sCommand.Instruction        = MODE_BIT_RESET_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_NONE;
    sCommand.NbData             = 0;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
* @brief    Sets all configuration to default
* @warn This will unset the QUAD bit and may result in write protection being set; see \ref Cypress_QSPI_DisableWP
* @param    hqspi: QSPI handle
* @return   HAL status
*/

HAL_StatusTypeDef Cypress_QSPI_ResetConfiguration(QSPI_HandleTypeDef *hqspi)
{
    Cypress_QSPI_WriteEnable(hqspi);

    QSPI_CommandTypeDef sCommand;
    uint8_t defaultConfig[] = { 0U, 0U };

    sCommand.Instruction        = WRITE_STATUS_CMD_REG_CMD;
    sCommand.Address            = 0;
    sCommand.AlternateBytes     = 0;
    sCommand.AddressSize        = QSPI_ADDRESS_32_BITS;
    sCommand.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    sCommand.DummyCycles        = 0;
    sCommand.InstructionMode    = QSPI_INSTRUCTION_1_LINE;
    sCommand.AddressMode        = QSPI_ADDRESS_NONE;
    sCommand.AlternateByteMode  = QSPI_ALTERNATE_BYTES_NONE;
    sCommand.DataMode           = QSPI_DATA_1_LINE;
    sCommand.NbData             = 2;
    sCommand.DdrMode            = QSPI_DDR_MODE_DISABLE;
    sCommand.DdrHoldHalfCycle   = QSPI_DDR_HHC_ANALOG_DELAY;
    sCommand.SIOOMode           = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(hqspi, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_QSPI_Transmit(hqspi, defaultConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
    {
        return HAL_ERROR;
    }

    Cypress_QSPI_WaitMemReady(hqspi, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    return HAL_OK;
}

/**
* @brief    Disables Write Protection
* @note When the QUAD bit is not set, IO2/WP acts as a write protect.
* @note When IO2/WP is low, it will disable programs, writes, and modifying the registers.
* @note The suggested use of this function is to use it to enable the QUAD bit, therefore disabling the WP input, and then restoring the pin using \ref Cypress_QSPI_ResetWP
* @pre      Set the User Label of pin IO2 to QUADSPI_WRITEPROT in the configurator
* @param    GPIO_Port: QUADSPI_WRITEPROT_GPIO_Port (if User Label set)
* @param    GPIO_Pin: QUADSPI_WRITEPROT_Pin (if User Label set)
* @warning  This will \b disable QSPI functionality. However, SPI and configuration operations (single data line, single address line) are available.
* @warning To re-enable QSPI functionality, use \ref Cypress_QSPI_ResetWP
*/

void Cypress_QSPI_DisableWP(GPIO_TypeDef *GPIO_Port, uint32_t GPIO_Pin)
{
    HAL_GPIO_DeInit(GPIO_Port, GPIO_Pin);
    // Re-init IO2/WP as standard GPIO out
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = 0;
    HAL_GPIO_Init(GPIO_Port, &GPIO_InitStruct);

    // Set high -> write protection disabled
    HAL_GPIO_WritePin(GPIO_Port, GPIO_Pin, GPIO_PIN_SET);
}

/**
* @brief    Resets write protection pin
* @note Used to restore \ref Cypress_QSPI_DisableWP
* @note This will re-enable QSPI operations
* @pre      Set the User Label of pin IO2 to QUADSPI_WRITEPROT in the configurator
* @param    GPIO_Port: QUADSPI_WRITEPROT_GPIO_Port (if User Label set)
* @param    GPIO_Pin: QUADSPI_WRITEPROT_Pin (if User Label set)
*/

void Cypress_QSPI_ResetWP(GPIO_TypeDef *GPIO_Port, uint32_t GPIO_Pin)
{
    HAL_GPIO_DeInit(GPIO_Port, GPIO_Pin);
    // Re-init IO2/WP back to QSPI operations
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
    HAL_GPIO_Init(GPIO_Port, &GPIO_InitStruct);

//    __HAL_RCC_QSPI_FORCE_RESET();
    //    __HAL_RCC_QSPI_RELEASE_RESET();


}

/** @} */

