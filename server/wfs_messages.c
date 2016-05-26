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
	server_add_message_job(WFS_SUBAP_SEND_CENTROIDS_REF,
                             message_wfs_subap_send_centroids_ref);
        server_add_message_job(WFS_SUBAP_SHIFT_ROT_SCALE_CENTROIDS_REF,
                         message_wfs_subap_shift_rot_scale_centroids_ref);
        server_add_message_job(WFS_SUBAP_WRITE_CENTROIDS_REF,
                         message_wfs_subap_write_centroids_ref);
        server_add_message_job(WFS_SUBAP_LOAD_CENTROIDS_REF,
                         message_wfs_subap_load_centroids_ref);
	server_add_message_job(WFS_OPEN_TT_DATA_SOCKET,
                             message_wfs_open_tt_data_socket);
	server_add_message_job(WFS_CENTROID_TYPE, message_wfs_centroid_type);
	server_add_message_job(WFS_SET_CLAMP_FLUXES,message_wfs_set_clamp_flux);
	server_add_message_job(WFS_GET_CLAMP_FLUXES,
                             message_wfs_get_clamp_fluxes);
	server_add_message_job(WFS_SUBAP_CALC_CENTROIDS_REF,
                             message_wfs_subap_calc_centroids_ref);
	server_add_message_job(WFS_SET_NUM_MEAN,
                             message_wfs_set_num_mean);
	server_add_message_job(WFS_SET_MODULATION,
                             message_wfs_set_modulation);
	server_add_message_job(WFS_SET_SERVO, message_wfs_set_servo);
	server_add_message_job(WFS_GET_SERVO, message_wfs_get_servo);
	server_add_message_job(WFS_CLOSELOOP_MESSAGE, 
			message_wfs_closeloop_message);
	server_add_message_job(WFS_SET_SEND, message_wfs_set_send);
	server_add_message_job(WFS_SAVE_TIPTILT, message_wfs_save_tiptilt);
	server_add_message_job(WFS_SAVE_DATA, message_wfs_save_data);
	server_add_message_job(WFS_ZERO_TIPTILT, message_wfs_zero_tiptilt);
	server_add_message_job(WFS_START_FOCUS_PARABOLA, 
		message_wfs_start_focus_parabola);
	server_add_message_job(WFS_START_FOCUS_TELESCOPE, 
		message_wfs_start_focus_telescope);
	server_add_message_job(WFS_START_ALIGN_BEACON, 
		message_wfs_start_align_beacon);
	server_add_message_job(WFS_STOP_AUTOALIGN, message_wfs_stop_autoalign);

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
#define NUM_GREY_LEVELS 32
int message_wfs_andor_current_frame(struct smessage *message)
{
	float *values, *compressed_values, *p1, max_cnts, min_cnts, cnts_scale;
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

	mess.type = WFS_SUBAP_GET_CENTROIDS_OFFSET;
	mess.length = sizeof(struct s_wfs_subap_centroids);
        mess.data = (unsigned char *)&subap_centroids_offset;

	if (server_send_message(active_socket, &mess) != NOERROR)
        {
                return error(ERROR,
                        "Failed to send current subap centroid offsets.");
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

	/* First, find the max and min to limited number of grey scale values.*/

	min_cnts = 1e8;
	max_cnts = -1e8;
	for(i = 1; i <= andor_setup.npixx; i++)
	for(j = 1; j <= andor_setup.npixy; j++)
	{
		if (data_frame[i][j] < min_cnts) min_cnts=data_frame[i][j];
		if (data_frame[i][j] > max_cnts) max_cnts=data_frame[i][j];
	}
	cnts_scale = (max_cnts - min_cnts)/NUM_GREY_LEVELS;

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
	    *p1++ = ((int)((data_frame[i][j] - min_cnts)/cnts_scale) * cnts_scale) + min_cnts;
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

/************************************************************************/
/* message_wfs_subap_send_centroids_ref()                               */
/*                                                                      */
/* received centroids info from client                                  */
/************************************************************************/

int message_wfs_subap_send_centroids_ref(struct smessage *message)
{

	float *p1;

	if (message->length !=  4 * sizeof(float))
	{
		return error(ERROR,
	       "Wrong number of data bytes in WFS_SUBAP_SEND_CENTROIDS_REF,");
	}


	p1 = (float *)message->data;

	wfs_simulation_centroid(*p1, *(p1+1), *(p1+2), *(p1+3));

	return NOERROR;

}/* message_wfs_subap_send_centroids_ref() */
	
/************************************************************************/
/* message_wfs_subap_shift_rot_scale_centroids_ref                      */
/*                                                                      */
/* to move the reference centroid up, down, left, right, rotation       */
/* and scaling while keeping the pattern                                */
/************************************************************************/

int message_wfs_subap_shift_rot_scale_centroids_ref(struct smessage *message)
{

	float step = 0;
	int ii= 0;
	float centerx = 0., centery = 0.;
	float disx = 0, disy = 0;
	float cosangle = 0.,  sinangle =0.;

	if (message->length !=  sizeof(float))
	{
	    return error(ERROR,
	    "Wrong number of data bytes in WFS_SUBAP_SHIFT_ROT_CENTROIDS_REF");
	}

	step = *((float *)(message->data));

	switch ((int)step/1000)
	{
		/* 1000 < step < 1999, moving up */
		case 1: step -= 1000;
			for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
			{
				subap_centroids_ref.y[ii] += step;
			}
			send_wfs_text_message("Moving up by %f", step);
			break;

		/* 2000 < step < 2999, moving down */      
		case 2: step -= 2000;
	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			subap_centroids_ref.y[ii] -= step;
	    		}
	  		send_wfs_text_message("Moving down by %f", step);
	  		break;

	  	/* 3000 < step < 3999, moving left */
		case 3: step -= 3000;
	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			subap_centroids_ref.x[ii] -= step;
	    		}
	  		send_wfs_text_message("Moving left by %f", step);
	  		break;

		/* 4000 < step < 4999, moving right */
		case 4: step -= 4000;
	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			subap_centroids_ref.x[ii] += step;
	    		}
	  		send_wfs_text_message("Moving right by %f", step);
	  		break;

	        /* 5000 < step < 5999, rotate in unit of degree*/
		case 5: step -= 5000;
	  		cosangle = cos(step/180.*PI);
	  		sinangle = sin(step/180.*PI);

	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			centerx += subap_centroids_ref.x[ii];
	      			centery += subap_centroids_ref.y[ii];
	    		}
	  		centerx /= WFS_DFT_SUBAP_NUMBER;
	  		centery /= WFS_DFT_SUBAP_NUMBER;

	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			disx = subap_centroids_ref.x[ii] - centerx;
				disy = subap_centroids_ref.y[ii] - centery;

				subap_centroids_ref.x[ii] = 
				    disx * cosangle - disy * sinangle + centerx;
	      			subap_centroids_ref.y[ii] = 
				    disx * sinangle + disy * cosangle + centery;

	    		}
	  		send_wfs_text_message("Rotating %f degrees", step);
	  		break;

	  	/* 6000 < step < 6999, scale the distance between spots*/
		case 6: step -= 6000;
	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			centerx += subap_centroids_ref.x[ii];
	      			centery += subap_centroids_ref.y[ii];
	    		}
	  		centerx /= WFS_DFT_SUBAP_NUMBER;
	  		centery /= WFS_DFT_SUBAP_NUMBER;

	  		for (ii = 0; ii < WFS_DFT_SUBAP_NUMBER; ii++)
	    		{
	      			disx = subap_centroids_ref.x[ii] - centerx;
	      			disy = subap_centroids_ref.y[ii] - centery;

	      			subap_centroids_ref.x[ii] = disx*step + centerx;
	      			subap_centroids_ref.y[ii] = disy*step + centery;
	    		}

	  		send_wfs_text_message("Scaled by %f ", step);
	  		break;

		default: return error(MESSAGE,"invalid input value");
	}

	/* Calculate new indexes */

	subap_calc_pix_mask();
	subap_centroids_ref = subap_centroids_ref;

	/* now send the new reference centroids position to the client */

	if (subap_send_centroids_ref() != NOERROR)
	{
		return error(ERROR, "reference centroids sent failed");
	}

	return NOERROR;

}/* message_wfs_subap_shift_rot_scale_centroids_ref() */

/************************************************************************/
/* message_wfs_subap_write_centroids_ref()				*/
/*                                                                      */
/* to write the reference centroids to a file                           */
/************************************************************************/

int message_wfs_subap_write_centroids_ref(struct smessage *message)
{
#warning THERE MAY BE OTHER THINGS TO SAVE HERE
	if (message->length ==  0)
		return wfs_write_ref_centroids(NULL);
	else
		return wfs_write_ref_centroids((char *)(message->data));

}/* message_wfs_subap_write_centroids_ref */

/************************************************************************/
/* message_wfs_subap_load_centroids_ref()				*/
/*                                                                      */
/* to load the reference centroids to a file                            */
/************************************************************************/

int message_wfs_subap_load_centroids_ref(struct smessage *message)
{
#warning THERE MAY BE OTHER THINGS TO LOAD HERE
	if (message->length ==  0)
		return wfs_load_ref_centroids(NULL);
	else
		return wfs_load_ref_centroids((char *)(message->data));

}/* message_wfs_subap_load_centroids_ref */

/************************************************************************/
/* int message_wfs_open_tt_data_socket()                                */
/*                                                                      */
/* to open data socket to tt correction                                 */
/************************************************************************/

int message_wfs_open_tt_data_socket(struct smessage *message)
{

        if (message->length !=  0)
        {
          return error(ERROR,
                       "Wrong number of data bytes in WFS_OPEN_TT_DATA_SOCKET");
        }

        if (open_tiptilt_data_socket(scope_types[scope_number]) != NOERROR)
        {
          return error(ERROR,
                   "Couldn't open data socket to tiptilt");
        }

	send_wfs_text_message(
		"Data socket to tiptilt has been opened successfully");

	return NOERROR;

} /* message_wfs_open_tt_data_socket() */

/************************************************************************/
/* int message_wfs_get_clamp_fluxes()                                   */
/*                                                                      */
/* Send current clamp fluxes.						*/
/************************************************************************/

int message_wfs_get_clamp_fluxes(struct smessage *mess)
{
        struct smessage message;

        if (mess->length !=  0)
        {
          return error(ERROR,
                       "Wrong number of data bytes in WFS_GET_CLAMP_FLUXES");
        }

        message.type = WFS_SET_CLAMP_FLUXES;
        message.length = sizeof(struct s_wfs_clamp_fluxes);
        message.data = (unsigned char *)&clamp_fluxes;

	if (server_send_message(active_socket, &message) != NOERROR)
        {
                return error(ERROR,
                        "Failed to send current clamp fluxes.");
        }


	return NOERROR;

} /* message_wfs_get_clamp_fluxes() */

/************************************************************************/
/* int message_wfs_subap_calc_centroids_ref()                           */
/*                                                                      */
/* Set boxes to current mean position.					*/
/************************************************************************/

int message_wfs_subap_calc_centroids_ref(struct smessage *mess)
{
        if (mess->length !=  0)
        {
          return error(ERROR,
	       "Wrong number of data bytes in WFS_SUBAP_CALC_CENTROIDS_REF");
        }

	if (wfs_tiptilt_servo.on)
	{
          send_wfs_text_message("Servo must be off to set centroids");
          return error(WARNING,"Servo must be off to set centroids");
	}

	set_subap_centroids_ref = TRUE;

	return NOERROR;

} /* message_wfs_subap_calc_centroids_ref() */

/************************************************************************/
/* int message_wfs_set_num_mean() 	                                */
/*                                                                      */
/*Set number of samples used for mean calculations.			*/
/************************************************************************/

int message_wfs_set_num_mean(struct smessage *mess)
{
	int	new;

        if (mess->length !=  sizeof(int))
        {
          return error(ERROR,
	       "Wrong number of data bytes in WFS_SET_NUM_MEAN");
        }

	new = *((int *)mess->data);

	if (new < 1 || new > 10000)
		num_mean_aberrations = 100;
	else
		num_mean_aberrations = new;

	error(MESSAGE,"Number of samples used in mean = %d",
		num_mean_aberrations);

	return NOERROR;

} /* message_wfs_set_num_mean() */

/************************************************************************/
/* int message_wfs_set_modulation()	 	                        */
/*                                                                      */
/* For controlling the test modulation.					*/
/************************************************************************/

int message_wfs_set_modulation(struct smessage *mess)
{
        if (mess->length !=  sizeof(struct s_wfs_tiptilt_modulation))
        {
          return error(ERROR,
	       "Wrong number of data bytes in WFS_SET_TIPTILT_MODULATION");
        }

	wfs_tiptilt_modulation = 
		*((struct s_wfs_tiptilt_modulation *)mess->data);

	return NOERROR;

} /* message_wfs_set_modulation() */

/************************************************************************/
/* int message_wfs_set_servo()	 	                  	        */
/*                                                                      */
/* For controlling the test servo.					*/
/************************************************************************/

int message_wfs_set_servo(struct smessage *mess)
{
        if (mess->length !=  sizeof(struct s_wfs_tiptilt_servo))
        {
          return error(ERROR,
	       "Wrong number of data bytes in WFS_SET_SERVO");
        }

	wfs_tiptilt_servo = 
		*((struct s_wfs_tiptilt_servo *)mess->data);

	if (wfs_tiptilt_servo.on)
		send_wfs_text_message("Tiptilt SERVO ON");
	else
		send_wfs_text_message("Tiptilt SERVO OFF");

	if (server_send_message_all(mess) != NOERROR)
			error(ERROR,"Failed to send WFS_SET_TIPTILT_SERVO.");
	return NOERROR;

} /* message_wfs_set_servo() */

/************************************************************************/
/* int message_wfs_get_servo()         		                        */
/*                                                                      */
/* Send current servo.							*/
/************************************************************************/

int message_wfs_get_servo(struct smessage *mess)
{
        struct smessage message;

        if (mess->length !=  0)
        {
          return error(ERROR, "Wrong number of data bytes in WFS_GET_SERVO");
        }

        message.type = WFS_SET_SERVO;
        message.length = sizeof(struct s_wfs_tiptilt_servo);
        message.data = (unsigned char *)&wfs_tiptilt_servo;

	if (server_send_message(active_socket, &message) != NOERROR)
        {
                return error(ERROR, "Failed to send current servo parameters.");
        }

	return NOERROR;

} /* message_wfs_get_servo() */

/************************************************************************/
/* int message_wfs_closeloop_message()		                        */
/*                                                                      */
/* Turn servo on or off directly.					*/
/************************************************************************/

int message_wfs_closeloop_message(struct smessage *mess)
{
        struct smessage message;

        if (mess->length !=  sizeof(int))
        {
          return error(ERROR, 
		"Wrong number of data bytes in WFS_CLOSELOOP_MESSAGE");
        }

	if (*((int *)mess->data))
	{
		wfs_tiptilt_servo.on = TRUE;
		send_wfs_text_message("Tiptilt SERVO ON");
	}
	else
	{
		wfs_tiptilt_servo.on = FALSE;
		wfs_tiptilt.correctx = 0.0;
		wfs_tiptilt.correcty = 0.0;
		send_wfs_text_message("Tiptilt SERVO OFF");
	}
	
        message.type = WFS_SET_SERVO;
        message.length = sizeof(struct s_wfs_tiptilt_servo);
        message.data = (unsigned char *)&wfs_tiptilt_servo;

	if (server_send_message_all(&message) != NOERROR)
        {
                return error(ERROR, "Failed to send current servo parameters.");
        }

	return NOERROR;

} /* message_wfs_closeloop_message() */

/************************************************************************/
/* int message_wfs_set_send()		                        	*/
/*                                                                      */
/* Turn sending on or off directly.					*/
/************************************************************************/

int message_wfs_set_send(struct smessage *mess)
{
        struct smessage message;

        if (mess->length !=  sizeof(int))
        {
          return error(ERROR, "Wrong number of data bytes in WFS_SET_SEND");
        }

	if (*((int *)mess->data))
		wfs_tiptilt_servo.send = TRUE;
	else
		wfs_tiptilt_servo.send = FALSE;
	
        message.type = WFS_SET_SERVO;
        message.length = sizeof(struct s_wfs_tiptilt_servo);
        message.data = (unsigned char *)&wfs_tiptilt_servo;

	if (server_send_message(active_socket, &message) != NOERROR)
        {
                return error(ERROR, "Failed to send current servo parameters.");
        }

	return NOERROR;

} /* message_wfs_set_send() */

/************************************************************************/
/* int message_wfs_zero_tiptilt()	                        	*/
/*                                                                      */
/* Zero tiptilt position.						*/
/************************************************************************/

int message_wfs_zero_tiptilt(struct smessage *mess)
{
        if (mess->length !=  0)
        {
          return error(ERROR, "Wrong number of data bytes in WFS_ZERO_TIPTILT");
        }

	wfs_tiptilt.correctx = 0.0;
	wfs_tiptilt.correcty = 0.0;
	
	return NOERROR;

} /* message_wfs_set_send() */
