/************************************************************************/
/* wfs_messages.c                                                      */
/*                                                                      */
/* Routines to respond to messages.					*/
/************************************************************************/
/*                                                                      */
/*     Center for High Angular Resolution Astronomy                     */
/* Georgia State University, Atlanta GA 30303-3083, U.S.A.              */
/*                                                                      */
/*                                                                      */
/* Telephone: 1-404-651-1882                                            */
/* Fax      : 1-404-651-1389                                            */
/* email    : theo@chara.gsu.edu                                        */
/* WWW      : http://www.chara.gsu.edu/~theo/theo.html                  */
/*                                                                      */
/* (C) This source code and its associated executable                   */
/* program(s) are copyright.                                            */
/*                                                                      */
/************************************************************************/
/*                                                                      */
/* Author : Theo ten Brummelaar                                  	*/
/* Date   : Aug 2012                                                    */
/************************************************************************/

#include "wfs_server.h"

/************************************************************************/
/* setup_wfs_messages()							*/
/*									*/
/************************************************************************/

void setup_wfs_messages(void)
{
        server_add_message_job(WFS_ANDOR_SETUP,
		message_wfs_andor_setup);
        server_add_message_job(WFS_ANDOR_SET_ACQMODE,
		message_wfs_andor_set_acqmode);
        server_add_message_job(WFS_ANDOR_REOPEN,
		message_wfs_andor_reopen);
        server_add_message_job(WFS_ANDOR_SET_EXPTIME,
		message_wfs_andor_set_exptime);
        server_add_message_job(WFS_ANDOR_SET_SHUTTER,
		message_wfs_andor_set_shutter);
        server_add_message_job(WFS_ANDOR_SET_IMAGE,
		message_wfs_andor_set_image);
        server_add_message_job(WFS_ANDOR_SET_AMPLIFIER,
		message_wfs_andor_set_amplifier);
        server_add_message_job(WFS_ANDOR_SET_TEMPERATURE,
		message_wfs_andor_set_temperature);
        server_add_message_job(WFS_ANDOR_USB_ONOFF,
		message_wfs_andor_usb_onoff);
        server_add_message_job(WFS_ANDOR_CURRENT_FRAME,
		message_wfs_andor_current_frame);
        server_add_message_job(WFS_ANDOR_PREAMP_GAIN,
		message_wfs_andor_preamp_gain);
        server_add_message_job(WFS_ANDOR_EM_ADVANCED,
		message_wfs_andor_em_advanced);
        server_add_message_job(WFS_ANDOR_COOLER_ONOFF,
		message_wfs_andor_cooler_onoff);
        server_add_message_job(WFS_ANDOR_SET_EM_GAIN,
		message_wfs_andor_set_em_gain);
        server_add_message_job(WFS_ANDOR_VERTICAL_SPEED,
		message_wfs_andor_vertical_speed);
        server_add_message_job(WFS_ANDOR_HORIZONTAL_SPEED,
		message_wfs_andor_horizontal_speed);
        server_add_message_job(WFS_ANDOR_GET_SETUP,
		message_wfs_andor_get_setup);
        server_add_message_job(WFS_ANDOR_SAVE_FITS,
		message_wfs_andor_save_fits);
        server_add_message_job(WFS_ANDOR_CAMLINK_ONOFF,
		message_wfs_andor_camlink_onoff);
        server_add_message_job(WFS_TAKE_BACKGROUND,
		message_wfs_take_background);
        server_add_message_job(WFS_RESET_BACKGROUND,
		message_wfs_reset_background);
        server_add_message_job(WFS_SET_THRESHOLD,
		message_wfs_set_threshold);
        server_add_message_job(WFS_SET_NUM_FRAMES,
		message_wfs_set_num_frames);

} /* setup_wfs_messages() */

/************************************************************************/
/* message_wfs_andor_setup()						*/
/*									*/
/************************************************************************/

int message_wfs_andor_setup(struct smessage *message)
{
	struct s_wfs_andor_setup *setup;

        if (message->length != sizeof(struct s_wfs_andor_setup))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SETUP.");
        }

	setup = (struct s_wfs_andor_setup *)message->data;

	return andor_setup_camera(*setup);

} /* message_wfs_andor_setup() */

/************************************************************************/
/* message_wfs_andor_set_acqmode()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_acqmode(struct smessage *message)
{
        int   *acqmode;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_ACQMODE.");
        }
        acqmode = (int *) message->data;
                
        andor_set_acqmode(*acqmode);

	return andor_send_setup();

} /* message_wfs_andor_set_acqmode() */

/************************************************************************/
/* message_wfs_andor_reopen()	   					*/
/*									*/
/************************************************************************/

int message_wfs_andor_reopen(struct smessage *message)
{
        if (message->length != 0)
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_REOPEN.");
        }
                
#warning This seems to cause the library to crash.
	error(MESSAGE,"This causes a crash in the Andor library.");
	error(MESSAGE,
	    "Stop the server, cycle the camera power and restart the server.");
/*
        andor_open(WFS_CAMERA, &(andor_setup.image),
		andor_setup.preamp_gain_index);
*/
	return andor_send_setup();

} /* message_wfs_andor_reopen() */

/************************************************************************/
/* message_wfs_andor_set_exptime()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_exptime(struct smessage *message)
{
        float   *exptime;
        
        if (message->length != sizeof(float))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_EXPTIME.");
        }
        exptime = (float *) message->data;
                
        andor_set_exptime(*exptime);
	return andor_send_setup();

} /* message_wfs_andor_set_exptime() */

/************************************************************************/
/* message_wfs_andor_set_shutter()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_shutter(struct smessage *message)
{
        int   *shutter;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_SHUTTER.");
        }
        shutter = (int *) message->data;
                
        andor_set_shutter(*shutter);
	return andor_send_setup();

} /* message_wfs_andor_set_shutter() */

/************************************************************************/
/* message_wfs_andor_set_image()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_image(struct smessage *message)
{
        struct s_wfs_andor_image   *image;
        
        if (message->length != sizeof(struct s_wfs_andor_image))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_IMAGE.");
        }
        image = (struct s_wfs_andor_image *) message->data;
                
        andor_set_image(*image);

	return andor_send_setup();

} /* message_wfs_andor_set_image() */

/************************************************************************/
/* message_wfs_andor_set_temperature()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_temperature(struct smessage *message)
{
        int *temperature;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_TEMPERATURE.");
        }
        temperature = (int *) message->data;
                
        andor_set_temperature(*temperature);
	return andor_send_setup();

} /* message_wfs_andor_set_temperature() */

/************************************************************************/
/* message_wfs_andor_set_amplifier()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_amplifier(struct smessage *message)
{
        int *amplifier;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_AMPLIFIER.");
        }
        amplifier = (int *) message->data;
                
        andor_set_amplifier(*amplifier);
	return andor_send_setup();

} /* message_wfs_andor_set_amplifier() */

/************************************************************************/
/* message_wfs_andor_usb_onoff()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_usb_onoff(struct smessage *message)
{
        int *onoff;
	struct smessage mess;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_USB_ONOFF.");
        }
        onoff = (int *) message->data;
                
	/* Has anything changed? */

	if (andor_setup.usb_running == *onoff) return NOERROR;

	/* OK, we need to start or stop things now. */

	if (*onoff)
	{
		return andor_start_usb();
	}
	else
	{
		mess.type = WFS_ANDOR_STOP_MOVIE;
		mess.length = 0;
		mess.data = NULL;
		if (server_send_message_all(&mess) != NOERROR)
			error(ERROR,"Failed to send WFS_ANDOR_STOP_MOVIE.");
		return andor_stop_usb();
	}

} /* message_wfs_andor_usb_onoff() */

/************************************************************************/
/* message_wfs_andor_current_frame()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_current_frame(struct smessage *message)
{
	float *values, *compressed_values, *p1;
	int i,j;
        uLongf  len, clen;
	struct smessage mess;

        if (message->length != 0)
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_CURRENT_FRAME.");
        }
                
	/* Has anything changed? */

	if (!andor_setup.running) return error(ERROR,
		"The camera is not running.");

	/* OK, First send the current information about the setup */

	if (andor_send_setup() != NOERROR) return ERROR;

	/* Make sure they know the reference centroids */

	mess.type = WFS_SUBAP_GET_CENTROIDS_REF;
	mess.length = sizeof(struct s_wfs_subap_centroids);
        mess.data = (unsigned char *)&subap_centroids_ref;

	if (server_send_message(active_socket, &mess) != NOERROR)
        {
                return error(ERROR,
                        "Failed to send current reference subap centroids.");
        }

	/* Make sure they know the calculated centroids */

	mess.type = WFS_SUBAP_GET_CENTROIDS;
	mess.length = sizeof(struct s_wfs_subap_centroids);
        mess.data = (unsigned char *)&subap_centroids;

	if (server_send_message(active_socket, &mess) != NOERROR)
        {
                return error(ERROR,
                        "Failed to send current subap centroids.");
        }

	/* Now copy and compress the current image */

	len = andor_setup.npix * sizeof(float);
	clen = 2*len;

	if ((values = (float *)malloc(len)) == NULL) 
		error(FATAL,"Out of memory.");
	if ((compressed_values = (float *)malloc(clen)) == NULL)
		error(FATAL,"Out of memory.");

	for(p1 = values, i = 1; i <= andor_setup.npixx; i++)
	for(j = 1; j <= andor_setup.npixy; j++)
	{
	    *p1++ = data_frame[i][j];
	}

	if ((i = compress((unsigned char *)compressed_values, &clen,
		 (unsigned char *)values, len)) < Z_OK)
	{
		return error(ERROR,"Failed to compress current image.");
	}

	mess.type = WFS_ANDOR_CURRENT_FRAME;
	mess.length = (int) clen;
	mess.data = (unsigned char *) compressed_values;

	if (server_send_message(active_socket, &mess) != NOERROR)
	{
		free(values);
		free(compressed_values);
                return error(ERROR,"Failed to send current frame.");
	}

	free(values);
	free(compressed_values);

	return NOERROR;

} /* message_wfs_andor_current_frame() */

/************************************************************************/
/* message_wfs_andor_preamp_gain()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_preamp_gain(struct smessage *message)
{
        int *gain;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_PREAMP_GAIN.");
        }
        gain = (int *) message->data;
                
	/* Has anything changed? */

	andor_set_preamp_gain(*gain);
	return andor_send_setup();

} /* message_wfs_andor_preamp_gain() */

/************************************************************************/
/* message_wfs_andor_em_advanced()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_em_advanced(struct smessage *message)
{
        int *on;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_EM_ADVANCED.");
        }
        on = (int *) message->data;
                
	/* Has anything changed? */

	if (*on)
		andor_set_em_advanced(1);
	else
		andor_set_em_advanced(0);

	return andor_send_setup();

} /* message_wfs_andor_em_advanced() */

/************************************************************************/
/* message_wfs_andor_cooler_onoff()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_cooler_onoff(struct smessage *message)
{
        int *onoff;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_COOLER_ONOFF.");
        }
        onoff = (int *) message->data;
                
	/* OK, we need to start or stop things now. */

	if (*onoff)
		andor_cooler_on();
	else
		andor_cooler_off();

	return andor_send_setup();

} /* message_wfs_andor_cooler_onoff() */

/************************************************************************/
/* message_wfs_andor_set_em_gain()		  			*/
/*									*/
/************************************************************************/

int message_wfs_andor_set_em_gain(struct smessage *message)
{
        int *gain;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SET_EM_GAIN.");
        }
        gain = (int *) message->data;
                
	/* Has anything changed? */

	andor_set_em_gain(*gain);
	return andor_send_setup();

} /* message_wfs_andor_set_em_gain() */

/************************************************************************/
/* message_wfs_andor_vertical_speed()		  			*/
/*									*/
/************************************************************************/

int message_wfs_andor_vertical_speed(struct smessage *message)
{
        int *speed;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_VERTICAL_SPEED.");
        }
        speed = (int *) message->data;
                
	/* Has anything changed? */

	andor_set_vertical_speed(*speed);
	return andor_send_setup();

} /* message_wfs_vertical_speed() */

/************************************************************************/
/* message_wfs_andor_horizontal_speed()		  			*/
/*									*/
/************************************************************************/

int message_wfs_andor_horizontal_speed(struct smessage *message)
{
        struct s_horizontal_speed *speed;
        
        if (message->length != sizeof(struct s_horizontal_speed))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_HORIZONTAL_SPEED.");
        }
        speed = (struct s_horizontal_speed *) message->data;
                
	/* Has anything changed? */

	andor_set_horizontal_speed(speed->type, speed->index);
	return andor_send_setup();

} /* message_wfs_horizontal_speed() */

/************************************************************************/
/* message_wfs_andor_get_setup()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_get_setup(struct smessage *message)
{
        if (message->length != 0)
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_GET_SETUP.");
        }
	return andor_send_setup();

} /* message_wfs_andor_get_setup() */

/************************************************************************/
/* message_wfs_andor_save_fits()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_save_fits(struct smessage *message)
{
        if (message->length != 0)
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_SAVE_FITS.");
        }

	lock_usb_mutex();
	lock_camlink_mutex();
	save_fits = TRUE;
	unlock_usb_mutex();
	unlock_camlink_mutex();

	return NOERROR;

} /* message_wfs_andor_save_fits() */

/************************************************************************/
/* message_wfs_andor_camlink_onoff()					*/
/*									*/
/************************************************************************/

int message_wfs_andor_camlink_onoff(struct smessage *message)
{
        int *onoff;
	struct smessage mess;
        
        if (message->length != sizeof(int))
        {
                return error(ERROR,
		"Wrong number of data bytes in WFS_ANDOR_CAMLINK_ONOFF.");
        }
        onoff = (int *) message->data;
                
	/* Has anything changed? */

	if (andor_setup.camlink_running == *onoff) return NOERROR;

	/* OK, we need to start or stop things now. */

	if (*onoff)
	{
		return andor_start_camlink();
	}
	else
	{
		mess.type = WFS_ANDOR_STOP_MOVIE;
		mess.length = 0;
		mess.data = NULL;
		if (server_send_message_all(&mess) != NOERROR)
			error(ERROR,"Failed to send WFS_ANDOR_STOP_MOVIE.");
		return andor_stop_camlink();
	}

} /* message_wfs_andor_camlink_onoff() */
