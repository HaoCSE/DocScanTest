//
//  Line.cpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#include "Line.hpp"

using namespace std;
using namespace cv;


Line::Line() {
  this->groups = new std::set<Group*>();
  para = NULL;
}

Line::~Line() {
  delete this->groups;
}

void Line::insert(Group *group) {
  this->groups->insert(group);
  group->line = this;
}

void Line::merge(Line *other) {
  for (auto otherGroup : *other->groups)
  {
    otherGroup->line = this;
  }
  this->groups->insert( other->groups->begin(), other->groups->end() );
}

bool Line::empty()
{
  return this->groups->empty();
}

void Line::erase(Group* group)
{
  this->groups->erase(group);
}

bool Line::groupsAreAligned(Group* group, Group* other, float minProximity)
{
  float proximity = other->verticalOverlap(group);
  if (proximity < minProximity)
    return false;
  
    if ((group->boundingRect.height == 0) || (other->boundingRect.height == 0))
      return false;
  
  float heightRatio = ((float)group->boundingRect.height)/((float)other->boundingRect.height);
  
  if (heightRatio < 1.0f)
    heightRatio = 1/heightRatio;
  
  return heightRatio < 2;
  
}

void Line::populateSetWithAlignedGroups(set<Line*>* lines,set<Group*>* groups ,float minProximity){
  for (auto &group : *(groups)) {
    for (auto &other : *(groups)) {
      if(groupsAreAligned(group,other,minProximity))
      {
        if (group->line == NULL)
        {
          if(other->line == NULL)
          {
            Line* line = new Line();
            lines->insert(line);
            line->insert(group);
            line->insert(other);
          }
          else  // if(other->line != NULL)
          {
            other->line->insert(group);
          }
        }
        else
        {
          if(other->line == NULL)
          {
            group->line->insert(other);
          }
          else if (other->line != group->line)
          {
            Line* otherLine = other->line;
            group->line->merge(otherLine);
            lines->erase(otherLine);
            delete otherLine;
          }
        }
      }
    }
    if (group->line == NULL)
    {
      Line* line = new Line();
      lines->insert(line);
      line->insert(group);
    }
  }
  for (auto & line : *lines)
  {
    line->parameterize();
  }
}

void Line::parameterize()
{
  this->boundingRect = cv::Rect(0,0,-1,-1);
  for (auto group : *this->groups)
  {
    
    if(this->boundingRect.empty())
    {
      this->boundingRect = group->boundingRect;
    }
    else {
      this->boundingRect |=  group->boundingRect;
    }
  }
}

std::vector<Group*> Line::sortedGroups() {
  std::vector<Group*> _sortedGroups = std::vector<Group*>( this->groups->begin(), this->groups->end() );

  
  std::sort(_sortedGroups.begin(), _sortedGroups.end(), [](Group* a, Group* b) {
    return (a->boundingRect.tl().x < b->boundingRect.tl().x);
  });
  return _sortedGroups;
}

std::string Line::text() {
  
  std::string textString = std::string("");
  
  bool first = true;
  
  for (auto group : sortedGroups())
  {
    if(!first)
      textString.append(" ");

    textString.append(group->text);
    
    first = false;

  }
  return textString;
}

