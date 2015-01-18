//
//  Analyzer.cpp
//  Whiteboard2Website
//
//  Created by Kyle Koser on 1/17/15.
//  Copyright (c) 2015 Zach Waterson. All rights reserved.
//

#include "Analyzer.h"

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

static double distance(cv::Point p1, cv::Point p2) {
    double dx = p1.x - p2.x;
    double dy = p1.y - p2.y;
    return sqrt(dx*dx + dy*dy);
}

static bool rectContains(cv::Rect r1, cv::Rect r2) {
    return r1.contains(r2.tl()) && r1.contains(r2.br());
}

struct item_comp
{
    inline bool operator() (const std::pair<cv::Rect, elementType>& struct1, const std::pair<cv::Rect, elementType>& struct2)
    {
        return (struct1.first.size().height > struct2.first.size().height);
    }
};

// Instance Methods

Analyzer::Analyzer(std::string aFileName) : fileName(aFileName ) {
    
}

Page Analyzer::createPage() {
    int threshold = 125;
    
    cv::Mat src = cv::imread(fileName);
    if (src.empty()) {
        std::cout << "Image not found" << std::endl;
        return Page(cv::Rect(0,0,0,0));
    }
    
    // Convert to grayscale
    cv::Mat gray;
    cv::cvtColor(src, gray, CV_BGR2GRAY);
    
    //threshold image
    cv::Mat threshed;
    cv::threshold(gray, threshed, threshold, 255, 0);
    
    //erode
    cv::Mat final;
    cv::Mat eroder = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3));
    cv::erode(threshed, final, eroder);
    
    // Use Canny instead of threshold to catch squares with gradient shading
    cv::Mat bw;
    cv::Canny(final, bw, 0, 50, 5);
    
    // Find contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(bw.clone(), contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    
    std::vector<cv::Point> approx;
    cv::Mat dst = final.clone();
    cv::Mat empty = cv::Mat::zeros(final.size().height, final.size().width, CV_8UC1);
    
    std::vector<cv::Rect> boxes;
    std::vector<std::pair<cv::Rect, elementType>> items;
    
    // We must save these intially so we can analyze them once all of the bounding boxes have been located
    std::vector<std::vector<cv::Point>> symbols;
    
    for (int i = 0; i < contours.size(); i++)
    {
        // Approximate contour with accuracy proportional
        // to the contour perimeter
        cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);
        
        bool alreadyFound = false;
        for (int j = 0; j < boxes.size() && !alreadyFound; j++) {
            cv::Rect bound = cv::boundingRect(approx);
            
            if(distance(boxes[j].tl(), bound.tl()) < (bound.width / 8))
                alreadyFound = true;
        }
        
        if(alreadyFound) {
            contours.erase(contours.begin() + i);
            i--;
            continue;
        }
        
        boxes.push_back(cv::boundingRect(approx));
        
        
        cv::rectangle(dst, cv::boundingRect(approx), cv::Scalar(0,255,255));
        // Skip small objects
        if (std::fabs(cv::contourArea(contours[i])) < 100)
            continue;
        
        cv::Scalar color(128, 255, 255);
        for (int j = 0; j < approx.size()-1; j++) {
            cv::line(empty, approx[j], approx[j+1], color);
        }
        cv::line(empty, approx.front(), approx.back(), color);
        
        
        // Otherwise, start looking for shapes
        //setLabel(dst, "SHAPE", contours[i]);
        if (approx.size() >= 4 && approx.size() <= 6 && cv::isContourConvex(approx))
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
            if (mincos >= -0.5 && maxcos <= 0.5) {
                //setLabel(dst, "RECT", contours[i]);
                continue;
            }
            //            else if (vtc == 5 && mincos >= -0.34 && maxcos <= -0.27)
            //                setLabel(dst, "PENTA", contours[i]);
            //            else if (vtc == 6 && mincos >= -0.55 && maxcos <= -0.45)
            //                setLabel(dst, "HEXA", contours[i]);
        }
        
        //Save the symbols for later now
        symbols.push_back(approx);
    }
    
    // Now go through all of the symbols
    for (int i = 0; i < symbols.size(); i++) {
        approx = symbols[i];
        //Symbol Recognition
        if (checkPic(approx)) {
            setLabel(dst, "PIC", approx);
            cv::Rect div = findContainer(cv::boundingRect(approx), boxes);
            cv::rectangle(dst, div, cv::Scalar(244,244,188));
            items.push_back(std::make_pair(div, Image));
            continue;
        }
        if (checkL(approx)) {
            setLabel(dst, "LINK", approx);
            cv::Rect div = findContainer(cv::boundingRect(approx), boxes);
            cv::rectangle(dst, div, cv::Scalar(244,188,244));
            items.push_back(std::make_pair(div, Link));
            continue;
        }
        if (checkT(approx)) {
            setLabel(dst, "TEXT", approx);
            cv::Rect div = findContainer(cv::boundingRect(approx), boxes);
            cv::rectangle(dst, div, cv::Scalar(255,255,0));
            items.push_back(std::make_pair(div, Text));
            continue;
        }
    }
    
    //    cv::drawContours( empty, contours, 10, cv::Scalar(128,255,255), 3);
    
    //    for (int i = 0; i < contours.size(); i++){
    //        drawContours(empty, contours, i, color, 1, 8);
    //    }
    //    for (int i = 0; i < contours.size(); i++) {
    //        for (int j = 0; j < contours[i].size(); j++) {
    //            cv::line(empty, contours[i][j], contours[i][j], color);
    //        }
    //    }
    
    //Draw a new image with the site on it
    cv::Mat res = cv::Mat::zeros(final.size().height, final.size().width, CV_8UC1);
    for (int i = 0; i < items.size(); i++) {
        cv::rectangle(res, items[i].first, CV_RGB(128, 255, 255));
        std::string type = "";
        switch (items[i].second) {
            case Text:
                type = "TEXT";
                break;
            case Link:
                type = "LINK";
                break;
            case Image:
                type = "PIC";
                break;
            default:
                break;
        }
        //setLabel(res, type, items[i].first, CV_RGB(128, 255, 255));
        std::stringstream oss;
        oss<<items[i].first.size().width<<"w, "<<items[i].first.size().height<<"h @ ("<<items[i].first.tl().x<<","<<items[i].first.tl().y<<")";
        setLabel(res, oss.str(), items[i].first, CV_RGB(128, 255, 255));
    }
    
    std::sort(items.begin(), items.end(), item_comp());
    
    //Now make the Page
    Page p(cv::Rect(0,0,600, 800));
    for (int i = 0; i < items.size(); i++) {
        p.addAndSortElement(items[i].first, items[i].second, p.rows);
    }
    
    
    imshow("contours", empty);
    cv::imshow("results", res);
    cv::imshow("dst", dst);
    std::cout<<p.generateHTML()<<std::endl;
    cv::waitKey(0);
    return p;
}

void Analyzer::setLabel(cv::Mat& im, const std::string label, std::vector<cv::Point>& contour, cv::Scalar color)
{
    int fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    
    cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
    cv::Rect r = cv::boundingRect(contour);
    
    cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
    cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), color, CV_FILLED);
    cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

void Analyzer::setLabel(cv::Mat& im, const std::string label, cv::Rect r, cv::Scalar color)
{
    int fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale = 0.4;
    int thickness = 1;
    int baseline = 0;
    
    cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
    
    cv::Point pt(r.x + ((r.width - text.width) / 2), r.y + ((r.height + text.height) / 2));
    cv::rectangle(im, pt + cv::Point(0, baseline), pt + cv::Point(text.width, -text.height), color, CV_FILLED);
    cv::putText(im, label, pt, fontface, scale, CV_RGB(0,0,0), thickness, 8);
}

bool Analyzer::checkL(std::vector<cv::Point> approx) {
    cv::Rect bound = cv::boundingRect(approx);
    double minDist = bound.area() / 150;
    cv::Point tl = bound.tl();
    cv::Point tr = tl;
    tr.x += bound.width;
    cv::Point bl = tl;
    bl.y += bound.height;
    cv::Point br = bound.br();
    
    bool foundTL = false;
    bool foundTR = false;
    bool foundBL = false;
    bool foundBR = false;
    
    for (int i = 0; i < approx.size(); i++) {
        if (distance(approx[i], tl) < minDist) {
            foundTL = true;
        }
        if (distance(approx[i], tr) < minDist) {
            foundTR = true;
        }
        if (distance(approx[i], bl) < minDist) {
            foundBL = true;
        }
        if (distance(approx[i], br) < minDist) {
            foundBR = true;
        }
    }
    
    return foundTL && foundBL && foundBR && !foundTR;
}

bool Analyzer::checkPic(std::vector<cv::Point> approx) {
    cv::Rect bound = cv::boundingRect(approx);
    double minDist = bound.area() / 150;
    cv::Point tl = bound.tl();
    cv::Point tr = tl;
    tr.x += bound.width;
    cv::Point bl = tl;
    bl.y += bound.height;
    cv::Point br = bound.br();
    
    bool foundTL = false;
    bool foundTR = false;
    bool foundBL = false;
    bool foundBR = false;
    
    for (int i = 0; i < approx.size(); i++) {
        if (distance(approx[i], tl) < minDist) {
            foundTL = true;
        }
        if (distance(approx[i], tr) < minDist) {
            foundTR = true;
        }
        if (distance(approx[i], bl) < minDist) {
            foundBL = true;
        }
        if (distance(approx[i], br) < minDist) {
            foundBR = true;
        }
    }
    
    return foundTL && foundBL && foundBR && foundTR;
}

bool Analyzer::checkT(std::vector<cv::Point> approx) {
    cv::Rect bound = cv::boundingRect(approx);
    double minDist = bound.area() / 200;
    cv::Point tl = bound.tl();
    cv::Point tr = tl;
    tr.x += bound.width;
    cv::Point bl = tl;
    bl.y += bound.height;
    
    bool foundTL = false;
    bool foundTR = false;
    
    for (int i = 0; i < approx.size(); i++) {
        if (distance(approx[i], tl) < minDist) {
            foundTL = true;
        }
        if (distance(approx[i], tr) < minDist) {
            foundTR = true;
        }
    }
    return foundTL && foundTR;
}

cv::Rect Analyzer::findContainer(cv::Rect r, std::vector<cv::Rect> rects) {
    for (int i = 0; i < rects.size(); i++) {
        if (rectContains(rects[i], r)) {
            return rects[i];
        }
    }
    return cv::Rect();
}

