/*Chương trình chiếu sáng Blinn-Phong (Phong sua doi) cho hình lập phương đơn vị, điều khiển quay bằng phím x, y, z, X, Y, Z.*/

#include "Angel.h"  /* Angel.h là file tự phát triển (tác giả Prof. Angel), có chứa cả khai báo includes glew và freeglut*/


// remember to prototype
void generateGeometry(void);
void initGPUBuffers(void);
void shaderSetup(void);
void display(void);
void keyboard(unsigned char key, int x, int y);

typedef vec4 point4;
typedef vec4 color4;
using namespace std;

// Số các đỉnh của các tam giác
const int NumPoints = 36;

point4 points[NumPoints]; /* Danh sách các đỉnh của các tam giác cần vẽ*/
color4 colors[NumPoints]; /* Danh sách các màu tương ứng cho các đỉnh trên*/
vec3 normals[NumPoints]; /*Danh sách các vector pháp tuyến ứng với mỗi đỉnh*/

point4 vertices[8]; /* Danh sách 8 đỉnh của hình lập phương*/
color4 vertex_colors[8]; /*Danh sách các màu tương ứng cho 8 đỉnh hình lập phương*/

GLuint program;

mat4 model;
GLuint model_loc;
mat4 projection;
GLuint projection_loc;
mat4 view;
GLuint view_loc;

int CurrentWidth = 700,
CurrentHeight = 700;

// Dùng biến đổi mô hình
float
r[] = { 0.0f, 0.0f, 0.0f },
s[] = { 1.0f, 1.0f, 1.0f },
t[] = { 0.0f, 0.0f, 0.0f };
//=============sử dụng cho camera============
int midWindowX;
int midWindowY;


bool keys[256]; // Array to keep track of pressed keys
bool mouseLocked = true;
float yaw = 0.0f;
float pitch = 0.0f;

float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 5.0f;
float moveSpeed = 0.1f;

// ======Dùng kiểm tra tịnh tiến, quay, co giãn====
bool
translated = false,
rotated = false,
scaled = false;
//bật tắt chiếu sáng
bool enableLighting = true;
GLuint enableLighting_loc;



void initCube()
{
	// Gán giá trị tọa độ vị trí cho các đỉnh của hình lập phương
	vertices[0] = point4(-0.5, -0.5, 0.5, 1.0);
	vertices[1] = point4(-0.5, 0.5, 0.5, 1.0);
	vertices[2] = point4(0.5, 0.5, 0.5, 1.0);
	vertices[3] = point4(0.5, -0.5, 0.5, 1.0);
	vertices[4] = point4(-0.5, -0.5, -0.5, 1.0);
	vertices[5] = point4(-0.5, 0.5, -0.5, 1.0);
	vertices[6] = point4(0.5, 0.5, -0.5, 1.0);
	vertices[7] = point4(0.5, -0.5, -0.5, 1.0);

	// Gán giá trị màu sắc cho các đỉnh của hình lập phương	
	vertex_colors[0] = color4(0.0, 0.0, 0.0, 1.0); // black
	vertex_colors[1] = color4(1.0, 0.0, 0.0, 1.0); // red
	vertex_colors[2] = color4(1.0, 1.0, 0.0, 1.0); // yellow
	vertex_colors[3] = color4(0.0, 1.0, 0.0, 1.0); // green
	vertex_colors[4] = color4(0.0, 0.0, 1.0, 1.0); // blue
	vertex_colors[5] = color4(1.0, 0.0, 1.0, 1.0); // magenta
	vertex_colors[6] = color4(1.0, 0.5, 0.0, 1.0); // orange
	vertex_colors[7] = color4(0.0, 1.0, 1.0, 1.0); // cyan
}
int Index = 0;
void quad(int a, int b, int c, int d)  /*Tạo một mặt hình lập phương = 2 tam giác, gán màu cho mỗi đỉnh tương ứng trong mảng colors*/
{
	vec4 u = vertices[b] - vertices[a];
	vec4 v = vertices[c] - vertices[b];
	vec3 normal = normalize(cross(u, v));

	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
	normals[Index] = normal; colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}
void makeColorCube(void)  /* Sinh ra 12 tam giác: 36 đỉnh, 36 màu*/
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}
void generateGeometry(void)
{
	initCube();
	makeColorCube();
}


void initGPUBuffers(void)
{
	// Tạo một VAO - vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Tạo và khởi tạo một buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), sizeof(normals), normals);


}

void shaderSetup(void)
{
	// Nạp các shader và sử dụng chương trình shader
	program = InitShader("vshader1.glsl", "fshader1.glsl");   // hàm InitShader khai báo trong Angel.h
	glUseProgram(program);

	// Khởi tạo thuộc tính vị trí đỉnh từ vertex shader
	GLuint loc_vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc_vPosition);
	glVertexAttribPointer(loc_vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint loc_vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(loc_vColor);
	glVertexAttribPointer(loc_vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

	GLuint loc_vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(loc_vNormal);
	glVertexAttribPointer(loc_vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points) + sizeof(colors)));


	model_loc = glGetUniformLocation(program, "Model");
	projection_loc = glGetUniformLocation(program, "Projection");
	view_loc = glGetUniformLocation(program, "View");

	enableLighting_loc = glGetUniformLocation(program, "enableLighting");
	glUniform1i(enableLighting_loc, enableLighting);


	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0, 1.0, 1.0, 1.0);	//xóa trắng
}

// ánh sáng
point4 light_position(0.0, 10.0, 5.0, 1.0);   // vị trí mặt trời
color4 light_ambient(0.3, 0.3, 0.3, 1.0);     // ánh sáng môi trường
color4 light_diffuse(1.0, 0.98, 0.95, 1.0);   // ánh sáng chiếu trực tiếp
color4 light_specular(1.0, 1.0, 1.0, 1.0);	  // ánh sáng phản lại

// set màu và vật liệu trước khi vẽ
void setMaterial(color4 material_ambient, color4 material_diffuse, color4 material_specular, float shininess)
{
	color4 ambient_product = light_ambient * material_ambient;
	color4 diffuse_product = light_diffuse * material_diffuse;
	color4 specular_product = light_specular * material_specular;

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product); //độ tối
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product); //màu gốc
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product); //độ bóng
	glUniform1f(glGetUniformLocation(program, "Shininess"), shininess); //độ sắc của bóng
}


void Tuong() 
{
	setMaterial(
		color4(0.20, 0.20, 0.20, 1.0), // ambient: sáng vừa trong bóng
		color4(0.60, 0.60, 0.60, 1.0), // diffuse: xám bê tông
		color4(0.02, 0.02, 0.02, 1.0), // specular: gần như không bóng
		5.0                            // shininess: rất nhám
	);

	mat4 instance = Scale(0.8, 10, 10);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, model*instance);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//view
	vec4 eye(cameraX, cameraY, cameraZ, 1.0);
	vec4 at(
		cameraX + sin(yaw),
		cameraY + sin(pitch),
		cameraZ - cos(yaw),
		1.0
	);
	vec4 up(0.0, 1.0, 0.0, 1.0);

	view = LookAt(eye, at, up);
	glUniformMatrix4fv(view_loc, 1, GL_TRUE, view);

	//draw
	glUniform1i(enableLighting_loc, enableLighting);
	Tuong();

	glutSwapBuffers();
}


void reshape(int width, int height)
{
	CurrentWidth = width;
	CurrentHeight = height;

	midWindowX = width / 2;
	midWindowY = height / 2;

	glutWarpPointer(midWindowX, midWindowY);

	float aspect = (float)width / height;
	projection = Perspective(90.0f, aspect, 0.1f, 100.0f);
	glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);

	glViewport(0, 0, width, height);
}
void keyboard(unsigned char key, int x, int y)
{
	vec3 forward(
		sin(yaw),
		0.0f,
		-cos(yaw)
	);

	vec3 right(
		-forward.z,
		0.0f,
		forward.x
	);

	int mod = glutGetModifiers();

	switch (key) {
	case 033:
		exit(1);
		break;

	case 'w':	//tiến
		cameraX += forward.x * moveSpeed;
		cameraZ += forward.z * moveSpeed;
		break;

	case 's':	//lùi
		cameraX -= forward.x * moveSpeed;
		cameraZ -= forward.z * moveSpeed;
		break;

	case 'a':   // sang trái
		cameraX -= right.x * moveSpeed / 2;
		cameraZ -= right.z * moveSpeed / 2;
		break;

	case 'd':   // sang phải
		cameraX += right.x * moveSpeed / 2;
		cameraZ += right.z * moveSpeed / 2;
		break;
	case ' ':	// lên / xuống
		if (mod & GLUT_ACTIVE_SHIFT)
		{
			cameraY -= 0.02f; // xuống
			glutPostRedisplay();
			return;
		}
		cameraY += 0.02f;
		break;

	case 'L':
	case 'l':	// bật/tắt sáng
		enableLighting = !enableLighting;
		glUniform1i(enableLighting_loc, enableLighting);
		break;
	}

	glutPostRedisplay();
}


void handleMouseMove(int x, int y) {
	if (mouseLocked) {
		int deltaX = x - midWindowX;
		int deltaY = y - midWindowY;

		yaw += deltaX * 0.001f;
		pitch += deltaY * -0.001f;

		if (pitch > 1.5f) pitch = 1.5f;
		else if (pitch < -1.5f) pitch = -1.5f;

		glutWarpPointer(midWindowX, midWindowY);
	}
	glutPostRedisplay();
}

void handleKeyRelease(unsigned char key, int x, int y) {
	keys[key] = false;
	glutPostRedisplay();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(640, 640);
	glutInitWindowPosition(100, 150);
	glutCreateWindow("A Cube is rotated by keyboard and shaded");

	glewInit();

	generateGeometry();
	initGPUBuffers();
	shaderSetup();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutSetCursor(GLUT_CURSOR_NONE);
	glutPassiveMotionFunc(handleMouseMove);
	glutKeyboardUpFunc(handleKeyRelease);


	glutReshapeFunc(reshape);


	glutMainLoop();
	return 0;
}
