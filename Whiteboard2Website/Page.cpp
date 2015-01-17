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
#define PEER_Y_BUFFER 20

Element::Element(cv::Rect rectangle, elementType inputType): x(rectangle.x), y(rectangle.y), width(rectangle.width), height(rectangle.height), type(inputType) {

}

Row::Row(Element element): keyElement(element) {
    allElements.push_back(element);
    
}

bool Row::shouldContainAsPeer(Element element) {
    //if y's are close enough
    if (abs(keyElement.y - element.y) < PEER_Y_BUFFER) {
        //if sizes are close enough
        if (abs(keyElement.height - element.height) < (keyElement.height > element.height ? PEER_HEIGHT_PERCENT*keyElement.height : PEER_HEIGHT_PERCENT*element.height)) {
            return true;
        }
    }
    return false;
}

bool Row::shouldContainAsChild(Element element) {
    //if y falls within buffer-enhanced range
    //within the height of the key element
    if ((element.y > keyElement.y - PEER_Y_BUFFER) && (element.y < keyElement.y + keyElement.height + PEER_Y_BUFFER)) {
        //if height is smaller
        if (keyElement.height - element.height > PEER_HEIGHT_PERCENT*keyElement.height && keyElement.height > element.height) {
            return true;
        }
    }
    return false;
}

void Row::layout() {
    
}

struct row_comp
{
    inline bool operator() (const Row row1, const Row row2)
    {
        return (row1.keyElement.y < row2.keyElement.y ? -1 : 1);
    }
};

void Row::sortSubrows() {
    std::sort(subRows.begin(), subRows.end(), row_comp());
}

void Page::sortRows() {
    std::sort(rows.begin(), rows.end(), row_comp());

}

void Page::addElementWithinRows(cv::Rect rectangle, elementType type, std::vector<Row> &rowVector) {
    Element element(rectangle, type);
    
    //check if it relates to another row as a child, peer, or parent

    for (int i = 0; i < rowVector.size(); i++) {
        if (rowVector[i].shouldContainAsChild(element)) {
            //add as a child
            //if has subrows, run recursively on them
            if (rowVector[i].subRows.size()) {
                addElementWithinRows(rectangle, type, rowVector[i].subRows);
            } else {
                //else create a new row and add it to the subrows
                rowVector[i].subRows.push_back(Row(element));
            }
            return;
        } else if (rowVector[i].shouldContainAsPeer(element)) {
            //add as a peer
            rowVector[i].allElements.push_back(element);
            
            return;
        }
        
//        } else if (rowVector[i].shouldBeContainedBy(element)) {
//            //should not happen
//            std::cout << "laaaaame";
////            //encapsulate
////            Row newRow(element);
////            //find every row that needs to become a child of the new row
////            for (int j = 0; j < rowVector.size(); j++) {
////                if (rowVector[j].shouldBeContainedBy(element)) {
////                    newRow.subRows.push_back(rowVector[j]);
////                }
////            }
////            
////            return;
//        }
    }
    
    //else make a row for it
    Row newRow(element);
    if (rowVector.size()) {
        //find where in the stack it should go
        //find the row
        for (int rowIndex = 0; rowIndex < rowVector.size() - 1; rowIndex++) {
            if (newRow.keyElement.y < rowVector[rowIndex].keyElement.y) {
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
    
    
//    double columnOfTwelve = 12 * rectangle.x / pageWidth;
//    columnOfTwelve = floor(columnOfTwelve);
    
}

void Page::addAndSortElement(cv::Rect rectangle, elementType type, std::vector<Row> &rowVector) {
    addElementWithinRows(rectangle, type, rowVector);
    sortRows();
}