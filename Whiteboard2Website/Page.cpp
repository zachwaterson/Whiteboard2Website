//
//  Page.cpp
//  Whiteboard2Website
//
//  Created by Zach Waterson on 1/17/15.
//  Copyright (c) 2015 Zach Waterson. All rights reserved.
//

#include "Page.h"
#include <iostream>
#include <cmath>

#define PEER_HEIGHT_PERCENT 0.4
#define PEER_Y_BUFFER 35

Element::Element(cv::Rect rectangle, elementType inputType): x(rectangle.x), y(rectangle.y), width(rectangle.width), height(rectangle.height), type(inputType) {

}

Row::Row(Element *element): keyElement(element), Element(cv::Rect(element->x, element->y, element->width, element->height), TypeRow) {
    allElements.push_back(element);
    
}

Page::Page(cv::Rect r) : Element(r, TypePage) {
    
}

bool Row::shouldContainAsPeer(Element *element) {
    //if y's are close enough
    if (abs(keyElement->y - element->y) < PEER_Y_BUFFER) {
        //if sizes are close enough
        if (abs(keyElement->height - element->height) < (keyElement->height > element->height ? PEER_HEIGHT_PERCENT*keyElement->height : PEER_HEIGHT_PERCENT*element->height)) {
            return true;
        }
    }
    return false;
}

bool Row::shouldContainAsChild(Element *element) {
    //if y falls within buffer-enhanced range
    //within the height of the key element
    if ((element->y > keyElement->y - PEER_Y_BUFFER) && (element->y < keyElement->y + keyElement->height + PEER_Y_BUFFER)) {
        //if height is smaller
        if (keyElement->height - element->height > PEER_HEIGHT_PERCENT*keyElement->height && keyElement->height > element->height) {
            return true;
        }
    }
    return false;
}


int compareRows(Row *row1, Row *row2) {
    int value = row1->keyElement->y < row2->keyElement->y; // ? -1 : 1;
    return value;
}

int compareElements( Element *elem1, Element *elem2) {
    int value = elem1->x < elem2->x; // ? -1 : 1;
    return value;
}

void Row::sortSubrows() {
    std::sort(subRows.begin(), subRows.end(), compareRows);
    for (int i = 0; i < subRows.size(); i++) {
        subRows[i]->sortSubrows();
    }
}

void Page::sortRows() {
    //sort initial rows
    std::sort(rows.begin(), rows.end(), compareRows);
    //sort subrows
    for (int i = 0; i < rows.size(); i++) {
        rows[i]->sortSubrows();
    }

}


void Page::addElementWithinRows(cv::Rect rectangle, elementType type, std::vector<Row *> &rowVector) {
    Element *element = new Element(rectangle, type);
    
    //check if it relates to another row as a child, peer, or parent

    for (int i = 0; i < rowVector.size(); i++) {
        if (rowVector[i]->shouldContainAsChild(element)) {
            //add as a child
            //if has subrows, run recursively on them
            if (rowVector[i]->subRows.size()) {
                addElementWithinRows(rectangle, type, rowVector[i]->subRows);
            } else {
                //else create a new row and add it to the subrows
                rowVector[i]->subRows.push_back(new Row(element));
            }
            return;
        } else if (rowVector[i]->shouldContainAsPeer(element)) {
            //add as a peer
            rowVector[i]->allElements.push_back(element);
            
            return;
        }
    }
    
    //else make a row for it
    Row *newRow = new Row(element);
    if (rowVector.size()) {
        //find where in the stack it should go
        //find the row
        for (int rowIndex = 0; rowIndex < rowVector.size() - 1; rowIndex++) {
            if (newRow->keyElement->y < rowVector[rowIndex]->keyElement->y) {
                rowVector.insert(rowVector.begin() + rowIndex, newRow);
                return;
            }
        }
        //if exited for loop without finding, needs to go at the end
        rowVector.push_back(newRow);
    } else {
        //if vector empty, initialize it
        rowVector.push_back(newRow);
    }
}

void Page::addAndSortElement(cv::Rect rectangle, elementType type, std::vector<Row *> &rowVector) {
    if (rectangle.size().width && rectangle.size().height) {
        addElementWithinRows(rectangle, type, rowVector);
        sortRows();
    }

}

void Row::updateFrame() {
    for (int i = 0; i < allElements.size(); i++) {
        if (allElements[i]->type == TypeRow) {
            ((Row*)allElements[i])->updateFrame();
        }
        if (allElements[i]-> x < x) {
            x = allElements[i]->x;
        }
        int diff = allElements[i]->x + allElements[i]->width - x;
        if (diff > width) {
            width = diff;
        }
    }
}


std::string Element::generateHTMLForParent(Element *parent) {
    
    std::stringstream HTML;
    int columnSize = 12*width/parent->width;
    switch (type) {
        case Text:
            HTML << "<div class='col-sm-"<<columnSize<<"'><h3 class='text-center'>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse viverra ex sed aliquet dignissim. Proin nulla mi, posuere non nulla vel, condimentum finibus orci. Integer vulputate dignissim lorem, in aliquam nibh vehicula eget. Nullam ullamcorper feugiat dolor a bibendum. Quisque laoreet pulvinar purus ac tristique. Suspendisse ullamcorper scelerisque dictum. Vestibulum id efficitur erat, non pharetra enim. Nunc condimentum justo in leo mattis porta.</h3></div>";
            break;
        case Image:
            HTML << "<div class='col-sm-"<<columnSize<<"'><img class='img-responsive center-block' src='http://www3.nd.edu/~kgsa/springmainbd.jpg'></img></div>";
            break;
        case Link:
            HTML << "<div class='col-sm-"<<columnSize<<"'><a class='btn btn-default center-block' style= 'max-width: 100px;' href='http://www.nd.edu'>Link</a></div>";
            break;
        case Spacing:
            HTML << "<div class='col-sm-"<<columnSize<<"'></div>";
            break;
        default:
            break;
    }
    return HTML.str();
    

}

std::string Row::generateHTMLForParent(Element *parent) {
    int columnSize = 12*width/parent->width;
    
    std::stringstream HTML;
    int matchingIndex = -1;
    
    //find yourself in the parent's array
    if (parent->type == TypeRow) {
        Row *R = static_cast<Row*>(parent);
        for (int i = 0; i < R->allElements.size(); i++) {
            if (this == R->allElements[i]) {
                matchingIndex = i;
                break;
            }
        }
    }
    
    //if parent is a row and element on left side isn't a row
    if(parent->type == TypeRow) {
        if (matchingIndex - 1 >= 0) {
            if (static_cast<Row*>(parent)->allElements[matchingIndex-1]->type != TypeRow) {
                HTML << "<div class='col-sm-"<<columnSize<<"'>";
            }
        }
    }
    HTML << "<div class='row'>";
    for (int i = 0; i < allElements.size(); i++) {
        HTML << allElements[i]->generateHTMLForParent(this);
    }
    
    HTML << "</div>";
    //if parent is a row and right element isn't a row
    if(parent->type == TypeRow) {
        if (matchingIndex + 1 < static_cast<Row*>(parent)->allElements.size()) {
            if (static_cast<Row*>(parent)->allElements[matchingIndex+1]->type != TypeRow) {
                HTML << "</div>";
            }
        } else {
            //close if no more peers
            HTML << "</div>";

        }
    }
    return HTML.str();
}

void Row::sortElements() {
    for (int i = 0; i < allElements.size(); i++) {
        if (allElements[i]->type == TypeRow) {
            ((Row)allElements[i]).updateFrame();
        }
    }
    updateFrame();
    
    std::sort(allElements.begin(), allElements.end(), compareElements);
    
    for (int i = 0; i < allElements.size(); i++) {
        if (allElements[i]->type == TypeRow) {
            ((Row)allElements[i]).updateFrame();
        }
    }
    updateFrame();
    
}

// This function assumes that the elements have already been sorted
void Row::insertSpacing() {
//    int colWidth = width/12;
//    //First check the space at the front (for top-level stuff)
//    double sp = (allElements[0]->x) / colWidth;
//    if (sp >= 1.0) {
//        //insert a spacing element in that area
//        int ispace = floor(sp);
//        Element *spaceElem = new Element(cv::Rect(0, y, ispace * colWidth, height), Spacing);
//        allElements.insert(allElements.begin(), spaceElem);
//    }
//    for (int i = 0; i < allElements.size()-1; i++) {
//        double space = abs(allElements[i]->x + allElements[i]->width - allElements[i+1]->x) / colWidth;
//        if (space >= 1.0) {
//            //insert a spacing element in that area
//            int ispace = floor(space);
//            Element *spaceElem = new Element(cv::Rect(allElements[i]->x + allElements[i]->width, y, ispace * colWidth, height), Spacing);
//            allElements.insert(allElements.begin() + i + 1, spaceElem);
//            i++;
//        }
//    }
//    
//    //Now check the end of the row
//    sp = abs(allElements.back()->x + allElements.back()->width - (x+width)) /colWidth;
//    if (sp >= 1.0) {
//        //insert a spacing element in that area
//        int ispace = floor(sp);
//        Element *spaceElem = new Element(cv::Rect(0, y, ispace * colWidth, height), Spacing);
//        allElements.insert(allElements.end(), spaceElem);
//    }
}

void Row::consolidateRows() {
    for (int i = 0; i < subRows.size(); i++) {
        subRows[i]->consolidateRows();
    }
    allElements.insert(allElements.end(), subRows.begin(), subRows.end());
}


std::string Page::generateHTML() {
    
    for (int i = 0; i < rows.size(); i++) {
//        rows[i]->allElements.insert(rows[i]->allElements.end(), rows[i]->subRows.begin(), rows[i]->subRows.end());
        rows[i]->consolidateRows();
        rows[i]->sortElements();
        rows[i]->insertSpacing();
    }
    
    std::stringstream HTML;
    HTML << "<!DOCTYPE html>"
    "<html lang='en'>"
    "<head>"
    "<meta charset='utf-8'>"
    "<meta http-equiv='X-UA-Compatible' content='IE=edge'>"
    "<meta name='viewport' content='width=device-width, initial-scale=1'>"
    "<title>Whiteboard2Website</title>"
    "<style>a {font-size: 250%} .row {margin: 5px} body {padding: 10px} </style>"

    "<!-- Latest compiled and minified CSS -->"
    "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.1/css/bootstrap.min.css'>"
    
    "<!-- Optional theme -->"
    "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.1/css/bootstrap-theme.min.css'>"
    
    "<!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->"
    "<script src='https://ajax.googleapis.com/ajax/libs/jquery/1.11.1/jquery.min.js'></script>"
    "<!-- Latest compiled and minified JavaScript -->"
    "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.1/js/bootstrap.min.js'></script>"

     "</head>"
     "<body>";
    
    for (int i = 0; i < rows.size(); i++) {
        HTML << rows[i]->generateHTMLForParent(this);
    }
    HTML << "</body>"
    "</html>";
    return HTML.str();
}