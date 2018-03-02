//
//  Group.cpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "Group.hpp"


using namespace std;
using namespace cv;


Group::Group()
{
  this->elements = new std::set<Element*>();
  line = NULL;
  text = string("");
}

Group::~Group()
{
  delete this->elements;
}

void Group::insert(Element *element)
{
  this->elements->insert(element);
  element->group = this;
}

void Group::merge(Group *other)
{
  for (auto otherContour : *other->elements)
  {
    otherContour->group = this;
  }
  this->elements->insert( other->elements->begin(), other->elements->end() );
}

void Group::splitPairedElements()
{
  vector<int> widths = vector<int>();
  for (auto element : *this->elements)
  {
    widths.push_back(element->boundingRect.width);
  }
  sort(widths.begin(),widths.end());
  int medianWidth = widths[widths.size()/2];
  for (auto element : *this->elements)
  {
    while (element->boundingRect.width > (float)medianWidth*1.8 )
    {
      Element* newElement = new Element();
      this->elements->insert(newElement);
      newElement->boundingRect = element->boundingRect;
      newElement->boundingRect.width = medianWidth;
      element->boundingRect.width -= medianWidth;
      element->boundingRect.x += medianWidth;
    };
  }
}


void Group::parameterize()
{
  maxElementHeight = 0;
  maxElementWidth = 0;

  //splitPairedElements();
  
  vector<cv::Point> baselinePoints =  vector<cv::Point>();
  this->boundingRect = cv::Rect(0,0,-1,-1);
  for (auto element : *this->elements)
  {
    maxElementHeight = std::max(maxElementHeight,this->boundingRect.height);
    maxElementWidth = std::max(maxElementWidth,this->boundingRect.width);

    baselinePoints.push_back(element->boundingRect.br());
    if(this->boundingRect.empty())
    {
      this->boundingRect = element->minimalRect();
    }
    else {
      this->boundingRect |= element->minimalRect();
    }
  }
  
  cv::fitLine(baselinePoints,baseline,CV_DIST_HUBER,0,0.01,0.01);
  baselineStart = baselineForX(boundingRect.tl().x);
  baselineEnd = baselineForX(boundingRect.br().x);
  
  int maxHeight = 1;
  
  for (auto element : *this->elements)
  {
    int centerX = element->boundingRect.x + element->boundingRect.width/2;
    cv::Point bottom = baselineForX(centerX);
    int height = bottom.y - element->boundingRect.y + 1;
    maxHeight = max(maxHeight,height);
    element->boundingRect.height = max(1,height);
    element->boundingRect.y = max(0, element->boundingRect.y-1);
    element->boundingRect.x = max(0, element->boundingRect.x-2);
    element->boundingRect.width = element->boundingRect.width+4;
  }
  
  for (auto element : *this->elements)
  {
    element->boundingRect.y =  element->boundingRect.y + element->boundingRect.height - maxHeight;
    element->boundingRect.height = maxHeight;
  }
}

cv::Point Group::baselineForX(float x)
{
 // Y - Y0 = M (X - X0)
  
  float Y0 = baseline[3];
  float X0 = baseline[2];
  float m = baseline[1]/baseline[0];
  float y = m*(x - X0)+Y0;
  cv::Point point = cv::Point((int)x, (int)y+4);
  return point;
}

bool Group::empty()
{
  return this->elements->empty();
}

void Group::erase(Element* element)
{
  this->elements->erase(element);
}

bool Group::elementsAreGrouped(Element* element, Element* other, int minProximity)
{
  int proximity = other->proximity(element);
  return (proximity <= minProximity); // || (trailingDistance < minProximity))
}

void Group::populateSetWithGroupedElements(set<Group*>* groups,set<Element*>* elements ,int minProximity){
  for (auto &element : *(elements)) {
    
    for (auto &other : *(elements)) {
      if(elementsAreGrouped(element,other,minProximity))
      {
        if (element->group == NULL)
        {
          if(other->group == NULL)
          {
            Group* group = new Group();
            groups->insert(group);
            group->insert(element);
            group->insert(other);
          }
          else  // if(other->group != NULL)
          {
            other->group->insert(element);
          }
        }
        else
        {
          if(other->group == NULL)
          {
            element->group->insert(other);
          }
          else if (other->group != element->group)
          {
            Group* otherGroup = other->group;
            element->group->merge(otherGroup);
            groups->erase(otherGroup);
            delete otherGroup;
          }
        }
      }
    }
    if (element->group == NULL)
    {
      Group* group = new Group();
      groups->insert(group);
      group->insert(element);
    }
  }
  for (auto & group : *groups)
  {
    group->parameterize();
  }
}

