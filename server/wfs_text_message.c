/************************************************************************/
/* wfs_andor_usb_data.c							*/
/*                                                                      */
/* Routines to perform the USB part of data collection.			*/
/************************************************************************/
/*                                                                      */
/*                    CHARA ARRAY SERVER LIB   				*/
/*                 Based on the CHARA User Interface			*/
/*                 Based on the SUSI User Interface			*/
/*		In turn based on the CHIP User interface		*/
/*                                                                      */
/*            Center for High Angular Resolution Astronomy              */
/*              Mount Wilson Observatory, CA 91001, USA			*/
/*                                                                      */
/* Telephone: 1-626-796-5405                                            */
/* Fax      : 1-626-796-6717                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu			                */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar 		                        */
/* Date   : Aug 2012							*/
/************************************************************************/

#include "wfs_server.h"

static pthread_mutex_t text_message_mutex = PTHREAD_MUTEX_INITIALIZER;
static char text_message[300];

/************************************************************************/
/* setup_text_message()							*/
/*									*/
/* Mostly to setup the mutex.						*/
/************************************************************************/

int setup_text_message(void)
{

	/* Create the thread */

        if (pthread_mutex_init(&text_message_mutex, NULL) != 0)
        {
                return error(ERROR, "Unable to create usb mutex.");
        }

	pthread_mutex_lock(&text_message_mutex);
	*text_message = 0;
	pthread_mutex_unlock(&text_message_mutex);

	return NOERROR;

} /* setup_text_message() */

/************************************************************************/
/* send_wfs_text_message()                                              */
/*                                                                      */
/* Printf like command to send text to show on client.                  */
/************************************************************************/

void send_wfs_text_message(char *fmt, ...)
{
        va_list args;

        va_start(args, fmt);

        pthread_mutex_lock(&text_message_mutex);
        vsprintf(text_message, fmt, args);
        pthread_mutex_unlock(&text_message_mutex);

} /* send_wfs_text_message() */

/************************************************************************/
/* broadcast_text_message()						*/
/*									*/
/* Will try and send a text message string if tehre is one.		*/
/************************************************************************/

void broadcast_text_message(void)
{

        struct smessage message;

        pthread_mutex_lock(&text_message_mutex);

	if (strlen(text_message) == 0) 
	{
		pthread_mutex_unlock(&text_message_mutex);
		return;
	}

        message.type = WFS_TEXT_MESSAGE;
        message.data = (unsigned char *)text_message;
        message.length = strlen(text_message)+1;

        server_send_message_all(&message);

	*text_message = 0;

	pthread_mutex_unlock(&text_message_mutex);

} /* send_wfs_text_message() */
