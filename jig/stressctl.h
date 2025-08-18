#ifndef LTECTL_H_
#define LTECTL_H_

#include <pthread.h>
#include <stdbool.h>
#include "menu.h"
#include "uartctl.h"
#include "api/queue.h"

#define AT_ECHO     0       // Echo after data
#define AT_CGMM     1       // Model identification ex) EC21
#define AT_CLIP     2       //
#define AT_CLVL     3       // Loud Speaker Volume
#define AT_CNUM     4
#define AT_CCLK     5
#define AT_CNMI     6       // SMS Event Reporting Configuration
#define AT_CMGF     7       // Message Format '0' is PDU mode. '1' is Text mode.
#define AT_CSQ      8
#define AT_QCDS     9
#define AT_CHK      10
#define AT_A        11
#define AT_QAUDMOD  12
#define AT_CMGR     13      // Read Message
#define AT_CMGS     14      // Send Message
#define AT_CMGS_MSG 15
#define AT_D        16
#define AT_CHUP     17
#define AT_CMGD     18      // Delete Message
#define AT_URCCFG   19
#define AT_IPR      20
#define AT_CMTI     21      // New message has been received
#define AT_CMMS     22      // More Message Send

#define MAX_SMS_SIZE        148
#define MAX_PHONE_NUM_SIZE  12
#define MAX_PDU_SIZE        ((MAX_SMS_SIZE*2)+(MAX_PHONE_NUM_SIZE*4))

#define SMS_PDU_SUBMIT      "0051FF"
#define SMS_PDU_INTL_FORMAT "A1"    
#define SMS_PDU_INDEX       "0084A75B0A2208"

enum LTE_STEPS {
    LTE_STEP_HEADER1 = 0,
    LTE_STEP_HEADER2,
    LTE_STEP_DATA,
    LTE_STEP_TRAILER,
};

enum PDU_DECODE_STEPS {
    PDU_SMSC_STEP = 0,
    PDU_FOCT_STEP,
    PDU_SENDER_STEP,
    PDU_PID_STEP,
    PDU_DCS_STEP,
    PDU_TIME_STEP,
    PDU_UDL_STEP,
    PDU_UD_STEP,
};

typedef struct st_lte_packet {
    char packet[MAX_UART_XFER_SIZE];
    int plen;
} lte_packet_t;

void lte_parser(char *buf, int len);
void lte_control(void);
void lte_init(void);

#endif
