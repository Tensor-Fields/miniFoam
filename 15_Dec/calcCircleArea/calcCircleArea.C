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
