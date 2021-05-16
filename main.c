#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gl.h>
#include <glu.h>
#include <glut.h>

typedef struct _Vector3D {
    float x, y, z;
} Vector3D;

typedef struct _Face {
    int a, b, c;
} Face;

int numberVertices;
Vector3D *verticesArray;

int numberFaces;
Face *facesArray;

Vector3D *normalsToFaceArray;
Vector3D *normalsToVertexArray;

float angleX = 0.0;
float angleY = 0.0;
float angleZ = 0.0;

int visualizationType = 0;

void loadObject(char *filename) {
    FILE *file;
    char line[255], *str;
    int vertexCounter = 0, faceCounter = 0;

    file = fopen(filename, "r");
    numberVertices = 0;
    numberFaces = 0;
    while (!feof(file)) {
        fgets(line, 255, file);
        if (strncmp(line, "v ", 2) == 0)
            numberVertices++;
        if (line[0] == 'f')
            numberFaces++;
    }

    fseek(file, 0, SEEK_SET);
    if ((numberVertices > 0) && (numberFaces > 0)) {
        verticesArray = calloc(numberVertices, sizeof(Vector3D));
        facesArray = calloc(numberFaces, sizeof(Face));
        while (!feof(file)) {
            fgets(line, 255, file);
            if (strncmp(line, "v ", 2) == 0) {
                str = strtok(line, " ");
                verticesArray[vertexCounter].x = atof(strtok(NULL, " "));
                verticesArray[vertexCounter].y = atof(strtok(NULL, " "));
                verticesArray[vertexCounter].z = atof(strtok(NULL, " "));
                vertexCounter++;
            }
            if (line[0] == 'f') {
                str = strtok(line, " ");
                facesArray[faceCounter].a = atoi(strtok(NULL, " ")) - 1;
                facesArray[faceCounter].b = atoi(strtok(NULL, " ")) - 1;
                facesArray[faceCounter].c = atoi(strtok(NULL, " ")) - 1;
                faceCounter++;
            }
            strcpy(line, "");
        }
    }
    fclose(file);
}

void normalizeVertices(double _MAX, double _MIN) {
    int i, j;
    float x, y, z;
    float xm, ym, zm;
    float xmax, ymax, zmax;
    float xmin, ymin, zmin;
    float _xmax, _ymax, _zmax;
    float _xmin, _ymin, _zmin;

    xmax = -1000000000.0;
    xmin = 1000000000.0;
    ymax = -1000000000.0;
    ymin = 1000000000.0;
    zmax = -1000000000.0;
    zmin = 1000000000.0;

    for (i = 0; i < numberVertices; i++) {
        x = verticesArray[i].x;
        y = verticesArray[i].y;
        z = verticesArray[i].z;

        if (x > xmax) xmax = x;
        if (y > ymax) ymax = y;
        if (z > zmax) zmax = z;
        if (x < xmin) xmin = x;
        if (y < ymin) ymin = y;
        if (z < zmin) zmin = z;
    }

    if (xmax == xmin) {
        xmax += 0.001;
        xmin -= 0.001;
    }
    if (ymax == ymin) {
        ymax += 0.001;
        ymin -= 0.001;
    }
    if (zmax == zmin) {
        zmax += 0.001;
        zmin -= 0.001;
    }

    xm = xmax - xmin;
    ym = ymax - ymin;
    zm = zmax - zmin;

    xm /= 2.0;
    ym /= 2.0;
    zm /= 2.0;

    for (i = 0; i < numberVertices; i++) {
        verticesArray[i].x = (verticesArray[i].x - xmax) + xm;
        verticesArray[i].y = (verticesArray[i].y - ymax) + ym;
        verticesArray[i].z = (verticesArray[i].z - zmax) + zm;
    }

    xmin = -xm;
    xmax = xm;
    ymin = -ym;
    ymax = ym;
    zmin = -zm;
    zmax = zm;

    _xmax = _MAX;
    _xmin = _MIN;
    _ymax = (_MAX * ymax) / xmax;
    _ymin = (_MIN * ymin) / xmin;
    _zmax = (_MAX * zmax) / xmax;
    _zmin = (_MIN * zmin) / xmin;

    if ((ym > xm) && (ym > zm)) {
        _xmax = (_MAX * xmax) / ymax;
        _xmin = (_MIN * xmin) / ymin;
        _ymax = _MAX;
        _ymin = _MIN;
        _zmax = (_MAX * zmax) / ymax;
        _zmin = (_MIN * zmin) / ymin;
    }

    if ((zm > xm) && (zm > ym)) {
        _xmax = (_MAX * xmax) / zmax;
        _xmin = (_MIN * xmin) / zmin;
        _ymax = (_MAX * ymax) / zmax;
        _ymin = (_MIN * ymin) / zmin;
        _zmax = _MAX;
        _zmin = _MIN;
    }

    for (i = 0; i < numberVertices; i++) {
        verticesArray[i].x = ((verticesArray[i].x - xmin) / (xmax - xmin)) * (_xmax - _xmin) + _xmin;
        verticesArray[i].y = ((verticesArray[i].y - ymin) / (ymax - ymin)) * (_ymax - _ymin) + _ymin;
        verticesArray[i].z = ((verticesArray[i].z - zmin) / (zmax - zmin)) * (_zmax - _zmin) + _zmin;
    }
}

void computeNormalsToFace() {
    int i;
    float ux, uy, uz, vx, vy, vz;

    normalsToFaceArray = calloc(numberFaces, sizeof(Vector3D));
    for (i = 0; i < numberFaces; i++) {
        ux = verticesArray[facesArray[i].a].x - verticesArray[facesArray[i].b].x;
        uy = verticesArray[facesArray[i].a].y - verticesArray[facesArray[i].b].y;
        uz = verticesArray[facesArray[i].a].z - verticesArray[facesArray[i].b].z;
        vx = verticesArray[facesArray[i].b].x - verticesArray[facesArray[i].c].x;
        vy = verticesArray[facesArray[i].b].y - verticesArray[facesArray[i].c].y;
        vz = verticesArray[facesArray[i].b].z - verticesArray[facesArray[i].c].z;

        normalsToFaceArray[i].x = uy * vz - uz * vy;
        normalsToFaceArray[i].y = uz * vx - ux * vz;
        normalsToFaceArray[i].z = ux * vy - uy * vx;
    }
}

void computeNormalsToVertex() {
    int i, j;
    normalsToVertexArray = calloc(numberVertices, sizeof(Vector3D));
    for (j = 0; j < numberVertices; j++) {
        for (i = 0; i < numberFaces; i++) {
            if ((facesArray[i].a == j) || (facesArray[i].b == j || (facesArray[i].c == j))) {
                normalsToVertexArray[j].x += normalsToFaceArray[i].x;
                normalsToVertexArray[j].y += normalsToFaceArray[i].y;
                normalsToVertexArray[j].z += normalsToFaceArray[i].z;
            }
        }
    }
}

void my_display() {
    int i;
    int a, b, c;
    float x1, y1, z1, x2, y2, z2, x3, y3, z3;

    Vector3D n;
    Vector3D n1, n2, n3;

    float light_pos[4] = {0.0, 0.0, -100.0, 0.0};

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1.0, 1.0, 250.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glTranslatef(0.0, 0.0, -20.0);
    glRotatef(angleX, 1.0, 0.0, 0.0);
    glRotatef(angleY, 0.0, 1.0, 0.0);
    glRotatef(angleZ, 0.0, 0.0, 1.0);

    srand(0);
    for (i = 0; i < numberFaces; i++) {
        a = facesArray[i].a;
        b = facesArray[i].b;
        c = facesArray[i].c;

        x1 = verticesArray[a].x;
        y1 = verticesArray[a].y;
        z1 = verticesArray[a].z;

        x2 = verticesArray[b].x;
        y2 = verticesArray[b].y;
        z2 = verticesArray[b].z;

        x3 = verticesArray[c].x;
        y3 = verticesArray[c].y;
        z3 = verticesArray[c].z;

        // Points.
        if (visualizationType == 0) {
            glDisable(GL_LIGHTING);
            glColor3ub(255, 255, 0);
            glPointSize(4.0);
            glBegin(GL_POINTS);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glEnd();
        }

        // Wireframe.
        if (visualizationType == 1) {
            glDisable(GL_LIGHTING);
            glColor3ub(255, 255, 0);
            glLineWidth(1.0);
            glBegin(GL_LINE_STRIP);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glEnd();

        }

        // Triangles.
        if (visualizationType == 2) {
            glDisable(GL_LIGHTING);
            glColor3ub(rand() % 255, rand() % 255, rand() % 255);
            glBegin(GL_TRIANGLES);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glEnd();
        }

        /* Flat shading. */
        if (visualizationType == 3) {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
            glShadeModel(GL_FLAT);
            n = normalsToFaceArray[i];
            glBegin(GL_TRIANGLES);
            glNormal3f(n.x, n.y, n.z);
            glVertex3f(x1, y1, z1);
            glVertex3f(x2, y2, z2);
            glVertex3f(x3, y3, z3);
            glEnd();
        }

        /* Gouraud shading. */
        if (visualizationType == 4) {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);
            glEnable(GL_NORMALIZE);
            glShadeModel(GL_SMOOTH);
            n1 = normalsToVertexArray[a];
            n2 = normalsToVertexArray[b];
            n3 = normalsToVertexArray[c];
            glBegin(GL_TRIANGLES);
            glNormal3f(n1.x, n1.y, n1.z);
            glVertex3f(x1, y1, z1);
            glNormal3f(n2.x, n2.y, n2.z);
            glVertex3f(x2, y2, z2);
            glNormal3f(n3.x, n3.y, n3.z);
            glVertex3f(x3, y3, z3);
            glEnd();
        }
    }

    angleX += 0.05;
    angleY += 0.05;
    angleZ += 0.05;

    glutSwapBuffers();
}

void my_keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 49:
            visualizationType = 0;
            break;
        case 50:
            visualizationType = 1;
            break;
        case 51:
            visualizationType = 2;
            break;
        case 52:
            visualizationType = 3;
            break;
        case 53:
            visualizationType = 4;
            break;
        case 27:
            exit(0);
    }
}

int main(int argc, char *argv[]) {
    loadObject("D:\\STUDIA_INFORMATYCZNE\\ROK_II\\SEM_IV\\03_GK\\reader_obj_2\\teapot.obj");
    normalizeVertices(10.0, -10.0);

    computeNormalsToFace();
    computeNormalsToVertex();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("OpenGL");

    glutDisplayFunc(my_display);
    glutIdleFunc(my_display);
    glutKeyboardFunc(my_keyboard);
    glutMainLoop();
}
