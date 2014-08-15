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

/************************************************************************/
/* process_data()							*/
/*									*/
/* Since it is called inside the USB thread for now the usb mutex will  */
/* be locked while it is running. 					*/
/************************************************************************/

void process_data(long time_stamp, int nx, int ny, unsigned short int *ccd)
{
	static time_t last_time = 0;
	static int dark_frame_count = 0;
	static int sum_frame_count = 1;
	int	i,j;
	at_u16  *pi1;

	/* Do we have somewhere to put the data? */

	if (data_frame == NULL || dark_frame == NULL) return;

	/* Copy the data over */

        for(pi1 = image_data, j=andor_setup.npixy; j > 0; j--)
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
		}

		/* Add this frame */

		for(i = 1; i <= andor_setup.npixx; i++)
		for(j = 1; j <= andor_setup.npixy; j++)
		{
			calc_dark_frame[i][j] += raw_frame[i][j];
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
			dark_frame_num = 0;
			dark_frame_count = 0;
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
		
#warning Here is where we actually do something with the data 

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

	if (verbose) error(MESSAGE, "Background reset.");

	return NOERROR;

} /* message_wfs_reset_background() */

/************************************************************************/
/* message_wfs_set_threshold()                                          */
/*                                                                      */
/************************************************************************/

int message_wfs_set_threshold(struct smessage *message)
{
        if (message->length != sizeof(float))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_SET_THRESHOLD.");
        }

	data_threshold = *((float *)message->data);

	if (verbose) error(MESSAGE, "Threshold set to %.1f",  data_threshold);

	server_send_message_all(message);

	return NOERROR;

} /* message_wfs_set_threshold() */

/************************************************************************/
/* message_wfs_set_num_frames()                                         */
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
