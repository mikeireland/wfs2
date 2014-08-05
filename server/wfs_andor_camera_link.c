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

static int number_of_camlink_frames = 0;
static int camlink_missed_frames = 0;
static time_t start_time_of_camlink_frames = 0;
static bool camlink_thread_running = FALSE;
static pthread_t camlink_thread;
static pthread_mutex_t camlink_mutex = PTHREAD_MUTEX_INITIALIZER;
static int first_number_camlink_images = 0;
static int fieldirqcount = 0;

/* We assume there will be only one camera here */

#define UNITS	1
#define UNITSMAP	((1<<UNITS)-1)
#define DRIVERPARMS ""
#define FORMATFILE    "/ctrscrut/chara/etc/wfs.fmt"

#ifdef USE_RT

#warning Compiling code for Pre-emptive RT Kernel.

#define MY_PRIORITY (49) /* we use 49 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

void stack_prefault(void) {

        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);
        return;
}
#else
#warning Compiling without pre-emptive kernel RT
#endif

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
	int	i;

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
	
	/* Create the thread */

        if (pthread_mutex_init(&camlink_mutex, NULL) != 0)
        {
                return error(ERROR, "Unable to create camlink mutex.");
        }

        camlink_thread_running = TRUE;
        if (pthread_create(&camlink_thread, NULL, 
		andor_camlink_thread, NULL) != 0)
        {
                return error(ERROR, "Error creating CAMERA_LINK thread.");
        }

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
                return error(ERROR, "Error waiting for camlink thread to stop.");
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

	if (andor_setup.usb_running) return error(ERROR,
		"The camera is already running in USB mode.");

	if (andor_setup.camlink_running) return error(ERROR,
		"The camera is already running in CAMERA LINK mode.");

	/* Initialize the globals */

	lock_camlink_mutex();
	number_of_camlink_frames = 0;
	andor_setup.camlink_frames_per_second = 0.0;
	andor_setup.cam_frames_per_second = 0.0;
	camlink_missed_frames = 0;

	/* Wait for the camera to be idle */

	if (andor_wait_for_idle(2) != NOERROR)
	{
		unlock_camlink_mutex();
		return error(ERROR,
		"Timed out wait for camera to be idle.");
	}

	/* Wait for the second to pass by */

	start_time_of_camlink_frames = time(NULL);
	while(time(NULL) <= start_time_of_camlink_frames);
	start_time_of_camlink_frames = time(NULL);
	first_number_camlink_images = pxd_videoFieldCount(1);

	/* Start the camera link and the camera going */

	if (andor_set_camera_link(1) != NOERROR)
	{
		unlock_camlink_mutex();
		error(ERROR,"What?");
                return ERROR;
	}

	if (!andor_setup.running && andor_start_acquisition() != NOERROR)
	{
		unlock_camlink_mutex();
                return ERROR;
	}

	/* That should be all */

	andor_setup.camlink_running = TRUE;
	unlock_camlink_mutex();

	if (verbose) error(MESSAGE,"Andor CAMERA_LINK data collection started.");

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

	/* Stop the thread */

	usleep(2e6*andor_setup.exposure_time);
	andor_setup.camlink_running = FALSE;
	usleep(2e6*andor_setup.exposure_time);

	/* Stop getting data */

	if (andor_setup.running) andor_abort_acquisition();

	/* Stop listening with teh camera link */;

	if (andor_set_camera_link(0) != NOERROR)
	{
		unlock_camlink_mutex();
                return ERROR;
	}

	/* Update globals */

	andor_setup.camlink_frames_per_second = 0.0;
	andor_setup.cam_frames_per_second = 0.0;
	camlink_missed_frames = 0;
	fieldirqcount = 0;

	/* That should be all */

	if (verbose) error(MESSAGE,"Andor CAMERA_LINK data collection stopped.");

	return andor_send_setup();

} /* andor_stop_camlink() */

/************************************************************************/
/* andor_camlink_thread()						*/
/*									*/
/* Thread to deal with the CAMERA_LINK stuff. WIll go as fast as	*/
/* possible Return error level.						*/
/************************************************************************/

#warning Work out what is different between this version and the new one.
void *andor_camlink_thread(void *arg)
{
	static time_t last_camlink_fps_time = 0;
	static int last_total_camlink_images = 0;
	static int last_number_camlink_images = 0;
	static int last_number_camlink_frames = 0;
	time_t now;
	int this_number_camlink_images = 0;
	int i;
	char	s[256];
#ifdef USE_RT
	struct timespec rt_time;
        struct sched_param rt_param;

	error(MESSAGE,"Setting up RT for Camera Link.");

	/* Declare ourself as a real time task */

        rt_param.sched_priority = MY_PRIORITY;
        if(sched_setscheduler(0, SCHED_FIFO, &rt_param) == -1)
                error(FATAL, "RT: sched_setscheduler failed");

        /* Lock memory */

        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
                error(FATAL, "RT: mlockall failed");

        /* Pre-fault our stack */

        stack_prefault();

        clock_gettime(CLOCK_MONOTONIC ,&rt_time);

        /* start after one second */

        rt_time.tv_sec++;
#endif
	error(MESSAGE,"Entering CAMERA_LINK Thread.");

	while(camlink_thread_running)
	{
#ifdef USE_RT
                /* wait until next shot */

                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &rt_time, NULL);
#endif

		/* Do we need to do anything? */

		if (!andor_setup.running || !andor_setup.camlink_running)
		{
#ifdef USE_RT
#warning YOU MAY WISH TO PLAY WITH THESE TIMES
                	rt_time.tv_nsec += 100000; /* 100uS */

                	while (rt_time.tv_nsec >= NSEC_PER_SEC)
			{
			       rt_time.tv_nsec -= NSEC_PER_SEC;
				rt_time.tv_sec++;
			}
#else
			usleep(1000);
#endif
		}

		/* Is there any data to get? */

		lock_camlink_mutex();

		/* How many frames so far? */

		this_number_camlink_images = pxd_videoFieldCount(1) -
			first_number_camlink_images;

		/* Is there something new? */

		if (this_number_camlink_images == last_total_camlink_images)
		{
			unlock_camlink_mutex();
#ifdef USE_RT
#warning YOU MAY WISH TO PLAY WITH THESE TIMES
                	rt_time.tv_nsec += 100000; /* 100uS */

                	while (rt_time.tv_nsec >= NSEC_PER_SEC)
			{
			       rt_time.tv_nsec -= NSEC_PER_SEC;
				rt_time.tv_sec++;
			}
#else
			usleep(100000);
#endif
			continue;
		}

		/* This is a new one. */

		last_total_camlink_images = this_number_camlink_images;

		/* OK, go get the data */

		if ((i = pxd_readushort(UNITSMAP, /* Which unit? */
			1,                       /* Which frame buffer? */
			0, 0,                    /* Upper left corner */
			andor_setup.npixx,
			andor_setup.npixy,     /* Lower right corner or whole*/
			usb_image,        /* Where to put it */
			sizeof(short)*andor_setup.npix,/* How big the buffer */
			"Grey")) != andor_setup.npix)
		{
			pxd_mesgFaultText(UNITSMAP, s, 256);
			error(ERROR,"Camera link error %d != %d - %s",
					i, andor_setup.npix, s);
			unlock_camlink_mutex();
#ifdef USE_RT
#warning YOU MAY WISH TO PLAY WITH THESE TIMES
                	rt_time.tv_nsec += 100000; /* 100uS */

                	while (rt_time.tv_nsec >= NSEC_PER_SEC)
			{
			       rt_time.tv_nsec -= NSEC_PER_SEC;
				rt_time.tv_sec++;
			}
#endif
			continue;
		}
		pxd_doSnap(UNITSMAP, 1, 0);

		/* OK, we got one more CAMERA_LINK frame */

		++number_of_camlink_frames;

		/* Is it time to make a new calculation? */

		if ((now = time(NULL)) > last_camlink_fps_time)
		{
		    last_camlink_fps_time = now;

		    andor_setup.camlink_frames_per_second = 
			number_of_camlink_frames-last_number_camlink_frames;
		    andor_setup.cam_frames_per_second = 
			this_number_camlink_images - last_number_camlink_images;
		    andor_setup.missed_frames_per_second = 
			andor_setup.cam_frames_per_second - 
			andor_setup.camlink_frames_per_second;

		    last_number_camlink_frames = number_of_camlink_frames;
		    last_number_camlink_images = this_number_camlink_images;
		}

		/* Here is the place holder call for the WFS */

		if (andor_setup.npixx == 90 && andor_setup.npixy == 90)
		{
#warning WE HAVE 90x90 WIRED IN
			process_data(chara_time_now(), 90, 90, usb_image);
		}

		/* That should be all */

		unlock_camlink_mutex();
#ifdef USE_RT
#warning YOU MAY WISH TO PLAY WITH THESE TIMES
		rt_time.tv_nsec += 500000; /* 500uS */

		while (rt_time.tv_nsec >= NSEC_PER_SEC)
		{
		       rt_time.tv_nsec -= NSEC_PER_SEC;
			rt_time.tv_sec++;
		}
#endif
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
