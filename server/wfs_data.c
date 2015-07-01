/************************************************************************/
/* wfs_data.c								*/
/*                                                                      */
/* Here is where the real WFS calculations will be done. For now, this	*/
/* is a place holder function and uses USB data. IT will eventually	*/
/* get data via CameraLink.						*/
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
#include <fitsio.h>

#define MAX_TIPTILT_RECORD	10000
#define MAX_DATA_RECORD		10000

static int tiptilt_record_num = 0;
static int tiptilt_record_count = 0;
static struct s_wfs_aberrations tiptilt_record[MAX_TIPTILT_RECORD];
static float *data_cube = NULL;
static float *data_pointer = NULL;
static int data_record_num = 0;
static int data_record_count = 0;
static int data_record_start = 0;
static int data_record_stop = 0;

/************************************************************************/
/* process_data()							*/
/*									*/
/* Since it is called inside the USB thread for now the usb mutex will  */
/* be locked while it is running. 					*/
/************************************************************************/

void process_data(long time_stamp)
{
	static time_t last_time = 0;
	static int dark_frame_count = 0;
	static int sum_frame_count = 1;
	static float dark_frame_mean2;
	static float dark_frame_mean_n;
	int	i,j;
	at_u16  *pi1;

	/* Do we have somewhere to put the data? */

	if (data_frame == NULL || dark_frame == NULL) return;

	/* Copy the data over */

        for(pi1 = image_data, j = 1; j <= andor_setup.npixy; j++)
        for(i = 1; i <= andor_setup.npixx; i++)
	{
		raw_frame[i][j] = (float)*pi1++;
	}

	/* Are we building a new dark frame? */

	if (dark_frame_num > 0)
	{
		/* Is this first? */

		if (dark_frame_count == 0)
		{
			for(i = 1; i <= andor_setup.npixx; i++)
			for(j = 1; j <= andor_setup.npixy; j++)
			{
				calc_dark_frame[i][j] = 0.0;
			}
			dark_frame_count = dark_frame_num;
			dark_frame_mean = 0.0;
			dark_frame_mean2 = 0.0;
			dark_frame_mean_n = 0.0;
		}

		/* Add this frame */

		for(i = 1; i <= andor_setup.npixx; i++)
		for(j = 1; j <= andor_setup.npixy; j++)
		{
			calc_dark_frame[i][j] += raw_frame[i][j];
			dark_frame_mean += raw_frame[i][j];
			dark_frame_mean2 += (raw_frame[i][j] * raw_frame[i][j]);
			dark_frame_mean_n += 1.0;
		}

		/* Are we done? */

		if (--dark_frame_count <= 0)
		{
			for(i = 1; i <= andor_setup.npixx; i++)
			for(j = 1; j <= andor_setup.npixy; j++)
			{
				dark_frame[i][j] = calc_dark_frame[i][j]/
					(float)dark_frame_num;
				calc_dark_frame[i][j] = 0.0;
			}
			dark_frame_mean /= dark_frame_mean_n;
			dark_frame_mean2 /= dark_frame_mean_n;
			dark_frame_stddev = sqrt(dark_frame_mean2 - 
				dark_frame_mean * dark_frame_mean);
			dark_frame_num = 0;
			dark_frame_count = 0;

			send_wfs_text_message(
			    "Dark frame complete. Mean = %.1f Sigma = %.1f.", 
			    dark_frame_mean, dark_frame_stddev);
		}
	}

	/* We now subtract the dark frame and do the thresholding */

        for(i = 1; i <= andor_setup.npixx; i++)
        for(j = 1; j <= andor_setup.npixy; j++)
	{
		raw_frame[i][j] -= dark_frame[i][j];
		if (raw_frame[i][j] < data_threshold) raw_frame[i][j] = 0.0;
	}

#warning Here is where we would implement photon identification

	/* Add this frame in */

        for(i = 1; i <= andor_setup.npixx; i++)
        for(j = 1; j <= andor_setup.npixy; j++)
	{
		sum_frame[i][j] += raw_frame[i][j];
	}

	/* Now we sum the frames if we must */

	if (--sum_frame_count <= 0)
	{
		sum_frame_count = sum_frame_num;
		for(i = 1; i <= andor_setup.npixx; i++)
		for(j = 1; j <= andor_setup.npixy; j++)
		{
			data_frame[i][j] = sum_frame[i][j];
			sum_frame[i][j] = 0.0;
		}
		
		/* Now calculate the centroids */

		calculate_centroids();

		/* Calculate Tip/Tilt */

		calculate_tiptilt();

		/* Do the servo part of teh tiptilt */

		servo_tiptilt();

		/* Are we recording the tiptilt data? */

		if (tiptilt_record_num > 0 && 
			tiptilt_record_count < tiptilt_record_num)
		{
			wfs_aberrations.time_stamp = time_stamp;
			tiptilt_record[tiptilt_record_count++]=wfs_aberrations;
		}

		/* Are we recording the raw data? */

		if (data_record_num > 0 && data_record_count < data_record_num)
		{
			if (data_record_count == 0)
				data_record_start = time_stamp;

			for(j = 1; j <= andor_setup.npixy; j++)
			for(i = 1; i <= andor_setup.npixx; i++)
			{
				*data_pointer++ = data_frame[i][j];
			}

			if (++data_record_count == data_record_num)
				data_record_stop = time_stamp;
		}

		/* OK, we have processed one more frame. */

		number_of_processed_frames++;
	}

	/* Is it time to check things out? */

	if (time(NULL) > last_time)
	{
		andor_setup.processed_frames_per_second = 
			number_of_processed_frames;
		number_of_processed_frames = 0;
		last_time = time(NULL);
	}

} /* process_data() */

/************************************************************************/
/* message_wfs_take_background()                                        */
/*                                                                      */
/************************************************************************/

int message_wfs_take_background(struct smessage *message)
{
        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_TAKE_BACKGROUND.");
        }

	/* Are we already work on this? */

	if (dark_frame_num != 0)
	{
		error(WARNING,"Already calculating a background.");
		return NOERROR;
	}

	/* Otherwise, go for it */

	dark_frame_num = *((int *)message->data);

	if (verbose) error(MESSAGE,
		"Creating new background based on %d frames.", dark_frame_num);
	send_wfs_text_message(
		"Creating new background based on %d frames.", dark_frame_num);

	return NOERROR;

} /* message_wfs_take_background() */

/************************************************************************/
/* message_wfs_reset_background()                                       */
/*                                                                      */
/************************************************************************/

int message_wfs_reset_background(struct smessage *message)
{
	int	i, j;

        if (message->length != 0)
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_RESET_BACKGROUND.");
        }

	/* Are we already work on this? */

	if (dark_frame_num != 0)
	{
		return error(NOERROR,"Already calculating a background.");
	}

	for(i = 1; i <= andor_setup.npixx; i++)
	for(j = 1; j <= andor_setup.npixy; j++)
	{
		dark_frame[i][j] = 0.0;
	}

	dark_frame_num = 0;
	dark_frame_mean = 0.0;
	dark_frame_stddev = 0.0;
	data_threshold = -1e32;

	if (verbose) error(MESSAGE, "Background reset.");
	send_wfs_text_message("Background reset.");

	return NOERROR;

} /* message_wfs_reset_background() */

/************************************************************************/
/* message_wfs_set_threshold()                                          */
/*                                                                      */
/************************************************************************/

int message_wfs_set_threshold(struct smessage *message)
{
	float	thresh;

        if (message->length != sizeof(float))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_SET_THRESHOLD.");
        }

	thresh = *((float *)message->data);

	if (thresh <= 0.0)
	{
		data_threshold = -1e32;
		if (verbose) error(MESSAGE, "Data thresholding OFF.");
		send_wfs_text_message("Data thresholding OFF.");

		return NOERROR;
	}

	if (dark_frame_stddev == 0.0)
	{
		return error(ERROR,
		"No dark frame - thresholding is not possible.");
	}

	data_threshold = (*((float *)message->data)) * dark_frame_stddev;

	if (verbose) error(MESSAGE, "Dark Sigma = %.1f, threshold set to %.1f", 
		dark_frame_stddev, data_threshold);

	send_wfs_text_message("Dark Sigma = %.1f, threshold set to %.1f", 
		dark_frame_stddev, data_threshold);

	server_send_message_all(message);

	return NOERROR;

} /* message_wfs_set_threshold() */

/************************************************************************/
/* message_wfs_set_num_frames()                                           */
/*                                                                      */
/************************************************************************/

int message_wfs_set_num_frames(struct smessage *message)
{
        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_SET_NUM_FRAMES.");
        }

	sum_frame_num = *((int *)message->data);

	if (verbose) error(MESSAGE, "Number of frames set to %d", 
		sum_frame_num);

	server_send_message_all(message);

	return NOERROR;

} /* message_wfs_set_num_frames() */

/************************************************************************/
/* message_wfs_save_tiptilt()						*/
/*									*/
/* Allocate memory, setup globals for recording tiptilt data.		*/
/************************************************************************/

int message_wfs_save_tiptilt(struct smessage *message)
{
	int	num = 0;

        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_SETUP_TIPTILT_RECORD.");
        }

	/* Are we already recording data? */

	if (tiptilt_record_num != 0)
	{
		send_wfs_text_message("We are already recording tiptilt data.");		return NOERROR;
	}

	num = *((int *)message->data);

	if (num <= 0) return NOERROR;
	if (num > MAX_TIPTILT_RECORD) num = MAX_TIPTILT_RECORD;

	lock_usb_mutex();
	lock_camlink_mutex();
	tiptilt_record_count = 0;
	tiptilt_record_num = num;
	unlock_usb_mutex();
	unlock_camlink_mutex();

	return NOERROR;

}  /* message_wfs_save_tiptilt() */

/************************************************************************/
/* complete_tiptilt_record()						*/
/*									*/
/* Should be called periodically to see if there is a complete set	*/
/* of tiptilt data recorded we need to save.				*/
/************************************************************************/

void complete_tiptilt_record(void)
{
	int	year, month, day;
	char	filename[345];
	char	s[345];
	FILE	*fp;
	time_t	now;
	struct tm *gmt_now;
	int	i;

	/* Have we finished? */

	lock_usb_mutex();
	lock_camlink_mutex();
	if (tiptilt_record_num == 0 ||
	    tiptilt_record_count < tiptilt_record_num)
	{
		unlock_usb_mutex();
		unlock_camlink_mutex();
		return;
	}
	unlock_usb_mutex();
	unlock_camlink_mutex();

	/* We have data, let's find a good filename */

	time(&now);
	gmt_now = gmtime(&now);
	year = gmt_now->tm_year + 1900;
	month = gmt_now->tm_mon+1;
	day = gmt_now->tm_mday;
	
	if (year < 1950) year+=100; /* Y2K.... */

	for(i=1; i<1000; i++)
	{
		sprintf(filename,"%s/%4d_%02d_%02d_%s_tiptilt_%03d.dat",
			get_data_directory(s), year, month, day, wfs_name, i);

		if ((fp = fopen(filename, "r")) == NULL) break;
		fclose(fp);
	}

	if (i > 999) 
	{
		error(ERROR,"Too many tiptilt files.");
		return;
	}

	/* OK, save the data */

	if ((fp = fopen(filename, "w")) == NULL)
	{
		error(ERROR, "Failed to open file %s", filename );
		return;
	}

	fprintf(fp,"# FILENAME        : %s\n",filename);
	fprintf(fp,"# GMT YEAR        : %d\n",year);
	fprintf(fp,"# GMT MONTH       : %d\n",month);
	fprintf(fp,"# GMT DAY         : %d\n",day);
	fprintf(fp,
	"# Time      Det X   Det Y   Mir X   Mir Y   Focus    A1      A2\n");

	
	for(i=0; i< tiptilt_record_num; i++)
		fprintf(fp, "%9d %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f %7.4f\n",
			tiptilt_record[i].time_stamp,			
			tiptilt_record[i].xtilt,			
			tiptilt_record[i].ytilt,			
			tiptilt_record[i].det_stddev,  /* Actually correctx. */
			tiptilt_record[i].mir_stddev,  /* Actually correcty. */
			tiptilt_record[i].focus,
			tiptilt_record[i].a1,
			tiptilt_record[i].a2);

	fclose(fp);
	lock_usb_mutex();
	lock_camlink_mutex();
	tiptilt_record_num = 0;
	tiptilt_record_count = 0;
	unlock_usb_mutex();
	unlock_camlink_mutex();

	error(MESSAGE,"Saved file %s", filename);
	send_wfs_text_message("Saved file %s", filename);
	sprintf(s,"chown observe %s; chgrp chara %s", filename, filename);
	system(s);

} /* complete_tiptilt_record() */

/************************************************************************/
/* message_wfs_save_data()						*/
/*									*/
/* Allocate memory, setup globals for recording raw data.		*/
/************************************************************************/

int message_wfs_save_data(struct smessage *message)
{
	int	num = 0;

        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_SETUP_DATA_RECORD.");
        }

	/* Are we already recording data? */

	if (data_record_num != 0)
	{
		send_wfs_text_message("We are already recording raw data.");
		return NOERROR;
	}

	num = *((int *)message->data);

	if (num <= 0) return NOERROR;
	if (num > MAX_DATA_RECORD) num = MAX_DATA_RECORD;

	lock_usb_mutex();
	lock_camlink_mutex();
	data_cube = (float *)calloc((size_t)andor_setup.npixx * 
				    (size_t)andor_setup.npixy * (size_t)num,
				    sizeof(float));
	if (data_cube == NULL)
	{
		unlock_usb_mutex();
		unlock_camlink_mutex();
		return error(ERROR,"Failed to allocate memory for raw data.");
	}
	data_pointer = data_cube;
	data_record_count = 0;
	data_record_num = num;
	unlock_usb_mutex();
	unlock_camlink_mutex();

	return NOERROR;

}  /* message_wfs_save_data() */

/************************************************************************/
/* complete_data_record()						*/
/*									*/
/* Should be called periodically to see if there is a complete set	*/
/* of data data recorded we need to save.				*/
/************************************************************************/

void complete_data_record(void)
{
	int	year, month, day;
	char	filename[345];
	char	s1[345];
	char	s2[345];
	FILE	*fp;
	time_t	now;
	struct tm *gmt_now;
	int	i;
        long int fpixel, nelements;
        int     bitpix;
        int      fits_status;
        long int naxis;
        long int naxes[3];
	fitsfile *fptr;
	float	x,y;

	/* Have we finished? */

	lock_usb_mutex();
	lock_camlink_mutex();
	if (data_record_num == 0 || data_record_count < data_record_num)
	{
		unlock_usb_mutex();
		unlock_camlink_mutex();
		return;
	}
	unlock_usb_mutex();
	unlock_camlink_mutex();

	/* We have data, let's find a good filename */

	time(&now);
	gmt_now = gmtime(&now);
	year = gmt_now->tm_year + 1900;
	month = gmt_now->tm_mon+1;
	day = gmt_now->tm_mday;
	
	if (year < 1950) year+=100; /* Y2K.... */

	for(i=1; i<1000; i++)
	{
		sprintf(filename,"%s/%4d_%02d_%02d_%s_data_%03d.fit",
			get_data_directory(s1), year, month, day, wfs_name, i);

		if ((fp = fopen(filename, "r")) == NULL) break;
		fclose(fp);
	}

	if (i > 999) 
	{
		error(ERROR,"Too many data files.");
		return;
	}

	/* Save the data */

	naxis = 3;
	bitpix = FLOAT_IMG;
	naxes[0] = andor_setup.npixx;
	naxes[1] = andor_setup.npixy;
	naxes[2] = data_record_num;
	fpixel = 1;
	nelements = naxes[0] * naxes[1] * naxes[2];

	/* Create a new FITS file */

	fits_status = 0;
	if (fits_create_file(&fptr, filename, &fits_status))
	{
		error(ERROR,"Failed to create FITS file (%d).",
			fits_status);
	}

	/* Write required keywords into the header */

	if (fits_create_img(fptr,  bitpix, naxis, naxes, &fits_status))
	{
		error(ERROR,"Failed to create FITS image (%d).",
			fits_status);
	}

	/* Write the FITS image */

	if (fits_write_img(fptr, TFLOAT, fpixel, nelements, data_cube,
                        &fits_status))
	{
		error(ERROR,"Failed to write FITS image (%d).",
                                fits_status);
	}

	/* Now all the relvant header information */

	if(fits_update_key(fptr, TINT, "WFSSTART", &data_record_start,
			"Time of first datum (mS)",&fits_status))
	{
		error(ERROR,"Failed to write WFSSTART (%d).", fits_status);
	}

	if(fits_update_key(fptr, TINT, "WFSSTOP", &data_record_stop,
			"Time of last datum (mS)",&fits_status))
	{
		error(ERROR,"Failed to write WFSSTOP (%d).", fits_status);
	}

	if(fits_update_key(fptr, TFLOAT, "WFSEXP",
                        &(andor_setup.exposure_time),
                        "Exposure time.",&fits_status))
	{
		error(ERROR,"Failed to write WFSEXP (%d).", fits_status);
	}

	if(fits_update_key(fptr, TINT, "WFSNPIXX", &(andor_setup.npixx),
			"Number of Pixels in X axis",&fits_status))
	{
		error(ERROR,"Failed to write WFSNPIXX (%d).", fits_status);
	}

	if(fits_update_key(fptr, TINT, "WFSNPIXY", &(andor_setup.npixy),
			"Number of Pixels in Y axis",&fits_status))
	{
		error(ERROR,"Failed to write WFSNPIXX (%d).", fits_status);
	}

	for(i=0; i < WFS_DFT_SUBAP_NUMBER; i++)
	{
		sprintf(s1,"WFSXRF%02d", i+1);
		sprintf(s2,"X position reference centroid %02d", i+1);

		x = subap_centroids_ref.x[i] + subap_centroids_offset.x[i];
		if(fits_update_key(fptr, TFLOAT, s1, &x, s2,&fits_status))
		{
			error(ERROR,"Failed to write %s (%d).", s1,fits_status);
		}

		sprintf(s1,"WFSYRF%02d", i+1);
		sprintf(s2,"Y position reference centroid %02d", i+1);

		y = subap_centroids_ref.y[i] + subap_centroids_offset.y[i];
		if(fits_update_key(fptr, TFLOAT, s1, &y, s2, &fits_status))
		{
			error(ERROR,"Failed to write %s (%d).", s1,fits_status);
		}
	}

	/* That should be enough! */

	if (fits_close_file(fptr, &fits_status))
	{
		error(ERROR,"Failed to close fits file (%d).", fits_status);
	} 

	/* Clean up */

	lock_usb_mutex();
	lock_camlink_mutex();
	data_record_num = 0;
	data_record_count = 0;
	free(data_cube);
	data_cube = NULL;
	data_pointer = NULL;
	unlock_usb_mutex();
	unlock_camlink_mutex();

	error(MESSAGE,"Saved file %s", filename);
	send_wfs_text_message("Saved file %s", filename);
	sprintf(s1,"chown observe %s; chgrp chara %s", filename, filename);
	system(s1);

} /* complete_data_record() */
