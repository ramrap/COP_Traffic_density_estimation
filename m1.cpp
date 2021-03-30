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

void writeSomething(vector<vector<double>> v,string s)
{
    ofstream out;
    out.open(s);
    for (int i = 0; i < v.size(); i++)
        out << v[i][0] << "," << v[i][1]<< "," << v[i][2] << endl;
}

void imageSubtraction(Mat h , Mat cropped_empty ,VideoCapture vid, int x){
     vector<vector<double>> density;

    //Capturing frames from the video and finding absdiff
    Mat frame, prev, dst, cropped;

    int i = 0;
    int n=0;
    // vid.set(CAP_PROP_POS_FRAMES, 5139);
    // vid >> frame;
    // imshow("empty wala frame",frame);
    // imwrite("./newEmpty.jpg", frame);
    bool y = true;
    while(y){
        Mat frame;
        vid>>frame;

        // cout<<i<<endl;
        if(i==0){
            prev=cropframe(frame,h);
            i++;
            continue;
        }
        if(frame.empty()){
            y= false;
            break;
        }
        i++;
        if(i%x==0){
            vector<double>frame_density;
            frame_density.push_back(i);

            Mat new_cropped_image =cropframe(frame,h);

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
            int size=new_cropped_image.rows*new_cropped_image.cols;
            frame_density.push_back(staticCount/size);
            frame_density.push_back(dynamicCount/size);
            //density.push_back(frame_density);
            cout<<frame_density[0]<<" "<<frame_density[1]<<" "<<frame_density[2]<<" \n";
            
            for (int j = 0; j<x ; j++){
                frame_density.at(0)= i+j;
                density.push_back(frame_density);
            }
            
            // n++;

            
            prev=new_cropped_image;
        }
        else{
            continue;
        }

        char c=(char)waitKey(25);
        if(c==27){
            cout<<"ESE pressed"<<endl;
            break;
        }
    }
        writeSomething(density, "a.csv");
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
    VideoCapture vid(video_name);

    //if fail to read the image
    if (vid.isOpened() == false)
    {
        cout << "Error loading the video" << endl;
        return -1;
    }
    // selecting the points on traffic.jpg
    selectpoints(cnt);

    // Four corners of the book in destination image.
    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));

    // Calculate Homography
    Mat h = findHomography(pts_src, pts_dst);

    //Finding the empty frame
    Mat cropped_empty = cropframe(imread("empty.jpg"), h);

        auto start = high_resolution_clock::now();

        imageSubtraction(h,cropped_empty,vid,x);

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with frame lapse of  "<<x<<"frames : "
             << duration.count() << " microseconds/ " << duration.count() / 1000000 << " sec" << endl;


    waitKey(0);
    
    return 0;
}
