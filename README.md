# STM32 Cypress QSPI
A library for communicating with Cypress FL- series QSPI flash memory chips

## Overview
This library offers a number of helper functions to simplify communications by black-boxing the QSPI HAL function.
All functions return type `HAL_StatusTypeDef` which will typically be either `HAL_OK` when the function succeeded, or `HAL_ERROR` if any error was encountered.

All functions have a **blocking** mode, where the functions will wait for the operation to complete. 
While acceptable for shorter operations, it is not preferred for longer operations as no other functions can be run at the same time.

Most functions have an **interrupt** (or IT) mode, where the function will offload operations to the QSPI peripheral. 
This allows the QSPI peripheral to run operations and interrupt only when more data is needed, so that the processor can do other things in the interim.
Once the operation is completed, the QSPI peripheral will trigger an user-defined interrupt using the NVIC interface.
This is preferred for any longer operations, such as reads, writes, or erases.

A few functions have a **direct memory access** (or DMA) mode, where the QSPI peripheral is granted direct access to system memory.
This means that the QSPI peripheral no longer needs to trigger an interrupt every time it needs more data.
However, due to the overhead associated with configuring DMA, this mode is only advantageous in longer reads or writes.
Like IT mode, the QSPI peripheral will trigger an user-defined interrupt once the operation is completed.

For any function using interrupts, the user should verify that no errors occurred during the operation, typically by using \ref Cypress_QSPI_CheckForErrors.

All documentation is generated by Doxygen, and is hosted [here](eosti.github.io/stm32-cypress-qspi).

## Usage
Use STM32CubeIDE to enable the QUADSPI module. 
Configure the peripheral according to the specs of the flash memory.
The most important changes are the flash size (edit to the number of bits in the address space, e.g. a 512 Mb flash will take 29 bits to address) and to enable the QUADSPI global interrupt if IT or DMA modes are used.
Additionally, register callbacks must be enabled, under Project Manager > Advanced Settings > Register Callback > QUADSPI > Enable. 

After generating the code, there will be a `QSPI_HandleTypeDef hqspi;`, which is the handle that you will pass to all functions.

## Compatibility
The target controller must have a hardware QSPI peripheral. 
This code was tested using an STM32H7 MCU, but many other families have the QSPI peripheral.

The flash memory must be from the Cypress FL- series, and must have QSPI capabilities.
This code was tested using the S25FL512S chip, but many other models are compatible. 

## Example
A example using this library is included for reference, based on the HAL QSPI examples built into STM32CubeIDE.
A new project should be created, and then the example code can be copied into `main.c` and `main.h`, and the library files copied into the correct location.

## Issues
Any issues, comments, complaints, suggestions? File an issue! Or, better yet, submit a PR!