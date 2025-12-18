/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  Tensor         | miniFOAM: The Computational Continuum Playground
   ~~~~~~~   Fields         | Website:  https://tensorfields.com
    O   O                   | Copyright (C) 2025 Tensorfields
      O                     |
-------------------------------------------------------------------------------

Application
    calcCircleArea

Description
    Defite a circle class, create a circle object, and call its area.

Author
    Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin

Date
    Dec 15 2025
\*---------------------------------------------------------------------------*/
#include<iostream>
#include<cmath>

/// Declaration
/// .H (header)
class circle
{
   double d_;

   public:
       double area();
       circle(double diameter);
};

/// Definitions
/// .C (source)
double circle::area()
{
    return 3.14 * pow(d_, 2)/4;
}

circle::circle(double d): d_(d)
{}

/// Application
/// .C

using namespace std;

/// Driver
int main()
{
    circle c1(2);
    cout<< "Circle's area is "
        << c1.area()
        << "\n";
}
