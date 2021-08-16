#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray. 
*/
float Cylinder ::intersect(glm::vec3 p0, glm::vec3 dir)
{
    float a = pow(dir.x, 2)+ pow(dir.z, 2) ;
    float b = 2 * ((p0.x - center.x)*dir.x + (p0.z - center.z)*dir.z);
    float c = pow((p0.x - center.x), 2) + pow((p0.z - center.z), 2) - pow(radius, 2);
    
    float delta = pow(b,2) - 4*a*c;

    if(fabs(delta) < 0.001) return -1.0;
    if(delta < 0.0) return -1.0;

    float t1 = (-b - sqrt(delta)) / (2*a);
    float t2 = (-b + sqrt(delta)) / (2*a);
    
    if (t2 > t1) {
      t2 = t1;
      t1 = t2;
    }

    if ((p0.y+t2*dir.y >= center.y) && (p0.y+t2*dir.y <= center.y + height)) {
        return t2;
    } else  {
        if ((p0.y+t1*dir.y >= center.y) && (p0.y+t1*dir.y <= center.y + height)) {
            return t1;
        } else {
           return -1;
        }
    }
}

/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the sphere.
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
	glm::vec3 n = glm::vec3 (p.x - center.x, 0, p.z - center.z);
    n = glm::normalize(n);
    return n;
}
