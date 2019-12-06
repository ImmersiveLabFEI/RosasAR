#include <glm.hpp>
#include <GL/glut.h>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <cstdio>

// Very, VERY simple OBJ loader.
// Here is a short list of features a real function would provide : 
// - Binary files. Reading a model should be just a few memcpy's away, not parsing a file at runtime. In short : OBJ is not very great.
// - Animations & bones (includes bones weights)
// - Multiple UVs
// - All attributes should be optional, not "forced"
// - More stable. Change a line in the OBJ file and it crashes.
// - More secure. Change another line and you can inject code.
// - Loading from memory, stream, etc

class color {

public:
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	float Ns, Ni, d;
	int illum;

	color(){}
	color(glm::vec3 a, glm::vec3 d, glm::vec3 s, float Ns, float Ni, float di = 1.0, int illum = 3) {
		this->ambient = a;
		this->diffuse = d;
		this->specular = s;
		this->Ns = Ns;
		this->Ni = Ni;
		this->d = di;
		this->illum = illum;
	}
};

std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;
std::vector<int> indO;
std::vector<std::string> mtls;
std::vector<std::string> objName;

std::unordered_map<std::string, color> colors;

GLfloat angle, fAspect;

void scale3DModel(float scaleFactor)
{
	for (int i = 0; i < vertices.size(); i += 1)
	{
		vertices[i] = vertices[i] * glm::vec3(scaleFactor * 1.0f, scaleFactor * 1.0f, scaleFactor * 1.0f);
	}

	for (int i = 0; i < normals.size(); i += 1)
	{
		normals[i] = normals[i] * glm::vec3(scaleFactor * 1.0f, scaleFactor * 1.0f, scaleFactor * 1.0f);
	}

	for (int i = 0; i < uvs.size(); i += 1)
	{
		uvs[i] = uvs[i] * glm::vec2(scaleFactor * 1.0f, scaleFactor * 1.0f);
	}
}

bool loadOBJ(const char * path) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}
	int i = 0;
	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
			i++;
		}
		else if (strcmp(lineHeader, "usemtl") == 0) {
			indO.push_back(i);
			char temp[128];
			fscanf(file, "%s", temp);
			mtls.push_back(temp);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}
	}
	
	// For each vertex of each triangle
	for (unsigned int i = 0; i < vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		vertices.push_back(vertex);
		uvs.push_back(uv);
		normals.push_back(normal);

	}

	return true;
}

bool loadMTL(const char * path) {
	printf("Loading MTL file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file = fopen(path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}
	char key[128];
	glm::vec3 a, d, s;
	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader
		
		float ns, ni, di;
		int illu;
		if (strcmp(lineHeader, "newmtl") == 0) {
			fscanf(file, "%s\n", key);
		}
		else if (strcmp(lineHeader, "Ka") == 0) {
			fscanf(file, "%f %f %f\n", &a.x, &a.y, &a.z);
		}
		else if (strcmp(lineHeader, "Kd") == 0) {
			fscanf(file, "%f %f %f\n", &d.x, &d.y, &d.z);
		}
		else if (strcmp(lineHeader, "Ks") == 0) {
			fscanf(file, "%f %f %f\n", &s.x, &s.y, &s.z);
		}
		else if (strcmp(lineHeader, "Ns") == 0) {
			fscanf(file, "%f\n", &ns);
		}
		else if (strcmp(lineHeader, "Ni") == 0) {
			fscanf(file, "%f\n", &ni);
		}
		else if (strcmp(lineHeader, "d") == 0) {
			fscanf(file, "%f\n", &di);
		}
		else if (strcmp(lineHeader, "illum") == 0) {
			fscanf(file, "%d\n", &illu);
			
			colors[key] = color(a, d, s, ns, ni, di, illu);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}


	return true;
}


void DrawOBJ() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	glm::vec3 a;
	glPushMatrix();
	glTranslatef(0, 0, -100);
	glRotatef(-90.0f, 0.0, 1.0, 0.0);

	
	int ind = 0;

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_TRIANGLES);

		for (int i = 0; i < vertices.size(); i ++)
		{
			if (i == indO[ind]) {
				std::cout << i << " " << ind << " " << indO[ind] << "\n";

				GLfloat mat_ambient[] = { colors[mtls[ind]].ambient.x, colors[mtls[ind]].ambient.y, colors[mtls[ind]].ambient.z, 1.0 };
				GLfloat mat_diffuse[] = { colors[mtls[ind]].diffuse.x, colors[mtls[ind]].diffuse.y, colors[mtls[ind]].diffuse.z, 1.0 };
				GLfloat mat_specular[] = { colors[mtls[ind]].specular.x, colors[mtls[ind]].specular.y, colors[mtls[ind]].specular.z, 1.0 };

				glEnable(GL_COLOR_MATERIAL);
				glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
				glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
				glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
				glColor3f(colors[mtls[ind]].diffuse.x, colors[mtls[ind]].diffuse.y, colors[mtls[ind]].diffuse.z);

				if(ind+1 < indO.size())
					ind++;
			}

			a = vertices[i];
			glNormal3f(a.x, a.y, a.z);
			glVertex3f(a.x, a.y, a.z);
		}
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
}

void displaySphere() {

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat mat_ambient_color[] = { 0.8, 0.01, 0.05, 1.0 };
	GLfloat mat_diffuse[] = { 0.1, 0.1, 0.8, 1.0 };
	GLfloat mat_specular[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat no_shininess[] = { 0.0 };
	GLfloat low_shininess[] = { 5.0 };
	GLfloat high_shininess[] = { 100.0 };
	GLfloat mat_emission[] = { 0.3, 0.2, 0.2, 0.0 };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*  draw sphere in first row, first column
	*  diffuse reflection only; no ambient or specular
	*/
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	/* now glColor* changes diffuse reflection  */
	glColor3f(0.1, 0.0, 0.9);
	/* draw some objects here */
	glColorMaterial(GL_FRONT, GL_SPECULAR);
	/* glColor* no longer changes diffuse reflection  */
	/* now glColor* changes specular reflection  */
	glColor3f(0.9, 0.0, 0.2);
	glPushMatrix();
	glTranslatef(-3.75, 3.0, -50.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
	glutSolidSphere(1.0, 16, 16);

	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
	/* draw other objects here */
	/*  draw sphere in first row, second column
	*  diffuse and specular reflection; low shininess; no ambient
	*/
	glPushMatrix();
	glTranslatef(-1.25, 3.0, -50.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_color);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, low_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
	glutSolidSphere(1.0, 16, 16);
	glPopMatrix();

	/*  draw sphere in first row, third column
	*  diffuse and specular reflection; high shininess; no ambient
	*/
	glPushMatrix();
	glTranslatef(1.25, 3.0, -50.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
	glutSolidSphere(1.0, 16, 16);
	glPopMatrix();

	/*  draw sphere in first row, fourth column
	*  diffuse reflection; emission; no ambient or specular refl.
	*/
	glPushMatrix();
	glTranslatef(3.75, 3.0, -50.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_color);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
	glMaterialfv(GL_FRONT, GL_SHININESS, no_shininess);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glutSolidSphere(1.0, 16, 16);
	glPopMatrix();


	glutSwapBuffers();
}


void init() {
	
	GLfloat luzAmbiente[4] = { 0.2,0.2,0.2,1.0 };
	GLfloat luzDifusa[4] = { 0.7,0.7,0.7,1.0 };	   // "cor" 
	GLfloat luzEspecular[4] = { 1.0, 1.0, 1.0, 1.0 };// "brilho" 
	GLfloat posicaoLuz[4] = { 0.0, 0.0, -50.0, 1.0 };

	// Capacidade de brilho do material
	GLfloat especularidade[4] = { 1.0,1.0,1.0,1.0 };
	GLint especMaterial = 60;

	// Especifica que a cor de fundo da janela será preta
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// Habilita o modelo de colorização de Gouraud
	glShadeModel(GL_SMOOTH);

	// Define a refletância do material 
	glMaterialfv(GL_FRONT, GL_SPECULAR, especularidade);
	// Define a concentração do brilho
	glMateriali(GL_FRONT, GL_SHININESS, especMaterial);

	// Ativa o uso da luz ambiente 
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, luzAmbiente);

	// Define os parâmetros da luz de número 0
	glLightfv(GL_LIGHT0, GL_AMBIENT, luzAmbiente);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, luzDifusa);
	glLightfv(GL_LIGHT0, GL_SPECULAR, luzEspecular);
	glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);

	// Habilita a definição da cor do material a partir da cor corrente
	glEnable(GL_COLOR_MATERIAL);
	//Habilita o uso de iluminação
	glEnable(GL_LIGHTING);
	// Habilita a luz de número 0
	glEnable(GL_LIGHT0);
	// Habilita o depth-buffering
	glEnable(GL_DEPTH_TEST);

	loadOBJ("carro_alegorico_v8.obj");
	loadMTL("carro_alegorico_v8.mtl");
	scale3DModel(1.0);
	angle = 45;
}

// Função usada para especificar o volume de visualização
void visualization(void)
{
	// Especifica sistema de coordenadas de projeção
	glMatrixMode(GL_PROJECTION);
	// Inicializa sistema de coordenadas de projeção
	glLoadIdentity();

	// Especifica a projeção perspectiva
	gluPerspective(angle, fAspect, 0.1, 500);

	// Especifica sistema de coordenadas do modelo
	glMatrixMode(GL_MODELVIEW);
	// Inicializa sistema de coordenadas do modelo
	glLoadIdentity();

	// Especifica posição do observador e do alvo
	gluLookAt(0, 80, 200, 0, 0, 0, 0, 1, 0);
}

// Função callback chamada quando o tamanho da janela é alterado 
void WindowReshape(GLsizei w, GLsizei h)
{
	// Para previnir uma divisão por zero
	if (h == 0) h = 1;

	// Especifica o tamanho da viewport
	glViewport(0, 0, w, h);

	// Calcula a correção de aspecto
	fAspect = (GLfloat)w / (GLfloat)h;

	visualization();
}

// Função callback chamada para gerenciar eventos do mouse
void mouseCallback(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON)
		if (state == GLUT_DOWN) {  // Zoom-in
			if (angle >= 10) angle -= 5;
		}
	if (button == GLUT_RIGHT_BUTTON)
		if (state == GLUT_DOWN) {  // Zoom-out
			if (angle <= 130) angle += 5;
		}
	visualization();
	glutPostRedisplay();
}

int main(int argc, char *argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(300, 100);
	glutCreateWindow("obj");
	init();
	glutDisplayFunc(DrawOBJ);
	glutMouseFunc(mouseCallback);
	glutReshapeFunc(WindowReshape);
	
	glutMainLoop();
	
}