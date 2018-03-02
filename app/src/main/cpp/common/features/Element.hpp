//
//  Element.hpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#ifndef Element_hpp
#define Element_hpp

//#include <opencv2.h>
#include <opencv2/opencv.hpp>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>


class Group;

struct Element {
  cv::Rect boundingRect;
  cv::Rect minimalRect();
  cv::Rect horizontalRect();
  cv::Point minimumDimension;
  int maxDimension();
  double baseLinePolinomial[5];
  virtual int proximity(Element* other){ return 0;};
  float verticalOverlap(Element* other);
  virtual void parameterize(){};
  Group* group;
};

#endif /* Element_hpp */
