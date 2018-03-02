//
//  Contour.hpp
//  AbaScan
//
//  Created by Lance Watson on 03/12/2017.
//  Copyright © 2017 Lance Watson. All rights reserved.
//

#ifndef Contour_hpp
#define Contour_hpp

#include <stdio.h>
#include "Element.hpp"

struct Contour : public Element {
    Contour(std::vector<cv::Point>);
    std::vector<cv::Point> contour;
    
    cv::Rect rectWithMinimumWidth(int width);
    bool operator==(const Contour& rhs)const;
    
    int proximity(Element* other);
};

#endif /* Contour_hpp */
