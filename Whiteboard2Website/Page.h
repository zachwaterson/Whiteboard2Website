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
    TypeRow,
    TypePage,
    Spacing
} elementType;

class Element {
public:
    elementType type;
    int x;
    int y;
    int width;
    int height;
    Element(cv::Rect rectangle, elementType type);
    
    virtual std::string generateHTMLForParent(Element *Parent);

private:
    
};

class Row : public Element {
public:
    Element *keyElement;
    std::vector<Row *> subRows;
    std::vector<Element *> allElements;
    Row(Element *element);
    bool shouldContainAsPeer(Element *element); //if y and size are within 80% of key element
    bool shouldContainAsChild(Element *element); //if y and size are less than 80% of key element
    void sortSubrows();
    void sortElements();
    void updateFrame();
    void consolidateRows();
    std::string determineAndGenerateSpacing(int currentX, int nextX);
    virtual std::string generateHTMLForParent(Element *Parent);
    void insertSpacing();
};

class Page : public Element {
public:
    Page(cv::Rect r);
    void addElement(Element *element);
    void addElementWithinRows(cv::Rect rectangle, elementType type, std::vector<Row *> &rowVector);
    void addAndSortElement(cv::Rect rectangle, elementType type, std::vector<Row *> &rowVector);
    std::string generateHTML();
    std::vector<Row *> rows; //top level rows of page
    void sortRows();
    

private:
    std::vector<Element *> elements; //list of every element
    void layoutRow(int rowIndex);
    
};




#endif /* defined(__Whiteboard2Website__Page__) */
