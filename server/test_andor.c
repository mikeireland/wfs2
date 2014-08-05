/************************************************************************/
/* test_andor.c								*/
/*                                                                      */
/* Tests out various features of teh Andor camera SDK.			*/
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
/* Date   : September 2012						*/
/************************************************************************/


#include <stdint.h>
#include <time.h>
#include <math.h>
#include <atmcdLXd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char **argv)
{
	int	status;
	int	bitdepth;
	int	width, height;
	int	min, max;
	int	i, j, numgains;
	float	gain;
	int	num_ad;
	int	num_vspeeds;
	int	num_hspeeds;
	float	speed;
	AndorCapabilities caps;
	at_32 lNumCameras;
	at_32 lCameraHandle;
	int	start_n, stop_n;
	int	npix;

	/* Check command line */

	npix = 90;

	if (argc > 1) sscanf(argv[1], "%d", &npix);
	
	/* FIRST, FIND OUT WHAT WE NEED TO KNOW ABOUT THIS CAMERA. */

	printf("Initializing Andor Camera.\n");

	/* How many cameras are connected */

	printf("GetAvailableCameras = %d\n", GetAvailableCameras(&lNumCameras));
	if (lNumCameras == 1)
		printf("There is %d camera connected.\n", lNumCameras);
	else
		printf("There are %d cameras connected.\n", lNumCameras);

	if (lNumCameras <= 0) exit(0);

	/* Get the camera we are working with. */

	printf("GetCameraHandle = %d\n", GetCameraHandle(0, &lCameraHandle));

	printf("SetCurrentCamera = %d\n", SetCurrentCamera(lCameraHandle));

	/* Initialize the CCD */

	printf("Initialize = %d\n", Initialize("/usr/local/etc/andor"));

	/* Wait for this to happen */

	sleep(2);

	/* What Capabilities do we have? */

	caps.ulSize = sizeof(caps);
	printf("GetCapabilities = %d\n", GetCapabilities(&caps));

	if (caps.ulCameraType & AC_CAMERATYPE_IXON)
		printf("Camera is an iXon.\n");
	else
		printf("Camera is not an iXon.\n");

	if (caps.ulAcqModes & AC_ACQMODE_FRAMETRANSFER)
		printf("Frame transfer is available.\n");
	else
		printf("Frame transfer is not available.\n");

	if (caps.ulSetFunctions & AC_SETFUNCTION_CROPMODE)
		printf("Crop mode is available.\n");
	else
		printf("Crop mode is not available.\n");

	/* Find out what the width and height are */

	printf("GetDetector = %d\n", GetDetector(&width, &height));
	printf("Andor full size %dx%d.\n", width, height);

	/* What is the allowable temperature range? */

	printf("GetTemperatureRange = %d\n", GetTemperatureRange(&min, &max));
	printf("Andor temperature range is %d to %d C.\n", min, max);
	
	/* How many preamp gains do we have? */

	printf("GetNumberPreAmpGains = %d\n", GetNumberPreAmpGains(&numgains));
	printf("Andor number of preamp gains is %d.\n", numgains);

	/* Let's find out what these gains are */

	for (i=0; i<numgains; i++)
	{
		printf("GetPreAmpGain = %d\n", GetPreAmpGain(i, &gain));
		printf("Andor Preamp Gain %d is %f.\n", i, gain);
	}

	/* How many vertical speeds do we have? */

	printf("GetNumberVSSpeeds = %d\n", GetNumberVSSpeeds(&num_vspeeds));
	printf("Andor number of Vertical Speeds is %d.\n", num_vspeeds);

	/* Let's find out what these VSpeeds are */

	for (i=0; i<num_vspeeds; i++)
	{
		printf("GetVSSpeed = %d\n", GetVSSpeed(i, &speed));
		printf("Andor Vertical Speed %d is %.2f uS.\n",
				i, speed);
	}

	for(j = 0; j < 1; j++)
	{
	    if (j == 0)
		    printf("For EMCCD output:\n");
	    else
		    printf("For CCD output:\n");

	    /* How many horizontal speeds do we have? */

	    printf("GetNumberHSSpeeds = %d\n",
		GetNumberHSSpeeds(0, j, &num_hspeeds));
	    printf("Andor number of Horizontal Speeds is %d.\n", num_hspeeds);

	    /* Let's find out what these speeds are */

	    for (i=0; i<num_hspeeds; i++)
	    {
		printf("GetHSSpeed = %d\n", GetHSSpeed(0, j, i, &speed));
		printf("Andor Horizontal Speed %d is %.2f MHz.\n", i, speed);
	    }

	}

	/* What is the range of gain settings - This is always wrong  */

	printf("GetEMGainRange = %d\n", GetEMGainRange(&min, &max));
	printf("Andor EM Gain range is %d to %d.\n", min, max);

	/* How many AD channels are there? */

	printf("GetNumberADChannels = %d\n", GetNumberADChannels(&num_ad));
	printf("Number of AD channels = %d\n", num_ad);

	/* What are the bit depths? */

	for(i=0; i < num_ad; i++)
	{
	    printf("GetBitDepth = %d\n", GetBitDepth(i, &bitdepth));
	    printf("AD channel %d has bit depth %d\n", i, bitdepth);
	}

	/* NOW WE START TO SET THINGS UP THE WAY WE WANT THEM */

	/* Turn on Cameralink mode */

	printf("SetCameraLinkMode = %d\n", SetCameraLinkMode(1));

	/* Put us in Frame Transfer Mode */

	printf( "Turning on Frame Transfer Mode.\n");
	printf("SetFrameTransferMode = %d\n", SetFrameTransferMode(1));

        /* Set output amplifier to EMCCD */

	printf("SetOutputAmplifier = %d\n", SetOutputAmplifier(0));

	/* Turn on advanced EM settings */

	printf("SetEMAdvanced = %d\n", SetEMAdvanced(1));

	/* Let's see if that worked... we should get 1000 as maximum */

	printf("GetEMGainRange = %d\n", GetEMGainRange(&min, &max));
	printf("Andor EM Gain range is %d to %d.\n", min, max);

	/* Set EM gain */

	printf("SetEMCCDGain = %d\n", SetEMCCDGain(300));

	/* Set our horizontal speed to the desired one. */

        printf("SetHSSpeed = %d\n", SetHSSpeed(0, 0));

	/* Set our vertical speed to the desired one. */

        printf("SetVSSpeed = %d\n", SetVSSpeed(0));

	/* Set our gain to the desired one. */

	printf("SetPreAmpGain = %d\n", SetPreAmpGain(2));

        /* Setup the read mode. I suspect this is the problem */

        printf("SetReadMode = %d\n", SetReadMode(4));

	/* Try to get frames like this */

        printf("SetAcquisitionMode = %d\n", SetAcquisitionMode(5));
	printf("PrepareAcquisition = %d\n", PrepareAcquisition());
	printf("StartAcquisition %d\n", StartAcquisition());
	sleep(1);
	printf("AbortAcquisition %d\n", AbortAcquisition());

	/* This sequence seemed to work with the server */

        //printf("SetHSSpeed = %d\n", SetHSSpeed(0, 1));

	/* DO we get the right gain range now? */

	printf("GetEMGainRange = %d\n", GetEMGainRange(&min, &max));
	printf("Andor EM Gain range is %d to %d.\n", min, max);

        /* We wish to use Cropped mode */

        printf("SetIsolatedCropMode = %d\n",
		SetIsolatedCropMode(1,npix,npix,1,1));

        /* Set the image to read the full area of cropped region */

        printf("SetImage = %d\n", SetImage(1, 1, 1, npix, 1, npix));

        /* Set exposure time to 1mS */

        printf("SetExposureTime = %d\n", SetExposureTime(0.001));

	/* Set Kinetic Cycle time to the smallest possible value */

	printf("SetKineticCycleTime = %d\n",SetKineticCycleTime(0.0));

	/* OK, let's see what the frame rate is */

        printf("SetAcquisitionMode = %d\n", SetAcquisitionMode(5));
	printf("StartAcquisition %d\n", StartAcquisition());
	sleep(1);
	printf("GetTotalNumberImagesAcquired = %d\n",
			GetTotalNumberImagesAcquired(&start_n));
	sleep(5);
	printf("GetTotalNumberImagesAcquired = %d\n",
			GetTotalNumberImagesAcquired(&stop_n));
	printf("Frame rate seems to be %.2f Hz\n",
			(double)(stop_n - start_n)/5.0);
	printf("AbortAcquisition = %d\n",AbortAcquisition());


	/* Get one frame... it seems things work better after this */

	GetStatus(&status);
	while(status==DRV_ACQUIRING) GetStatus(&status);
	printf("SaveAsFITS %d\n", SaveAsFITS("./image.fit", 4));

	/* That is all */

	printf("ShutDown = %d\n", ShutDown());

	exit(0);
}
