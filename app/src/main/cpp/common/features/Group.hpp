//
//  Group.hpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#ifndef Group_hpp
#define Group_hpp

#include <stdio.h>
#include "Element.hpp"
#include <stdlib.h>
#include <opencv2/opencv.hpp>

class Line;

struct Group : public Element
{
  Group();
  ~Group();
  std::set<Element*>* elements;
  int minProximity;
  int maxElementHeight;
  int maxElementWidth;


  bool empty();
  void insert(Element *element);
  void merge(Group *other);
  void erase(Element* element);
  cv::Mat* sourceImage;
  cv::Point baselineForX(float x);
  cv::Vec4f baseline;
  cv::Point baselineStart;
  cv::Point baselineEnd;
  static bool elementsAreGrouped(Element* element, Element* other, int minProximity);
  static void populateSetWithGroupedElements(std::set<Group*>* groups,std::set<Element*>* elements,int minProximity);
  void parameterize();
  void splitPairedElements();

  std::string text;
  Line* line;
};

#endif /* Group_hpp */
