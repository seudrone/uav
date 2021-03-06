#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "highgui.h"
#include "colotracker.h"
#include "region.h"
#include <string>
#include <signal.h>
#include<sys/time.h> 
#include <unistd.h>

extern "C"
{
#include "djicam.h"
}
#define FRAME_SIZE (1280*720*3/2)  /*format NV12*/
using namespace cv;
using namespace std;

cv::Point g_topLeft(0,0);
cv::Point g_botRight(0,0);
cv::Point g_botRight_tmp(0,0);
bool plot = false;
bool g_trackerInitialized = false;
ColorTracker * g_tracker = NULL;

static void onMouse( int event, int x, int y, int, void* param)
{
    cv::Mat img = ((cv::Mat *)param)->clone();
    if( event == cv::EVENT_LBUTTONDOWN && !g_trackerInitialized){
        std::cout << "DOWN " << std::endl;
        g_topLeft = Point(x,y);
        plot = true;
    }else if (event == cv::EVENT_LBUTTONUP && !g_trackerInitialized){
        std::cout << "UP " << std::endl;
        g_botRight = Point(x,y);
        plot = false;
        if (g_tracker != NULL)
            delete g_tracker;
        g_tracker = new ColorTracker();
        g_tracker->init(*(cv::Mat *)param, g_topLeft.x, g_topLeft.y, g_botRight.x, g_botRight.y);
        g_trackerInitialized = true;
    }else if (event == cv::EVENT_MOUSEMOVE && !g_trackerInitialized){
        //plot bbox
        g_botRight_tmp = Point(x,y);
        // if (plot){
        //     cv::rectangle(img, g_topLeft, current, cv::Scalar(0,255,0), 2);
        //     imshow("output", img);
        // }
    }
}


int main(int argc, char **argv) 
{
	int ret;
   	 BBox * bb = NULL;
    	cv::Mat img,img2;
	    /*
	    int captureDevice = 0;
	    if (argc > 1)
		captureDevice = atoi(argv[1]);

	    cv::VideoCapture webcam = cv::VideoCapture(captureDevice);
	    webcam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	    webcam.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	    if (!webcam.isOpened()){
		webcam.release();
		std::cerr << "Error during opening capture device!" << std::endl;
		return 1;
	    }
	*/

	int width = 1280, height = 720;
	unsigned char buffer0[FRAME_SIZE+8] = {0};
	unsigned int nframe;
/*	
	CvSize czsize;
	czsize.width=640;
	czsize.height=480;
	cv::Mat img=cvCreateImage(czsize,IPL_DEPTH_8U,1);
*/	
	int init=manifold_cam_init(GETBUFFER_MODE);
	//cout << "init:	" << init << endl;
	//cout<<"11111"<<endl;  
	usleep(100);
	
    	cv::namedWindow( "output");
    	//cv::waitKey(0);
    	//cv::namedWindow( "img2" );
    	cv::setMouseCallback( "output", onMouse, &img);
	//cv::setMouseCallback( "output", onMouse);
	
    	for(;;)
    	{
    		struct timeval tstart1,tfinish1;  
    		double timeUsed1,timeUsed2;
      
    		gettimeofday(&tstart1,NULL); 
    		int cam_read=manifold_cam_read( buffer0,  &nframe,  CAM_NON_BLOCK);
		//cout<<"manifold_cam_read:	"<<cam_read<<endl;;
		usleep(100);
		Mat yuv(height + height / 2, width, CV_8UC1, buffer0);
		cv::cvtColor(yuv, img2, CV_YUV2GRAY_NV12);
		
		//cv::imshow("img2", img2);
       		 //cv::waitKey(10);
        
		resize(img2,img,Size(640,480),0,0,CV_INTER_LINEAR);
        	//webcam >> img; 
		//cout<<"for1"<<endl;
        	char c = waitKey(1);
        	if ( c  == 'q') 
        	{
            		std::cout << " break" << std::endl;  
            		//manifold_cam_exit();
            		break;
        	}        
             
        	//some control
        	switch( (char)c ){
        	case 'i':
            		g_trackerInitialized = false;
            		g_topLeft = cv::Point(0,0);
            		g_botRight_tmp = cv::Point(0,0);
            		break;
        		default:;

        	}

		if (g_trackerInitialized && g_tracker != NULL){
		    bb = g_tracker->track(img);
		}

		if (!g_trackerInitialized && plot && g_botRight_tmp.x > 0){
		    cv::rectangle(img, g_topLeft, g_botRight_tmp, cv::Scalar(0,255,0), 2);
		}

		if (bb != NULL){
		    cv::rectangle(img, Point2i(bb->x, bb->y), Point2i(bb->x + bb->width, bb->y + bb->height), Scalar(255, 0, 0), 3);
		    //cv::rectangle(img, Point2i(bb->x-5, bb->y-5), Point2i(bb->x + bb->width+5, bb->y + bb->height+5), Scalar(0, 0, 255), 1);
		    delete bb;
		    bb = NULL;
       		 }

		cv::imshow("output", img);
		cv::waitKey(10);
		
		yuv.release();
		usleep(100);
		
		gettimeofday(&tfinish1,NULL);  
	    	timeUsed2=1000000*(tfinish1.tv_sec-tstart1.tv_sec)+tfinish1.tv_usec-tstart1.tv_usec;  
	    	cout<<" Total Time 2 ="<<timeUsed2/1000<<" ms"<<endl;  
    	}
	
   	if (g_tracker != NULL)
        	delete g_tracker;
        	
        
        manifold_cam_exit();
        kill(getpid(), SIGINT);	
    	while(!manifold_cam_exit()) /*make sure all threads exit ok*/
	{
		sleep(1);
		std::cout << "Exiting ..." << std::endl;
	}
	cout<<"exit succcefully "<<endl;
   
    	return 0;
}
