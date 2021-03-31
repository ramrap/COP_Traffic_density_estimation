#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <chrono>

using namespace std::chrono;
using namespace cv;
using namespace std;

//cnt for selecting number of points
int cnt = 0;

//Points to select from image on which we have to apply homography
vector<Point2f> pts_src;

//To store all cropped frames
vector<Mat> cropped_frame_vec;

//struct for pthread as parameter
struct Thread_Struct
{
    Mat emp;
    string videoName;
    vector<double> density;
    int start;
    int total;
    Mat h;
};

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

//for selecting points
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

//crop img with respect to homography h
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

//function to write csv
void writeSomething(vector<vector<double>> v, string s)
{
    ofstream out;
    out.open(s);
    for (int i = 0; i < v.size(); i++)
        out << v[i][0] << "," << v[i][1] << endl;
}


//pthread function
void *thread_run(void *arg)
{
    struct Thread_Struct *arg_st =
        (struct Thread_Struct *)arg;

    //defining variables from struct
    Mat cropped_empty = arg_st -> emp, frame, h = arg_st -> h;
    vector<double> density;
    density = arg_st  ->  density;
    int start = arg_st  ->  start, end = arg_st  ->  total;

    //finding length of current column which we need to 
    int length = cropped_empty.cols / end;
    

    int frame_num = 0;

    //intialising
    int last = (start + 1) * length;
      if (start == (end - 1))
        {
                // cout<<"HELLO HERE"<<endl;
                last = cropped_empty.cols;
        }

    
    //Iterating over all frames 
    for (frame_num = 0; frame_num < cropped_frame_vec.size(); frame_num++)
    {
        if (frame_num % 100 == 0)
        {
            // printing every 100th frame to get to know about working of code
            cout << "Thread No:" << start + 1 << " completed " << frame_num << " frames" << endl;
        }
       

        double staticCount = 0;
        Mat new_cropped_image = cropped_frame_vec[frame_num];
        
       //performing calculation only sepecific column of image 
        for (int i = 0; i < new_cropped_image.rows; i++)
        {
            for (int j = start * length; j < last; j++)
            {
                //calculating value of each pixel
                float cur_pix = new_cropped_image.at<uchar>(i, j);
                float back_pix = cropped_empty.at<uchar>(i, j);

                // If pixel value differ by value of 25 or more than only we will count it as differnce
                if (abs(cur_pix - back_pix) > 25)
                {
                    staticCount++;
                }
            }
        }

        // updating Array with pixel count
        density[frame_num] = staticCount;
        
    }
    //assigning value to Struct
    arg_st -> density = density;

    //exit current thread
    pthread_exit(0);
}

void imageSubtraction(Mat h, Mat cropped_empty, VideoCapture vi, int NUM_THREADS, string videoName, int size_video)
{
    // Intialising Useful vairables
    Mat frame, prev, dst, cropped;
    pthread_t tids[NUM_THREADS];
    vector<int> first(NUM_THREADS, 0);
    vector<double> density(size_video, 0);
    vector<Thread_Struct> alpha;

    //intialising thread struct for all pthreads
    for (int i = 0; i < NUM_THREADS; i++)
    {
        Thread_Struct arg = {
            cropped_empty,
            videoName,
            density,
            i,
            NUM_THREADS,
            h};
        alpha.push_back(arg);
    }
    int frame_size = cropped_empty.rows * cropped_empty.cols;

    //starting all thread with thread id tids[]
    for (int j = 0; j < NUM_THREADS; j++)
    {
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&tids[j], &attr, thread_run, &alpha[j]);
    }

    // joining all threads i.e. waiting for thread to end.
    for (int j = 0; j < NUM_THREADS; j++)
    {
        pthread_join(tids[j], NULL);
    }

    //vector to store final result
    vector<vector<double>> vec;

    //updating all data recived from all threads and merging them into one
    for (int i = 0; i < cropped_frame_vec.size(); i++)
    {
        vector<double> v;
        v.push_back(i);
        double val = 0;
        for (int j = 0; j < NUM_THREADS; j++)
        {
            val += alpha[j].density[i];
        }
        v.push_back(val / frame_size);
        vec.push_back(v);
    }

    //creating CSV from queue density obtained from above process
    string s="./M3/method3-data-thread-"+to_string(NUM_THREADS)+".csv";
    writeSomething(vec, s);
}
int main(int argc, char *argv[])
{
    if(argc<=2){
        cout<<"Error: Check Parameters \n";
        exit(1);
    }

    //taking parameters from cli
    string video_name = argv[1];
    video_name += ".mp4";
    VideoCapture vid(video_name);
    int NUM_THREADS = stoi(argv[2]);

    if(NUM_THREADS<0){
        cout<<"Error: Enter Valid Number of Threads \n";
        exit(1);
    }

    //if fail to read the image
    if (vid.isOpened() == false)
    {
        cout << "Error loading the video" << endl;
        return -1;
    }
    // selecting the points on traffic.jpg


    // selectpoints(cnt);

    // Four corners of the book in destination image.
    vector<Point2f> pts_dst;
    pts_dst.push_back(Point2f(472, 52));
    pts_dst.push_back(Point2f(472, 830));
    pts_dst.push_back(Point2f(800, 830));
    pts_dst.push_back(Point2f(800, 52));

    // temporary fixed destination point for every method
    vector<Point2f> pts_ds;
    pts_ds.push_back(Point2f(947, 280));
    pts_ds.push_back(Point2f(468, 1065));
    pts_ds.push_back(Point2f(1542, 1066));
    pts_ds.push_back(Point2f(1296, 269));

    // Calculate Homography
    Mat h = findHomography(pts_ds, pts_dst);

    //Aligning empty frame w.r.t homography
    Mat cropped_empty = cropframe(imread("empty.jpg"), h);

    int size_video = 0;
    auto start = high_resolution_clock::now();

    //iterating Over video to store each frame and get size of video.
    while (true)
    {
        Mat frame;
        vid >> frame;
        if (frame.empty())
        {
            break;
        }
        Mat new_cropped_image = cropframe(frame, h);

        cropped_frame_vec.push_back(new_cropped_image);
        // i++;
        cout << "Current frame for homography :" << cropped_frame_vec.size() << endl;
        // cout << size_video << endl;
        size_video++;
    }
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    double start_time = duration.count();// time required by above process

    // IF NUM_THREADS =0 then perform function on all thread from 1 to 10 else perform operation on Given Number of Threads
    if (NUM_THREADS)
    {
        auto start = high_resolution_clock::now();

        imageSubtraction(h, cropped_empty, vid, NUM_THREADS, video_name, size_video); //last parameter is size of video

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with threads " << NUM_THREADS << ": "
             << duration.count() + start_time << " microseconds/ " << endl;
    }
    else
    {
        vector<vector<double>> data;
        for (int i = 1; i <= 10; i++)
        {
            auto start = high_resolution_clock::now();

            imageSubtraction(h, cropped_empty, vid, i, video_name, size_video //sizevideo
            );

            auto stop = high_resolution_clock::now();

            auto duration = duration_cast<microseconds>(stop - start);

            cout << "Time taken by function with threads " << i << ": "
                 << duration.count() + start_time << " microseconds/ " << endl;

            vector<double> temp;
            int al = int(duration.count() + start_time);
            cout << al << endl;
            temp.push_back(i);
            temp.push_back(al/1e6);
            data.push_back(temp);
            cout << data.size() << endl;
        }
        //updating CSV for all threads
        writeSomething(data, "./M3/Method3_RUNTIME.csv");
    }

    exit(1);

    waitKey(0);

    return 0;
}
