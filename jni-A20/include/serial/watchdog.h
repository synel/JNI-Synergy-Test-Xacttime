#ifndef WATCHDOG_H
#define WATCHDOG_H

/*喂狗模块类型*/
typedef enum __MODEL_TYPE {
    DEFAULT=0,
    COMM_TCP,
    COMM_485,
    FINGER,
    CAMERA_RESET,
    SOUND_CONTINUE,
    COUNT_FILE,
    FINGER_ENROLL
} MODEL_TYPE;
#define WATCHDOG_FEEDTIME	4
#define WATCHDOG_TIME	0XF

int save_watchdog_record(char *dir, int value, long long num);
int send_watch_info(char value);
int init_watchdog();
char* keep_watchdog_alive();
int close_watchdog();
int close_watchdog_info();
int send_watchdog_info(int model);
int feed_watchdog(char value);
int send_watchdog_info_test(int watch_time);
#endif // SERIAL_DEVICES_H
