//
//  Page.h
//  Whiteboard2Website
//
//  Created by Zach Waterson on 1/17/15.
//  Copyright (c) 2015 Zach Waterson. All rights reserved.
//

#ifndef __Whiteboard2Website__Page__
#define __Whiteboard2Website__Page__

#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

typedef enum : int {
    Text,
    Image,
    Link,
} elementType;

class Element {
public:
    elementType type;
    int x;
    int y;
    int width;
    int height;
    Element(cv::Rect rectangle, elementType type);
    std::string generateHTML();
    
private:
    
};

class Row {
public:
    Element keyElement;
    std::vector<Row> subRows;
    std::vector<Element> allElements;
    Row(Element element);
    bool shouldContainAsPeer(Element element); //if y and size are within 80% of key element
    bool shouldContainAsChild(Element element); //if y and size are less than 80% of key element
//    bool shouldBeContainedBy(Element element); //if y and size eclipse key element
    void layout();
};

class Page {
public:
    void addElement(Element element);
    double pageHeight;
    double pageWidth;
    void addElementWithinRows(cv::Rect rectangle, elementType type, std::vector<Row> &rowVector);
    std::string generateHTML();
    std::vector<Row> rows; //top level rows of page

private:
    std::vector<Element> elements; //list of every element
    void layoutRow(int rowIndex);
    
    
};




#endif /* defined(__Whiteboard2Website__Page__) */
