/************************************************************************/
/* wfs_autoalign.c							*/
/*                                                                      */
/* Contains automation for aligning things.				*/
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
/* Date   : Dec 2015							*/
/************************************************************************/

#include "wfs_server.h"

static int telescope_server = -1;
static struct s_scope_status telescope_status;
static int autoalign_parabola_focus_count = 0;
static bool autoalign_parabola_focus = FALSE;
static int autoalign_beacon_count = 0;
static bool autoalign_beacon = FALSE;
static int autoalign_telescope_focus_count = 0;
static bool autoalign_telescope_focus = FALSE;

#define PARABOLA_FOCUS_STEP 2000
#define PARABOLA_FOCUS_LIMIT 0.05
#define PARABOLA_FOCUS_GAIN 40000
#define PARABOLA_FOCUS_DELAY 5

#define TELESCOPE_FOCUS_STEP 0.0
#define TELESCOPE_FOCUS_LIMIT 0.05
#define TELESCOPE_FOCUS_GAIN 0.0
#define TELESCOPE_FOCUS_DELAY 5

#define BEACON_STEP 50
#define BEACON_LIMIT 0.01
#define BEACON_GAIN 2000
#define BEACON_DELAY 2

/************************************************************************/
/* open_telescope_connection()                                          */
/*                                                                      */
/* Open connection to telescope server.                                 */
/************************************************************************/

int open_telescope_connection(int argc, char **argv)
{

        if (telescope_server != -1) close_server_socket(telescope_server);

        telescope_server = open_server_by_name(scope_types[scope_number]);

        if (telescope_server < 0)
        {
            telescope_server = -1;
           return error(ERROR, "Failed to open socket connect to telescope %s",
                        scope_types[scope_number]);
        }

        if (!add_message_job(telescope_server,
                        TELESCOPE_STATUS, message_telescope_status))
        {
                fprintf(stderr,"Failed to add server TELESCOPE_STATUS, job.\n");
                exit(-8);
        }

        if (verbose) error(MESSAGE, "Opened connection to Telescope Server.");
        send_wfs_text_message("Opened connection to Telescope Server.");

        return NOERROR;

} /* int open_telescope_connection() */

/************************************************************************/
/* message_telescope_status()                                           */
/*                                                                      */
/************************************************************************/

int message_telescope_status(int server, struct smessage *mess)
{
        struct s_scope_status *status;

        if (mess->length != sizeof(*status))
        {
            error(ERROR,"Got TELESCOPE_STATUS with bad data.");
            return TRUE;
        }

        status = (struct s_scope_status *)mess->data;

        telescope_status = *status;

        return TRUE;

} /* message_telescope_status() */

/************************************************************************/
/* message_wfs_start_focus_parabola()                                   */
/*                                                                      */
/* Begin focusing the parabola sending light to the WFS.		*/
/************************************************************************/

int message_wfs_start_focus_parabola(struct smessage *message)
{
        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_START_FOUCS_PARABOLA.");
        }

        autoalign_parabola_focus_count = *((int *)message->data);

	if (autoalign_beacon || autoalign_telescope_focus) 
		return error(ERROR,"Already autoaligning.");

	if (open_telescope_connection(0, NULL) != NOERROR) return ERROR;

	if (verbose)
	{
            error(MESSAGE, "Autoalignment of parabola focus begins. Trys = %d",
		autoalign_parabola_focus_count);
	}
        send_wfs_text_message(
		"Autoalignment of parabola focus begins. Trys = %d",
		autoalign_parabola_focus_count);

	autoalign_parabola_focus = TRUE;

	return NOERROR;

} /* message_wfs_start_focus_parabola() */

/************************************************************************/
/* autoalign_focus_parabola()						*/
/*									*/
/* Routine that attempts to align the focus of the WFS feed parabola.	*/
/************************************************************************/

void autoalign_focus_parabola(void)
{
	static time_t last_time = 0;
	struct smessage mess;
	struct s_aob_move_motor motor_move;

	/* Need we do anything at all? */

	if (!autoalign_parabola_focus) return;

	/* Don't do this too often */

	if (time(NULL) < last_time+PARABOLA_FOCUS_DELAY) return;
	last_time = time(NULL);

	/* Are we done? */

	if (fabs(wfs_mean_aberrations.focus) < PARABOLA_FOCUS_LIMIT)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of parabola focus complete.");
		}
		send_wfs_text_message(
			"Autoalignment of parabola focus complete.");

		autoalign_parabola_focus = FALSE;
		autoalign_parabola_focus_count = 0;
		return;
	}

	/* Have we tried too many times? */

	if (--autoalign_parabola_focus_count < 0)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of parabola focus failed.");
		}
		send_wfs_text_message(
			"Autoalignment of parabola focus failed.");

		autoalign_parabola_focus = FALSE;
		autoalign_parabola_focus_count = 0;
		return;
	}

	/* OK, move things around */

	mess.type = HUT_AOB_MOVE_RELATIVE;
	mess.length = sizeof(motor_move);
	mess.data = (unsigned char *)&motor_move;

	if (include_old_S2_code && scope_number == S2)
		motor_move.motor = AOB_S2_WFS_PAROB_FOC;
	else
		motor_move.motor = AOB_WFS_PAROB_FOC;

	motor_move.position = PARABOLA_FOCUS_GAIN * 
					fabs(wfs_mean_aberrations.focus)
				+ PARABOLA_FOCUS_STEP;

	if (wfs_mean_aberrations.focus < 0) motor_move.position *= -1.0;

	send_message(telescope_server, &mess);

	send_wfs_text_message("Tries %d Focus = %.2f Moving Focus %d.", 
		autoalign_parabola_focus_count, 
		wfs_mean_aberrations.focus, 
		motor_move.position);

} /* align_focus_parabola() */

/************************************************************************/
/* message_wfs_start_align_beacon()                               	*/
/*                                                                      */
/* Begin aligning the beacon to the WFS.				*/
/************************************************************************/

int message_wfs_start_align_beacon(struct smessage *message)
{
        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_START_FOUCS_PARABOLA.");
        }

        autoalign_beacon_count = *((int *)message->data);

	if (autoalign_parabola_focus || 
	    autoalign_telescope_focus) 
		return error(ERROR,"Already autoaligning.");

	if (open_telescope_connection(0, NULL) != NOERROR) return ERROR;

	if (verbose)
	{
            error(MESSAGE, "Autoalignment of beacon begins. Trys = %d",
		autoalign_beacon_count);
	}
        send_wfs_text_message(
		"Autoalignment of beacon begins. Trys = %d",
		autoalign_beacon_count);

	autoalign_beacon = TRUE;

	return NOERROR;

} /* message_wfs_start_align_beacon() */

/************************************************************************/
/* autoalign_beacon_to_wfs()						*/
/*									*/
/* Tries to align beacon to WFS.					*/
/************************************************************************/

void autoalign_beacon_to_wfs(void)
{
	static time_t last_time = 0;
	struct smessage mess;
	struct s_aob_move_motor motor_move;

	/* Need we do anything at all? */

	if (!autoalign_beacon) return;

	/* Don't do this too often */

	if (time(NULL) < last_time+BEACON_DELAY) return;
	last_time = time(NULL);

	/* Are we done? */

	if (fabs(wfs_mean_aberrations.xtilt) < BEACON_LIMIT ||
	    fabs(wfs_mean_aberrations.ytilt) < BEACON_LIMIT)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of beacon complete.");
		}
		send_wfs_text_message(
			"Autoalignment of beacon complete.");

		autoalign_beacon = FALSE;
		autoalign_beacon_count = 0;
		return;
	}

	/* Have we tried too many times? */

	if (--autoalign_beacon_count < 0)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of beacon failed.");
		}
		send_wfs_text_message(
			"Autoalignment of beacon failed.");

		autoalign_beacon = FALSE;
		autoalign_beacon_count = 0;
		return;
	}

	/* OK, move things around */

	mess.type = HUT_AOB_MOVE_RELATIVE;
	mess.length = sizeof(motor_move);
	mess.data = (unsigned char *)&motor_move;

	/* Do the axis one at a time. */

	if (autoalign_beacon_count%2)
	{
	    if (fabs(wfs_mean_aberrations.xtilt) >  BEACON_LIMIT)
	    {
		/* Do X */

		if (include_old_S2_code && scope_number == S2)
			motor_move.motor = AOB_S2_BFLAT_2;
		else
			motor_move.motor = AOB_BFLAT_2;

		motor_move.position = BEACON_GAIN * 
				fabs(wfs_mean_aberrations.xtilt)
				+ BEACON_STEP;

		if (wfs_mean_aberrations.xtilt < 0) motor_move.position *= -1.0;

		send_message(telescope_server, &mess);

		send_wfs_text_message("Tries %d Xtilt = %.2f Moving X %d.", 
			autoalign_beacon_count, 
			wfs_mean_aberrations.xtilt, 
			motor_move.position);
	    }
	    else
	    {
		send_wfs_text_message("Tries %d Xtilt OK.", 
			autoalign_beacon_count); 
	    }

	    return;
	}

	/* Do Y */

	if (fabs(wfs_mean_aberrations.ytilt) >  BEACON_LIMIT)
	{
		if (include_old_S2_code && scope_number == S2)
			motor_move.motor = AOB_S2_BFLAT_1;
		else
			motor_move.motor = AOB_BFLAT_1;

		motor_move.position = BEACON_GAIN * 
				fabs(wfs_mean_aberrations.ytilt)
				+ BEACON_STEP;

		if (wfs_mean_aberrations.ytilt > 0) motor_move.position *= -1.0;

		send_message(telescope_server, &mess);

		send_wfs_text_message("Tries %d Ytilt = %.2f Moving X %d.", 
			autoalign_beacon_count, 
			wfs_mean_aberrations.ytilt, 
			motor_move.position);
	}
	else
	{
		send_wfs_text_message("Tries %d Ytilt OK.", 
			autoalign_beacon_count); 
	}

} /* autoalign_beacon_to_wfs() */

/************************************************************************/
/* message_wfs_start_focus_telescope()                                   */
/*                                                                      */
/* Begin focusing the telescope sending starlight to the WFS.		*/
/************************************************************************/

int message_wfs_start_focus_telescope(struct smessage *message)
{
        if (message->length != sizeof(int))
        {
                return error(ERROR,
                "Wrong number of data bytes in WFS_START_FOUCS_TELESCOPE.");
        }

        autoalign_telescope_focus_count = *((int *)message->data);

	if (autoalign_beacon || autoalign_parabola_focus) 
		return error(ERROR,"Already autoaligning.");

	if (open_mercury_server() != NOERROR) return ERROR;

	if (verbose)
	{
            error(MESSAGE, "Autoalignment of telescope focus begins. Trys = %d",
		autoalign_telescope_focus_count);
	}
        send_wfs_text_message(
		"Autoalignment of telescope focus begins. Trys = %d",
		autoalign_telescope_focus_count);

	autoalign_telescope_focus = TRUE;

	return NOERROR;

} /* message_wfs_start_focus_telescope() */

/************************************************************************/
/* autoalign_focus_telescope()						*/
/*									*/
/* Routine that attempts to align the focus of the telescope.		*/
/************************************************************************/

void autoalign_focus_telescope(void)
{
	static time_t last_time = 0;
	float	      position = 0.0;

	/* Need we do anything at all? */

	if (!autoalign_telescope_focus) return;

	/* Don't do this too often */

	if (time(NULL) < last_time+TELESCOPE_FOCUS_DELAY) return;
	last_time = time(NULL);

	/* Are we done? */

	if (fabs(wfs_mean_aberrations.focus) < TELESCOPE_FOCUS_LIMIT)
	{
		if (verbose)
		{
		    error(MESSAGE,"Autoalignment of telescope focus complete.");
		}
		send_wfs_text_message(
			"Autoalignment of telescope focus complete.");

		autoalign_telescope_focus = FALSE;
		autoalign_telescope_focus_count = 0;
		return;
	}

	/* Have we tried too many times? */

	if (--autoalign_telescope_focus_count < 0)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of telescope focus failed.");
		}
		send_wfs_text_message(
			"Autoalignment of telescope focus failed.");

		autoalign_telescope_focus = FALSE;
		autoalign_telescope_focus_count = 0;
		return;
	}

	/* OK, move things around */

	position = TELESCOPE_FOCUS_GAIN * fabs(wfs_mean_aberrations.focus)
				+ TELESCOPE_FOCUS_STEP;

	if (wfs_mean_aberrations.focus < 0) position *= -1.0;

	mercury_focus(position);

	send_wfs_text_message("Tries %d Focus = %.2f Moving Scope Focus %.1f", 
		autoalign_telescope_focus_count, 
		wfs_mean_aberrations.focus, 
		position);

} /* align_focus_telescope() */

/************************************************************************/
/* message_wfs_stop_autoalign()  	                                */
/*                                                                      */
/* Stop all autoalignment proceedures.					*/
/************************************************************************/

int message_wfs_stop_autoalign(struct smessage *message)
{
        if (message->length != 0)
        {
                error(ERROR,
                "Wrong number of data bytes in WFS_STOP_AUTOALIGN.");
	}

	autoalign_parabola_focus = FALSE;
	autoalign_parabola_focus_count = 0;
	autoalign_beacon = FALSE;
	autoalign_beacon_count = 0;
	autoalign_telescope_focus = FALSE;
	autoalign_telescope_focus_count = 0;

	send_wfs_text_message("All Autoalignment STOPPED.");

	return NOERROR;

} /* message_wfs_stop_autoalign() */
