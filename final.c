#include "CSCIx229.h"
#include "F16.h"

int th=0;
double ph=0;
int fov=55;
int asp=1;
double dim=5.0;
int mode=1;
double distance  =   1;  // Light distance
int inc       =  10;  // Ball increment
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  30;  // Ambient intensity (%)
int diffuse   = 100;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shinyvec[1];    // Shininess (value)
int zh        =  90;  // Light azimuth
unsigned int texture[15];
double Dx = 1;   //  Direction
double Dy = 0;   //  Direction
double Dz = 0;   //  Direction
double Ux = 0;   //  Up
double Uy = 1;   //  Up
double Uz = 0;   //  Up
double Ox = 0;   //  LookAt
double Oy = 0;   //  LookAt
double Oz = 0;   //  LookAt
double Ex = 1.5;   //  Eye
double Ey = 0.3;   //  Eye
double Ez = 1.5;   //  Eye
static GLfloat angle = -90.0;
float PI = 3.14159f;
float rad = 0;
float N[] = {0, -1, 0}; // Normal vector for the plane
float E[] = {0, 0, 0 }; // Point of the plane
int    depth=0;   // Stencil depth
int    light;     // Light mode: true=draw polygon, false=draw shadow volume
float  Position[4];   // Light position
float  Position1[4];   // Light position
#define MAX_PARTICLES 3000
float slowdown = 2.0f;
GLuint loop;
typedef struct
{
    int active;
    float life;
    float fade;
    float r;  //color of particles
    float g;
    float b;
    float x;  //place of particles
    float y;
    float z;
    float xi;  //direction of particles
    float yi;
    float zi;
    float xg;  //acceleration of particles
    float yg;
    float zg;
} particles;
particles particle[MAX_PARTICLES];
int    F16[3];   //  F16 display list
double Yl=2;     //  Elevation of light
double roll=0;   //  Roll angle
double pitch=0;  //  Pitch angle
double yaw=0;    //  Yaw angle
int    pwr=100;  //  Power setting (%)

/*
 *  Initialization of the rain particles
 */
void rain_init()
{
	for(loop=0;loop<MAX_PARTICLES;loop++)
	{
		particle[loop].active = 1;
		particle[loop].life = 1;
		particle[loop].fade = (float)(rand()%100)/1000.0f+0.003f;
		particle[loop].r = 1;
		particle[loop].g = 1;
		particle[loop].b = 1;
		particle[loop].xi = (float)(rand()%30)/100.0;
		particle[loop].yi = (float)(rand()%30)/100.0-1.0f;
		particle[loop].zi = (float)(rand()%30)/100.0;
		particle[loop].xg = 0;
		particle[loop].yg = -2.0f;
		particle[loop].zg = 0;
	}
}

/*
 *  Set the rain particle engine
 */
void rain()
{
	for(loop=0;loop<MAX_PARTICLES;loop++)
	{
		//  set the position
		float x = particle[loop].x;
		float y = particle[loop].y;
		float z = particle[loop].z;
                //  draw the particles and fade them based on life
            	glColor4f(particle[loop].r,particle[loop].g,particle[loop].g,particle[loop].life);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		glVertex3f(x,y,z);
		glVertex3f(x,y-0.1,z);
		glEnd();
		particle[loop].x += particle[loop].xi/(slowdown*1000);  
		//  move on the x Axis by x speed
		particle[loop].y += particle[loop].yi/(slowdown*1000);
		//  move on the y Axis by y speed
		particle[loop].z += particle[loop].zi/(slowdown*1000);  
		//  move on the z Axis by z speed
		particle[loop].xi += particle[loop].xg;
		// Take Pull On X Axis Into Account
		particle[loop].yi += particle[loop].yg;
		// Take Pull On X Axis Into Account
		particle[loop].zi += particle[loop].zg;
		// Take Pull On X Axis Into Account
		particle[loop].life -= particle[loop].fade;
		// Reduce Particles Life By 'Fade'
		if (particle[loop].life<0.0f)			
		// If Particle Is Burned Out
		{
			particle[loop].life = 1.0f;
			particle[loop].fade = (float)(rand()%100)/1000.0+0.003f;	//  random fade value
			particle[loop].x = (float)(rand()%1000)/100.0-5.0f;
			particle[loop].y = (float)(rand()%500)/100.0;
			particle[loop].z = (float)(rand()%1000)/100.0-5.0f;
			particle[loop].xi = (float)(rand()%30)/100.0;
			particle[loop].yi = (float)(rand()%30)/100.0-1.0f;
			particle[loop].zi = (float)(rand()%30)/100.0;
			particle[loop].r = 1;
			particle[loop].g = 1;
			particle[loop].b = 1;
		}
	}
	glLoadIdentity();
}

/*
 *  Draw Set of Facets
 */
static void Facets(int k)
{
   	int i,j;
   	glBegin(GL_TRIANGLES);
   	for (i=nFacet[k];i<nFacet[k+1];i++)
      		for (j=0;j<3;j++)
      		{
         		glTexCoord2fv(Texture[Facet[i][2][j]]);
         		glNormal3fv(Normal[Facet[i][1][j]]);
         		glVertex3fv(Vertex[Facet[i][0][j]]);
      		}
   	glEnd();
}

/*
 *  Compile F16 display list
 */
static void CompileF16(void)
{
   	float black[] = {0,0,0,1};
   	int tex[2];
   	//  Load textures
   	tex[0] = LoadTexBMP("F16s.bmp");
   	tex[1] = LoadTexBMP("F16t.bmp");
   	//  Body list
   	F16[0] = glGenLists(1);
   	glNewList(F16[0],GL_COMPILE);
   	glColor3f(1,1,1);
   	glEnable(GL_TEXTURE_2D);
   	glBindTexture(GL_TEXTURE_2D,tex[0]);
   	Facets(0);
   	glDisable(GL_TEXTURE_2D);
   	glEndList();
   	//  Engine list
   	F16[1] = glGenLists(1);
   	glNewList(F16[1],GL_COMPILE);
   	glColor3f(1,1,1);
   	glEnable(GL_TEXTURE_2D);
   	glBindTexture(GL_TEXTURE_2D,tex[0]);
   	Facets(1);
   	glDisable(GL_TEXTURE_2D);
   	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,black);
   	glEndList();
   	//  Canopy list
   	F16[2] = glGenLists(1);
   	glNewList(F16[2],GL_COMPILE);
   	glColor4f(1,1,1,0.33);
   	glEnable(GL_TEXTURE_2D);
   	glBindTexture(GL_TEXTURE_2D,tex[1]);
   	glEnable(GL_BLEND);
   	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
   	glDepthMask(0);
   	Facets(2);
   	glDepthMask(1);
   	glDisable(GL_BLEND);
   	glDisable(GL_TEXTURE_2D);
   	glEndList();
}	

/*
 *  Draw Flight
 */
static void DrawFlight(double x0,double y0,double z0,
                       double dx,double dy,double dz,
                       double ux,double uy,double uz)
{
   	int i,k;
   	//  Position of members
   	double X[] = {-0.5,+0.5,+0.0,+0.0};
   	double Y[] = {-0.3,+0.3,+0.0,+0.0};
   	double Z[] = {+0.0,+0.0,-0.8,+0.8};
   	//  Unit vector in direction of flght
   	double D0 = sqrt(dx*dx+dy*dy+dz*dz);
   	double X0 = dx/D0;
   	double Y0 = dy/D0;
   	double Z0 = dz/D0;
   	//  Unit vector in "up" direction
   	double D1 = sqrt(ux*ux+uy*uy+uz*uz);
   	double X1 = ux/D1;
   	double Y1 = uy/D1;
   	double Z1 = uz/D1;
   	//  Cross product gives the third vector
   	double X2 = Y0*Z1-Y1*Z0;
   	double Y2 = Z0*X1-Z1*X0;
   	double Z2 = X0*Y1-X1*Y0;
   	//  Rotation matrix
   	double M[16] = {X0,Y0,Z0,0 , X1,Y1,Z1,0 , X2,Y2,Z2,0 , 0,0,0,1};
   	//  Save current transforms
   	glPushMatrix();
   	//  Offset and rotate
   	glTranslated(x0,y0,z0);
   	glMultMatrixd(M);
   	//  k=0  draw body
   	//  k=1  draw engine
   	//  k=2  draw canopy
   	for (k=0;k<3;k++)
   	//  Draw 4 F16s
      	for (i=0;i<4;i++)
      	{
		glPushMatrix();
         	glTranslated(X[i],Y[i],Z[i]);
	 	glScaled(0.3,0.3,0.3);
	 	//glTranslated(3*Position1[0]+X[i],Sin(zh)+3.5+Y[i],3*Position1[2]+Z[i]);
         	glRotated(yaw,0,1,0);
         	glRotated(pitch,0,0,1);
         	glRotated(roll,1,0,0);
         	if (k==1)
	 	{
            		float power[] = {0.01*pwr,0.01*pwr,0.01*pwr,1};
            		glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,power);
	 	}
         	glCallList(F16[k]);
         	glPopMatrix();
      	}
   	//  Undo transformations
   	glPopMatrix();
}

/* 
 * Multiply the current ModelView-Matrix with a shadow-projetion matrix.
 *
 * L is the position of the light source
 * E is a point within the plane on which the shadow is to be projected.  
 * N is the normal vector of the plane.
 *
 * Everything that is drawn after this call is "squashed" down to the plane.
 */
void ShadowProjection(float L[4], float E[4], float N[4])
{
   	float mat[16];
   	float e = E[0]*N[0] + E[1]*N[1] + E[2]*N[2];
   	float l = L[0]*N[0] + L[1]*N[1] + L[2]*N[2];
   	float c = e - l;
   	//  Create the matrix.
   	mat[0] = N[0]*L[0]+c; mat[4] = N[1]*L[0];   mat[8]  = N[2]*L[0];   mat[12] = -e*L[0];
   	mat[1] = N[0]*L[1];   mat[5] = N[1]*L[1]+c; mat[9]  = N[2]*L[1];   mat[13] = -e*L[1];
   	mat[2] = N[0]*L[2];   mat[6] = N[1]*L[2];   mat[10] = N[2]*L[2]+c; mat[14] = -e*L[2];
   	mat[3] = N[0];        mat[7] = N[1];        mat[11] = N[2];        mat[15] = -l;
   	//  Multiply modelview matrix
   	glMultMatrixf(mat);
}

/*
 *  Draw a building enable offset
 *     cebter of bottom at (x,y,z)
 *     dimentions (dx,dy,dz)
 *     rotated th about the y axis
 */
static void building_1(double x,double y,double z,
                       double dx,double dy,double dz,
                       double th)
{
    	//  Set specular color to white
    	float white[] = {1,1,1,1};
    	float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glRotated(th,0,1,0);
    	glScaled(dx,dy,dz);
    	//  Enable textures
    	glEnable(GL_TEXTURE_2D);
    	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	//  Front
    	glColor3f(1,1,1);
    	glBindTexture(GL_TEXTURE_2D,texture[0]);
    	glBegin(GL_QUADS);
    	glNormal3f(0,0,-1);
	glVertex3f(-1,0,1); glTexCoord2f(0,0);
	glVertex3f(+1,0,1); glTexCoord2f(1,0);
	glVertex3f(+1,2,1); glTexCoord2f(1,1);
	glVertex3f(-1,2,1); glTexCoord2f(0,1);
	glEnd();
	//  Back
    	glColor3f(1,1,1);
    	glBindTexture(GL_TEXTURE_2D,texture[0]);
    	glBegin(GL_QUADS);
    	glNormal3f(0,0,1);
	glVertex3f(+1,0,-1); glTexCoord2f(0,0);
	glVertex3f(-1,0,-1); glTexCoord2f(1,0);
	glVertex3f(-1,2,-1); glTexCoord2f(1,1);
	glVertex3f(+1,2,-1); glTexCoord2f(0,1);
	glEnd();
	//  Right
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[1]);
	glBegin(GL_QUADS);
	glNormal3f(-1,0,0);
	glVertex3f(1,0,+1); glTexCoord2f(0,0);
	glVertex3f(1,0,-1); glTexCoord2f(1,0);
	glVertex3f(1,2,-1); glTexCoord2f(1,1);
	glVertex3f(1,2,+1); glTexCoord2f(0,1);
	glEnd();
	//  Left
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[1]);
	glBegin(GL_QUADS);
	glNormal3f(1,0,0);
	glVertex3f(-1,0,-1); glTexCoord2f(0,0);
	glVertex3f(-1,0,+1); glTexCoord2f(1,0);
	glVertex3f(-1,2,+1); glTexCoord2f(1,1);
	glVertex3f(-1,2,-1); glTexCoord2f(0,1);
	glEnd();
	//  Roof
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[2]);
	glBegin(GL_QUADS);
	glNormal3f(0,1,0);
	glVertex3f(-1,2,+1); glTexCoord2f(0,0);
	glVertex3f(+1,2,+1); glTexCoord2f(1,0);
	glVertex3f(+1,2,-1); glTexCoord2f(1,1);
	glVertex3f(-1,2,-1); glTexCoord2f(0,1);
	glEnd();
    	//  Undo transformations and textures
    	glPopMatrix();
    	glDisable(GL_TEXTURE_2D);
}

static void building_2(double x,double y,double z,
                       double dx,double dy,double dz,
                       double th)
{
    	//  Set specular color to white
    	float white[] = {1,1,1,1};
    	float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glRotated(th,0,1,0);
    	glScaled(dx,dy,dz);
    	//  Enable textures
    	glEnable(GL_TEXTURE_2D);
    	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	//  Enable offset
    	glEnable(GL_POLYGON_OFFSET_FILL);
    	glPolygonOffset(1,1);
	//  Front
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[3]);
	glBegin(GL_QUADS);
	glNormal3f(0,0,1);
	glVertex3f(-1,0,1); glTexCoord2f(0,0);
	glVertex3f(+1,0,1); glTexCoord2f(1,0);
	glVertex3f(+1,2,1); glTexCoord2f(1,1);
	glVertex3f(-1,2,1); glTexCoord2f(0,1);
	glEnd();
	//  Back
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[3]);
	glBegin(GL_QUADS);
    	glNormal3f(0,0,-1);
	glVertex3f(-1,0,-1); glTexCoord2f(0,0);
	glVertex3f(+1,0,-1); glTexCoord2f(1,0);
	glVertex3f(+1,2,-1); glTexCoord2f(1,1);
	glVertex3f(-1,2,-1); glTexCoord2f(0,1);
	glEnd();
	//  Right
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[4]);
	glBegin(GL_QUADS);
	glNormal3f(1,0,0);
	glVertex3f(1,0,+1); glTexCoord2f(0,0);
	glVertex3f(1,0,-1); glTexCoord2f(1,0);
	glVertex3f(1,2,-1); glTexCoord2f(1,1);
	glVertex3f(1,2,+1); glTexCoord2f(0,1);
	glEnd();
	//  Left
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[4]);
	glBegin(GL_QUADS);
	glNormal3f(-1,0,0);
	glVertex3f(-1,0,-1); glTexCoord2f(0,0);
	glVertex3f(-1,0,+1); glTexCoord2f(1,0);
	glVertex3f(-1,2,+1); glTexCoord2f(1,1);
	glVertex3f(-1,2,-1); glTexCoord2f(0,1);
	glEnd();
	// Roof
    	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[5]);
	glBegin(GL_QUADS);
	glNormal3f(0,1.732,1);
	glVertex3f(-1,2,1); glTexCoord2f(0,0);
	glVertex3f(+1,2,1); glTexCoord2f(1,0);
	glVertex3f(+1,2.577,0); glTexCoord2f(1,1);
	glVertex3f(-1,2.577,0); glTexCoord2f(0,1);
	glEnd();

    	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[5]);
	glBegin(GL_QUADS);
	glNormal3f(0,1.732,-1);
	glVertex3f(-1,2.577,0); glTexCoord2f(0,0);
	glVertex3f(+1,2.577,0); glTexCoord2f(1,0);
	glVertex3f(+1,2,-1); glTexCoord2f(1,1);
	glVertex3f(-1,2,-1); glTexCoord2f(0,1);
	glEnd();
	//  Roof Left
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[4]);
	glBegin(GL_TRIANGLES);
	glNormal3f(-1,0,0);
	glVertex3f(-1,2,1); glTexCoord2f(0,0);
	glVertex3f(-1,2,-1); glTexCoord2f(1,0);
	glVertex3f(-1,2.577,0); glTexCoord2f(1,1);
	glEnd();
	//  Roof Right
	glColor3f(1,1,1);
	glBindTexture(GL_TEXTURE_2D,texture[4]);
	glBegin(GL_TRIANGLES);
	glNormal3f(1,0,0);
	glVertex3f(1,2,1); glTexCoord2f(0,0);
	glVertex3f(1,2,-1); glTexCoord2f(1,0);
	glVertex3f(1,2.577,0); glTexCoord2f(1,1);
	glEnd();
    	//  Disable offset
	glDisable(GL_POLYGON_OFFSET_FILL);
    	//  Undo transformations and textures
    	glPopMatrix();
    	glDisable(GL_TEXTURE_2D);
}

/*
 * Draw grass, center at x,y,z
 */
static void lawn(double x,double y,double z,
                 double dx,double dy,double dz,
                 double th)
{
    	//  Set specular color to white
    	float white[] = {1,1,1,1};
    	float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glRotated(th,0,1,0);
    	glScaled(dx,dy,dz);
    	//  Enable textures
    	glEnable(GL_TEXTURE_2D);
    	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	//  Enable offset
    	glEnable(GL_POLYGON_OFFSET_FILL);
    	glPolygonOffset(3,1);
    	//  Draw the lawn
    	glColor3f(1,1,1);
    	glBindTexture(GL_TEXTURE_2D,texture[8]);
    	glBegin(GL_QUADS);
    	glNormal3f(0,1,0);
    	glVertex3f(-1,0,+1); glTexCoord2f(0,0);
    	glVertex3f(+1,0,+1); glTexCoord2f(100,0);
    	glVertex3f(+1,0,-1); glTexCoord2f(100,100);
    	glVertex3f(-1,0,-1); glTexCoord2f(0,100);
    	glEnd();
    	//  Disable offset
	glDisable(GL_POLYGON_OFFSET_FILL);
    	//  Undo transformations and textures
    	glPopMatrix();
    	glDisable(GL_TEXTURE_2D);
}

//  Draw road, center at x,y,z
static void road_1(double x,double y,double z,
                   double dx,double dy,double dz,
                   double th)
{
    	//  Set specular color to white
    	float white[] = {1,1,1,1};
    	float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glRotated(th,0,1,0);
    	glScaled(dx,dy,dz);
    	//  Enable textures
    	glEnable(GL_TEXTURE_2D);
    	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	//  Enable offset
    	glEnable(GL_POLYGON_OFFSET_FILL);
    	glPolygonOffset(2,1);
    	//  Draw the lawn
    	glColor3f(1,1,1);
    	glBindTexture(GL_TEXTURE_2D,texture[9]);
    	glBegin(GL_QUADS);
    	glNormal3f(0,1,0);
    	glVertex3f(-1,0,+1); glTexCoord2f(0,0);
    	glVertex3f(+1,0,+1); glTexCoord2f(1,0);
    	glVertex3f(+1,0,-1); glTexCoord2f(1,1);
    	glVertex3f(-1,0,-1); glTexCoord2f(0,1);
    	glEnd();
    	//  Disable offset
	glDisable(GL_POLYGON_OFFSET_FILL);
    	//  Undo transformations and textures
    	glPopMatrix();
    	glDisable(GL_TEXTURE_2D);
}

static void road_2(double x,double y,double z,
                   double dx,double dy,double dz,
                   double th)
{
    	//  Set specular color to white
    	float white[] = {1,1,1,1};
    	float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glRotated(th,0,1,0);
    	glScaled(dx,dy,dz);
    	//  Enable textures
    	glEnable(GL_TEXTURE_2D);
    	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	//  Enable offset
    	glEnable(GL_POLYGON_OFFSET_FILL);
    	glPolygonOffset(1,1);
    	//  Draw the lawn
    	glColor3f(1,1,1);
    	glBindTexture(GL_TEXTURE_2D,texture[10]);
    	glBegin(GL_QUADS);
    	glNormal3f(0,1,0);
    	glVertex3f(-1,0,+1); glTexCoord2f(0,0);
    	glVertex3f(+1,0,+1); glTexCoord2f(1,0);
    	glVertex3f(+1,0,-1); glTexCoord2f(1,1);
    	glVertex3f(-1,0,-1); glTexCoord2f(0,1);
    	glEnd();
    	//  Disable offset
	glDisable(GL_POLYGON_OFFSET_FILL);
    	//  Undo transformations and textures
    	glPopMatrix();
    	glDisable(GL_TEXTURE_2D);
}

static void road_3(double x,double y,double z,
                   double dx,double dy,double dz,
                   double th)
{
    	//  Set specular color to white
    	float white[] = {1,1,1,1};
    	float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,shinyvec);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    	glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glRotated(th,0,1,0);
    	glScaled(dx,dy,dz);
    	//  Enable textures
    	glEnable(GL_TEXTURE_2D);
    	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    	//  Enable offset
    	glEnable(GL_POLYGON_OFFSET_FILL);
    	glPolygonOffset(1,1);
    	//  Draw the lawn
    	glColor3f(1,1,1);
    	glBindTexture(GL_TEXTURE_2D,texture[11]);
    	glBegin(GL_QUADS);
    	glNormal3f(0,1,0);
    	glVertex3f(-1,0,+1); glTexCoord2f(0,0);
    	glVertex3f(+1,0,+1); glTexCoord2f(1,0);
    	glVertex3f(+1,0,-1); glTexCoord2f(1,1);
    	glVertex3f(-1,0,-1); glTexCoord2f(0,1);
    	glEnd();
    	//  Disable offset
	glDisable(GL_POLYGON_OFFSET_FILL);
    	//  Undo transformations and textures
    	glPopMatrix();
    	glDisable(GL_TEXTURE_2D);
}

/*
 *  Draw sky box
 */
static void Sky(double D)
{
    	glColor3f(1,1,1);
    	glEnable(GL_TEXTURE_2D);
    	//  Sides
    	glBindTexture(GL_TEXTURE_2D,texture[12]);
    	glBegin(GL_QUADS);
    	glTexCoord2f(0.00,0); glVertex3f(-D,0,-D);
    	glTexCoord2f(0.25,0); glVertex3f(+D,0,-D);
    	glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);
    	glTexCoord2f(0.00,1); glVertex3f(-D,+D,-D);

    	glTexCoord2f(0.25,0); glVertex3f(+D,0,-D);
    	glTexCoord2f(0.50,0); glVertex3f(+D,0,+D);
    	glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);
    	glTexCoord2f(0.25,1); glVertex3f(+D,+D,-D);

    	glTexCoord2f(0.50,0); glVertex3f(+D,0,+D);
    	glTexCoord2f(0.75,0); glVertex3f(-D,0,+D);
    	glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
    	glTexCoord2f(0.50,1); glVertex3f(+D,+D,+D);

    	glTexCoord2f(0.75,0); glVertex3f(-D,0,+D);
    	glTexCoord2f(1.00,0); glVertex3f(-D,0,-D);
    	glTexCoord2f(1.00,1); glVertex3f(-D,+D,-D);
    	glTexCoord2f(0.75,1); glVertex3f(-D,+D,+D);
    	glEnd();
    	//  Top
    	glBindTexture(GL_TEXTURE_2D,texture[13]);
    	glBegin(GL_QUADS);
    	glTexCoord2f(0.0,0); glVertex3f(+D,+D,-D);
    	glTexCoord2f(0.5,0); glVertex3f(+D,+D,+D);
    	glTexCoord2f(0.5,1); glVertex3f(-D,+D,+D);
    	glTexCoord2f(0.0,1); glVertex3f(-D,+D,-D);
    	glEnd();

    	glDisable(GL_TEXTURE_2D);
}

static void ball(double x,double y,double z,double r,
                 double cr,double cg,double cb)
{
    	//  Save transformation
    	glPushMatrix();
    	//  Offset, scale and rotate
    	glTranslated(x,y,z);
    	glScaled(r,r,r);
    	glColor3f(cr,cg,cb);
    	glutSolidSphere(1.0,16,16);
    	//  Undo transofrmations
    	glPopMatrix();
}

/*
 *  Draw a tree
 *     at (x,y,z)
 *     radius r
 *     color cr,cg,cb
 */
static void tree(double x,double y,double z,double r)
{
    	glPushMatrix();
    	glTranslated(x,y,z);
    	glScaled(r,r,r);
    	glColor3f(0,1,0);
    	glutSolidSphere(1.0,16,16);
	glPopMatrix();
}

void shadow_object()
{
    	building_1(0,0,0,    0.5,1,0.5,    0);
    	building_2(0,0,-3,    0.5,0.167,0.25,     0);
    	building_2(3,0,0,    0.5,0.167,0.25,     90);
    	building_2(0,0,3,   0.5,0.167,0.25,     0);
    	building_2(-3,0,0,   0.5,0.167,0.25,     270);
    	building_2(0,0,-4,    0.5,0.167,0.25,     180);
    	building_2(4,0,0,    0.5,0.167,0.25,     90);
    	building_2(0,0,4,   0.5,0.167,0.25,     0);
    	building_2(-4,0,0,   0.5,0.167,0.25,     270);
    	building_2(3.5,0,3.5,    0.5,0.167,0.25,     45);
    	building_2(3.5,0,-3.5,    0.5,0.167,0.25,     135);
    	building_2(-3.5,0,-3.5,   0.5,0.167,0.25,     -135);
    	building_2(-3.5,0,3.5,   0.5,0.167,0.25,     -45);
    	tree(0.85,0.167,-2.5,  0.167);
    	tree(-0.85,0.167,-2.5,  0.167);
    	tree(0.85,0.167,2.5,  0.167);
    	tree(-0.85,0.167,2.5,  0.167);
    	tree(-2.5,0.167,-0.85,  0.167);
    	tree(-2.5,0.167,0.85,  0.167);
    	tree(2.5,0.167,-0.85,  0.167);
    	tree(2.5,0.167,0.85,  0.167);
}

/*
 *  OpenGL (GLUT) calls this routine to display the scenedistance*Sin(zh)
 */
void display()
{
    	//  Erase the window and the depth buffer
    	glClearColor(0,0,0,0);
    	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    	//  Enable Z-buffering in OpenGL
    	glEnable(GL_DEPTH_TEST);
    	rad = PI*angle/180.0;
	//  Set the first person view feature
    	Ux = 0, Uy = cos(ph), Uz = 0;
    	Ox = Ex+1000*cos(rad);
    	Oz = Ex+1000*sin(rad);
    	Oy = Ey+1000*sin(ph);
	//  Set the collisiton detection to ensure people can only walk on the road
	if (Ey > 4.8)	Ey = 4.8;
	if (Ey < 0.3)	Ey = 0.3;
	if (Ex > 4.8)	Ex = 4.8;
	if (Ex <-4.8)	Ex =-4.8;
	if (Ez > 4.8)	Ez = 4.8;
	if (Ez <-4.8)	Ez =-4.8;
	if (Ez >-1.15 && Ez < 1.15 && Ex >-1.15 && Ex < 1.15)
	{
		if(Ex < Ez && Ex > -Ez) 	Ez = 1.15;
		else if(Ex < Ez && Ex <-Ez)	Ex =-1.15;
		else if(Ex > Ez && Ex <-Ez)	Ez =-1.15;
		else if(Ex > Ez && Ex >-Ez)	Ex = 1.15;
	}
	if(Ex > 2.15 && Ez > 2.15)
	{
		if(Ex > Ez)	Ez = 2.15;
		if(Ex < Ez)	Ex = 2.15;
	}
	if(Ez > 2.15 && Ex >-1.15 && Ex < 1.15)
	{
		if(Ex >-Ez+3.3 && Ex > 0)		Ex = 1.15;
		else if(Ex > Ez-3.3 && Ex <-Ez+3.3)	Ez = 2.15;
		else if(Ex < Ez-3.3 && Ex < 0)		Ex =-1.15;
	}
	if(Ex <-2.15 && Ez > 2.15)
	{
		if(Ex >-Ez)	Ex =-2.15;
		if(Ex <-Ez)	Ez = 2.15;
	}
	if(Ex <-2.15 && Ez >-1.15 && Ez < 1.15)
	{
		if(Ex < Ez-3.3 && Ez > 0)		Ez = 1.15;
		else if(Ex > Ez-3.3 && Ex >-Ez-3.3)	Ex =-2.15;
		else if(Ex <-Ez-3.3 && Ez < 0)		Ez =-1.15;
	}
	if(Ex <-2.15 && Ez <-2.15)
	{
		if(Ex > Ez)	Ex =-2.15;
		if(Ex < Ez)	Ez =-2.15;
	}
	if(Ez <-2.15 && Ex >-1.15 && Ex < 1.15)
	{
		if(Ex > Ez+3.3 && Ex > 0)		Ex = 1.15;
		else if(Ex >-Ez-3.3 && Ex < Ez+3.3)	Ez =-2.15;
		else if(Ex <-Ez-3.3 && Ex < 0)		Ex =-1.15;
	}
	if(Ex > 2.15 && Ez <-2.15)
	{
		if(Ex >-Ez)	Ez =-2.15;
		if(Ex <-Ez)	Ex = 2.15;
	}
	if(Ex > 2.15 && Ez >-1.15 && Ez < 1.15)
	{
		if(Ex >-Ez+3.3 && Ez > 0)		Ez = 1.15;
		else if(Ex <-Ez+3.3 && Ex < Ez+3.3)	Ex = 2.15;
		else if(Ex > Ez+3.3 && Ez < 0)		Ez =-1.15;
	}
    	glLoadIdentity();
    	gluLookAt(Ex,Ey,Ez , Ox,Oy,Oz , Ux,Uy,Uz);
    	//  Draw sky
    	Sky(dim);
        //  Translate intensity to color vectors
        float Ambient[]   = {0.001*ambient ,0.001*ambient ,0.001*ambient ,1.0};
        float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
        //  Light position
        float Position[]  = {distance*Cos(zh),3.7,distance*Sin(zh),1.0};
        float Position1[]  = {-distance*Cos(zh),3.7,-distance*Sin(zh),1.0};
        //  Draw light position as ball (still no lighting here)
        ball(Position[0],Position[1],Position[2] , 0.1 , 1,1,1);

        glEnable(GL_NORMALIZE);
        glEnable(GL_LIGHTING);
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        //  Enable light 0
        glEnable(GL_LIGHT0);
        //  Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position1);
    	shadow_object();
    	lawn(0,0,0, 5,1,5,  0);
    	road_1(0,0,-1.65,    5,1,0.5,   0);
    	road_1(1.65,0,0,    5,1,0.5,   90);
    	road_1(0,0,1.65,   5,1,0.5,   180);
    	road_1(-1.65,0,0,   5,1,0.5,   -90);
    	road_2(0,0,-0.75,   0.4,1,0.4,    0);
    	road_2(0.75,0,0,    0.4,1,0.4,    90);
    	road_2(0,0,0.75,    0.4,1,0.4,    180);
    	road_2(-0.75,0,0,   0.4,1,0.4,    -90);
    	road_3(1.65,0,-1.65,    0.5,1,0.5,    0);
    	road_3(1.65,0,1.65,     0.5,1,0.5,    0);
    	road_3(-1.65,0,1.65,    0.5,1,0.5,    0);
    	road_3(-1.65,0,-1.65,   0.5,1,0.5,    0);
   	//  Draw flight of F16s
   	DrawFlight(3*Position1[0],3.5,3*Position1[2] , Sin(zh),Dy,-Cos(zh) , Ux,Uy,Uz);
   	//  Save what is glEnabled
   	glPushAttrib(GL_ENABLE_BIT);
   	//  Draw shadow
        glDisable(GL_LIGHTING);
        //  Enable stencil operations
        glEnable(GL_STENCIL_TEST);
        //  Existing value of stencil buffer doesn't matter
        glStencilFunc(GL_ALWAYS,1,0xFFFFFFFF);
        //  Set the value to 1 (REF=1 in StencilFunc)
        //  only if Z-buffer would allow write
        glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
        //  Make Z-buffer and color buffer read-only
        glDepthMask(0);
        glColorMask(0,0,0,0);
        //  Draw flattened scene
        glPushMatrix();
        ShadowProjection(Position,E,N);
	shadow_object();
        glPopMatrix();
        //  Make Z-buffer and color buffer read-write
        glDepthMask(1);
        glColorMask(1,1,1,1);
        //  Set the stencil test draw where stencil buffer is > 0
        glStencilFunc(GL_LESS,0,0xFFFFFFFF);
        //  Make the stencil buffer read-only
        glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
        //  Enable blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0,0,0,0.5);
        //  Draw the shadow over the entire floor
        glBegin(GL_QUADS);
        glVertex3f(-5,0,-5);
        glVertex3f(+5,0,-5);
        glVertex3f(+5,0,+5);
        glVertex3f(-5,0,+5);
        glEnd();
   	//  Undo glEnables
   	glPopAttrib();
	rain();      
    	//  Display parameters
    	glWindowPos2i(5,5);
    	Print("X=%.1f  Y=%.1f Z=%.1f",Ex,Ey,Ez);
    	//  Render the scene and make it visible
    	ErrCheck("display");
    	glFlush();
    	glutSwapBuffers();
}
		
void idle()
{
	double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
	zh = fmod(90*t,360);
	glutPostRedisplay();
}

void special(int key,int x,int y)
{
	if (key == GLUT_KEY_LEFT)
	{
		Ex += sin(PI*angle/180.0)*0.1;
		Ez -= cos(PI*angle/180.0)*0.1;
	}
	else if (key == GLUT_KEY_RIGHT)
	{		
		Ex -= sin(PI*angle/180.0)*0.1;
		Ez += cos(PI*angle/180.0)*0.1;
	}
	else if (key == GLUT_KEY_UP)
	{
		Ex += cos(PI*angle/180.0)*0.1;
		Ez += sin(PI*angle/180.0)*0.1;
	}
	else if (key == GLUT_KEY_DOWN)
	{		
		Ex -= cos(PI*angle/180.0)*0.1;
		Ez -= sin(PI*angle/180.0)*0.1;
	}
	th %= 360;

	Project(mode?fov:0,asp,dim);
	glutPostRedisplay();
}

void key(unsigned char ch,int x,int y)
{
	if(ch == 27)
		exit(0);
	else if(ch == 'w')
		ph += 0.1;
	else if(ch == 's')
		ph -= 0.1;
	else if(ch == 'a')
		angle -= 5.0;
	else if(ch == 'd')
		angle += 5.0;
	else if(ch == ' ')
		Ey += 0.1;
	else if(ch == 'c')
		Ey -= 0.1;

	Project(mode?fov:0,asp,dim);
	glutPostRedisplay();
}

void reshape(int width,int height)
{
   	asp = (height>0) ? (double)width/height : 1;
   	glViewport(0,0, width,height);

   	Project(mode?fov:0,asp,dim);
}

int main(int argc,char* argv[])
{
	glutInit(&argc,argv);
	glutInitWindowSize(600,600);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE | GLUT_STENCIL);
	glutCreateWindow("City_Yisha Wu");

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(key);
   	//  Load and Compile G16
   	CompileF16();
   	//  Check stencil depth
   	glGetIntegerv(GL_STENCIL_BITS,&depth);
   	if (depth<=0) Fatal("No stencil buffer\n");
	//load textures
	texture[0] = LoadTexBMP("building_1_front_back.bmp");
	texture[1] = LoadTexBMP("building_1_right_left.bmp");
	texture[2] = LoadTexBMP("building_1_roof.bmp");
	texture[3] = LoadTexBMP("building_2_front_back.bmp");
	texture[4] = LoadTexBMP("building_2_right_left.bmp");
	texture[5] = LoadTexBMP("building_2_roof.bmp");
	texture[8] = LoadTexBMP("grass.bmp");
	texture[9] = LoadTexBMP("road_1.bmp");
	texture[10] = LoadTexBMP("road_2.bmp");
	texture[11] = LoadTexBMP("road_3.bmp");
	texture[12] = LoadTexBMP("sky0.bmp");
	texture[13] = LoadTexBMP("sky1.bmp");

   	ErrCheck("init");
	rain_init();
	glutMainLoop();
	return 0;
}
