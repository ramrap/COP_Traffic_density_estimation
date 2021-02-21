//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
 //open the video file for reading
 cout<<"hello World \n";
 // Read source image.
    Mat im_src = imread("empty.jpg");
    // Four corners of the book in source image
    vector<Point2f> pts_src;
    pts_src.push_back(Point2f(141, 131));
    pts_src.push_back(Point2f(480, 159));
    pts_src.push_back(Point2f(493, 630));
    pts_src.push_back(Point2f(64, 601));
    cout<<"hello World";


    // Read destination image.
    Mat im_dst = imread("traffic.jpg");
    // Four corners of the book in destination image.
    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(318, 256));
    pts_dst.push_back(Point2f(534, 372));
    pts_dst.push_back(Point2f(316, 670));
    pts_dst.push_back(Point2f(73, 473));

     cout<<"hello World 3";

    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);

    // Output image
    Mat im_out;
    // Warp source image to destination based on homography
    warpPerspective(im_src, im_out, h, im_dst.size());

    // Display images
    imshow("Source Image", im_src);
    imshow("Destination Image", im_dst);
    imshow("Warped Source Image", im_out);

    waitKey(0);

 return 0;

}
