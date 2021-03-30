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

struct Thread_Struct
{
    Mat emp;
    Mat cur;
    vector<double> density;
    int ind;
    int start;
    int total;
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
    Mat new_cropped_image = arg_st->cur, cropped_empty = arg_st->emp;
    vector<double> density;
    density = arg_st->density;

    int start = arg_st->start, end = arg_st->total;
    int ind = arg_st->ind;
    int length = new_cropped_image.cols / end;
    double staticCount = 0;

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
    double val = staticCount;

    density[ind] = val;
    arg_st->density = density;

    pthread_exit(0);
}

void imageSubtraction(Mat h, Mat cropped_empty, VideoCapture vi, int NUM_THREADS, string videoName,int size_video)
{
    Mat frame, prev, dst, cropped;

    
    VideoCapture vid(videoName);

    pthread_t tids[NUM_THREADS];
    vector<int> first(NUM_THREADS, 0);
    vector<vector<double>> vec;
    vector<double> density(size_video, 0);

    vector<Thread_Struct> alpha;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        Thread_Struct arg = {
            cropped_empty,
            cropped_empty,
            density,
            0,
            i,
            NUM_THREADS};
        alpha.push_back(arg);
    }
    int size = cropped_empty.rows * cropped_empty.cols;

    //first iteration
    vid >> frame;
    Mat new_cropped_image = cropframe(frame, h);
    for (int j = 0; j < NUM_THREADS; j++)
    {

        pthread_attr_t attr;
        alpha[j].cur = new_cropped_image;
        alpha[j].ind = 0;
        pthread_attr_init(&attr);
        pthread_create(&tids[j], &attr, thread_run, &alpha[j]);
    }

    int i = 1;
    while (true)
    {
        if(i==100){
            break;
        }

        Mat frame;
        vid >> frame;
        if (frame.empty())
        {
            break;
        }
        Mat new_cropped_image = cropframe(frame, h);

        for (int j = 0; j < NUM_THREADS; j++)
        {
            pthread_join(tids[j], NULL);

            pthread_attr_t attr;
            alpha[j].cur = new_cropped_image;
            alpha[j].ind = i;
            pthread_attr_init(&attr);
            pthread_create(&tids[j], &attr, thread_run, &alpha[j]);
        }

        vector<double> v;
        v.push_back(i - 1);
        double val = 0;
        for (int j = 0; j < NUM_THREADS; j++)
        {
            val += alpha[j].density[i - 1];
        }

        val /= size;
        v.push_back(val);

        vec.push_back(v);
        i++;
    }

    //last iteration
    vector<double> v;
    v.push_back(i - 1);
    double val = 0;
    for (int j = 0; j < NUM_THREADS; j++)
    {
        pthread_join(tids[j], NULL);
        val += alpha[j].density[i - 1];
    }
    val /= size;
    cout << i - 1 << " " << val << endl;
    v.push_back(val);
    vec.push_back(v);

    
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

    int size_video = 5736;
    while (false)
    {
        Mat frame;
        vid >> frame;
        if (frame.empty())
        {
            break;
        }
        // cout << size_video << endl;
        size_video++;
    }

    if(NUM_THREADS){
        auto start = high_resolution_clock::now();

        imageSubtraction(h, cropped_empty, vid, NUM_THREADS, video_name,size_video);

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with threads "<<NUM_THREADS<<": "
             << duration.count() << " microseconds/ " << duration.count() / 1000000 << " sec" << endl;

    }
    else{
        vector<vector<double> >data;
    for (int i = 1; i <= 100; i++)
    {
        auto start = high_resolution_clock::now();

        imageSubtraction(h, cropped_empty, vid, i, video_name,size_video);

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with threads "<<i<<": "
             << duration.count() << " microseconds/ " << duration.count() / 1000000 << " sec" << endl;

        vector<double>temp;
        temp.push_back(i);temp.push_back(duration.count());
        
    }
    writeSomething(data,"Method3.csv");
    }

   
    exit(1);

    waitKey(0);

    return 0;
}
