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

//=============sử dụng cho camera============
int midWindowX;
int midWindowY;

bool mouseLocked = true;
float yaw = 0.0f;
float pitch = 0.0f;

float cameraX = 0.0f;
float cameraY = 0.0f;
float cameraZ = 5.0f;
float moveSpeed = 0.12f;

// màu bầu trời
GLfloat sky_red = 1, sky_green = 0.98, sky_blue = 0.9;


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

	point4 light_position(0.0, 1.0, 0.0, 0);   // vị trí mặt trời
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);

	glEnable(GL_DEPTH_TEST);
}

// ánh sáng
color4 light_ambient(0.3, 0.3, 0.3, 1.0);     // ánh sáng môi trường
color4 light_diffuse(1.0, 0.98, 0.95, 1.0);   // ánh sáng chiếu trực tiếp
color4 light_specular(1.0, 1.0, 1.0, 1.0);	  // ánh sáng phản lại
bool inlight = true;
bool inday = true;
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

// vẽ quạt 
mat4 ceilingFan_cmt;
mat4 ceilingFan_model;
float ceilingFan_control=0;
float ceilingFan_levels[4] = { 0, 10, 20, 30};
int ceilingFan_level = 0;
float ceilingFan_v = 0;

void trucquat()
{
	setMaterial(
		color4(0.08, 0.08, 0.10, 1.0),   // ambient
		color4(0.31, 0.78, 0.47, 1.0),   // diffuse 
		color4(0.8, 0.8, 0.8, 1.0),      // specular
		64.0                            // shininess
	);

	for (int i = 0; i <= 360; i += 20) {
		ceilingFan_model = Translate(0, 0.4, 0) * RotateY(i) * Scale(0.07, 0.7, 0.07);
		glUniformMatrix4fv(model_loc, 1, GL_TRUE, ceilingFan_cmt * ceilingFan_model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}
void dongco()
{
	setMaterial(
		color4(0.10, 0.10, 0.12, 1.0),   // ambient
		color4(0.31, 0.78, 0.47, 1.0),   // diffuse
		color4(0.4, 0.4, 0.4, 1.0),      // specular
		24.0                            // shininess
	);

	for (int i = 0; i <= 360; i += 10) {
		ceilingFan_model = RotateY(i) * Scale(0.5, 0.2, 0.1);
		glUniformMatrix4fv(model_loc, 1, GL_TRUE, ceilingFan_cmt * ceilingFan_model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}
void canhquat()
{
	setMaterial(
		color4(0.07, 0.07, 0.09, 1.0),   // ambient
		color4(0.31, 0.78, 0.47, 1.0),   // diffuse 
		color4(0.3, 0.3, 0.3, 1.0),      // specular
		16.0                            // shininess
	);

	for (int i = 0; i < 360; i += 120)
	{
		ceilingFan_model = RotateY(i) * Translate(0.6, 0.0, 0.0) * Scale(1.2, 0.02, 0.3);
		glUniformMatrix4fv(model_loc, 1, GL_TRUE, ceilingFan_cmt * ceilingFan_model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}
void quat() 
{
	ceilingFan_cmt = Translate(0, 1.75, -2);
	trucquat();
	dongco();

	ceilingFan_cmt *= RotateY(ceilingFan_control);
	canhquat();
}

// điều hòa
mat4 block_airCondition_model;
mat4 block_airCondition_ctm;
GLfloat airCondition_door_control;
void drawBlock(vec3 pos, vec3 size)
{
	block_airCondition_model = Translate(pos) * Scale(size);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, block_airCondition_ctm * block_airCondition_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

void canhdh() {
	setMaterial(
		color4(0.30, 0.30, 0.30, 1.0),
		color4(0.45, 0.45, 0.45, 1.0),
		color4(0.10, 0.10, 0.10, 1.0),
		10.0
	);
	drawBlock(vec3(0.0, -0.1, 0.2), vec3(0.8, 0.1, 0.01));
}

void dieuhoa() {
	setMaterial(
		color4(0.85, 0.85, 0.85, 1.0),   // Ambient
		color4(0.95, 0.95, 0.95, 1.0),   // Diffuse
		color4(0.20, 0.20, 0.20, 1.0),   // Specular
		20.0                             // Shininess
	);
	block_airCondition_ctm = Translate(2.95, 2.2, -3.5) * RotateY(-90);
	drawBlock(vec3(0.0, 0.0, 0.01), vec3(0.8, 0.3, 0.02));
	drawBlock(vec3(0.0, 0.05, 0.19), vec3(0.8, 0.2, 0.02));

	drawBlock(vec3(0.0, 0.14, 0.10), vec3(0.8, 0.02, 0.16));
	drawBlock(vec3(0.0, -0.14, 0.10), vec3(0.8, 0.02, 0.16));

	drawBlock(vec3(-0.41, 0.0, 0.10), vec3(0.02, 0.3, 0.2));
	drawBlock(vec3(0.41, 0.0, 0.10), vec3(0.02, 0.3, 0.2));
	block_airCondition_ctm *= Translate(0,-0.05,0.2) *  RotateX(-airCondition_door_control) * Translate(0, 0.05, -0.2);
	canhdh();
}

// nhà
mat4 block_wall;
void drawWallBlock(vec3 pos, vec3 size)
{
	block_wall = Translate(pos) * Scale(size);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, block_wall);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
void drawRoom()
{
	setMaterial(
		color4(0.20, 0.12, 0.05, 1.0), 
		color4(0.55, 0.35, 0.15, 1.0),
		color4(0.08, 0.08, 0.08, 1.0), 
		15.0
	);

	// SÀN
	drawWallBlock(vec3(0, -1.5, -2), vec3(6, 0.05, 8.0));

	setMaterial(
		color4(0.25, 0.25, 0.25, 1.0),
		color4(0.7, 0.7, 0.7, 1.0),
		color4(0.05, 0.05, 0.05, 1.0),
		8.0
	);
	// Bên trong
	// TRẦN
	drawWallBlock(vec3(0, 2.5, -2), vec3(6, 0.05, 8.0));

	//  TƯỜNG SAU
	drawWallBlock(vec3(0, 0.5, -6), vec3(6, 4, 0.05));

	// TƯỜNG TRÁI
	drawWallBlock(vec3(-3, 0.5, -2), vec3(0.05, 4.0, 8.0));

	// TƯỜNG PHẢI
	drawWallBlock(vec3(3, 0.5, -2), vec3(0.05, 4.0, 8.0));

	// TƯỜNG TRƯỚC
	// 2 bên
	drawWallBlock(vec3(-2.0, 0.5, 2), vec3(2.0, 4, 0.05));
	drawWallBlock(vec3(2.0, 0.5, 2), vec3(2.0, 4, 0.05));
	// trên
	drawWallBlock(vec3(0, 2.0, 2), vec3(2.0, 1.0, 0.05));

	// Bên ngoài
	color4 tmp = light_diffuse;
	if(inday) 
		light_diffuse = vec4(1.0, 0.98, 0.95, 1.0);
	else 
		light_diffuse = vec4(0.3, 0.3, 0.3, 1.0);
	setMaterial(
		color4(0.25, 0.25, 0.25, 1.0),
		color4(0.7, 0.7, 0.7, 1.0),
		color4(0.05, 0.05, 0.05, 1.0),
		8.0
	);

	// sàn
	drawWallBlock(vec3(0, -1.55, -2), vec3(6.1, 0.05, 8.1));
	// TRẦN
	drawWallBlock(vec3(0, 2.55, -2), vec3(6.1, 0.05, 8.1));

	//  TƯỜNG SAU
	drawWallBlock(vec3(0, 0.5, -6.05), vec3(6.1, 4.1, 0.05));

	// TƯỜNG TRÁI
	drawWallBlock(vec3(-3.03, 0.5, -2), vec3(0.05, 4.1, 8.1));

	// TƯỜNG PHẢI
	drawWallBlock(vec3(3.03, 0.5, -2), vec3(0.05, 4.1, 8.1));

	// TƯỜNG TRƯỚC
	// 2 bên
	drawWallBlock(vec3(-2.0, 0.5, 2.05), vec3(2, 4.1, 0.05));
	drawWallBlock(vec3(2.0, 0.5, 2.05), vec3(2, 4.1, 0.05));
	// trên
	drawWallBlock(vec3(0, 2.025, 2.05), vec3(2, 1.025, 0.05));

	light_diffuse = tmp;
}

void display(void)
{
	glClearColor(sky_red, sky_green, sky_blue, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//view
	vec4 eye(cameraX, cameraY, cameraZ, 1.0);
	vec3 dir(
		cos(pitch) * sin(yaw),
		sin(pitch),
		-cos(pitch) * cos(yaw)
	);

	vec4 at = eye + vec4(dir, 0.0);

	vec4 up(0.0, 1.0, 0.0, 1.0);

	view = LookAt(eye, at, up);
	glUniformMatrix4fv(view_loc, 1, GL_TRUE, view);

	//light internal
	if (!inlight) {
		light_diffuse = vec4(1.0, 0.98, 0.95, 1.0); 

		if (!inday) light_diffuse = vec4(0.9, 0.9, 0.9, 1.0);
	}
	else {
		light_diffuse = vec4(0.2, 0.2, 0.2, 1.0);

		if (!inday) light_diffuse = vec4(0.01, 0.01, 0.01, 1.0);
	}
	//draw
	drawRoom();
	quat();
	dieuhoa();


	glutSwapBuffers();
}


void reshape(int width, int height)
{
	CurrentWidth = width;
	CurrentHeight = height;

	midWindowX = width / 2;
	midWindowY = height / 2;

	glutWarpPointer(midWindowX, midWindowY); // tâm chuột

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
		cameraX -= right.x * moveSpeed;
		cameraZ -= right.z * moveSpeed;
		break;

	case 'd':   // sang phải
		cameraX += right.x * moveSpeed;
		cameraZ += right.z * moveSpeed;
		break;
	case ' ':	// lên / xuống
		if (mod & GLUT_ACTIVE_SHIFT)
		{
			cameraY -= 0.08f; // xuống
			return;
		}
		cameraY += 0.08f;
		break;

	case '`': // thoát chuột
		if (mouseLocked)
			glutSetCursor(GLUT_CURSOR_INHERIT);
		else
			glutSetCursor(GLUT_CURSOR_NONE);
		mouseLocked = !mouseLocked;
		break;

	case 'l': // sáng/tối
		inlight = !inlight;
		break;
	case 'L': // ngày/đêm
		if (!inday) {
			sky_red = 1;
			sky_green = 0.98;
			sky_blue = 0.9;
		}
		else {
			sky_red = 0.1;
			sky_green = 0.1;
			sky_blue = 0.25;
		}
		inday = !inday;
		break;
	case 'e':	//mở cửa điều hòa
		if (airCondition_door_control < 45) airCondition_door_control += 3;
		break;
	case 'r':	// đóng cửa điều hòa
		if (airCondition_door_control > 0) airCondition_door_control -= 3;
		break;
	case 'q':
		ceilingFan_level = (ceilingFan_level + 1) % 4;
		break;
	}
}

void timer(int)
{
	// tính tốc độ quay của quạt
	if (ceilingFan_v < ceilingFan_levels[ceilingFan_level])
		ceilingFan_v += 0.1f;

	if (ceilingFan_level == 0)
	{
		ceilingFan_v -= 0.15f;
		if (ceilingFan_v < 0)
			ceilingFan_v = 0;
	}

	ceilingFan_control += ceilingFan_v;

	if (ceilingFan_control > 360)
		ceilingFan_control -= 360;

	glutPostRedisplay();                 // gọi vẽ lại
	glutTimerFunc(1000 / 60, timer, 0); // lặp lại mỗi 16ms
}

void Instructor() {
	cout << "a, s, w, d, space, shift + space, mouse: các phím di chuyển \n";

}


void handleMouseMove(int x, int y) {
	if (mouseLocked) {
		int deltaX = x - midWindowX;
		int deltaY = y - midWindowY;

		yaw += deltaX * 0.001f;
		pitch += deltaY * -0.001f;

		if (pitch > 1.5f) pitch = 1.5f;
		else if (pitch < -1.5) pitch = -1.5f;

		glutWarpPointer(midWindowX, midWindowY);
	}
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

	glutReshapeFunc(reshape);
	glutTimerFunc(0, timer, 0);
	
	Instructor();
	glutMainLoop();
	return 0;
}
