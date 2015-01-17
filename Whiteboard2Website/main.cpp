////
////  main.cpp
////  OpenCVTest
////
////  Created by Zach Waterson on 1/16/15.
////  Copyright (c) 2015 Zach Waterson. All rights reserved.
////
//
//#include <iostream>
//#include <opencv2/opencv.hpp>
//
//using namespace std;
//using namespace cv;
//
//int main (int argc, const char * argv[])
//{
//    Mat img = imread("/Users/Zach/Desktop/joey.png"); //Change the image path here.
//    if (img.data == 0) {
//        cerr << "Image not found!" << endl;
//        return -1;
//    }
//    namedWindow("image", CV_WINDOW_AUTOSIZE);
//    imshow("image", img);
//    waitKey();
//}

/**
 * Simple shape detector program.
 * It loads an image and tries to find simple shapes (rectangle, triangle, circle, etc) in it.
 * This program is a modified version of `squares.cpp` found in the OpenCV sample dir.
 */

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <iostream>

/**
 * Helper function to find a cosine of angle between vectors
 * from pt0->pt1 and pt0->pt2
 */
static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

/**
 * Helper function to display text in the center of a contour
 */
void setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour)
{
    int fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    
    cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
    cv::Rect r = cv::boundingRect(contour);
    
    cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
    cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), CV_RGB(255,255,255), CV_FILLED);
    cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

int main()
{
    //cv::Mat src = cv::imread("polygon.png");
    std::string filename;
    
    std::cout << "Please enter the path of the image: ";
    std::getline( std::cin, filename );
    
    cv::Mat src = cv::imread(filename);
    if (src.empty())
        return -1;
    
    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(src, gray, CV_BGR2GRAY);
    
    //threshold image
    cv::Mat threshed;
    cv::threshold(gray, threshed, 140, 255, 0);
    
    //erode
    cv::Mat final;
    cv::Mat eroder = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    cv::erode(threshed, final, eroder);
    
    // Use Canny instead of threshold to catch squares with gradient shading
    cv::Mat bw;
    cv::Canny(final, bw, 0, 50, 5);
    
    // Find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(bw.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::Point> approx;
    cv::Mat dst = final.clone();
    
    std::vector<cv::Rect> boxes;
    
    for (int i = 0; i < contours.size(); i++)
    {
        // Approximate contour with accuracy proportional
        // to the contour perimeter
        cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
        
        bool alreadyFound = false;
        for (int j = 0; j < boxes.size() && !alreadyFound; j++) {
            cv::Rect bound = cv::boundingRect(approx);

            double dx = boxes[j].tl().x - bound.tl().x;
            double dy = boxes[j].tl().y - bound.tl().y;
            if(sqrt(dx*dx + dy*dy) < 15)
                alreadyFound = true;
            
            //std::cout<<boxes[j].tl().x<<", "<<boxes[j].tl().y<<" vs "<<bound.tl().x<<", "<<bound.tl().y<<" dist: "<<sqrt(dx*dx + dy*dy)<<std::endl;
        }
        
        if(alreadyFound) {
            continue;
        }
        
        boxes.push_back(cv::boundingRect(approx));

        
        cv::rectangle(dst, cv::boundingRect(approx), cv::Scalar(0,255,255));
        // Skip small objects
        if (std::fabs(cv::contourArea(contours[i])) < 100)
            continue;
        
        // Otherwise, start looking for shapes
        //setLabel(dst, "SHAPE", contours[i]);
        if (approx.size() >= 4 && approx.size() <= 6)
        {
            // Number of vertices of polygonal curve
            int vtc = (int)approx.size();
            
            // Get the cosines of all corners
            std::vector<double> cos;
            for (int j = 2; j < vtc+1; j++)
                cos.push_back(angle(approx[j%vtc], approx[j-2], approx[j-1]));
            
            // Sort ascending the cosine values
            std::sort(cos.begin(), cos.end());
            
            // Get the lowest and the highest cosine
            double mincos = cos.front();
            double maxcos = cos.back();
            
            // Use the degrees obtained above and the number of vertices
            // to determine the shape of the contour
            //if (vtc == 4 && mincos >= -0.1 && maxcos <= 0.3)
            if (vtc == 4 && mincos >= -0.4 && maxcos <= 0.4) {
                //setLabel(dst, "RECT", contours[i]);
                continue;
            }
//            else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
//                setLabel(dst, "PENTA", contours[i]);
//            else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
//                setLabel(dst, "HEXA", contours[i]);
        }
//        else
//        {
//            // Detect and label circles
//            double area = cv::contourArea(contours[i]);
//            cv::Rect r = cv::boundingRect(contours[i]);
//            int radius = r.width / 2;
//            
//            if (std::abs(1 - ((double)r.width / r.height)) <= 0.2 &&
//                std::abs(1 - (area / (CV_PI * std::pow(radius, 2)))) <= 0.2)
//                setLabel(dst, "CIR", contours[i]);
//        }
        
        //Symbol Recognition
        if(approx.size() == 9 && !cv::isContourConvex(approx)) {
            setLabel(dst, "T", contours[i]);
            continue;
        }
        std::ostringstream strb;
        strb<<cv::boundingRect(approx).tl();
        setLabel(dst, strb.str(), contours[i]);
        //cv::rectangle(dst, cv::boundingRect(approx), cv::Scalar(255,255,255));

        
    }
    
    cv::imshow("dst", dst);
    cv::waitKey(0);
    return 0;
}