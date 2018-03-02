//
//  Line.hpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#ifndef Line_hpp
#define Line_hpp

#include <stdio.h>
#include "Group.hpp"

class Para;

class Line : public Element
{
public: 
  Line();
  ~Line();
  
  std::set<Group*>* groups;
  Para* para;
  
  void insert(Group *group);
  void merge(Line *other);
  bool empty();
  void erase(Group* group);
  void parameterize();
  
  static bool groupsAreAligned(Group* group, Group* other, float minProximity);
  static void populateSetWithAlignedGroups(std::set<Line*>* lines,std::set<Group*>* groups ,float minProximity);
  
  std::string text();
  std::vector<Group*> sortedGroups();
  
};

#endif /* Line_hpp */
