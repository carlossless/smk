#pragma once

// USBIE1, EP1CON and EP2CON are left to the part headers: the parts disagree on USBIE1 bits 3 and 7 and on whether EPxCON has the BUFSEL pair.

/**
 * \name Bits from register USBCON
 * @{
 */
#define _GOSUSP (1u << 0)
#define _WKUP   (1u << 1)
#define _SW2CON (1u << 2)
#define _DMSTA  (1u << 3)
#define _DPSTA  (1u << 4)
#define _SWRST  (1u << 5)
#define _SW1CON (1u << 6)
#define _ENUSB  (1u << 7)
/**@}*/

/**
 * \name Bits from register USBIF1
 * @{
 */
#define _USBRSTIF (1u << 0)
#define _SUSPIF   (1u << 1)
#define _RESMIF   (1u << 2)
#define _SOFIF    (1u << 3)
#define _SETUPIF  (1u << 4)
#define _OW       (1u << 5)
#define _OVERIF   (1u << 6)
#define _PUPIF    (1u << 7)
/**@}*/

/**
 * \name Bits from register USBIF2
 * @{
 */
#define _IEP0IF (1u << 0)
#define _IEP1IF (1u << 1)
#define _IEP2IF (1u << 2)
#define _OEP0IF (1u << 4)
#define _OEP1IF (1u << 5)
#define _OEP2IF (1u << 6)
/**@}*/

/**
 * \name Bits from register USBIE2
 * @{
 */
#define _IEP0IE (1u << 0)
#define _IEP1IE (1u << 1)
#define _IEP2IE (1u << 2)
#define _OEP0IE (1u << 4)
#define _OEP1IE (1u << 5)
#define _OEP2IE (1u << 6)
/**@}*/

/**
 * \name Bits from register EP0CON
 * @{
 */
#define _OEP0RDY (1u << 0)
#define _OEP0STL (1u << 1)
#define _IEP0RDY (1u << 2)
#define _IEP0STL (1u << 3)
#define _OEP0DTG (1u << 6)
#define _IEP0DTG (1u << 7)
/**@}*/

#define EP0_BUF_SIZE 8u
#define EP1_BUF_SIZE 16u
#define EP2_BUF_SIZE 64u
