
// --------------------------------------------------
//
//  main.cpp
//  neocis_xcode_2
//
//  Created by 黃俊穎 on 2020/6/4.
//  Copyright © 2020 Jin. All rights reserved.
//
// --------------------------------------------------



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

void draw_circle(cv::Mat src_ref, cv::Mat *frame, cv::Point center, cv::Vec3b color, int patch_size, int point_size){
    
    // initialize the parameters
    float radius = 0;
    float count = 0;
    float distance;
    
    // find mean radius
    for(int jj = 0; jj<src_ref.rows; jj++){
        for(int ii = 0; ii<src_ref.cols; ii++){
            if(src_ref.at<float>(jj, ii) == 1.){
                // rescale back to original size
                jj = jj*patch_size + (point_size+1)/2;
                ii = ii*patch_size + (point_size+1)/2;
                radius += sqrt(pow((float)jj-(float)center.y, 2)+pow((float)ii-(float)center.x, 2));
                count++;
            }
        }
    }
    
    // find the radius
    radius /= count;
    
    // mark the center as red (added to better visualize the result)
    (*frame).at<cv::Vec3b>(center.y, center.x) = color;
    
    // draw the calculated circle
    for(int jj = 0; jj<(*frame).rows; jj++){
        for(int ii = 0; ii<(*frame).cols; ii++){
            distance = sqrt(pow((float)jj-(float)center.y, 2)+pow((float)ii-(float)center.x, 2));
            // set the threshold to a pixel size
            if(abs(distance-radius)<1.)
                (*frame).at<cv::Vec3b>(jj, ii) = color;
        }
    }
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
    cv::Point center(0, 0);
    float radius, radius_tmp;
    int count = 0;
    bool clicked = false;
    
    while (true)
    {
        // generate the circle when click the 'generate' button
        if (cvui::button(frame, image_size/2-80, image_size+30, 100, 40, "Generate")){
            // to regularize the generate behavior
            if(!clicked && count!=0){
                center.x /= count;
                center.y /= count;
                draw_circle(src_ref, &frame, center, blue, patch_size, point_size);
                clicked = true;
            }
            else{
                std::cerr<<"Warning: Please select points before generate"<<std::endl;
            }
        }
        
        // deal with selecting points
        if (cvui::mouse(cvui::LEFT_BUTTON, cvui::UP)){
            cursor = cvui::mouse();
            cursor.x = cursor.x/patch_size;
            cursor.y = cursor.y/patch_size;
            
            // mark the point if it hasn't been selected
            if(frame.at<cv::Vec3b>(cursor.y*patch_size+1, cursor.x*patch_size+1)==gray){
                center.x += cursor.x*patch_size + (point_size+1)/2;
                center.y += cursor.y*patch_size + (point_size+1)/2;
                count += 1;
                src_ref.at<float>(cursor.y, cursor.x) = 1.;
                draw(&frame, cursor, blue, point_size, patch_size);
            }
            
            // unmark the point if it has already been selected
            else if(frame.at<cv::Vec3b>(cursor.y*patch_size+1, cursor.x*patch_size+1)==blue){
                center.x -= cursor.x*patch_size + (point_size+1)/2;
                center.y -= cursor.y*patch_size + (point_size+1)/2;
                count -= 1;
                src_ref.at<float>(cursor.y, cursor.x) = 0.;
                draw(&frame, cursor, gray, point_size, patch_size);
            }
        }
        
        // reset the system by right clicking the mouse
        if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::UP)){
            src.copyTo(frame);
            src_ref = cv::Mat::zeros(20, 20, CV_32F);
            center.x = 0;
            center.y = 0;
            count = 0;
            clicked = false;
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
