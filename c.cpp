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
        cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
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
    Mat img_rgb = imread(image_name);

    //if fail to read the image
    if (img_rgb.empty())
    {
        cout << "Error loading the image" << endl;
        return -1;
    }
    Mat img;
    

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

    for (int i = 0; i < pts_src.size(); i++)
    {
        cout << "hellyeah"
             << " " << i << endl;
        cout << pts_src[i].x << " " << pts_src[i].y << " ";
    }
    cout << endl;

    // Read destination image.
    Mat im_dst = imread(image_name);
    // Four corners of the book in destination image.
    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));

    cout << "hello World 3";

    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);

    // Output image
    Mat im_out, cropped_img;
    // Warp source image to destination based on homography
    cout << im_dst.size() << endl;
    warpPerspective(img, im_out, h, im_dst.size());

    // Display images
    imshow("Source Image", img);

    imshow("Warped Source Image", im_out);

    Rect crop_region(472, 52, 800 - 472, 830 - 52);
    cropped_img = im_out(crop_region);

    imshow("cropped image", cropped_img);

    //for saving image Add desired path according to need
    imwrite("./WrappedImage.jpg", im_out);
    imwrite("./WrappedCroppedImage.jpg", cropped_img);

    waitKey(0);

    return 0;
}
