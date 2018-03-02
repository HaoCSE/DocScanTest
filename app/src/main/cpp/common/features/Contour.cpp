//
//  Contour.cpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "Contour.hpp"

using namespace std;
using namespace cv;

Contour::Contour(std::vector<cv::Point> aContour)
{
    this->contour = aContour;
    this->boundingRect = cv::boundingRect(aContour);
    group = NULL;
}

bool Contour::operator==(const Contour& rhs)const {
    if (this->boundingRect ==rhs.boundingRect)
        return true;
    return false;
}

int Contour::proximity(Element* other)
{
  // check for intersect

  Rect expandedRect = this->minimalRect();
  Rect otherExpandedRect= other->minimalRect();
  Rect intersect = (expandedRect & otherExpandedRect);

  if (intersect.area() > 0) {
    return 0;
  }
  
  float overlap =this->verticalOverlap(other);
  
  if(overlap <= 0.0) {
    return INT_MAX;
  }

  
  if(overlap < 0.5) {
    return INT_MAX;
  }
  
  int proximity1 = abs(boundingRect.tl().x - other->boundingRect.br().x);
  int proximity2 = abs(boundingRect.br().x - other->boundingRect.tl().x);
  return std::min(proximity1,proximity2);
  
}
