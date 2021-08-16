#include "Cone.h"
#include <math.h>

/**
* Cone's intersection method.  The input is a ray. 
*/
float Cone ::intersect(glm::vec3 p0, glm::vec3 dir)
{
	
	float a = pow(dir.x, 2)+pow(dir.z,2)-pow((radius/height)*dir.y, 2);
    float b = 2*(dir.x*(p0.x-center.x)+dir.z*(p0.z-center.z)-(pow(radius/height,2.0)*dir.y*(p0.y - height  - center.y)));
    float c = pow(p0.x-center.x,2)+pow(p0.z-center.z,2)-pow(radius/height,2.0)*(pow(p0.y-center.y,2.0)+height*(height-2*p0.y+2*center.y));

    float delta = pow(b,2) - 4*a*c;
    
    if(fabs(delta) < 0.0001 || delta<0.0) return -1.0;
    
    float t1= (-b - sqrt(delta)) / (2*a);
    float t2= (-b + sqrt(delta)) / (2*a);
    
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
glm::vec3 Cone::normal(glm::vec3 p)
{
    float theta = radius/height;
    float alpha = glm::atan((p.x-center.x)/(p.z-center.z));
    glm::vec3 n = glm::vec3(glm::sin(alpha)*glm::cos(theta), glm::sin(theta),glm::cos(alpha)*glm::cos(theta));
    n = glm::normalize(n);
    return n;
}
