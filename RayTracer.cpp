/*==================================================================================
* COSC 363  Computer Graphics (2021)
* Department of Computer Science and Software Engineering, University of Canterbury.
*
* A basic ray tracer
* See Lab07.pdf  for details.
*===================================================================================
*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "Sphere.h"
#include "SceneObject.h"
#include "Ray.h"
#include "Plane.h"
#include "Cone.h"
#include "Cylinder.h"
#include "TextureBMP.h"
#include <GL/freeglut.h>
using namespace std;

const float WIDTH = 20.0;  
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

vector<SceneObject*> sceneObjects;
TextureBMP texture;
TextureBMP texture1;

GLuint fogMode[]={GL_LINEAR, GL_EXP, GL_EXP2}; 
GLuint fog=0;   
bool bFog = true;   


//---The most important function in a ray tracer! ---------------------------------- 
//   Computes the colour value obtained by tracing a ray and finding its 
//     closest point of intersection with objects in the scene.
//----------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
	glm::vec3 backgroundCol(0);						//Background colour = (0,0,0)
	glm::vec3 lightPos(90, 90, -3);					//Light's position
	glm::vec3 lightPos2(-90, 90, -3);					//Light's position
	glm::vec3 color(0);
	SceneObject* obj;

    ray.closestPt(sceneObjects);					//Compare the ray with all objects in the scene
    if(ray.index == -1) return backgroundCol;		//no intersection
	obj = sceneObjects[ray.index];					//object on which the closest point of intersection is found
	
	if(ray.index == 3)  //sun
    {
		glm::vec3 centre(10, 10.0, -75);
		glm::vec3 d = glm::normalize(ray.hit-centre);
		float u = atan2(d.x, d.z) / (2*M_PI) + 0.5;
		float v = 0.5 - asin(d.y) / M_PI;
		color=texture1.getColorAt(u, v);
		obj->setColor(color);
    }
	
	if (ray.index == 4) //floor
	{
		 //Stripe pattern
		 int stripeWidth = 5;
		 int iz = (ray.hit.z+200) / stripeWidth;
		 int ix = (ray.hit.x+200) / stripeWidth;
		 int kz = iz % 2; //2 colors
		 int kx = ix % 2; //2 colors
		 if ((kx == 0 && kz ==0) || (kx == 1 && kz ==1) ) color = glm::vec3(0.9,1.0,0.9);
		 else color = glm::vec3(1,0.68,0.73);
		 obj->setColor(color);
	}
	if(ray.index == 5)  //background
    {
		float texcoords = (ray.hit.x+70)/(140);
		float texcoordt = (ray.hit.y+20)/(70);
		color=texture.getColorAt(texcoords, texcoordt);
		obj->setColor(color);
    }
    if(ray.index == 12 || ray.index == 13 || ray.index == 14) //procedural pattern
    {
        if ((int(ray.hit.x + ray.hit.y) % 3 == 0)){
            color = glm::vec3(1,0.73,0.13);
        }
        else if((int(ray.hit.x) % 2 == 0)){
            color = glm::vec3(0.46,0.93,0.77);
        }
        else{
            color = glm::vec3(1,1,1);
        }
        obj->setColor(color);
    }
	
	color = obj->lighting(lightPos,-ray.dir,ray.hit);						//Object's colour
	
	glm::vec3 lightVec = lightPos - ray.hit;
	glm::vec3 lightVec2 = lightPos2 - ray.hit;
	Ray shadowRay(ray.hit, lightVec); 
	Ray shadowRay2(ray.hit, lightVec2); 
	
	float lightDist = glm::length(lightVec);
	float lightDist2 = glm::length(lightVec2);
	//shadow
	lightVec = glm::normalize(lightVec);
	lightVec2 = glm::normalize(lightVec2);
	glm::vec3 normalVec = sceneObjects[ray.index]->normal(ray.hit);
	normalVec = glm::normalize(normalVec);
	float ln = glm::dot(lightVec, normalVec);
	glm::vec3 reflVec = glm::reflect(-lightVec, normalVec);
	glm::vec3 reflVec2 = glm::reflect(-lightVec2, normalVec);
	reflVec = glm::normalize(reflVec);
	reflVec2 = glm::normalize(reflVec2);
	float rv = glm::dot(reflVec, -ray.dir);
	float rv2 = glm::dot(reflVec2, -ray.dir);
	float specularTerm;
	float specularTerm2;
    float f = 30.0;
	if(rv < 0) {
        specularTerm = 0;
    }else{
        specularTerm = pow(rv, f);
    }
    if(rv2 < 0) {
        specularTerm2 = 0;
    }else{
        specularTerm2 = pow(rv2, f);
    }
	shadowRay.closestPt(sceneObjects);
	shadowRay2.closestPt(sceneObjects);
	if(shadowRay.index > -1 && shadowRay.dist < lightDist) {
		color = 0.1f*obj->getColor();
		if (shadowRay.index == 1 || shadowRay.index == 2){
            color += obj->getColor()*0.2f;
        } 
	} else {
		color = 0.1f*obj->getColor() + (ln * obj->getColor() + specularTerm)*0.6f;
	}
	if(shadowRay2.index > -1 && shadowRay2.dist < lightDist2) {
		color += 0.1f*obj->getColor();
		if (shadowRay2.index == 1 || shadowRay2.index == 2){
            color += obj->getColor()*0.2f;
        } 
	} else {
		color  += 0.1f*obj->getColor() + (ln*obj->getColor() + specularTerm2)*0.6f ;
	}

	
	if (obj->isReflective() && step < MAX_STEPS)
	{
		float rho = obj->getReflectionCoeff();
		glm::vec3 normalVec = obj->normal(ray.hit);
		glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVec);
		Ray reflectedRay(ray.hit, reflectedDir);
		glm::vec3 reflectedColor = trace(reflectedRay, step + 1);
		color = color + (rho * reflectedColor);
	}
	if (ray.index == 2) //refraction
	{
		float coeff_refraction = 0.4;
        float eta = 1/1.03;
        glm::vec3 n = sceneObjects[ray.index]->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, n, eta);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        if(refrRay.index == -1) return backgroundCol;
        glm::vec3 m = sceneObjects[refrRay.index]->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta);
        Ray refrRay2(refrRay.hit, h);
        refrRay2.closestPt(sceneObjects);
        if(refrRay2.index == -1) return backgroundCol;

        glm::vec3 refractionColor = trace(refrRay2, step+1);
        color = color * coeff_refraction + refractionColor*(1 - coeff_refraction);

		 obj->setColor(color);
	}
	
	 if (ray.index == 1) //transparent
	{
		float coeff_transparency = 0.4;
        float eta = 1/1.0;
        glm::vec3 n = obj->normal(ray.hit);
        glm::vec3 g = glm::refract(ray.dir, n, eta);
        Ray refrRay(ray.hit, g);
        refrRay.closestPt(sceneObjects);
        if(refrRay.index == -1) return backgroundCol;
        glm::vec3 m = sceneObjects[refrRay.index]->normal(refrRay.hit);
        glm::vec3 h = glm::refract(g, -m, 1.0f/eta);
        Ray refrRay2(refrRay.hit, h);
        refrRay2.closestPt(sceneObjects);
        if(refrRay2.index == -1) return backgroundCol;

        glm::vec3 transparentColor = trace(refrRay2, step+1);
        color = color * coeff_transparency + transparentColor*(1 - coeff_transparency);

		obj->setColor(color);
	}
    

	return color;
}

//---The main display module -----------------------------------------------------------
// In a ray tracing application, it just displays the ray traced image by drawing
// each cell as a quad.
//---------------------------------------------------------------------------------------
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float xp, yp;  //grid point
	float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
	float cellY = (YMAX-YMIN)/NUMDIV;  //cell height
	glm::vec3 eye(0., 0., 0.);

	glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	glEnable(GL_LIGHT2);
    float spotdir[] = {5.0,-10.0,-80.0};
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 10.0);
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 2.0);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spotdir);

	glBegin(GL_QUADS);  //Each cell is a tiny quad.

	for(int i = 0; i < NUMDIV; i++)	//Scan every cell of the image plane
	{
		xp = XMIN + i*cellX;
		for(int j = 0; j < NUMDIV; j++)
		{
			yp = YMIN + j*cellY;

		    glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray

		    //Ray ray = Ray(eye, dir);
		    
		    //anti-alisaing
		    glm::vec3 dir1(xp+0.25*cellX, yp+0.25*cellX, -EDIST);
			glm::vec3 dir2(xp+0.75*cellX, yp+0.75*cellX, -EDIST);
			glm::vec3 dir3(xp+0.75*cellX, yp+0.25*cellX, -EDIST);
			glm::vec3 dir4(xp+0.25*cellX, yp+0.75*cellX, -EDIST);
			Ray ray1 = Ray(eye, dir1);
			Ray ray2 = Ray(eye, dir2);
			Ray ray3 = Ray(eye, dir3);
			Ray ray4 = Ray(eye, dir4);
			
			ray1.normal();
			ray2.normal();
			ray3.normal();
			ray4.normal();
		    
		    glm::vec3 col = 0.25f* trace (ray1, 1) + 0.25f* trace (ray2, 1) + 0.25f* trace (ray3, 1) + 0.25f* trace (ray4, 1);

		    //glm::vec3 col = trace (ray, 1); //Trace the primary ray and get the colour value

			glColor3f(col.r, col.g, col.b);
			glVertex2f(xp, yp);				//Draw each cell with its color value
			glVertex2f(xp+cellX, yp);
			glVertex2f(xp+cellX, yp+cellY);
			glVertex2f(xp, yp+cellY);
        }
    }

    glEnd();
    glFlush();
}

//---This function initializes the scene ------------------------------------------- 
//   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
//     and add them to the list of scene objects.
//   It also initializes the OpenGL orthographc projection matrix for drawing the
//     the ray traced image.
//----------------------------------------------------------------------------------
void initialize()
{
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);;
	glEnable(GL_LIGHT2);

    //glClearColor(0, 0, 0, 1);
    glClearColor(0.5f,0.5f,0.5f,1.0f);

	Sphere *sphere1 = new Sphere(glm::vec3(-12.5, 6.5, -90.0), 9.0);//0
	sphere1->setColor(glm::vec3(0.0,0.0,1.0));   //Set colour to blue
	sceneObjects.push_back(sphere1);		 //Add sphere to scene objects
	//sphere1->setSpecularity(false);
	//sphere1->setShininess(5); 
	sphere1->setReflectivity(true, 0.8);
	
	Sphere *sphere2 = new Sphere(glm::vec3(6, -10.0, -65.0), 4.0);//1
	sphere2->setColor(glm::vec3(1, 0, 0));   //Set colour
	sceneObjects.push_back(sphere2);		 //Add sphere to scene objects
	//sphere2->setReflectivity(true, 0.8);
	
	Sphere *sphere3 = new Sphere(glm::vec3(-1.5, -11.0, -93.0), 4.0);//2 refra
	sphere3->setColor(glm::vec3(0, 1, 0));   //Set colour
	sceneObjects.push_back(sphere3);		 //Add sphere to scene objects
	
	Sphere *sphere4= new Sphere(glm::vec3(10, 10.0, -75), 3.0);//3
	sphere4->setColor(glm::vec3(0, 1, 1));   //Set colour
	sceneObjects.push_back(sphere4);		 //Add sphere to scene objects
	texture1 = TextureBMP("sun.bmp");
	
	Plane *plane = new Plane (glm::vec3(-60., -15, -40), //Point A 4
	glm::vec3(60., -15, -40), //Point B
	glm::vec3(60., -15, -200), //Point C
	glm::vec3(-60., -15, -200)); //Point D
	plane->setColor(glm::vec3(0.8, 0.8, 0));
	sceneObjects.push_back(plane);
	plane->setSpecularity(false);
	 
	Plane *background = new Plane (glm::vec3(-70., -15, -200), //Point A 5
	glm::vec3(70., -15, -200), //Point B
	glm::vec3(70., 60, -200), //Point C
	glm::vec3(-70., 60, -200)); //Point D
	background->setColor(glm::vec3(0, 0, 0));
	sceneObjects.push_back(background);
	background->setSpecularity(false);
	texture = TextureBMP("bg.bmp");
	
    glm::vec3 p1 = glm::vec3(5,-15,-80);
    glm::vec3 p2 = glm::vec3(10,-15,-80);
    glm::vec3 p3 = glm::vec3(10,-10,-80);
    glm::vec3 p4 = glm::vec3(5,-10,-80);
    glm::vec3 p5 = glm::vec3(10,-15,-85);
    glm::vec3 p6 = glm::vec3(10,-10,-85);
    glm::vec3 p7 = glm::vec3(5,-10,-85);
    glm::vec3 p8 = glm::vec3(5,-15,-85);

    Plane *plane1 = new Plane(p1, p2, p3, p4);//6
    Plane *plane2 = new Plane(p4, p3, p6, p7);//7
    Plane *plane3 = new Plane(p8, p5, p2, p1);//8
    Plane *plane4 = new Plane(p4, p7, p8, p1);//9
    Plane *plane5 = new Plane(p2, p5, p6 ,p3);//10
    Plane *plane6 = new Plane(p5, p8, p7, p6);//11
    
    
    plane1->setColor(glm::vec3(1, 0, 0));
    plane2->setColor(glm::vec3(1, 0, 0));
    plane3->setColor(glm::vec3(1, 0, 0));
    plane4->setColor(glm::vec3(1, 0, 0));
    plane5->setColor(glm::vec3(1, 0, 0));
    plane6->setColor(glm::vec3(1, 0, 0));

    sceneObjects.push_back(plane1);
    sceneObjects.push_back(plane2);
    sceneObjects.push_back(plane3);
    sceneObjects.push_back(plane4);
    sceneObjects.push_back(plane5);
    sceneObjects.push_back(plane6);
    
    Cone *cone1 = new Cone(glm::vec3(-10,-8,-70), 4, 6);//12
	cone1->setColor(glm::vec3(0, 1, 1));   //Set colour
	sceneObjects.push_back(cone1);
	Cone *cone2 = new Cone(glm::vec3(-10,-10,-70), 4, 6);//13
	cone2->setColor(glm::vec3(0, 1, 1));   //Set colour
	sceneObjects.push_back(cone2);
	
	Cylinder *cylinder = new Cylinder(glm::vec3(-10,-16,-70), 1.5, 10);//14
	cylinder->setColor(glm::vec3(0, 1, 1));   //Set colour
	sceneObjects.push_back(cylinder);
	
	//glEnable(GL_FOG); 
	glHint(GL_FOG_HINT, GL_NICEST);
    float fogColor[4] = {1,1,1,1.0f};     
    glFogi(GL_FOG_MODE, GL_LINEAR);    
    glFogfv(GL_FOG_COLOR, fogColor); 
    glFogf(GL_FOG_DENSITY, 0.5f); 
    glHint(GL_FOG_HINT, GL_DONT_CARE); 
    glFogf(GL_FOG_START, -50.0f);
    glFogf(GL_FOG_END, 20.0f); 
}


int main(int argc, char *argv[]) {
	//glEnable(GL_FOG);
	glHint(GL_FOG_HINT, GL_NICEST);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracing");

    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
