#include "ltectl.h"


static queue_t lte_queue;
static char lte_modem_number[MAX_PHONE_NUM_SIZE];
static bool sms_incoming = false;

/* AT COMMAND FORMAT
 * ATCMD<><CR>
 *
 * AT COMMAND RESPONSE FORMAT
 * <CR><LF><response><CR><LF>
 */
static void lte_tx_atcmd(int cmd, char *buf)
{
    char atcmd[1024] = {0};

    switch (cmd) {
        case AT_ECHO:
            sprintf(atcmd, "ATE%s\r", buf);
            break;
        case AT_CGMM:
            sprintf(atcmd, "AT+CGMM%s\r", buf);
            break;
        case AT_CLIP:
            sprintf(atcmd, "AT+CLIP%s\r", buf);
            break;
        case AT_CLVL:
            sprintf(atcmd, "AT+CLVL%s\r", buf);
            break;
        case AT_CNUM:
            sprintf(atcmd, "AT+CNUM%s\r", buf);
            break;
        case AT_CCLK:
            sprintf(atcmd, "AT+CCLK%s\r", buf);
            break;
        case AT_CNMI:
            sprintf(atcmd, "AT+CNMI%s\r", buf);
            break;
        case AT_CMGF:
            sprintf(atcmd, "AT+CMGF%s\r", buf);
            break;
        case AT_CSQ:
            sprintf(atcmd, "AT+CSQ%s\r", buf);
            break;
        case AT_QCDS:
            sprintf(atcmd, "AT+QCDS%s\r", buf);
            break;
        case AT_CHK:
            sprintf(atcmd, "AT+CHK%s\r", buf);
            break;
        case AT_A:
            sprintf(atcmd, "ATA%s\r", buf);
            break;
        case AT_QAUDMOD:
            sprintf(atcmd, "AT+QAUDMOD%s\r", buf);
            break;
        case AT_CMGR:
            sprintf(atcmd, "AT+CMGR%s\r", buf);
            break;
        case AT_CMGS:
            sprintf(atcmd, "AT+CMGS%s\r", buf);
            break;
        case AT_CMGS_MSG:
            sprintf(atcmd, "%s", buf);
            break;
        case AT_D:
            sprintf(atcmd, "ATD%s\r", buf);
            break;
        case AT_CHUP:
            sprintf(atcmd, "AT+CHUP%s\r", buf);
            break;
        case AT_CMGD:
            sprintf(atcmd, "AT+CMGD%s\r", buf);
            break;
        case AT_URCCFG:
            sprintf(atcmd, "AT+QURCCFG%s\r", buf);
            break;
        case AT_IPR:
            sprintf(atcmd, "AT+IPR%s\r", buf);
            break;
        case AT_CMTI:
            sprintf(atcmd, "AT+CMTI%s\r", buf);
            break;
        case AT_CMMS:
            sprintf(atcmd, "AT+CMMS%s\r", buf);
            break;
        default:
            return;
    }
    lte_tx(atcmd, strlen(atcmd));

    usleep(300*1000);
}

char htoa(char hex)
{
    char ascii;

    if (hex < 10)
        ascii = hex + 0x30;
    else
        ascii = hex + 0x37;
    return ascii;
}

static int pdu_encode_num(char *src, char *des)
{
    int i, len = strlen(src);

    if (len > MAX_PHONE_NUM_SIZE)
        return -1;

    if (len%2) {
        src[len] = 'F';
        len++;
    }

    for (i = 0; i < len; i++) {
        if (i%2 == 0)
            des[i+1] = src[i];
        else
            des[i-1] = src[i];
    }
    return 0;
}

static int pdu_encode_sms(char *src, char *des)
{
    int i, j, len = strlen(src);

    if (len > MAX_SMS_SIZE)
        return -1;

    for (i = 0, j = 0; i < len; i++, j+=2) {
        des[j] = htoa((src[i] >> 4) & 0x0f);
        des[j+1] = htoa(src[i] & 0x0f);
    }
    return 0;
}

static char *lte_sms_pdu_encode(char *receiver, char *sms)
{
    char *ret = (char *)calloc(MAX_PDU_SIZE, sizeof(char));
    char encoded_sender[MAX_PHONE_NUM_SIZE*2] = {0};
    char encoded_receiver[MAX_PHONE_NUM_SIZE*2] = {0};
    char encoded_sms[MAX_SMS_SIZE*2] = {0};
    char sender[MAX_PHONE_NUM_SIZE] = {0};
    int receiver_len = strlen(receiver), sender_len = strlen(lte_modem_number);

    memcpy(sender, lte_modem_number, sender_len);

    if (!ret)
        return NULL;

    if (pdu_encode_num(sender, encoded_sender) < 0)
        goto free;
    if (pdu_encode_num(receiver, encoded_receiver) < 0)
        goto free;
    if (pdu_encode_sms(sms, encoded_sms) < 0)
        goto free;

    sprintf(ret, 
            SMS_PDU_SUBMIT
            "%c%c"
            SMS_PDU_INTL_FORMAT
            "%s"
            SMS_PDU_INDEX
            "%c%c"
            SMS_PDU_INTL_FORMAT
            "%s%s",
            htoa(((receiver_len >> 4) & 0x0f)), htoa((receiver_len & 0x0f)),
            encoded_receiver,
            htoa(((sender_len >> 4) & 0x0f)), htoa((sender_len & 0x0f)),
            encoded_sender, encoded_sms
           );

    return ret;

free:
    free(ret);
    return NULL;
}

static void pdu_decode_7bit_gsm(char *src, char *des)
{
    int i, j, src_len = strlen(src);
    int carry_bits = 0, carry_cnt = 0;
    char byte;

    for (i = 0, j = 0; i < src_len; i += 2, j++) {
        byte = (src[i] >= 'A')? (src[i] - 'A' + 10):(src[i] - '0');
        if (i%2 == 0)
            byte <<= 4;
        byte |= (src[i+1] >= 'A')? (src[i+1] - 'A' + 10):(src[i+1] - '0');

        char septet = (byte << carry_cnt) | carry_bits;
        carry_bits = byte >> (7 - carry_cnt);
        carry_cnt++;

        des[j] = septet & 0x7f;

        if (carry_bits == 7) {
            des[j++] = carry_bits & 0x7f;
            carry_bits = 0;
            carry_cnt = 0;
        }
    }
    //printf("decoded_7bit : %s\n", des);
}

static void pdu_decode_8bit(char *src, char *des)
{
    int i, j, src_len = strlen(src);
    char byte;

    for (i = 0, j = 0; i < src_len; i += 2, j++) {
        byte = (src[i] >= 'A')? (src[i] - 'A' + 10):(src[i] - '0');
        if (i%2 == 0)
            byte <<= 4;
        byte |= (src[i+1] >= 'A')? (src[i+1] - 'A' + 10):(src[i+1] - '0');

        des[j] = byte;
    }
    //printf("decoded_8bit : %s\n", des);
}

static void pdu_decode_ucs2(char *src, char *des)
{
    int i, j, src_len = strlen(src);
}

static char *lte_sms_pdu_decode(char *buf, int len)
{
    char *ret = (char *)calloc(MAX_PDU_SIZE, sizeof(char));
    int i, step;
    long smsc_len, foct, sender_len, pid, dcs, udl, udhl, iei, iedl;
    char ied[MAX_PHONE_NUM_SIZE*2];
    char ud[MAX_SMS_SIZE*2];
    char tmp[3];

    if (!ret)
        return NULL;

    for (i = 0, step = PDU_SMSC_STEP; i < len; step++) {
        switch (step){
            case PDU_SMSC_STEP:
                strncpy(tmp, &buf[i], 2);
                tmp[2] = '\0';
                smsc_len = strtol(tmp, NULL, 16);
                i += (smsc_len + 1) * 2;
                //printf("smsc_len : %ld\n", smsc_len);
                break;
            case PDU_FOCT_STEP:
                strncpy(tmp, &buf[i], 2);
                tmp[2] = '\0';
                foct = strtol(tmp, NULL, 16);
                i += 2;
                //printf("foct : %ld\n", foct);
                break;
            case PDU_SENDER_STEP:
                strncpy(tmp, &buf[i], 2);
                tmp[2] = '\0';
                sender_len = strtol(tmp, NULL, 16);
                i += 4 + sender_len + (sender_len % 2);
                //printf("sender_len : %ld\n", sender_len);
                break;
            case PDU_PID_STEP:
                strncpy(tmp, &buf[i], 2);
                tmp[2] = '\0';
                pid = strtol(tmp, NULL, 16);
                i += 2;
                //printf("pid : %ld\n", pid);
                break;
            case PDU_DCS_STEP:
                strncpy(tmp, &buf[i], 2);
                tmp[2] = '\0';
                dcs = strtol(tmp, NULL, 16);
                i += 2;
                //printf("dcs : %ld\n", dcs);
                break;
            case PDU_TIME_STEP:
                i += 14;
                //printf("timestamp\n");
                break;
            case PDU_UDL_STEP:
                strncpy(tmp, &buf[i], 2);
                tmp[2] = '\0';
                udl = strtol(tmp, NULL, 16);
                i += 2;
                if (foct & 0x40) {
                    strncpy(tmp, &buf[i], 2);
                    tmp[2] = '\0';
                    udhl = strtol(tmp, NULL, 16);
                    i += 2;
                    //printf("udhl : %ld\n", udhl);

                    strncpy(tmp, &buf[i], 2);
                    tmp[2] = '\0';
                    iei = strtol(tmp, NULL, 16);
                    i += 2;
                    //printf("iei : %ld\n", iei);

                    strncpy(tmp, &buf[i], 2);
                    tmp[2] = '\0';
                    iedl = strtol(tmp, NULL, 16);
                    i += 2;
                    //printf("iedl : %ld\n", iedl);

                    memset(ied, 0x00, sizeof(ied));
                    strncpy(ied, &buf[i], iedl * 2);
                    i += iedl * 2;
                    //printf("ied : %s\n", ied);

                    udl = (udl - (iedl + 3));
                }
                //printf("udl : %ld\n", udl);
                break;
            case PDU_UD_STEP:
                memset(ud, 0x00, sizeof(ud));
                strncpy(ud, &buf[i], udl * 2);
                i += udl * 2;
                //printf("ud : %s\n", ud);
                break;
            default:
                goto free;
        }
    }

    if (dcs & 0x04)
        pdu_decode_8bit(ud, ret);
    else
        pdu_decode_7bit_gsm(ud, ret);

    return ret;

free:
    free(ret);
    return NULL;
}

static int lte_rssi(char *rssi_str)
{
    int rssi = atoi(rssi_str);

    if (rssi < 99) {
        rssi += (rssi * 2) - 113;
    } else if (rssi == 99) {
        rssi = 999;
    } else if (rssi < 199) {
        rssi += rssi - 116;
    } else if (rssi == 199) {
        rssi = 999;
    } else
        rssi = 999;
    return rssi;
}

static int lte_ber(char *ber_str)
{
    int ber = atoi(ber_str);

    if (ber < 99) {
    } else
        ber = 99;
    return ber;
}

static void lte_sms(void)
{
    int i, sms_len;
    long long num;
    char c, tmp[MAX_PHONE_NUM_SIZE+MAX_SMS_SIZE];
    char *phone_num, *sms, *encoded_sms;

    while (1) {
        memset(tmp, 0x00, sizeof(tmp));
        printf("\n\n***********************************************\n");
        printf("\n  Input Format: [target phone number, message]\n");
        printf("\n  (ex) >> 01012345678,abcdefghijklmnopqrs...\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < sizeof(tmp))
                tmp[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail\n");

        num = check_ascii_num(tmp, i);
        if (num == 0)
            return;
        if (num < 0 && i == 0)
            continue;

        phone_num = strtok(tmp, ",");
        sms = strtok(NULL, ",");
        if (!phone_num || !sms)
            continue;

        if (check_ascii_num(phone_num, strlen(phone_num)) < 0)
            continue;

        encoded_sms = lte_sms_pdu_encode(phone_num, sms);
        if (!encoded_sms)
            continue;

        memset(tmp, 0x00, sizeof(tmp));
        sms_len = (strlen(encoded_sms))/2 - 1; // pdu size
        sprintf(tmp, "=%d", sms_len);
        lte_tx_atcmd(AT_CMGS, tmp);
        lte_tx_atcmd(AT_CMGS_MSG, encoded_sms);
        free(encoded_sms);
    }
}

static void lte_call(void)
{
    int i;
    long long num;
    char c, phone_num[MAX_PHONE_NUM_SIZE], tmp[MAX_PHONE_NUM_SIZE+1];

    while (1) {
        memset(phone_num, 0x00, sizeof(phone_num));
        printf("\n\n***********************************************\n");
        printf("\n\tInput Format: [phone number]\n\n");
        printf("\t   (ex) >> 01012345678\n");
        printf("\n***********************************************\n");
        printf("|  0: Back\n");
        printf("+---------------------------------------------+\n");
        printf(">> ");

        i = 0;
        for (;;) {
            if (scanf("%c", &c) < 0)
                continue;
            if (c == 0x0a)
                break;
            if (i < MAX_PHONE_NUM_SIZE-1)
                phone_num[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail\n");

        num = check_ascii_num(phone_num, i);
        if (num == 0)
            return;
        if (num < 0)
            continue;

        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp, "%s;", phone_num);
        lte_tx_atcmd(AT_D, tmp);
    }
}

static void lte_number(void)
{
    lte_tx_atcmd(AT_CNUM, "");
}

static void lte_hangup(void)
{
    lte_tx_atcmd(AT_CHUP, "");
}

static void lte_csq(void)
{
    lte_tx_atcmd(AT_CSQ, "");
}

static void lte_uart9600(void)
{
    lte_tx_atcmd(AT_IPR, "=9600;&W");
}

static void lte_uart115200(void)
{
    lte_tx_atcmd(AT_IPR, "=115200;&W");
}

static void lte_handle_at(lte_packet_t *packet)
{
    if (!strncmp(packet->packet, "OK", strlen("OK"))) {
    } else if (!strncmp(packet->packet, "ERROR", strlen("ERROR"))) {   
    } else if (!strncmp(packet->packet, "+CNUM", strlen("+CNUM"))) {
        strtok(packet->packet, ",");
        char *number = strtok(NULL, ",");
        memset(lte_modem_number, 0x00, sizeof(lte_modem_number));
        number++; // front " erase
        memcpy(lte_modem_number, number, strlen(number)-1); // rear " erase
        printf ("\n***********************************************\n");
        printf ("\n\t\tModem Number\n\n\t      > %s\n", lte_modem_number);
        printf ("\n***********************************************\n");
    } else if (!strncmp(packet->packet, "RING", strlen("RING"))) {
        lte_tx_atcmd(AT_A, "");
    } else if (!strncmp(packet->packet, "+CLIP", strlen("+CLIP"))) {
    } else if (!strncmp(packet->packet, "+CHUP", strlen("+CHUP"))) {
    } else if (!strncmp(packet->packet, "+CMTI", strlen("+CMTI"))) {
        strtok(packet->packet, ",");
        char *msg = strtok(NULL, ","), tmp[8] = {0};
        sprintf(tmp, "=%s", msg);
        lte_tx_atcmd(AT_CMGR, tmp);
    } else if (!strncmp(packet->packet, "+CMGR", strlen("+CMGR"))) {
        sms_incoming = true;
        lte_tx_atcmd(AT_CMGD, "=0,4");
    } else if (!strncmp(packet->packet, "NO CARRIER", strlen("NO CARRIER"))) {
        lte_tx_atcmd(AT_CHUP, "");
    } else if (!strncmp(packet->packet, "+CSQ", strlen("+CSQ"))) {
        strtok(packet->packet, " ");
        char *rssi = strtok(NULL, ",");
        char *ber = strtok(NULL, ",");
        printf ("\n***********************************************\n");
        printf ("\n\t  Signal Quality Report\n\n\t    RSSI : %ddBm\n\t     BER : %d\n", lte_rssi(rssi), lte_ber(ber));
        printf ("\n***********************************************\n");
    } else if (!strncmp(packet->packet, "+CMGS", strlen("+CMGS"))) {
    } else
        printf("Unexpected Packet!\n");
}


static void lte_handle_sms(lte_packet_t *packet)
{
    char *decoded_sms;

    decoded_sms = lte_sms_pdu_decode(packet->packet, packet->plen);
    if (!decoded_sms)
        return;

    printf ("\n***********************************************\n");
    printf ("\n\t RECEIVED SMS MESSAGE\n\n  > %s\n", decoded_sms);
    printf ("\n***********************************************\n");
    free(decoded_sms);
}

void lte_parser(char *buf, int len)
{
    static int lte_parser_step = LTE_STEP_HEADER1;
    static lte_packet_t packet;
    int i;

    for (i = 0; i < len; i++) {
        switch (lte_parser_step) {
            case LTE_STEP_HEADER1:
                memset(&packet, 0x00, sizeof(packet));
                if (buf[i] == '\r')
                    lte_parser_step = LTE_STEP_HEADER2;
                if (sms_incoming) {
                    packet.packet[packet.plen++] = buf[i];
                    lte_parser_step = LTE_STEP_DATA;
                }
                break;
            case LTE_STEP_HEADER2:
                if (buf[i] == '\n') {
                    if (buf[i+1] == '>')
                        lte_parser_step = LTE_STEP_HEADER1;
                    else
                        lte_parser_step = LTE_STEP_DATA;
                } else
                    lte_parser_step = LTE_STEP_HEADER1;
                break;
            case LTE_STEP_DATA:
                if (buf[i] == '\r') {
                    lte_parser_step = LTE_STEP_TRAILER;
                    break;
                }
                if (packet.plen > MAX_UART_XFER_SIZE) {
                    lte_parser_step = LTE_STEP_HEADER1;
                    break;
                }
                packet.packet[packet.plen++] = buf[i];
                break;
            case LTE_STEP_TRAILER:
                if (buf[i] == '\n') {
                    if (sms_incoming) {
                        lte_handle_sms(&packet);
                        sms_incoming = false;
                    } else
                        lte_handle_at(&packet);
                } else {
                    if (sms_incoming)
                        sms_incoming = false;
                }
                lte_parser_step = LTE_STEP_HEADER1;
                break;
            default:
                lte_parser_step = LTE_STEP_HEADER1;
                break;
        }
    }
}

static void lte_rx_thread(void)
{}

static menu_t lte_menus[] = {
    {lte_sms, "SMS TO YOUR NUMBER"},
    {lte_call, "CALL TO YOUR NUMBER"},
    {lte_hangup, "HANG UP CALL"},
    {lte_number, "SHOW MODEM NUMBER"},
    {lte_csq, "SHOW SIGNAL QUALITY"},
    {lte_uart9600, "BAUDRATE TO 9600"},
    {lte_uart115200, "BAUDRATE TO 115200"},
};

void lte_control(void)
{
    char *des = "\t\tLTE Control Menu";
    menu_print(lte_menus, sizeof(lte_menus)/sizeof(menu_t), des);
}

void lte_init(void)
{
    pthread_t rx_thread;

    lte_tx_atcmd(AT_CGMM, "");
    lte_tx_atcmd(AT_ECHO, "0");
    lte_tx_atcmd(AT_CNUM, "");
    lte_tx_atcmd(AT_QAUDMOD, "=2"); // audio speaker, no headset
    lte_tx_atcmd(AT_CLVL, "=3");
    lte_tx_atcmd(AT_CCLK, "");
    lte_tx_atcmd(AT_CMGF, "=0");
    //lte_tx_atcmd(AT_CMMS, "=0");
    lte_tx_atcmd(AT_URCCFG, "=\"urcport\",\"all\"");
    lte_tx_atcmd(AT_CLIP, "=1");
    lte_tx_atcmd(AT_CNMI, "=2,1,0,0,0"); // URC TE 즉시 전달, URC(+CMTI) 이용하여 TE 알람, 
                                         // CMD disabled, SMS_STATUS_REPORT disabled
    lte_tx_atcmd(AT_QCDS, "");
    lte_tx_atcmd(AT_CSQ, "");

    if (pthread_create(&rx_thread, NULL, (void *)lte_rx_thread, NULL) < 0)
        exit(1);

    printf("LTE Init Ok.\n");
}
