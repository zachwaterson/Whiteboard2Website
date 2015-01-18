//
//  Analyzer.h
//  Whiteboard2Website
//
//  Created by Kyle Koser on 1/17/15.
//  Copyright (c) 2015 Zach Waterson. All rights reserved.
//

#ifndef __Whiteboard2Website__Analyzer__
#define __Whiteboard2Website__Analyzer__

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>
#include "Page.h"

class Analyzer {
public:
    Analyzer(std::string aFileName);
    
    Page createPage();
    
private:
    void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour, cv::Scalar color = CV_RGB(255, 255, 255));
    void setLabel(cv::Mat& im, const std::string label, cv::Rect r, cv::Scalar color = CV_RGB(255, 255, 255));
    bool checkL(std::vector<cv::Point> approx);
    bool checkPic(std::vector<cv::Point> approx);
    bool checkT(std::vector<cv::Point> approx);
    cv::Rect findContainer(cv::Rect r, std::vector<cv::Rect> rects);
    
    std::string fileName;
    
};

#endif /* defined(__Whiteboard2Website__Analyzer__) */
