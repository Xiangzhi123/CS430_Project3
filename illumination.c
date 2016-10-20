#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "newParser.c"


// create a stuct that represents a single pixel, same as what we did in class
typedef struct PPMRGBpixel {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} PPMRGBpixel;

// create struct that represents a single image
typedef struct PPMimage {
	int width;
	int height;
	int maxColorValue;
	unsigned char *data;
} PPMimage;


// return the square value of v
static inline double sqr(double v) {
  	return v*v;
}

// normalize the vector to a 3d unit vector
static inline void normalize(double* v) {
 	double len = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
	v[0] /= len;
	v[1] /= len;
  v[2] /= len;
}


// this function writes the body data from buffer->data to output file
int PPMDataWrite(char ppmVersionNum, FILE *outputFile, PPMimage* buffer) {
	// write image data to the file if the ppm version is P6
	if (ppmVersionNum == '6') {
		// using fwrite to write data, basically it just like copy and paste data for P6
		fwrite(buffer->data, sizeof(PPMRGBpixel), buffer->width*buffer->height, outputFile);
		printf("The file saved successfully! \n");
		return (0);
	}
	// write image data to the file if the ppm version is p3
	else if (ppmVersionNum == '3') {
		int i, j;
		for (i = 0; i < buffer->height; i++) {
			for (j = 0; j < buffer->width; j++) {
				// similar thing as we did in reading body data for P3, but we use fprintf here to write data.
				fprintf(outputFile, "%d %d %d ", buffer->data[i*buffer->width*3+j*3], buffer->data[i*buffer->width*3+j*3+1], buffer->data[i*buffer->width*3+2]);
			}
			fprintf(outputFile, "\n");
		}

		printf("The file saved successfully! \n");
		return (0);
	}
	else {
		fprintf(stderr, "Error: incorrect ppm version. \n");
		return (1);
	}
}

// this function writes the header data from buffer to output file
int PPMWrite(char *outPPMVersion, char *outputFilename, PPMimage* buffer) {
	int width = buffer->width;
	int height = buffer->height;
	int maxColorValue = buffer->maxColorValue;
	char ppmVersionNum = outPPMVersion[1];
	FILE *fh = fopen(outputFilename, "wb");
	if (fh == NULL) {
		fprintf(stderr, "Error: open the file unscuccessfully. \n");
		return (1);
	}
	char *comment = "# output.ppm";
	fprintf(fh, "P%c\n%s\n%d %d\n%d\n", ppmVersionNum, comment, width, height, 255);
	// call the PPMDataWrite function which writes the body data
	PPMDataWrite(ppmVersionNum, fh, buffer);
	fclose(fh);
}


double sphereIntersection(double* Ro, double* Rd, double* Center, double r) {
  	// x = Rox + Rdx*t
  	// y = Roy + Rdy*t
  	// z = Roz + Rdz*t
  	// (x - Centerx)^2 + (y - Centery)^2 + (z - Centerz)^2 = r^2
  	//
  	// Then, (Rox + Rdx*t - Centerx)^2 + (Roy + Rdy*t - Centery)^2
  	// + (Roz + Rdz*t - Centery)^2 = r^2
  	//
  	// Then, Rox^2 + Rdx^2*t^2 + Centerx^2 + 2*Rox*Rdx*t - 2*Rox*Centerx - 2*Rdx*t*Centerx
  	// + Roy^2 + Rdy^2*t^2 + Centery^2 + 2*Roy*Rdy*t - 2*Roy*Centery - 2*Rdy*t*Centery
  	// + Roz^2 + Rdz^2*t^2 + Centerz^2 + 2*Roz*Rdz*t - 2*Roz*Centerz - 2*Rdz*t*Centerz
  	//
  	// Then, t^2(Rdx^2 + Rdy^2 + Rdz^2) +
  	// t*(2*Rox*Rdx + 2*TRoz*Rdz + 2*Roy*Rdy - 2*Rdx*Centerx - 2*Rdy*Centery - 2*Rdz*Centerz) +
  	// Rox^2 + Centerx^2 + Roy^2 + Centery^2 + Roz^2 + Centerz^2 -
  	// 2*Rox*Centerx - 2*Roy*Centery - 2*Roz*Centerz
  	// - r^2 = 0
  	double a = sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]);
  	double b = 2*(Ro[0]*Rd[0] + Ro[1]*Rd[1] + Ro[2]*Rd[2] - Rd[0]*Center[0] - Rd[1]*Center[1] - Rd[2]*Center[2]);
  	double c = sqr(Ro[0]) + sqr(Ro[1]) + sqr(Ro[2]) + sqr(Center[0]) +
             sqr(Center[1]) + sqr(Center[2]) - 2*(Ro[0]*Center[0]
             + Ro[1]*Center[1] + Ro[2]*Center[2]) - sqr(r);
  	double det = sqr(b) - 4*a*c;
  	if (det < 0) return -1;
  	det = sqrt(det);
  	double t0 = (-b - det) / (2*a);
 	double t1 = (-b + det) / (2*a);
	if (t0 > 0) return t0;
 	if (t1 > 0) return t1;

  	return -1;
}

double planeIntersection(double* Ro, double* Rd, double* position, double* normal) {
  	// A(X0 + Xd * t) + B(Y0 + Yd * t) + (Z0 + Zd * t) + D = 0
  	// A(Rox + Rdx*t) + B(Roy + Rdy*t) + C(Roz + Rdz*t) + D = 0
  	// it could also be written as:
  	// A(Rox + Rdx*t - positionx) + B(Roy + Rdy*t - postiony) + C(Roz + Rdz*t - positionz) = 0
  	// t(A*Rdx + B*Rdy + C*Rdz) + (A*Rox + B*Roy + C*Roz - A*positionx - B*positiony - C*postionz) = 0
  	// t = - (A*Rox + B*Roy + C*Roz - A*positionx - B*positiony - C*positionz) / (A*Rdx + B*Rdy + C*Rdz)
  	double t = - (normal[0]*Ro[0] + normal[1]*Ro[1] + normal[2]*Ro[2] - normal[0]*position[0]
                - normal[1]*position[1] - normal[2]*position[2]) / (normal[0]*Rd[0]
                + normal[1]*Rd[1] + normal[2]*Rd[2]);

  	if (t > 0) return t;
	return -1;
}

double* intersect(double* Rd, int objectNum, Object** objects) {
	int closestObjectNum = -1;
	double bestT = INFINITY;
	int i;
	double t;
	double Ro[3] = { 0, 0, 0 };
	for (i = 0; i<objectNum; i++){
		if (objects[i]->kind == 1) {
			t = sphereIntersection(Ro, Rd, objects[i]->sphere.position, objects[i]->sphere.radius);
			if (t) {
				if (t > 0 && t <= bestT) {
					bestT = t;
					closestObjectNum = i;
				}
			}
			else {
				fprintf(stderr, "Error: finding the distance unsuccessfully.\n");
				exit(1);
			}
		}
		else if (objects[i]->kind == 2) {
			t = planeIntersection(Ro, Rd, objects[i]->plane.position, objects[i]->plane.normal);
			if (t) {
				if (t > 0 && t <= bestT) {
					bestT = t;
					closestObjectNum = i;
				}
			}
			else {
				fprintf(stderr, "Error: finding the distance unsuccessfully.\n");
				exit(1);
			}
		}
	}
  int result[2];
  result[0] = (double) closestObjectNum;
  result[1] = bestT;
	return result;
}

double frad(int lightIndex, double* intersectPosition, Object** objects){
	double lightPostion[3];
	double radial[3];
	double distance;
	double result;
	lightPostion[0] = objects[lightIndex]->light.position[0];
	lightPostion[1] = objects[lightIndex]->light.position[1];
	lightPostion[2] = objects[lightIndex]->light.position[2];
	distance = sqr(lightPostion[0]-intersectPosition[0]) + sqr(lightPostion[1]-intersectPosition[1]) +
						sqr(lightPostion[2]-intersectPosition[2]);
	distance = sqrt(distance);
	radial[0] = objects[lightIndex]->light.radialA0;
	radial[1] = objects[lightIndex]->light.radialA1;
	radial[2] = objects[lightIndex]->light.radialA2;
	if (distance == INFINITY){
		return 1.0;
	}
	else {
		result = 1 / (radial[2]*sqr(distance) + radial[1]*distance + radial[0]);
		return result;
	}
}

double fang(int lightIndex, double* intersectPosition, Object** objects){
	double Vl[3];
	double Vo[3];
	double angular;
	double theta;
	Vl[0] = objects[lightIndex]->light.direction[0];
	Vl[1] = objects[lightIndex]->light.direction[1];
	Vl[2] = objects[lightIndex]->light.direction[2];
	Vl = normalize(Vl);
	// Vo = intersectPosition - lightPostion
	Vo[0] = intersectPosition[0] - objects[lightIndex]->light.position[0];
	Vo[1] = intersectPosition[1] - objects[lightIndex]->light.position[1];
	Vo[2] = intersectPosition[2] - objects[lightIndex]->light.position[2];
	Vo = normalize(Vo);

	double cosa = Vl[0]*Vo[0]+Vl[1]*Vo[1]+Vl[2]*Vo[2];
	if (objects[lightIndex]->light.angularA0 != 0){
		angular = objects[lightIndex]->light.angularA0;
	}
	else {
		return 1.0;
	}
	theta = objects[lightIndex]->light.theta;
	if (cos(theta) > cosa){
		return 0;
	}
	else {
		return cosa^angualar;
	}
}

double* diffuse(int objectIndex, int lightIndex, double* N, double* L, Object** objects){
	double value = N[0]*L[0]+N[1]*L[1]+N[2]*L[2]; // N*L
	double result[3];
	if (value <= 0){
		result[0] = 0;
		result[1] = 0;
		result[2] = 0;
	}
	else {
		if (objects[objectIndex]->kind == 1){
			double KI[3];
			double NL;
			KI[0] = objects[objectIndex]->sphere.diffuseColor[0]*objects[lightIndex]->light.color[0];
			KI[1] = objects[objectIndex]->sphere.diffuseColor[1]*objects[lightIndex]->light.color[1];
			KI[2] = objects[objectIndex]->sphere.diffuseColor[2]*objects[lightIndex]->light.color[2];
			NL = value;
			result[0] = KI[0]*value;
			result[1] = KI[1]*value;
			result[2] = KI[2]*value;
		}
		else if (objects[objectIndex]->kind == 2){
			double KI[3];
			double NL;
			KI[0] = objects[objectIndex]->plane.diffuseColor[0]*objects[lightIndex]->light.color[0];
			KI[1] = objects[objectIndex]->plane.diffuseColor[1]*objects[lightIndex]->light.color[1];
			KI[2] = objects[objectIndex]->plane.diffuseColor[2]*objects[lightIndex]->light.color[2];
			NL = value;
			result[0] = KI[0]*value;
			result[1] = KI[1]*value;
			result[2] = KI[2]*value;
		}
	}
	return result;
}

double* specular(int objectIndex, int lightIndex, double NL, double* V, double* R, Object** objects){
	double value = V[0]*R[0]+V[1]*R[1]+V[2]*R[2];
	double result[3];
	if (NL <= 0 || value <= 0){
		result[0] = 0;
		result[1] = 0;
		result[2] = 0;
	}
	else {
		if (objects[objectIndex]->kind == 1){
			double KI[3];
			double VR;
			KI[0] = objects[objectIndex]->sphere.specularColor[0]*objects[lightIndex]->light.color[0];
			KI[1] = objects[objectIndex]->sphere.specularColor[1]*objects[lightIndex]->light.color[1];
			KI[2] = objects[objectIndex]->sphere.specularColor[2]*objects[lightIndex]->light.color[2];
			VR = value;
			result[0] = KI[0]*pow(VR, 20);  // I set up the ns to 20
			result[1] = KI[1]*pow(VR, 20);
			result[2] = KI[2]*pow(VR, 20);
		}
		else if (objects[objectIndex]->kind == 2){
			double KI[3];
			double VR;
			KI[0] = objects[objectIndex]->plane.specularColor[0]*objects[lightIndex]->light.color[0];
			KI[1] = objects[objectIndex]->plane.specularColor[1]*objects[lightIndex]->light.color[1];
			KI[2] = objects[objectIndex]->plane.specularColor[2]*objects[lightIndex]->light.color[2];
			VR = value;
			result[0] = KI[0]*pow(VR, 20);  // I set up the ns to 20
			result[1] = KI[1]*pow(VR, 20);
			result[2] = KI[2]*pow(VR, 20);
		}
	}
	return result;
}

double clamp(double num){
	if (num <= 0){
		return 0;
	}
	else if (num >= 1){
		return 1;
	}
	else {
		return num;
	}
}

PPMimage* rayCasting(char* filename, int w, int h, Object** objects) {
	PPMimage* buffer = (PPMimage*)malloc(sizeof(PPMimage));
	if (objects[0] == NULL) {
		fprintf(stderr, "Error: no object found");
		exit(1);
	}
	int cameraFound = 0;
	int lightCount = 0;
	double width;
	double height;
	int i;
	for (i = 0; objects[i] != 0; i += 1) {
		if (objects[i]->kind == 0) {
			cameraFound = 1;
			width = objects[i]->camera.width;
			height = objects[i]->camera.height;
			if (width <= 0 || height <= 0) {
				fprintf(stderr, "Error: invalid size for camera");
				exit(1);
			}
		}
		else if (objects[i]->kind == 3){
			lightCount += 1;
		}
	}
	if (cameraFound == 0) {
		fprintf(stderr, "Error: Camera is not found");
		exit(1);
	}

	buffer->data = (unsigned char*)malloc(w*h * sizeof(PPMRGBpixel));
	PPMRGBpixel *pixel = (PPMRGBpixel*)malloc(sizeof(PPMRGBpixel));
	if (buffer->data == NULL || buffer == NULL) {
		fprintf(stderr, "Error: allocate the memory un successfully. \n");
		exit(1);
	}

	double pixwidth = width / w;
	double pixheight = height / h;
	double pointx, pointy, pointz;
	int j, k;
  double Ro[3] = {0, 0, 0};
	for (k = 0; k<h; k++) {
		int count = (h-k-1)*w*3;
		double vy = -height / 2 + pixheight * (k + 0.5);
		for (j = 0; j<h; j++) {
			double vx = -width / 2 + pixwidth * (j + 0.5);
			double Rd[3] = { vx, vy, 1 };

			normalize(Rd);
			double intersect[2];
      intersect = intersect(Rd, i, objects);
      intersection = (int)intersect[0];
      bestT = intersect[1];
			if (intersection>=0) {
				double Ron[3];
				double N[3];
				double V[3];
				V[0] = -Rd[0];
				V[1] = -Rd[1];
				V[2] = -Rd[2];
				Ron[0] = bestT*Rd[0] + Ro[0];
				Ron[1] = bestT*Rd[1] + Ro[1];
				Ron[2] = bestT*Rd[2] + Ro[2];
				if (objects[intersection]->kind == 1){
					N[0] = Ron[0] - objects[intersection]->sphere.position[0];
					N[1] = Ron[1] - objects[intersection]->sphere.position[1];
					N[2] = Ron[2] - objects[intersection]->sphere.position[2];
				}
				else if (objects[intersection]->kind == 2){
					N[0] = objects[intersection]->plane.normal[0];
					N[1] = objects[intersection]->plane.normal[1];
					N[2] = objects[intersection]->plane.normal[2];
				}
				double intersectPosition[3];
				intersectPosition[0] = Ro[0]+Rd[0]*bestT;
				intersectPosition[1] = Ro[1]+Rd[1]*bestT;
				intersectPosition[2] = Ro[2]+Rd[2]*bestT;
				for (int w=0; w<lightCount; w++){
        	double L[3];
        	double R[3];
					double Rdn[3]; // Rdn = light position - Ron;
					for (int z = 0; objects[z] != 0; z++){
						if (objects[z]->kind == 3){
							Rdn[0] = objects[z]->light.position[0] - Ron[0];
							Rdn[1] = objects[z]->light.position[1] - Ron[1];
							Rdn[2] = objects[z]->light.position[2] - Ron[2];
							L = Rdn;
							R[0] = 2*N[0]*L[0]*N[0] - L[0]; // R=(2N*L)N - L
							R[1] = 2*N[1]*L[1]*N[1] - L[1];
							R[2] = 2*N[2]*L[2]*N[2] - L[2];
							double diff[3];
							double spec[3];
							double NL = N[0]*L[0]+N[1]*L[1]+N[2]*L[2];
							double frad = frad(z, intersectPosition, objects);
							double fang = fang(z, intersectPosition, objects);
							diff = diffuse(intersection, z, N, L, objects);
							sepc = specular(intersection, z, NL, V, R, objects);
							pixel->r += frad*fang*(diff[0]+spec[0]);
							pixle->g += frad*fang*(diff[1]+spec[1]);
							pixel->b += frad*fang*(diff[2]+spec[2]);
						}
					}
				}
			}
			else {
				pixel->r = 0;
				pixel->g = 0;
				pixel->b = 0;
			}
			buffer->data[count++] = 255 * clamp(pixel->r);
			buffer->data[count++] = 255 * clamp(pixel->g);
			buffer->data[count++] = 255 * clamp(pixel->b);
			}
		}
	return buffer;
}


int main(int argc, char **argv) {
	if (argc != 5) {
		fprintf(stderr, "Error: incorrect format('raycast width height input.json output.ppm')");
		return (1);
	}
	char *w = argv[1];
	char *h = argv[2];
	char *inputFilename = argv[3];
	char *outputFilename = argv[4];

	Object** objects = malloc(sizeof(Object*) * 128);
	int width = atoi(w);
	int height = atoi(h);
	if (width <= 0) {
		fprintf(stderr, "Error: Invalid width input!");
		return (1);
	}
	if (height <= 0) {
		fprintf(stderr, "Error: Invalid height input!");
		return (1);
	}
	readScene(inputFilename, objects);
	PPMimage* buffer = rayCasting(inputFilename, width, height, objects);
	buffer->width = width;
	buffer->height = height;
	PPMWrite("P3", outputFilename, buffer);
	return (0);
}
