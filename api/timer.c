#include <signal.h>
#include <time.h>
#include <api/timer.h>

/**
 * @brief 타이머 함수를 등록하는 함수
 *
 * @param timer     타이머 변수 포인터
 * @param func      타이머로 지정할 함수 포인터
 * @param timeout   초 단위의 타임아웃 시간
 *
 * @return 성공 시 0, 실패 시 -1
 */
int set_timer(timer_t *timer, void (*func)(void), int timeout)
{
    struct itimerspec ts;
    struct sigevent se;

    ts.it_value.tv_sec = timeout;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;

    se.sigev_notify = SIGEV_THREAD;
    se.sigev_value.sival_ptr = timer;
    se.sigev_notify_function = func;
    se.sigev_notify_attributes = NULL;

    if (timer_create(CLOCK_MONOTONIC, &se, timer) < 0) {
        perror("timer_create() fail");
        return -1;
    }

    if (timer_settime(*timer, 0, &ts, NULL) < 0) {
        perror("timer_settime() fail");
        timer_delete(*timer);
        return -1;
    }

    return 0;
}

/**
 * @brief 등록된 타이머를 해제 하는 함수
 *
 * @param timer 타이머 디스크립터
 *
 * @return 성공 시 0, 실패 시 -1
 */
int del_timer(timer_t timer)
{
    return timer_delete(timer);
}

