/************************************************************************/
/* wfs_andor_camera_link.c						*/
/*                                                                      */
/* Looks after the camera lonk low level stuff.				*/
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

#warning CAMLINK is not reliable, but the issue is not the new or old code.
#include "wfs_server.h"
#include <cext.h>     
#include <pximage.h>           
#include <pxerrno.h>          
#include <pxlibvs.h>
#include <xclibvs.h>
#include <pxlibsv.h>
#include <xclibsv.h>
#include <xclibsc.h>
#include <pxlibcam.h>
#include <sched.h>
#include <sys/mman.h>

static bool camlink_thread_running = FALSE;
static pthread_t camlink_thread;
static pthread_mutex_t camlink_mutex = PTHREAD_MUTEX_INITIALIZER;
static int fieldirqcount = 0;

/* We assume there will be only one camera here */

#define UNITS	1
#define UNITSMAP	((1<<UNITS)-1)
#define DRIVERPARMS ""
#warning FORMAT FILE HAS 84x84 NOT 90x90
//#define FORMATFILE    "/ctrscrut/chara/etc/wfs.fmt"
#define FORMATFILE    "./wfs.fmt"

/************************************************************************/
/* videoirqfunc()							*/
/*									*/
/* As a place holder. This function gets called whenever a video frame  */
/* appears.								*/
/************************************************************************/

void videoirqfunc(int sig)
{

    fieldirqcount++;

} /* videoirqfunc() */

/************************************************************************/
/* andor_start_camlink_thread()						*/
/*									*/
/* Setup and start the CAMERA_LINK thread.				*/
/************************************************************************/

int andor_start_camlink_thread(void)
{
	char	s[256];
	int     i, buffers, boards, xdim, ydim, colors, bpp;

	error(MESSAGE,"Entering CAMERA_LINK Thread.");

	/* First open the camera link device */

	if ((i = pxd_PIXCIopen(DRIVERPARMS, "", FORMATFILE)) == 0)
        {
                /* Setup a callback routine */

                signal(SIGUSR1, videoirqfunc);
                pxd_eventCapturedFieldCreate(1, SIGUSR1, NULL);
                if (verbose) error(MESSAGE, "Camera Link Open OK");
        }
        else
        {
                pxd_mesgFaultText(UNITSMAP, s, 256);
                error(FATAL, "Camera Link Open Error %d: %s", i,s);
        }
	
	/* Find out what we know about the camera */

        buffers = pxd_imageZdim();
        boards = pxd_infoUnits();
        xdim = pxd_imageXdim();
        ydim = pxd_imageYdim();
        colors = pxd_imageCdim();
        bpp = pxd_imageCdim()*pxd_imageBdim();

        /* Tell user if we have to */

        if (verbose)
        {
                error(MESSAGE, "Image frame buffers : %d", buffers);
                error(MESSAGE, "Number of boards    : %d", boards);
                error(MESSAGE, "xdim                : %d", xdim);
                error(MESSAGE, "ydim                : %d", ydim);
                error(MESSAGE, "colors              : %d", colors);
                error(MESSAGE, "bits per pixel      : %d", bpp);
        }

	/* Create the thread */

        if (pthread_mutex_init(&camlink_mutex, NULL) != 0)
        {
                return error(ERROR, "Unable to create camlink mutex.");
        }

        if (pthread_create(&camlink_thread, NULL, 
		andor_camlink_thread, NULL) != 0)
        {
                return error(ERROR, "Error creating CAMERA_LINK thread.");
        }
        camlink_thread_running = TRUE;

	error(MESSAGE,"Setup CAMERA_LINK thread complete.");

	return NOERROR;

} /* andor_start_camlink_thread() */

/************************************************************************/
/* andor_stop_camlink_thread()						*/
/*									*/
/* Stop the CAMERA_LINK thread.						*/
/************************************************************************/

int andor_stop_camlink_thread(void)
{
	if (!camlink_thread_running) return NOERROR;

	/* Let the thread know to stop */

        camlink_thread_running = FALSE;

        /* Wait for the thread to stop */

        error(MESSAGE,"Waiting for camlink thread to terminate.");
        if (pthread_join(camlink_thread,NULL) != 0)
        {
                return error(ERROR, 
		"Error waiting for camlink thread to stop.");
        }

	/* Close connection */

        pxd_PIXCIclose();

	return NOERROR;

} /* andor_stop_camlink_thread() */

/************************************************************************/
/* andor_start_camlink()						*/
/*									*/
/* Begin data collection using CAMERA_LINK.				*/
/* Return error level.							*/
/************************************************************************/

int andor_start_camlink(void)
{
	if (!use_cameralink) return error(ERROR,
		"The Camera link interface is disabled.");

	if (andor_setup.camlink_running) return error(ERROR,
		"The camera is already running in CAMERA LINK mode.");

	/* Initialize the globals */

	lock_camlink_mutex();
	andor_setup.camlink_frames_per_second = 0.0;
	andor_setup.usb_frames_per_second = 0.0;
	andor_setup.cam_frames_per_second = 0.0;

        /* Go get the first one. */

        pxd_doSnap(UNITSMAP, 1, 0);

	/* That should be all */

	andor_setup.camlink_running = TRUE;
	unlock_camlink_mutex();

	if (verbose) 
		error(MESSAGE,"Andor CAMERA_LINK data collection started.");

	return andor_send_setup();

} /* andor_start_camlink() */

/************************************************************************/
/* andor_stop_camlink()							*/
/*									*/
/* End data collection using CAMERA_LINK.				*/
/* Return error level.							*/
/************************************************************************/

int andor_stop_camlink(void)
{
	if (!andor_setup.camlink_running) return NOERROR;

	andor_setup.camlink_running = FALSE;

	/* Update globals */

	andor_setup.camlink_frames_per_second = 0.0;
	andor_setup.cam_frames_per_second = 0.0;
	andor_setup.processed_frames_per_second = 0.0;
	andor_setup.missed_frames_per_second = 0;
	fieldirqcount = 0;

	/* That should be all */

	if (verbose) 
		error(MESSAGE,"Andor CAMERA_LINK data collection stopped.");

	return andor_send_setup();

} /* andor_stop_camlink() */

/************************************************************************/
/* andor_camlink_thread()						*/
/*									*/
/* Thread to deal with the CAMERA_LINK stuff. WIll go as fast as	*/
/* possible Return error level.						*/
/************************************************************************/

void *andor_camlink_thread(void *arg)
{
	int number_of_camlink_frames = 0;
	static time_t last_camlink_fps_time = 0;
	static int last_number_camlink_images = 0; // Hardware
	time_t now;
	int this_number_camlink_images = 0;
	int i, n;
	char	s[256];
	int	run_count = 0;
	int	last_run_count = 0;

	error(MESSAGE,"Entering CAMERA_LINK Thread.");

	while(camlink_thread_running)
	{
		run_count++;

		/* Do we need to do anything? */

		if (!andor_setup.running || !andor_setup.camlink_running)
		{
			usleep(100);
			continue; 
		}

		/* Is there any data to get? */

		lock_camlink_mutex();

		/* How many frames so far? */

		this_number_camlink_images = pxd_videoFieldCount(1);

		/* Is there something new? */

		if (this_number_camlink_images == last_number_camlink_images)
		{
			unlock_camlink_mutex();
			usleep(100);
			continue;
		}

		/* This is a new one. */

		n = this_number_camlink_images - last_number_camlink_images;
		last_number_camlink_images = this_number_camlink_images;

		/* OK, go get the data */

		//for(i=0; i< n ; i++)
		//{
		    pxd_doSnap(UNITSMAP, 1, 0);
		    if ((i = pxd_readushort(UNITSMAP, /* Which unit? */
			1,                       /* Which frame buffer? */
			0, 0,                    /* Upper left corner */
			andor_setup.npixx,
			andor_setup.npixy,     /* Lower right corner or whole*/
			image_data,        /* Where to put it */
			sizeof(short)*andor_setup.npix,/* How big the buffer */
			"Grey")) != andor_setup.npix)
		    {
			pxd_mesgFaultText(UNITSMAP, s, 256);
			error(ERROR,"Camera link error %d != %d - %s",
					i, andor_setup.npix, s);
		    }
		    else
		    {

		  	/* OK, we got one more CAMERA_LINK frame */

		  	++number_of_camlink_frames;

			/* Here is the place holder call for the WFS */

			process_data(chara_time_now(), 
				andor_setup.npixx, 
				andor_setup.npixy, 
				image_data);
		    }

		    /* Give other threads some time */

		    usleep(100);
		//}

		/* Is it time to make a new calculation? */

		if ((now = time(NULL)) > last_camlink_fps_time)
		{
		    last_camlink_fps_time = now;

		    andor_setup.camlink_frames_per_second = 
			number_of_camlink_frames;
		    andor_setup.missed_frames_per_second = 
			andor_setup.cam_frames_per_second - 
			andor_setup.camlink_frames_per_second;

		    number_of_camlink_frames = 0;

		    printf("Run count %d %d\n", run_count, run_count - last_run_count);
		    fflush(stdout);
			last_run_count = run_count;
		}

		/* That should be all */

		unlock_camlink_mutex();
	}
	error(MESSAGE,"Leaving CAMERA_LINK Thread.");

	return NULL;

} /* andor_camlink_thread() */

/************************************************************************/
/* lock_camlink_mutex()                                                 */
/*                                                                      */
/* So outside programs can lock the mutex.                              */
/************************************************************************/

void lock_camlink_mutex(void)
{
#warning Note that we are ignoring the mutex completely
        //pthread_mutex_lock(&camlink_mutex);

} /* lock_camlink_mutex() */

/************************************************************************/
/* unlock_camlink_mutex()                                               */
/*                                                                      */
/* So outside programs can unlock the mutex.                            */
/************************************************************************/

void unlock_camlink_mutex(void)
{
        //pthread_mutex_unlock(&camlink_mutex);

} /* unlock_camlink_mutex() */
