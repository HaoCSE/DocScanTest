//
//  Para.hpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#ifndef Para_hpp
#define Para_hpp

#include <stdio.h>
#include "Line.hpp"

class Para : public Element
{
public: 
  Para();
  ~Para();
  
  std::set<Line*>* lines;
  void insert(Line *line);
  void merge(Para *other);
  bool empty();
  void erase(Line* line);
  void parameterize();
  std::string text();
  
  static bool linesAreClose(Line* line, Line* other, float minProximity);
  static void populateSetWithLines(std::set<Para*>* paras,std::set<Line*>* lines ,float minProximity);
  std::vector<Line*> sortedLines();
};
#endif /* Para_hpp */
