//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;



int cnt=0;
vector<Point2f> pts_src;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)

{   
     if  ( event == EVENT_LBUTTONDOWN )
     {
        cnt++;
        pts_src.push_back(Point2f(x,y));
          cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" <<cnt << endl;
     }
     else if  ( event == EVENT_RBUTTONDOWN )
     {
        //   cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
        cout<<endl;
     }
     else if  ( event == EVENT_MBUTTONDOWN )
     {
          cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
          cout<<" "<<endl;
     }
     
     else if ( event == EVENT_MOUSEMOVE )
     {
        //   cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;

     }
}


int main(int argc, char* argv[])
{
 //open the video file for reading
 Mat img = imread("traffic.jpg");

     //if fail to read the image
     if ( img.empty() ) 
     { 
          cout << "Error loading the image" << endl;
          return -1; 
     }

     //Create a window
     namedWindow("My Window", 1);

     //set the callback function for any mouse event
     
     setMouseCallback("My Window", CallBackFunc, NULL);
     
     //show the image
     while(cnt<4){

     imshow("My Window", img);
      waitKey(50);
     
     }


     
    
   for(int i=0;i<pts_src.size();i++){
       cout<<"hellyeah"<<" "<<i<<endl;
       cout<<pts_src[i]<<" "<<pts_src[i]<<" ";
   }
   cout<<endl;


    

     // Wait until user press some key
     waitKey(0);

     

















 // Read source image.
    // Mat im_src = imread("empty.jpg");
    // // Four corners of the book in source image
    // vector<Point2f> pts_src;
    // // (472,52),(472,830),(800,830),(800,52)
    // pts_src.push_back(Point2f(64, 601));
    // pts_src.push_back(Point2f(141, 131));
    // pts_src.push_back(Point2f(480, 159));
    // pts_src.push_back(Point2f(493, 630));
    
    // cout<<"hello World";


    // // Read destination image.
    // Mat im_dst = imread("empty.jpg");
    // // Four corners of the book in destination image.
    // vector<Point2f> pts_dst;
    // pts_dst.push_back(Point2f(472,52));
    // pts_dst.push_back(Point2f(472,830));
    // pts_dst.push_back(Point2f(800,830));
    // pts_dst.push_back(Point2f(800,52));

    //  cout<<"hello World 3";

    // // Calculate Homography
    // Mat h = findHomography(pts_src, pts_dst);

    // // Output image
    // Mat im_out;
    // // Warp source image to destination based on homography
    // cout<<im_dst.size()<<endl;
    // warpPerspective(im_src, im_out, h, im_dst.size());

    // // Display images
    // imshow("Source Image", im_src);
    // imshow("Destination Image", im_dst);
    // imshow("Warped Source Image", im_out);

    // waitKey(0);

 return 0;

}
