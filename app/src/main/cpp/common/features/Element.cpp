//
//  Element.cpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "Element.hpp"

using namespace cv;

cv::Rect Element::minimalRect()
{
    
    int width = std::max(minimumDimension.x, boundingRect.width);
    
    return cv::Rect(boundingRect.x + boundingRect.width/2 - width/2, boundingRect.y, width, boundingRect.height);
}

cv::Rect Element::horizontalRect()
{
    
    int width = INT_MAX;
    
    return cv::Rect(- width/2, boundingRect.y, width, boundingRect.height);
}

float Element::verticalOverlap(Element* other)
{
  // check for intersect
  
  Rect expandedRect = this->horizontalRect();
  Rect otherExpandedRect= other->horizontalRect();
  Rect intersect = (expandedRect & otherExpandedRect);
  
  int minHeight = std::min(expandedRect.height,otherExpandedRect.height);
  
  if (minHeight <= 0) {
    return 0;
  }
 
  return ((double)intersect.height)/((double)minHeight);
}

int Element::maxDimension()
{
    return  std::max(boundingRect.width, boundingRect.height);
}
