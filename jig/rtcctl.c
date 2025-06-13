#include "rtcctl.h"

static void rtc_get_time(void)
{
    printf("\n* Kernel Time: \n");
    if (system("date") < 0)
        printf("date command fail\n");
    printf("\n* RTC Time: \n");
    if (system("hwclock -r") < 0)
        printf("hwclock -r command fail\n");
}

static void rtc_set_time(void)
{
    int i, num;
    char c, date[32], tmp[256];

    while (1) {
        memset(date, 0x00, sizeof(date));
        printf("\n\n***********************************************\n");
        printf("\n\tInput format: [MMDDhhmmYYYY]\n\n");
        printf("\t   (ex) >> 051615192024\n");
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
            if (i < sizeof(date))
                date[i++] = c;
        }

        if (system("clear") < 0)
            printf("clear fail!\n");

        num = check_ascii_num(date, i);
        if (num == 0)
            return;
        if (num < 0)
            continue;

        memset(tmp, 0x00, sizeof(tmp));
        sprintf(tmp, "date %s >> /dev/null", date);
        if (system(tmp) < 0) {
            printf("Time format is wrong.\n");
            continue;
        }
        if (system("hwclock -w") < 0)
            printf("clock sync fail!\n");
    }
}

static menu_t rtc_menus[] = {
    {rtc_get_time, "GET RTC TIME"},
    {rtc_set_time, "SET RTC TIME"},
};

void rtc_control(void)
{
    char *des = "\t\tRTC Control Menu";
    menu_print(rtc_menus, sizeof(rtc_menus)/sizeof(menu_t), des);
}
