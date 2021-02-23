//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int cnt = 0;
vector<Point2f> pts_src;

void CallBackFunc(int event, int x, int y, int flags, void *userdata)

{
    if (event == EVENT_LBUTTONDOWN)
    {
        cnt++;
        pts_src.push_back(Point2f(x, y));
        cout << "Point Selected :" << x << ", " << y << ")" << endl;
    }
}

int main(int argc, char *argv[])
{
    //open the video file for reading
    if (argc <= 1)
    {
        cout << "enter argument as image name also \n ./video \"imageName\" ";
    }
    string image_name = argv[1];
    image_name += ".jpg";
    Mat img = imread(image_name);

    //if fail to read the image
    if (img.empty())
    {
        cout << "Error loading the image" << endl;
        return -1;
    }

    //Create a window
    namedWindow("My Window", 1);

    //set the callback function for any mouse event

    setMouseCallback("My Window", CallBackFunc, NULL);

    //show the image
    while (cnt < 4)
    {

        imshow("My Window", img);
        waitKey(50);
    }

    // Read destination image.
    Mat im_dst = imread(image_name);
    // Four corners of the book in destination image.
    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));

    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);

    // Output image
    Mat im_out, cropped_img;
    // Warp source image to destination based on homography
    warpPerspective(img, im_out, h, im_dst.size());

    Rect crop_region(472, 52, 800 - 472, 830 - 52);
    cropped_img = im_out(crop_region);

    //saving image in folder
    imwrite("./transformed.jpg", im_out);
    imwrite("./cropped.jpg", cropped_img);

    // Display images
    imshow("Source Image", img);
    imshow("Warped Source Image", im_out);
    imshow("cropped image", cropped_img);

    

    return 0;
}
