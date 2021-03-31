#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace std::chrono;
using namespace cv;
using namespace std;

int cnt = 0;
vector<Point2f> pts_src;
RNG rng(12345);

//for capturing mouse click
void CallBackFunc(int event, int x, int y, int flags, void *userdata)

{
    if (event == EVENT_LBUTTONDOWN)
    {
        cnt++;
        pts_src.push_back(Point2f(x, y));
        cout << "Point Selected :" << x << ", " << y << ")" << endl;
    }
}
// for selecting points on the empty image
void selectpoints(int &cnt)
{
    Mat img = imread("empty.jpg");

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
}
// to crop the frame according to the homography
Mat cropframe(Mat img, Mat h)
{
    // Output image
    Mat im_out, cropped_im, cropped_img;
    // Warp source image to destination based on homography
    warpPerspective(img, im_out, h, img.size());

    Rect crop_region(472, 52, 800 - 472, 830 - 52);
    cropped_im = im_out(crop_region);
    cvtColor(cropped_im, cropped_img, COLOR_BGR2GRAY);
    return cropped_img;
}
// to write the output in a csv file 
void writeSomething(vector<vector<double>> v,string s)
{
    ofstream out;
    out.open(s);
    for (int i = 0; i < v.size(); i++)
        out << v[i][0] << "," << v[i][1]<< "," << v[i][2] << endl;
}

void imageSubtraction(Mat h , Mat cropped_empty ,VideoCapture vid, int x){
     vector<vector<double>> density;

    //Capturing frames from the video
    Mat frame, prev, dst, cropped;

    int i = 0;
    int n=0;
    bool y = true;
    while(y){
        Mat frame;
        vid>>frame;

        //checking for the first frame
        if(i==0){
            prev=cropped_empty;
            i++;
            continue;
        }
        // checking for end of video
        if(frame.empty()){
            y= false;
            break;
        }

        if(i%x==1){
            vector<double>frame_density;
            frame_density.push_back(i);
            
            //cropping the frame
            Mat new_cropped_image =cropframe(frame,h);

            //iterating over the frame to get the value of counts
            int count =0;
            double staticCount=0, dynamicCount=0;
            for(int i=0; i<new_cropped_image.rows ;i++){
                for(int j=0; j< new_cropped_image.cols ;j++){
                    float cur_pix = new_cropped_image.at<uchar>(i,j);
                    float last_pix= prev.at<uchar>(i,j);
                    float back_pix = cropped_empty.at<uchar>(i,j);

                    if(abs(cur_pix-back_pix)>25){
                        staticCount++;
                    }
                    if(abs(cur_pix-last_pix)>25){
                        dynamicCount++;
                    } 
                }
            }
            //dividing count by size to get density values
            int size=new_cropped_image.rows*new_cropped_image.cols;
            frame_density.push_back(staticCount/size);
            frame_density.push_back(dynamicCount/size);
            
            cout<<frame_density[0]<<" "<<frame_density[1]<<" "<<frame_density[2]<<" \n";
            
            for (int j = 0; j<x ; j++){
                if (i+j<=5736){
                frame_density.at(0)= i+j;
                density.push_back(frame_density);
            }
            }
            prev=new_cropped_image;
            i++;
        }
        else{
            i++;
            continue;
        }
        //check if escape key is pressed
        char c=(char)waitKey(25);
        if(c==27){
            cout<<"ESE pressed"<<endl;
            break;
        }
    }
    string s="m1_"+to_string(x-1)+".csv";
    writeSomething(density, s);
}
int main(int argc, char *argv[])
{
    //open the video file for reading
    if (argc <= 1)
    {
        cout << "enter argument as image name also \n ./main \"VideoName\" ";
    }
    string video_name = argv[1];
    video_name += ".mp4";
    int x = stoi(argv[2]);
    //checking if x>0
    if (x<=0){
        cout <<" argument should be greater than 0"<<endl;
        return -1;
    }
    VideoCapture vid(video_name);

    //if fail to read the image
    if (vid.isOpened() == false)
    {
        cout << "Error loading the video" << endl;
        return -1;
    }
    // selecting the points on empty.jpg
    selectpoints(cnt);

    // Four corners of the book in destination image.
    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));

    // points to check for hardcoded value
    vector<Point2f> pts_ds;
    pts_ds.push_back(Point2f(947, 280));
    pts_ds.push_back(Point2f(468, 1065));
    pts_ds.push_back(Point2f(1542, 1066));
    pts_ds.push_back(Point2f(1296, 269));

    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);

    //Finding the empty frame
    Mat cropped_empty = cropframe(imread("empty.jpg"), h);

    //starting the clock
    auto start = high_resolution_clock::now();

    //caling subtraction function
    imageSubtraction(h,cropped_empty,vid,x+1);

    //stopping the clock
    auto stop = high_resolution_clock::now();

    // finding duration
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function with frame lapse of  "<<x<<" frames : "
             << duration.count() << " microseconds/ " << duration.count() / 1000000 << " sec" << endl;

    exit(1);
    waitKey(0);
    
    return 0;
}
