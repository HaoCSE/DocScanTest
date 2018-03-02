//
//  Para.cpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "Para.hpp"
using namespace std;
using namespace cv;


Para::Para() {
  this->lines = new std::set<Line*>();
}

Para::~Para() {
  delete this->lines;
}

void Para::insert(Line *line) {
  this->lines->insert(line);
  line->para = this;
}

void Para::merge(Para *other) {
  for (auto otherLine : *other->lines)
  {
    otherLine->para = this;
  }
  this->lines->insert( other->lines->begin(), other->lines->end() );
}

bool Para::empty()
{
  return this->lines->empty();
}

void Para::erase(Line* line)
{
  this->lines->erase(line);
}

bool Para::linesAreClose(Line* line, Line* other, float minProximity)
{
    
  if ((line->boundingRect.height == 0) || (other->boundingRect.height == 0))
    return false;
  
  float heightRatio = ((float)line->boundingRect.height)/((float)other->boundingRect.height);
  
  if (heightRatio < 1.0f)
    heightRatio = 1/heightRatio;
  
  if (heightRatio > 1.8)
    return false;
  
  Rect intersect = (line->horizontalRect() & other->horizontalRect());
  
  if (intersect.area() > 0) {
    return true;
  }
  
  float distance = std::min(abs(line->boundingRect.tl().y - other->boundingRect.br().y), abs(line->boundingRect.br().y - other->boundingRect.tl().y) );
  
  float maxHeight = std::min(line->boundingRect.height,other->boundingRect.height);
  
  return (distance/maxHeight) < minProximity;
}

void Para::populateSetWithLines(set<Para*>* paras,set<Line*>* lines ,float minProximity)
{
  for (auto &line : *(lines)) {
    
    for (auto &other : *(lines)) {
      if(linesAreClose(line,other,minProximity))
      {
        if (line->para == NULL)
        {
          if(other->para == NULL)
          {
            Para* para = new Para();
            paras->insert(para);
            para->insert(line);
            para->insert(other);
          }
          else  // if(other->para != NULL)
          {
            other->para->insert(line);
          }
        }
        else
        {
          if(other->para == NULL)
          {
            line->para->insert(other);
          }
          else if (other->para != line->para)
          {
            Para* otherPara = other->para;
            line->para->merge(otherPara);
            paras->erase(otherPara);
            delete otherPara;
          }
        }
      }
    }
    if (line->para == NULL)
    {
      Para* para = new Para();
      paras->insert(para);
      para->insert(line);
    }
  }
  for (auto & para : *paras)
  {
    para->parameterize();
  }
}

void Para::parameterize()
{
  this->boundingRect = cv::Rect(0,0,-1,-1);
  for (auto line : *this->lines)
  {
    
    if(this->boundingRect.empty())
    {
      this->boundingRect = line->boundingRect;
    }
    else {
      this->boundingRect |=  line->boundingRect;
    }
  }
}


std::vector<Line*> Para::sortedLines() {
  std::vector<Line*> _sortedLines = std::vector<Line*>( lines->begin(), lines->end() );
  
  std::sort(_sortedLines.begin(), _sortedLines.end(), [](Line* a, Line* b) {
    return (a->boundingRect.tl().y < b->boundingRect.tl().y);
  });
  return _sortedLines;
}

std::string Para::text() {
  std::vector<Line *> sortedLines =  this->sortedLines();
  
  std::string textString = std::string("");
  
  bool first = true;
  
  for (auto line : sortedLines)
  {
    if(!first)
      textString.append("\n\r");
    
    textString.append(line->text());
    
    first = false;
    
  }
  return textString;
}


