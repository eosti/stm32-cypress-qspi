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

## Functions
This is a non-exhaustive list of functions included in this library.
Parameters and specifics can be found within the doxygen comments.

### Cypress_QSPI_Read, Cypress_QSPI_ReadQuad
These functions allow for reading from the external flash memory, in either blocking, IT, or DMA modes.
The user passes the starting address and number of sequential bytes to read.

### Cypress_QSPI_Program, Cypress_QSPI_ProgramQuad
These functions allow for programming the external flash memory, in either blocking, IT, or DMA modes. 
The user passes the starting address and number of sequential bytes to write, as well as a pointer to the data to write.

### Cypress_QSPI_SectorErase, Cypress_QSPI_BulkErase
These functions allow for erasing a single sector, or the entire flash memory.
These can be operated in both blocking and IT modes.

### Configuration functions
Various functions exist to read and write the configuration of the flash memory. 
All functions are blocking only.
The available operations include reading SR1, SR2, and CR, writing SR1 and CR, and enabling or disabling the write mode.

## Example
A example using this library is included for reference, based on the HAL QSPI examples built into STM32CubeIDE.
A new project should be created, and then the example code can be copied into `main.c` and `main.h`.

## Issues
Any issues, comments, complaints, suggestions? File an issue! Or, better yet, submit a PR!
