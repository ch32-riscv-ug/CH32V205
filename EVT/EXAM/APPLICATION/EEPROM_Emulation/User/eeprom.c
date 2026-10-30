/********************************** (C) COPYRIGHT *******************************
 * File Name          : eeprom.c
 * Author             : WCH
 * Version            : V1.0.0
 * Date               : 2026/10/27
 * Description        : eeprom functions.
 *********************************************************************************
 * Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
 * Attention: This software (modified or not) and binary are used for
 * microcontroller manufactured by Nanjing Qinheng Microelectronics.
 *******************************************************************************/
#include "debug.h"
#include "eeprom.h"

#pragma GCC push_options
#pragma GCC optimize("no-jump-tables")

uint16_t VirtAddVarTab[NB_VAR] = {0x5a00, 0x5a01, 0x5a02};
uint16_t VarDataTab[NB_VAR] = {0, 0, 0};
uint16_t VarValue, VarDataTmp = 0;
uint16_t DataVar = 0;

/* Private functions ---------------------------------------------------------*/
static FLASH_Status EEPROM_Format (void);
static FLASH_Status EEPROM_SetPageState (uint32_t PageAddress, uint16_t PageState);
static uint16_t EEPROM_FindValidPage (uint8_t Operation);
static uint16_t EEPROM_VerifyPageFullWriteVariable (uint16_t VirtAddress, uint16_t Data);
static uint16_t EEPROM_PageTransfer (uint16_t VirtAddress, uint16_t Data);
static uint16_t EEPROM_VerifyPageFullyErased (uint32_t Address);


/*********************************************************************
 * @fn      EEPROM_FindValidPage
 *
 * @brief   Find valid Page for write or read operation.
 *
 * @param   Operation - READ_VALID or WRITE_VALID.
 *
 * @return  uint16_t - Valid page number or NO_VALID_PAGE.
 */
static uint16_t EEPROM_FindValidPage (uint8_t Operation) {
    uint16_t PageStatus0 = 0xFFFF, PageStatus1 = 0xFFFF;

    /* Get Page0 status */
    PageStatus0 = (*(__IO uint16_t *)EEPROM_PART0_BASE_ADDRESS);

    /* Get Page1 status */
    PageStatus1 = (*(__IO uint16_t *)EEPROM_PART1_BASE_ADDRESS);

    /* Write or read operation */
    switch (Operation) {
    case WRITE_VALID: /* Write operation */
        if (PageStatus1 == PAGE_STATE_VALID) {
            /* Page0 receiving data */
            if (PageStatus0 == PAGE_STATE_RECEIVE) {
                return PAGE0; /* Page0 valid */
            } else {
                return PAGE1; /* Page1 valid */
            }
        } else if (PageStatus0 == PAGE_STATE_VALID) {
            /* Page1 receiving data */
            if (PageStatus1 == PAGE_STATE_RECEIVE) {
                return PAGE1; /* Page1 valid */
            } else {
                return PAGE0; /* Page0 valid */
            }
        } else {
            return NO_VALID_PAGE; /* No valid Page */
        }

    case READ_VALID:              /* Read operation */
        if (PageStatus0 == PAGE_STATE_VALID) {
            return PAGE0;         /* Page0 valid */
        } else if (PageStatus1 == PAGE_STATE_VALID) {
            return PAGE1;         /* Page1 valid */
        } else {
            return NO_VALID_PAGE; /* No valid Page */
        }

    default:
        return PAGE0; /* Page0 valid */
    }
}


/*********************************************************************
 * @fn      EEPROM_VerifyPageFullWriteVariable
 *
 * @brief   Verify if active page is full and writes variable in EEPROM.
 *
 * @param   VirtAddress - Variable virtual address.
 *          Data - Variable data to be written.
 *
 * @return  uint16_t - Flash operation status.
 */
static uint16_t EEPROM_VerifyPageFullWriteVariable (uint16_t VirtAddress, uint16_t Data) {
    FLASH_Status FlashStatus = FLASH_COMPLETE;
    uint16_t ValidPage = PAGE0, RecIdx = 0;
    uint32_t Address = EEPROM_START_ADDRESS, PageEndAddress = EEPROM_START_ADDRESS + EEPROM_PAGE_SIZE;
    uint32_t RecPageAddress = EEPROM_START_ADDRESS;
    uint32_t RecBuf[FLASH_PAGE_SIZE / 4];

    /* Get valid Page for write operation */
    ValidPage = EEPROM_FindValidPage (WRITE_VALID);

    /* Check if there is no valid page */
    if (ValidPage == NO_VALID_PAGE) {
        return NO_VALID_PAGE;
    }

    /* Get the valid Page start Address, the first page of the valid page stores the page state */
    Address = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * EEPROM_PAGE_SIZE) + FLASH_PAGE_SIZE);

    /* Get the valid Page end Address */
    PageEndAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)((ValidPage + 1) * EEPROM_PAGE_SIZE));

    /* Check each record starting from beginning */
    while (Address < PageEndAddress) {
        /* Verify if the record contents are erased */
        if ((*(__IO uint32_t *)Address) == ((((uint32_t)PAGE_STATE_EMPTY) <<16) | (uint32_t)PAGE_STATE_EMPTY)) {
            /* Get the page which contains the erased record */
            RecPageAddress = (uint32_t)(Address & (~((uint32_t)FLASH_PAGE_SIZE - 1)));

            /* Read the whole page, the records which are already in this page must be kept */
            for (RecIdx = 0; RecIdx < (FLASH_PAGE_SIZE / 4); RecIdx++) {
                RecBuf[RecIdx] = (*(__IO uint32_t *)(RecPageAddress + 4 * (uint32_t)RecIdx));
            }

            /* Set the new record, the low half word is the variable data, the high half word is the virtual address */
            RecBuf[(Address - RecPageAddress) / EEPROM_RECORD_SIZE] = (((uint32_t)VirtAddress <<16) | (uint32_t)Data);

            /* Fast program/erase works by a whole page, erase the record page first */
            FlashStatus = FLASH_ROM_ERASE (RecPageAddress, FLASH_PAGE_SIZE);

            /* If erase operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return (uint16_t)FlashStatus;
            }

            /* Program the whole record page */
            FlashStatus = FLASH_ROM_WRITE (RecPageAddress, RecBuf, FLASH_PAGE_SIZE);

            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return (uint16_t)FlashStatus;
            }

            /* Return program operation status */
            return (uint16_t)FlashStatus;
        } else {
            /* Next record location */
            Address = Address + EEPROM_RECORD_SIZE;
        }
    }

    /* Return PAGE_FULL in case the valid page is full */
    return PAGE_FULL;
}


/*********************************************************************
 * @fn      EEPROM_VerifyPageFullyErased
 *
 * @brief   Verify if specified page is fully erased.
 *
 * @param   Address - Page base address to be verified.
 *
 * @return  uint16_t - (0: Page not erased, 1: Page erased).
 */
static uint16_t EEPROM_VerifyPageFullyErased (uint32_t Address) {
    uint32_t EndAddress;
    uint32_t ReadStatus = 1;
    uint32_t AddressValue = 0x55555555;

    /* end-address */
    EndAddress = (uint32_t)(Address + (EEPROM_PAGE_SIZE - 4U));

    /* Check each active page address starting from end */
    while (Address <= EndAddress) {
        /* Get the current location content to be compared with virtual address */
        AddressValue = (*(__IO uint32_t *)Address);

        /* Compare the read address with the virtual address */
        if (AddressValue != (((uint32_t)PAGE_STATE_EMPTY) | (((uint32_t)PAGE_STATE_EMPTY) << 16))) {

            /* In case variable value is read, reset ReadStatus flag */
            ReadStatus = 0;

            break;
        }
        /* Next address location */
        Address = Address + 4;
    }

    /* Return ReadStatus value: (0: Page not erased, 1: Sector erased) */
    return ReadStatus;
}


/*********************************************************************
 * @fn      EEPROM_SetPageState
 *
 * @brief   Writes the state of the specified EEPROM page, the page state is
 *          stored at the beginning of the first FLASH_PAGE_SIZE page.
 *
 * @param   PageAddress - Base address of the EEPROM page.
 *          PageState - Page state to be written: PAGE_STATE_VALID or PAGE_STATE_RECEIVE.
 *
 * @return  FLASH_Status - Flash operation status.
 */
static FLASH_Status EEPROM_SetPageState (uint32_t PageAddress, uint16_t PageState) {
    FLASH_Status FlashStatus = FLASH_COMPLETE;
    uint16_t StateIdx = 0;
    uint32_t StateBuf[FLASH_PAGE_SIZE / 4];

    /* Build the state page, the state is stored at the beginning of the page */
    for (StateIdx = 0; StateIdx < (FLASH_PAGE_SIZE / 4); StateIdx++) {
        StateBuf[StateIdx] = ((((uint32_t)PAGE_STATE_EMPTY) <<16) | (uint32_t)PAGE_STATE_EMPTY);
    }
    StateBuf[0] = (uint32_t)PageState;

    /* Fast program/erase works by a whole page, erase the state page first */
    FlashStatus = FLASH_ROM_ERASE (PageAddress, FLASH_PAGE_SIZE);

    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE) {
        return FlashStatus;
    }

    /* Program the whole state page */
    FlashStatus = FLASH_ROM_WRITE (PageAddress, StateBuf, FLASH_PAGE_SIZE);

    /* Return last operation flash status */
    return FlashStatus;
}


/*********************************************************************
 * @fn      EEPROM_Format
 *
 * @brief   Erases PAGE0 and PAGE1 and writes VALID_PAGE header to PAGE0.
 *
 * @return  FLASH_Status - Flash operation status.
 */
static FLASH_Status EEPROM_Format (void) {
    FLASH_Status FlashStatus = FLASH_COMPLETE;

    uint32_t valid = 0x0000;

    /* Erase Page0 */
    if (!EEPROM_VerifyPageFullyErased (EEPROM_PART0_BASE_ADDRESS)) {
        FlashStatus = FLASH_ROM_ERASE (EEPROM_PART0_BASE_ADDRESS, EEPROM_PAGE_SIZE);

        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE) {
            return FlashStatus;
        }
    }

    /* Set Page0 as valid page: Write VALID_PAGE at Page0 base address */
    FlashStatus = EEPROM_SetPageState (EEPROM_PART0_BASE_ADDRESS, (uint16_t)valid);

    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE) {
        return FlashStatus;
    }

    /* Erase Page1 */
    if (!EEPROM_VerifyPageFullyErased (EEPROM_PART1_BASE_ADDRESS)) {

        FlashStatus = FLASH_ROM_ERASE (EEPROM_PART1_BASE_ADDRESS, EEPROM_PAGE_SIZE);
        /* If erase operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE) {
            return FlashStatus;
        }
    }

    return FLASH_COMPLETE;
}


/*********************************************************************
 * @fn      EEPROM_PageTransfer
 *
 * @brief   Transfers last updated variables data from the full Page to an empty one.
 *
 * @param   VirtAddress - Variable virtual address.
 *          Data - Variable data to be written.
 *
 * @return  uint16_t - Flash operation status.
 */
static uint16_t EEPROM_PageTransfer (uint16_t VirtAddress, uint16_t Data) {
    FLASH_Status FlashStatus = FLASH_COMPLETE;
    uint32_t NewPageAddress = EEPROM_START_ADDRESS;
    uint16_t OldPageId = 0;
    uint16_t ValidPage = PAGE0, VarIdx = 0;
    uint16_t EepromStatus = 0, ReadStatus = 0;

    uint32_t valid = 0x0000;
    uint32_t receive = PAGE_STATE_RECEIVE;

    /* Get active Page for read operation */
    ValidPage = EEPROM_FindValidPage (READ_VALID);

    if (ValidPage == PAGE1) /* Page1 valid */
    {
        /* New page address where variable will be moved to */
        NewPageAddress = EEPROM_PART0_BASE_ADDRESS;

        /* Old page ID where variable will be taken from */
        OldPageId = PAGE1;
    } else if (ValidPage == PAGE0) /* Page0 valid */
    {
        /* New page address  where variable will be moved to */
        NewPageAddress = EEPROM_PART1_BASE_ADDRESS;

        /* Old page ID where variable will be taken from */
        OldPageId = PAGE0;
    } else {
        return NO_VALID_PAGE; /* No valid Page */
    }

    /* Set the new Page status to RECEIVE_DATA status */

    FlashStatus = EEPROM_SetPageState (NewPageAddress, (uint16_t)receive);

    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE) {
        return FlashStatus;
    }

    /* Write the variable passed as parameter in the new active page */
    EepromStatus = EEPROM_VerifyPageFullWriteVariable (VirtAddress, Data);
    /* If program operation was failed, a Flash error code is returned */
    if (EepromStatus != FLASH_COMPLETE) {
        return EepromStatus;
    }

    /* Transfer process: transfer variables from old to the new active page */
    for (VarIdx = 0; VarIdx < NB_VAR; VarIdx++) {
        if (VirtAddVarTab[VarIdx] != VirtAddress) /* Check each variable except the one passed as parameter */
        {
            /* Read the other last variable updates */
            ReadStatus = EEPROM_Read (VirtAddVarTab[VarIdx], &DataVar);
            /* In case variable corresponding to the virtual address was found */
            if (ReadStatus != 0x1) {
                /* Transfer the variable to the new active page */
                EepromStatus = EEPROM_VerifyPageFullWriteVariable (VirtAddVarTab[VarIdx], DataVar);
                /* If program operation was failed, a Flash error code is returned */
                if (EepromStatus != FLASH_COMPLETE) {
                    return EepromStatus;
                }
            }
        }
    }

    /* Erase the old Page: Set old Page status to ERASED status */


    if (OldPageId == PAGE0) {
        FlashStatus = FLASH_ROM_ERASE (EEPROM_PART0_BASE_ADDRESS, EEPROM_PAGE_SIZE);
    } else if (OldPageId == PAGE1) {
        FlashStatus = FLASH_ROM_ERASE (EEPROM_PART1_BASE_ADDRESS, EEPROM_PAGE_SIZE);
    }

    /* If erase operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE) {
        return FlashStatus;
    }

    /* Set new Page status to VALID_PAGE status */

    FlashStatus = EEPROM_SetPageState (NewPageAddress, (uint16_t)valid);

    /* If program operation was failed, a Flash error code is returned */
    if (FlashStatus != FLASH_COMPLETE) {
        return FlashStatus;
    }

    /* Return last operation flash status */
    return FlashStatus;
}


/*********************************************************************
 * @fn      EEPROM_Init
 *
 * @brief   Restore the pages to a known good state in case of page's status corruption after a power loss.
 *
 * @return  uint16_t - Flash operation status.
 */
uint16_t EEPROM_Init (void) {
    uint16_t PageStatus0 = 6, PageStatus1 = 6;
    uint16_t VarIdx = 0;
    uint16_t EepromStatus = 0, ReadStatus = 0;
    int16_t x = -1;
    FLASH_Status FlashStatus;

    uint32_t valid = 0x0000;

    /* Get Page0 status */
    PageStatus0 = (*(__IO uint16_t *)EEPROM_PART0_BASE_ADDRESS);
    /* Get Page1 status */
    PageStatus1 = (*(__IO uint16_t *)EEPROM_PART1_BASE_ADDRESS);

    /* Check for invalid header states and repair if necessary */
    switch (PageStatus0) {
    case PAGE_STATE_EMPTY:
        if (PageStatus1 == PAGE_STATE_VALID) /* Page0 erased, Page1 valid */
        {
            /* Erase Page0 */
            if (!EEPROM_VerifyPageFullyErased (EEPROM_PART0_BASE_ADDRESS)) {

                FlashStatus = FLASH_ROM_ERASE (EEPROM_PART0_BASE_ADDRESS, EEPROM_PAGE_SIZE);

                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != FLASH_COMPLETE) {
                    return FlashStatus;
                }
            }
        } else if (PageStatus1 == PAGE_STATE_RECEIVE) /* Page0 erased, Page1 receive */
        {
            /* Erase Page0 */
            if (!EEPROM_VerifyPageFullyErased (EEPROM_PART0_BASE_ADDRESS)) {

                FlashStatus = FLASH_ROM_ERASE (EEPROM_PART0_BASE_ADDRESS, EEPROM_PAGE_SIZE);
                /* If erase operation was failed, a Flash error code is returned */

                if (FlashStatus != FLASH_COMPLETE) {
                    return FlashStatus;
                }
            }
            /* Mark Page1 as valid */

            FlashStatus = EEPROM_SetPageState (EEPROM_PART1_BASE_ADDRESS, (uint16_t)valid);

            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }
        } else /* First EEPROM access (Page0&1 are erased) or invalid state -> format EEPROM */
        {
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            FlashStatus = EEPROM_Format();
            /* If erase/program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }
        }
        break;

    case PAGE_STATE_RECEIVE:
        if (PageStatus1 == PAGE_STATE_VALID) /* Page0 receive, Page1 valid */
        {
            /* Transfer data from Page1 to Page0 */
            for (VarIdx = 0; VarIdx < NB_VAR; VarIdx++) {
                if ((*(__IO uint16_t *)(EEPROM_PART0_BASE_ADDRESS + FLASH_PAGE_SIZE + 2)) == VirtAddVarTab[VarIdx]) {
                    x = VarIdx;
                }
                if (VarIdx != x) {
                    /* Read the last variables' updates */
                    ReadStatus = EEPROM_Read (VirtAddVarTab[VarIdx], &DataVar);
                    /* In case variable corresponding to the virtual address was found */
                    if (ReadStatus != 0x1) {
                        /* Transfer the variable to the Page0 */
                        EepromStatus = EEPROM_VerifyPageFullWriteVariable (VirtAddVarTab[VarIdx], DataVar);
                        /* If program operation was failed, a Flash error code is returned */
                        if (EepromStatus != FLASH_COMPLETE) {
                            return EepromStatus;
                        }
                    }
                }
            }
            /* Mark Page0 as valid */

            FlashStatus = EEPROM_SetPageState (EEPROM_PART0_BASE_ADDRESS, (uint16_t)valid);

            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }

            /* Erase Page1 */
            if (!EEPROM_VerifyPageFullyErased (EEPROM_PART1_BASE_ADDRESS)) {

                FlashStatus = FLASH_ROM_ERASE (EEPROM_PART1_BASE_ADDRESS, EEPROM_PAGE_SIZE);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != FLASH_COMPLETE) {
                    return FlashStatus;
                }
            }
        } else if (PageStatus1 == PAGE_STATE_EMPTY) /* Page0 receive, Page1 erased */
        {
            /* Erase Page1 */
            if (!EEPROM_VerifyPageFullyErased (EEPROM_PART1_BASE_ADDRESS)) {

                FlashStatus = FLASH_ROM_ERASE (EEPROM_PART1_BASE_ADDRESS, EEPROM_PAGE_SIZE);

                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != FLASH_COMPLETE) {
                    return FlashStatus;
                }
            }
            /* Mark Page0 as valid */

            FlashStatus = EEPROM_SetPageState (EEPROM_PART0_BASE_ADDRESS, (uint16_t)valid);

            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }
        } else /* Invalid state -> format eeprom */
        {
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            FlashStatus = EEPROM_Format();
            /* If erase/program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }
        }
        break;

    case PAGE_STATE_VALID:
        if (PageStatus1 == PAGE_STATE_VALID) /* Invalid state -> format eeprom */
        {
            /* Erase both Page0 and Page1 and set Page0 as valid page */
            FlashStatus = EEPROM_Format();
            /* If erase/program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }
        } else if (PageStatus1 == PAGE_STATE_EMPTY) /* Page0 valid, Page1 erased */
        {
            /* Erase Page1 */
            if (!EEPROM_VerifyPageFullyErased (EEPROM_PART1_BASE_ADDRESS)) {

                FlashStatus = FLASH_ROM_ERASE (EEPROM_PART1_BASE_ADDRESS, EEPROM_PAGE_SIZE);
                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != FLASH_COMPLETE) {
                    return FlashStatus;
                }
            }
        } else /* Page0 valid, Page1 receive */
        {
            /* Transfer data from Page0 to Page1 */
            for (VarIdx = 0; VarIdx < NB_VAR; VarIdx++) {
                if ((*(__IO uint16_t *)(EEPROM_PART1_BASE_ADDRESS + FLASH_PAGE_SIZE + 2)) == VirtAddVarTab[VarIdx]) {
                    x = VarIdx;
                }
                if (VarIdx != x) {
                    /* Read the last variables' updates */
                    ReadStatus = EEPROM_Read (VirtAddVarTab[VarIdx], &DataVar);
                    /* In case variable corresponding to the virtual address was found */
                    if (ReadStatus != 0x1) {
                        /* Transfer the variable to the Page1 */
                        EepromStatus = EEPROM_VerifyPageFullWriteVariable (VirtAddVarTab[VarIdx], DataVar);
                        /* If program operation was failed, a Flash error code is returned */
                        if (EepromStatus != FLASH_COMPLETE) {
                            return EepromStatus;
                        }
                    }
                }
            }
            /* Mark Page1 as valid */

            FlashStatus = EEPROM_SetPageState (EEPROM_PART1_BASE_ADDRESS, (uint16_t)valid);

            /* If program operation was failed, a Flash error code is returned */
            if (FlashStatus != FLASH_COMPLETE) {
                return FlashStatus;
            }
            /* Erase Page0 */
            if (!EEPROM_VerifyPageFullyErased (EEPROM_PART0_BASE_ADDRESS)) {

                FlashStatus = FLASH_ROM_ERASE (EEPROM_PART0_BASE_ADDRESS, EEPROM_PAGE_SIZE);

                /* If erase operation was failed, a Flash error code is returned */
                if (FlashStatus != FLASH_COMPLETE) {
                    return FlashStatus;
                }
            }
        }
        break;

    default: /* Any other state -> format eeprom */
        /* Erase both Page0 and Page1 and set Page0 as valid page */
        FlashStatus = EEPROM_Format();
        /* If erase/program operation was failed, a Flash error code is returned */
        if (FlashStatus != FLASH_COMPLETE) {
            return FlashStatus;
        }
        break;
    }

    return FLASH_COMPLETE;
}


/*********************************************************************
 * @fn      EEPROM_Read
 *
 * @brief   Reads the last stored variable data, if found, which correspond to the passed virtual address.
 *
 * @param   VirtAddress - Variable virtual address.
 *          Data - Pointer to the variable data.
 *
 * @return  uint16_t - (0: variable exist, 1: variable doesn't exist).
 */
uint16_t EEPROM_Read (uint16_t VirtAddress, uint16_t *Data) {
    uint16_t ValidPage = PAGE0;
    uint16_t ReadStatus = 1;
    uint32_t Address = EEPROM_START_ADDRESS, PageStartAddress = EEPROM_START_ADDRESS;
    uint32_t AddressValue = 0x55555555;

    /* Get active Page for read operation */
    ValidPage = EEPROM_FindValidPage (READ_VALID);

    /* Check if there is no valid page */
    if (ValidPage == NO_VALID_PAGE) {
        return NO_VALID_PAGE;
    }

    /* Get the valid Page start Address */
    PageStartAddress = (uint32_t)(EEPROM_START_ADDRESS + (uint32_t)(ValidPage * EEPROM_PAGE_SIZE));

    /* Get the valid Page end Address, the first page of the valid page stores the page state */
    Address = (uint32_t)((EEPROM_START_ADDRESS - EEPROM_RECORD_SIZE) + (uint32_t)((1 + ValidPage) * EEPROM_PAGE_SIZE));

    /* Check each record starting from end */
    while (Address >= (PageStartAddress + FLASH_PAGE_SIZE)) {
        /* Get the current record content */
        AddressValue = (*(__IO uint32_t *)Address);

        /* The record is erased, no valid record here */
        if (AddressValue != ((((uint32_t)PAGE_STATE_EMPTY) <<16) | (uint32_t)PAGE_STATE_EMPTY)) {
            /* Compare the virtual address stored in the record with the passed virtual address */
            if ((uint16_t)(AddressValue >>16) == VirtAddress) {
                /* Get the variable value which is the low half word of the record */
                *Data = (uint16_t)AddressValue;

                /* In case variable value is read, reset ReadStatus flag */
                ReadStatus = 0;

                break;
            }
        }
        /* Next record location */
        Address = Address - EEPROM_RECORD_SIZE;
    }

    /* Return ReadStatus value: (0: variable exist, 1: variable doesn't exist) */
    return ReadStatus;
}


/*********************************************************************
 * @fn      EEPROM_Write
 *
 * @brief   Writes the variable data to the emulated EEPROM.
 *
 * @param   VirtAddress - Variable virtual address.
 *          Data - Variable data to be written.
 *
 * @return  uint16_t - Flash operation status.
 */
uint16_t EEPROM_Write (uint16_t VirtAddress, uint16_t Data) {
    uint16_t Status = 0;

    /* Write the variable virtual address and value in the EEPROM */
    Status = EEPROM_VerifyPageFullWriteVariable (VirtAddress, Data);

    /* In case the EEPROM active page is full */
    if (Status == PAGE_FULL) {
        /* Perform Page transfer */
        Status = EEPROM_PageTransfer (VirtAddress, Data);
    }

    /* Return last operation status */
    return Status;
}

#pragma GCC pop_options