



#define CVUI_IMPLEMENTATION

#include <math.h> 
#include <opencv2/opencv.hpp>
#include "cvui.h"

#define WINDOW_NAME "CVUI"


class circleUI
{
public:
    // initialize sizes
    int point_size;
    int patch_size;
    int image_size;
    int point_num;
    
    // initialize images
    cv::Mat src;
    cv::Mat src_ref;
    cv::Mat frame;
    
    // initialize parameters
    cv::Point center;
    bool clicked = false;
    
    // initialization
    circleUI(const int p, const int pn, cv::Vec3b color){
        point_num = pn;
        point_size = p;
        patch_size= 3*point_size;
        image_size = 60*point_size;
        src = cv::Mat(image_size+100, image_size, CV_8UC3, cv::Scalar(255, 255, 255));
        for(int j=0; j<image_size; j++){
            for(int i=0; i<image_size; i++){
                if(i%patch_size<point_size && j%patch_size<point_size){
                    src.at<cv::Vec3b>(j, i) = color;
                }
            }
        }
        src_ref = cv::Mat::zeros(point_num, point_num, CV_32F);
        frame = src.clone();
    }
    
    // draw function
    void draw(cv::Point xy, cv::Vec3b color){
        
        if(color==cv::Vec3b(255, 0, 0)){
            center.x += xy.x*patch_size + (point_size+1)/2;
            center.y += xy.y*patch_size + (point_size+1)/2;
            src_ref.at<float>(xy.y, xy.x) = 1.;
        }
        else{
            center.x -= xy.x*patch_size + (point_size+1)/2;
            center.y -= xy.y*patch_size + (point_size+1)/2;
            src_ref.at<float>(xy.y, xy.x) = 0.;
        }
        
        // rescale the selected point back to the original size
        xy.x = xy.x*patch_size;
        xy.y = xy.y*patch_size;
        
        // draw the points in specified color
        for(int jj = 0; jj<point_size; jj++){
            for(int ii = 0; ii<point_size; ii++){
                frame.at<cv::Vec3b>(xy.y+jj, xy.x+ii) = color;
            }
        }
    }
    
    // draw circle function
    void draw_circle(int count_, cv::Vec3b color){
        
        center.x /= count_;
        center.y /= count_;
        clicked = true;
        
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
        frame.at<cv::Vec3b>(center.y, center.x) = color;
        
        // draw the calculated circle
        for(int jj = 0; jj<frame.rows; jj++){
            for(int ii = 0; ii<frame.cols; ii++){
                distance = sqrt(pow((float)jj-(float)center.y, 2)+pow((float)ii-(float)center.x, 2));
                // set the threshold to a pixel size
                if(abs(distance-radius)<1.)
                    frame.at<cv::Vec3b>(jj, ii) = color;
            }
        }
    }
    
    // reset
    void reset(){
        frame = src.clone();
        src_ref = cv::Mat::zeros(point_num, point_num, CV_32F);
        center.x = 0;
        center.y = 0;
        clicked = false;
    }
    
};

int main()
{
    
    // initialize colors
    cv::Vec3b gray(150, 150, 150);
    cv::Vec3b blue(255, 0, 0);
    cv::Vec3b red(0, 0, 255);
    
    // initialize function class
    circleUI object(9, 20, gray);

    // Init a OpenCV window and tell cvui to use it.
    cv::namedWindow(WINDOW_NAME);
    cvui::init(WINDOW_NAME);
    
    // initialize templates and paremeters
    cv::Point cursor;
    int count = 0;
    
    
    while (true)
    {
        // generate the circle when click the 'generate' button
        if (cvui::button(object.frame, object.image_size/2-80, object.image_size+30, 100, 40, "Generate")){
            // to regularize the generate behavior
            if(!object.clicked && count!=0){
                object.draw_circle(count, blue);
            }
            else{
                std::cerr<<"Warning: Please select points before generate"<<std::endl;
            }
        }
        
        // deal with selecting points
        if (cvui::mouse(cvui::LEFT_BUTTON, cvui::UP)){
            cursor = cvui::mouse();
            cursor.x = cursor.x/object.patch_size;
            cursor.y = cursor.y/object.patch_size;
            
            // mark the point if it hasn't been selected
            if(object.frame.at<cv::Vec3b>(cursor.y*object.patch_size+1, cursor.x*object.patch_size+1)==gray){
                count += 1;
                object.draw(cursor, blue);
            }
            
            // unmark the point if it has already been selected
            else if(object.frame.at<cv::Vec3b>(cursor.y*object.patch_size+1, cursor.x*object.patch_size+1)==blue){
                object.draw(cursor, gray);
            }
        }
        
        // reset the system by right clicking the mouse
        if (cvui::mouse(cvui::RIGHT_BUTTON, cvui::UP)){
            object.reset();
            count = 0;
        }
        
        // Update cvui internal stuff
        cvui::update();
        imshow(WINDOW_NAME, object.frame);
        
        // press ESC to exit the system
        if (cv::waitKey(30) == 27)
        {
            break;
        }
    }
    return 0;
}
