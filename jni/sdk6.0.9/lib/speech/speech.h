/*
 * speech.h
 *
 *  Created on: 2014-8-26
 *      Author: aduo
 */

#ifndef SPEECH_H_
#define SPEECH_H_

int speech_init(int port,int baud_rate);
int speech_play(int encoding,char *text);
int speech_uninit();

#endif /* SPEECH_H_ */
