/**
 * @file   readcard.h
 * @author 刘训
 * @date   Thu Jul 14 16:47:39 2011
 *
 * @brief
 *
 *
 */
#ifndef READ_CARD_H
#define READ_CARD_H
#include "_precomp.h"

void set_track_number(int track);

int mf1_read_sectors(int uart_port,int beg_sectors,int end_sectors,int beg_block,
                int end_block,char keymode,char *key, char *value);
int mf1_write_sectors(int uart_port,int beg_sectors,int end_sectors,int beg_block,
                int end_block,char keymode,char *key, char *value);
int card_synelhandle(char *cardsno,char*param);
int card_linearhandle(char *cardsno,char*param);
void right(char * sno,int rightnum);
void cutzero(char *src);
int set_textcard_mode(int t_start, int t_end, int t_len, int d_start, int d_len);
int read_text_card(int uart_port,char *value);

int active_homing_card(int uart_port,char *value);
int send_and_recv_apdu(int uart_port,char *send,char *resp);
int read_xbt_card(int uart_port,char *value);

int active_read_card(int uart_port, char *value);
int active_read_card_cpu(int uart_port, char *value) ;
int mf1_read_sectors_bk(int uart_port,int beg_sectors,int end_sectors,int beg_block,
                     int end_block,char keymode,char *key, char *value, int outtime);
#endif
