////
////  main.cpp
////  OpenCVTest
////
////  Created by Zach Waterson on 1/17/15.
////  Copyright (c) 2015 Zach Waterson. All rights reserved.
////

#include <iostream>
#include "Page.h"
#include "Analyzer.h"

int main()
{
    //cv::Mat src = cv::imread("polygon.png");
    std::string filename;
    
    std::cout << "Please enter the path of the image: ";
    std::getline( std::cin, filename );
    
    Analyzer an(filename);
    std::cout<<an.createPage().generateHTML()<<std::endl;
    
    return 0;
}