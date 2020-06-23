
// --------------------------------------------------
//
//  main.cpp
//  neocis_xcode_1
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


void draw(cv::Mat src, cv::Mat *frame, cv::Point xy, cv::Vec3b color, const int point_size, int patch_size){
    
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

void draw_boundary(cv::Mat *frame, cv::Point center, cv::Vec3b color, int patch_size, int point_size){
    
    float distance;
    cv::Vec3b red(0, 0, 255);
    cv::Vec3b blue(255, 0, 0);
    
    // rescale the center back to the original size
    center.x = center.x*patch_size + (point_size+1)/2;
    center.y = center.y*patch_size + (point_size+1)/2;
    
    // find max, min, and target radius
    float max_radius = 0;
    float min_radius = 100000000;
    float radius;
    for(int jj = 0; jj<(*frame).rows; jj++){
        for(int ii = 0; ii<(*frame).cols; ii++){
            if((*frame).at<cv::Vec3b>(jj, ii) == color){
                distance = sqrt(pow((float)jj-(float)center.y, 2)+pow((float)ii-(float)center.x, 2));
                if(distance>max_radius)
                    max_radius = distance;
                else if(distance<min_radius)
                    min_radius = distance;
            }
        }
    }
    radius = (max_radius+min_radius)/2.;
    
    // draw the boundary points and the target circle points in blue
    for(int jj = 0; jj<(*frame).rows; jj++){
        for(int ii = 0; ii<(*frame).cols; ii++){
            distance = sqrt(pow((float)jj-(float)center.y, 2)+pow((float)ii-(float)center.x, 2));
            // set the threshold to a pixel size
            if(abs(distance-max_radius)<1. || abs(distance-min_radius)<1.)
                (*frame).at<cv::Vec3b>(jj, ii) = red;
            else if(abs(distance-radius)<1.){
                (*frame).at<cv::Vec3b>(jj, ii) = blue;
            }
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
    cv::Mat src(image_size, image_size, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::Mat src_ref = cv::Mat::zeros(20, 20, CV_8U); // may be deleted in this application
    for(int j=0; j<src.rows; j++){
        for(int i=0; i<src.cols; i++){
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
    cv::Point cursor_down;
    cv::Point cursor_up;
    bool clicked = false;
    float radius, radius_tmp;
    float max_radius = 0.;
    float min_radius = 10000000.;
    int jj, ii;

    // enter GUI
    while (true)
    {
        // set center of the circle
        if (cvui::mouse(cvui::LEFT_BUTTON, cvui::DOWN) && !clicked) {
            cursor_down = cvui::mouse();
            cursor_down.x = cursor_down.x/patch_size;
            cursor_down.y = cursor_down.y/patch_size;
            std::cout << "begin point" << std::endl;
            std::cout << "x: " << cursor_down.x << " y: " << cursor_down.y << std::endl;
        }
        
        // set the target circumference point
        if (cvui::mouse(cvui::LEFT_BUTTON, cvui::UP) && !clicked) {
            cursor_up = cvui::mouse();
            cursor_up.x = cursor_up.x/patch_size;
            cursor_up.y = cursor_up.y/patch_size;
            std::cout << "end point" << std::endl;
            std::cout << "x: " << cursor_up.x << " y: " << cursor_up.y << std::endl;
            clicked = true;
        }
        
        // reset the system by right clicking the mouse
        if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::DOWN) && clicked){
            src.copyTo(frame);
            clicked = false;
        }
        
        // clicked indicates the system is ready to draw
        if(clicked){
            
            // mark center(red)
            draw(src, &frame, cursor_down, red, point_size, patch_size);
            
            // find circumference and mark the pixels as blue
            radius = sqrt(pow((float)cursor_up.x-(float)cursor_down.x, 2) + pow((float)cursor_up.y-(float)cursor_down.y, 2));
            for(int j=-ceil(radius); j<ceil(radius)+1; j++){
                for(int i=-ceil(radius); i<ceil(radius)+1; i++){
                    jj = j+cursor_down.y;
                    ii = i+cursor_down.x;
                    if(jj>=0 && jj<src_ref.rows && ii>=0 && ii<src_ref.cols){
                        radius_tmp = sqrt(pow((float)j, 2) + pow((float)i, 2));
                        // the threshold is set to be half of a pixel size
                        if(abs(radius_tmp-radius) < 0.5){
                            draw(src, &frame, cv::Point(ii, jj), blue, point_size, patch_size);
                        }
                        
                    }
                }
            }
            
            // draw boundary
            draw_boundary(&frame, cursor_down, blue, patch_size, point_size);
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
