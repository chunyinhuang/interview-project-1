



#define CVUI_IMPLEMENTATION

#include <math.h> 
#include <opencv2/opencv.hpp>
#include "cvui.h"

#define WINDOW_NAME "CVUI"

void draw(cv::Mat *frame, cv::Point xy, cv::Vec3b color, const int point_size, int patch_size){
    
    // rescale the selected point back to the original size
    xy.x = xy.x*patch_size;
    xy.y = xy.y*patch_size;
    
    // draw the points in specified color
    for(int jj = 0; jj<point_size; jj++){
        for(int ii = 0; ii<point_size; ii++){
            (*frame).at<cv::Vec3b>(xy.y+jj, xy.x+ii) = color;
        }
    }
}

void draw_ellipse(cv::Mat *frame, std::vector<cv::Point> points, cv::Vec3b color){
    
    // initialize the parameters
    cv::RotatedRect theEllipse;
    
    // find ellipse
    if( points.size() > 5 )
        theEllipse = cv::fitEllipse(cv::Mat(points));
    else
        std::cerr<<"WARNING : The system needs more than 5 points to generate an ellipse!"<<std::endl;
    
    // draw ellipse
    ellipse( *frame, theEllipse, color);
    
}

int main()
{
    // initialize sizes
    const int point_size = 9;
    const int patch_size = 3*point_size;
    const int image_size = 60*point_size;
    
    // initialize colors
    cv::Vec3b gray(150, 150, 150);
    cv::Vec3b blue(255, 0, 0);
    cv::Vec3b red(0, 0, 255);
    
    // initialize images
    cv::Mat src(image_size+100, image_size, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Mat src_ref = cv::Mat::zeros(20, 20, CV_32F);
    for(int j=0; j<image_size; j++){
        for(int i=0; i<image_size; i++){
            if(i%patch_size<point_size && j%patch_size<point_size){
                src.at<cv::Vec3b>(j, i) = gray;
            }
        }
    }
    cv::Mat frame = src.clone();

    // Init a OpenCV window and tell cvui to use it.
    cv::namedWindow(WINDOW_NAME);
    cvui::init(WINDOW_NAME);
    
    // initialize templates and paremeters
    cv::Point cursor;
    std::vector<cv::Point> cir_points;
    float radius, radius_tmp;
    int count = 0;
    bool clicked = false;
    
    while (true)
    {
        // generate the circle when click the 'generate' button
        if (cvui::button(frame, image_size/2-80, image_size+30, 100, 40, "Generate")){
            // to regularize the generate behavior
            if(!clicked){
                draw_ellipse(&frame, cir_points, blue);
                clicked = true;
            }
        }
        
        // deal with selecting points
        if (cvui::mouse(cvui::LEFT_BUTTON, cvui::UP)){
            cursor = cvui::mouse();
            cursor.x = cursor.x/patch_size;
            cursor.y = cursor.y/patch_size;
            cir_points.push_back(cv::Point(cursor.x*patch_size+(point_size+1)/2, cursor.y*patch_size+(point_size+1)/2));
            // mark the point if it hasn't been selected
            if(frame.at<cv::Vec3b>(cursor.y*patch_size+1, cursor.x*patch_size+1)==gray){
                src_ref.at<float>(cursor.y, cursor.x) = 1.;
                draw(&frame, cursor, blue, point_size, patch_size);
            }
            
            // unmark the point if it has already been selected
            else if(frame.at<cv::Vec3b>(cursor.y*patch_size+1, cursor.x*patch_size+1)==blue){
                src_ref.at<float>(cursor.y, cursor.x) = 0.;
                draw(&frame, cursor, gray, point_size, patch_size);
            }
        }
        
        // reset the system by right clicking the mouse
        if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::UP)){
            src.copyTo(frame);
            src_ref = cv::Mat::zeros(20, 20, CV_32F);
            count = 0;
            clicked = false;
            cir_points.clear();
        }
        
        // Update cvui internal stuff
        cvui::update();
        imshow(WINDOW_NAME, frame);
        
        // press ESC to exit the system
        if (cv::waitKey(30) == 27)
        {
            break;
        }
    }
    return 0;
}
