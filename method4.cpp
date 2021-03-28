#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <pthread.h>

using namespace cv;
using namespace std;

int cnt = 0;
vector<Point2f> pts_src;
RNG rng(12345);


struct Thread_Struct{
    Mat emp;
    Mat cur;
    vector<double>density;
    int ind;

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

void writeSomething(vector<vector<double>> v)
{
    ofstream out;
    out.open("data.csv");
    for (int i = 0; i < v.size(); i++)
        out << v[i][0] << "," << v[i][1] << endl;
}

Mat reduce_ImgSize(Mat inImg, double scale)
{
    Mat dst;

    // Creating the Size object
    resize(inImg, dst, cv::Size(inImg.cols * scale, inImg.rows * scale), 0, 0, INTER_LINEAR);

    return dst;
}

void* thread_run(void* arg)
    {
        
	struct Thread_Struct *arg_st =
			(struct Thread_Struct*) arg;
    // cout<<"HELLO :"<<arg_st->ind<<endl;
    Mat new_cropped_image=arg_st->cur ,  cropped_empty = arg_st->emp;
    vector<double>density ;
    density= arg_st->density;
    int ind = arg_st->ind;
        double staticCount=0;
            for(int i=0; i<new_cropped_image.rows ;i++){
                for(int j=0; j< new_cropped_image.cols ;j++){
                    float cur_pix = new_cropped_image.at<uchar>(i,j);
                    float back_pix = cropped_empty.at<uchar>(i,j);

                    if(abs(cur_pix-back_pix)>25){
                        staticCount++;
                    }
                    
                }
            }
        // cout<<arg_st<<endl;
        double val=staticCount/(cropped_empty.cols*cropped_empty.rows);

        density[ind]=val;
        cout<<ind<<" "<<val<<endl;
        cout<<"density assign ke phele \n";
    arg_st->density=density;
    cout<<"density assigned after \n";
    

	pthread_exit(0);
}

void imageSubtraction(Mat h, Mat cropped_empty, VideoCapture vi, int NUM_THREADS,string videoName)
{
    // vector<vector<double>> density;

    //Capturing frames from the video and finding absdiff
    Mat frame, prev, dst, cropped;


    int size_video=5736;
    // while(true){
    //     Mat frame;
    //     vi>>frame;
    //     if(frame.empty()){
    //         break;
    //     }
    //     cout<<size_video<<endl;
    //     size_video++;
    // }
    VideoCapture vid(videoName);

    pthread_t tids[NUM_THREADS];
    vector<int>first(NUM_THREADS,0);
    vector<vector<double> >vec;
    vector<double> density (size_video,0);

    vector<Thread_Struct> alpha;

    for(int i=0;i<NUM_THREADS;i++){
        Thread_Struct arg={
            cropped_empty,
            cropped_empty,
            density,
            0
            };
        alpha.push_back(arg);
    }
    
    
    int i=0;
    while(true){
        Mat frame;
        vid >> frame;
        // cout<<i<<endl;
        if (frame.empty())
        {
            break;
        }
        int thread_num= i%NUM_THREADS;

        Mat new_cropped_image = cropframe(frame, h);
        
        cout<<thread_num<<endl;
        if(first[thread_num]==0){
            first[thread_num]++;
            pthread_attr_t attr;
            alpha[thread_num].cur = new_cropped_image;
            alpha[thread_num].ind = i;
		    pthread_attr_init(&attr);
            // cout<<"HELLO"<<endl;
            pthread_create(&tids[thread_num], &attr, thread_run, &alpha[thread_num]);
            
        }
        else{

            cout<<thread_num<<" thread Joined \n";
            pthread_join(tids[thread_num], NULL);

            pthread_attr_t attr;
		    // cout<<"---attr printed"<<endl;
            alpha[thread_num].cur = new_cropped_image;
            alpha[thread_num].ind = i;
		    pthread_attr_init(&attr);
            // cout<<"attr modified"<<endl;
            // cout<<"HELLO"<<endl;
            pthread_create(&tids[thread_num], &attr, thread_run, &alpha[thread_num]);
        }
        i++;
    }
    for(int i=0;i<size_video;i++){
        int j=i%NUM_THREADS;
        
            vector<double>v;
            // cout<<i<<" "<<NUM_THREADS<<" "<<i*N
            v.push_back(i);
            v.push_back(alpha[j].density[i]);
            vec.push_back(v);
        
    }
    writeSomething(vec);
    
    

    
// vector<double > temp= 


    // while (true)
    // {
    //     Mat frame;
    //     vid >> frame;
    //     // cout<<i<<endl;
    //     if (frame.empty())
    //     {
    //         break;
    //     }
    //     // vector<double> frame_density;
    //     // frame_density.push_back(i);
    //     int thread_num= i%NUM_THREADS;



    //     Mat new_cropped_image = cropframe(frame, h);
    //     Thread_Struct arg={
    //         cropped_empty,
    //         new_cropped_image,
    //         0,
    //         i
    //         };
        
    //     density_struct.push_back(arg);
    //     cout<<i<<" "<<density_struct.size()<<endl;
    //     if(first[i]){
    //         pthread_attr_t attr;
	// 	    pthread_attr_init(&attr);
    //         cout<<"HELLO"<<endl;
    //         pthread_create(&tids[thread_num], &attr, thread_run, &density_struct[i-1]);
    //         first[i]=0;
    //     }
    //     else{
    //         cout<<thread_num<<" thread Joined \n";
    //         pthread_join(tids[thread_num], NULL);

    //         pthread_attr_t attr;
	// 	    pthread_attr_init(&attr);
    //         pthread_create(&tids[thread_num], &attr, thread_run, &density_struct[i-1]);

    //     }
    
    //     int size = new_cropped_image.rows * new_cropped_image.cols;
        
    //     char c = (char)waitKey(25);
    //     if (c == 27)
    //     {
    //         cout << "ESE pressed" << endl;
    //         break;
    //     }
    //     i++;
    // }
    // // writeSomething(density);
}
int main(int argc, char *argv[])
{
    //open the video file for reading
    // if (argc <2)
    // {
    //     cout << "enter argument as image name also \n ./main \"VideoName\" ";
    // }
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

    // cropped_empty=reduce_ImgSize(cropped_empty);

    time_t start, end;
    time(&start);

    imageSubtraction(h, cropped_empty, vid, NUM_THREADS,video_name);

    time(&end);
    double time_taken = double(end - start);
    cout << "Time taken by program is : " << fixed
         << time_taken << setprecision(5);
    cout << " sec " << endl;
    exit(1);

    waitKey(0);

    return 0;
}
