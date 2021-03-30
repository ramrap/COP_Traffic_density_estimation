#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <chrono>

using namespace std::chrono;
using namespace cv;
using namespace std;

int cnt = 0;
vector<Point2f> pts_src;
RNG rng(12345);
vector<Mat> cropped_frame_vec;

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
        out << v[i][0] << "," << v[i][1] << endl;
}

Mat reduce_ImgSize(Mat inImg, double scale)
{
    Mat dst;
    resize(inImg, dst, cv::Size(inImg.cols * scale, inImg.rows * scale), 0, 0, INTER_LINEAR);

    return dst;
}

void *thread_run(void *arg)
{
    struct Thread_Struct *arg_st =
        (struct Thread_Struct *)arg;
    // Mat new_cropped_image = arg_st->cur;
    Mat cropped_empty = arg_st->emp,frame,h = arg_st->h;
    vector<double> density;
    density = arg_st->density;

    int start = arg_st->start, end = arg_st->total;

    int length = cropped_empty.cols / end;
    string videoName = arg_st->videoName;
    
    VideoCapture vid(videoName);
    int frame_num=0;

    for(frame_num=0;frame_num<cropped_frame_vec.size();frame_num++){
        if(frame_num%100==0){
            cout<<start<<" "<<frame_num<<endl;
        }
        
        double staticCount = 0; 
        Mat new_cropped_image = cropped_frame_vec[frame_num];
         for (int i = 0; i < new_cropped_image.rows; i++)
        {
        for (int j = start * length; j < (start + 1) * length; j++)
        {
            float cur_pix = new_cropped_image.at<uchar>(i, j);
            float back_pix = cropped_empty.at<uchar>(i, j);
            if (abs(cur_pix - back_pix) > 25)
            {
                staticCount++;
            }
        }
        }
        density[frame_num]=staticCount;
        // cout<<start<<" - "<<frame_num<<"--"<<staticCount<<"\n";
        
    }
    arg_st->density=density;

    pthread_exit(0);
}

void imageSubtraction(Mat h, Mat cropped_empty, VideoCapture vi, int NUM_THREADS, string videoName,int size_video)
{
    Mat frame, prev, dst, cropped;

    
    VideoCapture vid(videoName);

    int i=0;
    // while(true){
    //     vid>>frame;
    //     if(frame.empty()){
    //         break;
    //     }
    //     Mat new_cropped_image = cropframe(frame, h);

    //     cropped_frame_vec.push_back(new_cropped_image);
    //     i++;
    //     cout<<"vector :"<<i<<endl;
    // }

    pthread_t tids[NUM_THREADS];
    vector<int> first(NUM_THREADS, 0);
    
    vector<double> density(size_video, 0);

    vector<Thread_Struct> alpha;


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

    
    for (int j = 0; j < NUM_THREADS; j++)
    {

        pthread_attr_t attr;
        // alpha[j].ind = 0;
        pthread_attr_init(&attr);
        pthread_create(&tids[j], &attr, thread_run, &alpha[j]);
    }
    for (int j = 0; j < NUM_THREADS; j++)
    {
        pthread_join(tids[j], NULL);
    }
    vector<vector<double>> vec;
    
    for(int i=0;i<cropped_frame_vec.size();i++){
        vector<double>v;
        v.push_back(i);
        double val=0;
        for (int j = 0; j < NUM_THREADS; j++)
        {
            val+=alpha[j].density[i];
        }
        v.push_back(val/frame_size);
        // cout<<i<<" "<<v[1]<<endl;
        vec.push_back(v);
    }

    writeSomething(vec,"data.csv");

    
}
int main(int argc, char *argv[])
{
    
    string video_name = argv[1];
    video_name += ".mp4";
    VideoCapture vid(video_name);
    int NUM_THREADS = stoi(argv[2]);

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

    vector<Point2f> pts_ds;
    pts_ds.push_back(Point2f(947, 280));
    pts_ds.push_back(Point2f(468, 1065));
    pts_ds.push_back(Point2f(1542, 1066));
    pts_ds.push_back(Point2f(1296, 269));

    // Calculate Homography
    Mat h = findHomography(pts_ds, pts_dst);

    //Finding the empty frame
    Mat cropped_empty = cropframe(imread("empty.jpg"), h);

    int size_video = 0;
    auto start = high_resolution_clock::now();
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
        cout<<"vector :"<<cropped_frame_vec.size()<<endl;
        // cout << size_video << endl;
        size_video++;
    }
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
    double start_time = duration.count();


    if(NUM_THREADS){
        auto start = high_resolution_clock::now();

        imageSubtraction(h, cropped_empty, vid, NUM_THREADS, video_name,size_video); //last parameter is size of video

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with threads "<<NUM_THREADS<<": "
             << duration.count() + start_time << " microseconds/ "  << endl;

    }
    else{
        vector<vector<double> >data;
    for (int i = 1; i <= 10; i++)
    {
        auto start = high_resolution_clock::now();

        imageSubtraction(h, cropped_empty, vid, i, video_name, size_video //sizevideo
        );

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with threads "<<i<<": "
             << duration.count() +start_time<< " microseconds/ "  << endl;

        vector<double>temp;
        int al=int(duration.count()+start_time) ;
        cout<<al<<endl;
        temp.push_back(i);temp.push_back(al);
        data.push_back(temp);
        cout<<data.size()<<endl;
        
    }

    writeSomething(data,"Method3.csv");
    }

   
    exit(1);

    waitKey(0);

    return 0;
}
