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

void writeSomething(vector<vector<double>> v, string s)
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
    Mat cropped_empty = arg_st->emp, frame, h = arg_st->h;
    vector<double> density;
    density = arg_st->density;

    int thread_num = arg_st->start, total_thread = arg_st->total;

    string videoName = arg_st->videoName;

    VideoCapture vid(videoName);
    int frame_num = 0;
    int img_size=cropped_empty.rows*cropped_empty.cols;

    int i=0;
    while (true)
    {
        if (frame_num % 100 == 0)
        {
            cout << thread_num << " " << frame_num << endl;
        }

        vid >> frame;

        if (frame.empty())
        {
            break;
        }
        if (frame_num % total_thread == thread_num)
        {
            double staticCount = 0;

            Mat im_out, cropped_im, cropped_img;
            // Warp source image to destination based on homography
            warpPerspective(frame, im_out, h, frame.size());

            Rect crop_region(472, 52, 800 - 472, 830 - 52);
            cropped_im = im_out(crop_region);
            cvtColor(cropped_im, cropped_img, COLOR_BGR2GRAY);

            Mat new_cropped_image = cropped_img;





            for (int i = 0; i < new_cropped_image.rows; i++)
            {
                for (int j = 0; j < new_cropped_image.cols; j++)
                {
                    float cur_pix = new_cropped_image.at<uchar>(i, j);
                    float back_pix = cropped_empty.at<uchar>(i, j);
                    if (abs(cur_pix - back_pix) > 25)
                    {
                        staticCount++;
                    }
                }
            }
            // cout<<i<<" "<<thread_num<<endl;
            density[i] = staticCount/img_size;
            i++;
        }

        frame_num++;
    }
    cout<<"denisty intialise before"<<endl;
    arg_st->density = density;
    cout<<"density after"<<endl;
    pthread_exit(0);
}

void imageSubtraction(Mat h, Mat cropped_empty, VideoCapture vi, int NUM_THREADS, string videoName, int size_video)
{
    Mat frame, prev, dst, cropped;

    VideoCapture vid(videoName);

    pthread_t tids[NUM_THREADS];
    vector<int> first(NUM_THREADS, 0);

    vector<double> density(size_video / NUM_THREADS + 10, 0);

    cout<<"size: "<<size_video / NUM_THREADS + 10<<endl;

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

    for (int i = 0; i < size_video/NUM_THREADS+5; i++)
    {
        
        
        for (int j = 0; j < NUM_THREADS; j++)
        {
            vector<double> v;
            cout<<alpha[j].density[i]<<" ";
            v.push_back(i*NUM_THREADS+j);
            v.push_back(alpha[j].density[i]);
            vec.push_back(v);
            // cout<<i*NUM_THREADS+j<<" "<<vec.size()<<" "<<v[1]<<endl;
        }
        cout<<endl;
        
    }

    writeSomething(vec, "data.csv");
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

    if (NUM_THREADS)
    {
        auto start = high_resolution_clock::now();

        imageSubtraction(h, cropped_empty, vid, NUM_THREADS, video_name, size_video); //last parameter is size of video

        auto stop = high_resolution_clock::now();

        auto duration = duration_cast<microseconds>(stop - start);

        cout << "Time taken by function with threads " << NUM_THREADS << ": "
             << duration.count() << " microseconds/ " << duration.count() / 1000000 << " sec" << endl;
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
                 << duration.count() << " microseconds/ " << duration.count() / 1000000 << " sec" << endl;

            vector<double> temp;
            temp.push_back(i);
            temp.push_back(duration.count());
            data.push_back(temp);
        }
        writeSomething(data, "Method4.csv");
    }

    exit(1);

    waitKey(0);

    return 0;
}
