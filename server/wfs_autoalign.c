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
static int autoalign_focus_count = 0;
static bool autoalign_focus = FALSE;

#define FOCUS_STEP 10000
#define FOCUS_LIMIT 0.02
#define FOCUS_GAIN 500000

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
/* message_wfs_start_focus_wfs_parabola()                               */
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

        autoalign_focus_count = *((int *)message->data);

	if (open_telescope_connection(0, NULL) != NOERROR) return ERROR;

	if (verbose)
	{
            error(MESSAGE, "Autoalignment of parabola focus begins. Trys = %d",
		autoalign_focus_count);
	}
        send_wfs_text_message(
		"Autoalignment of parabola focus begins. Trys = %d",
		autoalign_focus_count);

	autoalign_focus = TRUE;

	return NOERROR;

} /* message_wfs_andor_setup() */

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

	if (!autoalign_focus) return;

	/* Don't do this too often */

	if (time(NULL) < last_time+3) return;
	last_time = time(NULL);

	/* Are we done? */

	if (fabs(wfs_mean_aberrations.focus) < FOCUS_LIMIT)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of parabola focus complete.");
		}
		send_wfs_text_message(
			"Autoalignment of parabola focus complete.");

		autoalign_focus = FALSE;
		autoalign_focus_count = 0;
		return;
	}

	/* Have we tried too many times? */

	if (--autoalign_focus_count < 0)
	{
		if (verbose)
		{
		    error(MESSAGE, "Autoalignment of parabola focus failed.");
		}
		send_wfs_text_message(
			"Autoalignment of parabola focus failed.");

		autoalign_focus = FALSE;
		autoalign_focus_count = 0;
		return;
	}

	/* OK, move things around */

	mess.type = HUT_AOB_MOVE_RELATIVE;
	mess.length = sizeof(motor_move);
	mess.data = (unsigned char *)&motor_move;

	if (scope_number == S2)
		motor_move.motor = AOB_S2_WFS_PAROB_FOC;
	else
		motor_move.motor = AOB_WFS_PAROB_FOC;

	motor_move.position = FOCUS_GAIN * fabs(wfs_mean_aberrations.focus)
				+ FOCUS_STEP;

	if (wfs_mean_aberrations.focus < 0) motor_move.position *= -1.0;

	send_message(telescope_server, &mess);

	send_wfs_text_message("Tries %d Focus = %.2f Moving Focus %d.", 
		autoalign_focus_count, 
		wfs_mean_aberrations.focus, 
		motor_move.position);

} /* align_focus_parabola() */

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
                "Wrong number of data bytes in WFS_START_FOUCS_PARABOLA.");
	}

	autoalign_focus = FALSE;
	autoalign_focus_count = 0;

	send_wfs_text_message("All Autoalignment STOPPED.");

	return NOERROR;

} /* message_wfs_stop_autoalign() */
