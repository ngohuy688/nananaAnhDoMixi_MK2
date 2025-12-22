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
float ceilingFan_control = 0;
float ceilingFan_levels[4] = { 0, 10, 20, 30 };
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
// ====== CỬA PHÒNG ======
mat4 door_ctm;
mat4 door_model;
GLfloat door_angle = 0.0f;      // góc mở cửa
GLfloat knob_angle = 0.0f;      // góc xoay núm
// ===== CAMERA AN NINH =====
mat4 securityCam_ctm;
mat4 securityCam_model;
bool securityCam_on = false;

GLfloat securityCam_angle = 0.0f;
GLint securityCam_dir = 1;   // 1: sang phải, -1: sang trái
vec3 securityCam_pos(-2.0, 1.4, -5.2);
float securityCam_baseYaw = 130.0f;   // quay vào phòng
float securityCam_pitch = -50.0f;
vec3 playerSavedPos;
float playerSavedYaw, playerSavedPitch;
bool inSecurityView = false;


void switchToSecurityCamera()
{
	// đặt vị trí
	cameraX = securityCam_pos.x;
	cameraY = securityCam_pos.y;
	cameraZ = securityCam_pos.z;

	// yaw = hướng camera an ninh + góc quét
	yaw = (securityCam_baseYaw + securityCam_angle) * DegreesToRadians;
	pitch = securityCam_pitch * DegreesToRadians;

}


void drawSecurityCamera()
{
	// ===== VỊ TRÍ GẮN CAMERA (TRÊN TƯỜNG) =====
	securityCam_ctm = Translate(-2.5, 1.8, -5.9); // góc tường trước
	securityCam_ctm *= RotateY(180);             // quay vào trong phòng
	securityCam_ctm *= RotateX(-20);

	// ===== GIÁ TREO =====
	setMaterial(
		color4(0.15, 0.15, 0.15, 1.0),
		color4(0.35, 0.35, 0.35, 1.0),
		color4(0.3, 0.3, 0.3, 1.0),
		32.0
	);
	securityCam_model = Scale(0.15, 0.05, 0.15);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, securityCam_ctm * securityCam_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// ===== PHẦN XOAY (PAN) =====
	mat4 pan = securityCam_ctm * RotateY(securityCam_angle);

	// ===== THÂN CAMERA =====
	setMaterial(
		color4(0.2, 0.2, 0.2, 1.0),
		color4(0.6, 0.6, 0.6, 1.0),
		color4(0.9, 0.9, 0.9, 1.0),
		64.0
	);
	securityCam_model = Translate(0, 0, -0.3) * Scale(0.25, 0.2, 0.5);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pan * securityCam_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// ===== ỐNG KÍNH =====
	setMaterial(
		color4(0.05, 0.05, 0.05, 1.0),
		color4(0.1, 0.1, 0.1, 1.0),
		color4(1.0, 1.0, 1.0, 1.0),
		96.0
	);
	// ===== CAMERA AN NINH QUÉT =====
	if (securityCam_on)
	{
		securityCam_angle += 0.6f * securityCam_dir;

		if (securityCam_angle > 45)
			securityCam_dir = -1;
		else if (securityCam_angle < -45)
			securityCam_dir = 1;
	}
	securityCam_model = Translate(0, 0, -0.58) * Scale(0.12, 0.12, 0.12);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pan * securityCam_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}



void drawRoundDoorKnob(mat4 door_base)
{
	setMaterial(
		color4(0.18, 0.18, 0.18, 1.0),
		color4(0.85, 0.85, 0.85, 1.0),
		color4(1.0, 1.0, 1.0, 1.0),
		80.0
	);

	// ===== NÚM TRÒN XOAY ĐÚNG TÂM =====
	mat4 knob =
		door_base
		* Translate(-0.6, -0.3, 0.13)   // vị trí núm
		* RotateZ(knob_angle)          // xoay tại CHÍNH NÚM
		* Scale(0.12, 0.12, 0.12);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, knob);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// ===== TRỤ NÚM (GẮN LIỀN) =====
	mat4 stem =
		door_base
		* Translate(-0.58, -0.3, 0.03)
		* Scale(0.06, 0.06, 0.08);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, stem);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}



void drawDoor()
{
	setMaterial(
		color4(0.35, 0.20, 0.05, 1.0),  // gỗ
		color4(0.60, 0.35, 0.10, 1.0),
		color4(0.20, 0.20, 0.20, 1.0),
		32.0
	);

	// Vị trí bản lề (bên trái cửa)
	door_ctm = Translate(0, 0.02, 2.02);

	// quay quanh bản lề
	door_ctm *= Translate(1, 0.0, 0.0)
		* RotateY(door_angle)
		* Translate(-1, 0.0, 0.0);


	// thân cửa
	door_model = Scale(2.0, 3.0, 0.05);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, door_ctm * door_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	// núm cửa
	drawRoundDoorKnob(door_ctm);
}


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
	block_airCondition_ctm *= Translate(0, -0.05, 0.2) * RotateX(-airCondition_door_control) * Translate(0, 0.05, -0.2);
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
	if (inday)
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

// Bàn thu ngân
mat4 pos_model;
float cashDrawerOpenAmount = 0.0f;   // biến điều khiển ngăn kéo

// Hàm vẽ khối hộp chữ nhật chung cho POS
void drawPosBlock(vec3 pos, vec3 size) {
	pos_model = Translate(pos) * Scale(size);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

// HÀM VẼ NGĂN KÉO TIỀN 
void drawCashDrawer() {
	// --- 1. VẼ VỎ NGOÀI CỐ ĐỊNH
	// Phần này đứng yên, bao bọc bên ngoài
	setMaterial(
		color4(0.05, 0.05, 0.05, 1.0), color4(0.1, 0.1, 0.1, 1.0),
		color4(0.7, 0.7, 0.7, 1.0), 64.0
	);
	drawPosBlock(vec3(1.56, -0.4, 1.0), vec3(0.5, 0.15, 0.6));

	// --- 2. VẼ NGĂN KÉO DI ĐỘNG (Phần trượt ra) ---
	// Tạo ma trận gốc cho phần di động (Di chuyển theo trục X)
	mat4 drawer_ctm = Translate(1.58 + cashDrawerOpenAmount, -0.4, 1.0);

	// Kích thước ngăn kéo 
	float d_len = 0.45f;   // Chiều sâu (trục X)
	float d_h = 0.12f;     // Chiều cao (trục Y)
	float d_wid = 0.55f;   // Chiều rộng (trục Z)
	float t = 0.02f;       // Độ dày của ván gỗ

	// -- a. Vẽ Đáy ngăn kéo --
	pos_model = drawer_ctm * Translate(0.0, -d_h / 2 + t / 2, 0.0) * Scale(d_len, t, d_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// -- b. Vẽ Thành trước (Phía tay nắm, trục X dương) --
	pos_model = drawer_ctm * Translate(d_len / 2 - t / 2, 0.0, 0.0) * Scale(t, d_h, d_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// -- c. Vẽ Thành sau (Phía trong, trục X âm) --
	pos_model = drawer_ctm * Translate(-d_len / 2 + t / 2, 0.0, 0.0) * Scale(t, d_h, d_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// -- d. Vẽ Thành trái (Trục Z âm) --
	pos_model = drawer_ctm * Translate(0.0, 0.0, -d_wid / 2 + t / 2) * Scale(d_len - 2 * t, d_h, t);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// -- e. Vẽ Thành phải (Trục Z dương) --
	pos_model = drawer_ctm * Translate(0.0, 0.0, d_wid / 2 - t / 2) * Scale(d_len - 2 * t, d_h, t);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);


	// --- 3. VẼ TIỀN BÊN TRONG (3 tờ) ---
	// Tiền phải nằm trên đáy 1 chút (Y cao hơn đáy)
	float money_y = -d_h / 2 + t + 0.005f;
	float m_len = 0.24f; // Dài theo X
	float m_wid = 0.14f; // Rộng theo Z (chia 3 ngăn)

	// Tờ 1: Xanh nước biển (Blue) - Nằm bên trái (Z âm)
	setMaterial(color4(0.0, 0.0, 0.8, 1.0), color4(0.2, 0.2, 1.0, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	pos_model = drawer_ctm * Translate(0.0, money_y, -0.16) * Scale(m_len, 0.005, m_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Tờ 2: Xanh lá cây (Green) - Nằm giữa
	setMaterial(color4(0.0, 0.6, 0.0, 1.0), color4(0.0, 0.8, 0.0, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	pos_model = drawer_ctm * Translate(0.0, money_y, 0.0) * Scale(m_len, 0.005, m_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Tờ 3: Hồng đỏ (Pink/Red) - Nằm bên phải (Z dương)
	setMaterial(color4(0.8, 0.0, 0.2, 1.0), color4(1.0, 0.2, 0.4, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	pos_model = drawer_ctm * Translate(0.0, money_y, 0.16) * Scale(m_len, 0.005, m_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);


	// --- 4. VẼ TAY NẮM (Khe kéo) ---
	setMaterial(color4(0.1, 0.1, 0.1, 1.0), color4(0.2, 0.2, 0.2, 1.0), color4(0.5, 0.5, 0.5, 1.0), 32.0);
	// Dịch ra trước mặt ngăn kéo (d_len/2)
	pos_model = drawer_ctm * Translate(d_len / 2 + 0.01, 0.0, 0.0) * Scale(0.02, 0.02, 0.3);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

// HÀM VẼ MÀN HÌNH POS
void drawPosScreen() {
	setMaterial(
		color4(0.02, 0.02, 0.02, 1.0), color4(0.08, 0.08, 0.08, 1.0),
		color4(0.8, 0.8, 0.8, 1.0), 128.0
	);
	// Chân đế màn hình 
	drawPosBlock(vec3(1.56, -0.25, 1.0), vec3(0.15, 0.15, 0.15));

	// Thân màn hình chính 
	drawPosBlock(vec3(1.56, -0.1, 1.0), vec3(0.05, 0.35, 0.5));

	// Màn hình hiển thị - Đặt lệch về phía X dương (hướng tường)
	setMaterial(color4(0.8, 0.8, 0.8, 1.0), color4(0.9, 0.9, 0.9, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	// 2.225 là vị trí lệch ra khỏi thân màn hình 1 chút về phía tường
	drawPosBlock(vec3(1.585, -0.1, 1.0), vec3(0.01, 0.3, 0.45));
}

// HÀM VẼ MÁY IN HÓA ĐƠN 
void drawReceiptPrinter() {
	setMaterial(
		color4(0.1, 0.1, 0.1, 1.0), color4(0.15, 0.15, 0.15, 1.0),
		color4(0.3, 0.3, 0.3, 1.0), 32.0
	);
	// Thân máy in 
	// Di chuyển vị trí từ bên trái (x=1.7) sang bên cạnh theo trục Z (z=1.5) để không bị rơi khỏi bàn
	drawPosBlock(vec3(1.58, -0.4, 1.5), vec3(0.3, 0.25, 0.25));

	// Khe nhả giấy (màu tối hơn) - Hướng lên trên hoặc hướng ra phía tường
	setMaterial(color4(0.05, 0.05, 0.05, 1.0), color4(0.05, 0.05, 0.05, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	drawPosBlock(vec3(1.66, -0.27, 1.5), vec3(0.05, 0.01, 0.15));
}
void banthungan() {
	// 1. VẼ BÀN GỖ
	setMaterial(
		color4(0.1, 0.1, 0.1, 1.0),   // Ambient
		color4(0.55, 0.35, 0.15, 1.0),// Diffuse (Màu gỗ nâu)
		color4(0.3, 0.3, 0.3, 1.0),   // Specular (Bóng vừa phải)
		32.0                          // Shininess
	);

	// Thân bàn chính (Đặt ở bên phải cửa: x=2.2, z=1.0, sàn y=-1.5)
	// Chiều cao bàn khoảng 1.0 -> Tâm y = -1.5 + 0.5 = -1.0
	drawPosBlock(vec3(1.56, -1.0, 0.5), vec3(0.8, 1.0, 2.7));

	// Mặt bàn (Mỏng hơn, rộng hơn thân một chút)
	drawPosBlock(vec3(1.56, -0.48, 0.5), vec3(0.9, 0.05, 2.7));

	// Các thành phần trên bàn thu ngân
	drawCashDrawer();
	drawPosScreen();
	drawReceiptPrinter();

}

// Ghế xoay
float chair_angle = 0.0f; // Góc xoay của ghế 

// Hàm hỗ trợ vẽ khối
void drawInstance(mat4 instance_matrix) {
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, instance_matrix);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

// HÀM VẼ GHẾ XOAY VĂN PHÒNG (TRỤC VÀ BÁNH XE TRÒN)
void gheXoay() {
	float chairX = 2.5f;
	float chairY = -1.476f; // Sàn nhà
	float chairZ = 1.0f;

	//CÁC THÔNG SỐ KÍCH THƯỚC
	float legLen = 0.45f;  // Chân
	float wheelR = 0.05f;  // Bán kính bánh xe
	float axisH = 0.48f;    // Trục 
	float seatSize = 0.6f; // Đệm ngồi 

	// 1. CHÂN GHẾ & BÁNH XE
	setMaterial(
		color4(0.05, 0.05, 0.05, 1.0), color4(0.1, 0.1, 0.1, 1.0),
		color4(0.3, 0.3, 0.3, 1.0), 32.0
	);

	for (int i = 0; i < 360; i += 72) {
		float rad = i * DegreesToRadians;

		// a. Vẽ thanh chân ghế
		float legY = chairY + wheelR * 2 + 0.02;
		mat4 leg_ctm = Translate(chairX, legY, chairZ) * RotateY(i) * Translate(legLen / 2, 0.0, 0.0);
		drawInstance(leg_ctm * Scale(legLen, 0.05, 0.06));

		// b. Vẽ bánh xe 
		// Tâm bánh xe
		mat4 wheel_center = Translate(chairX, chairY + wheelR, chairZ) * RotateY(i) * Translate(legLen - 0.02, 0.0, 0.0);

		// Dùng vòng lặp tạo hình trụ tròn cho bánh xe
		// Xoay 90 độ quanh Z để bánh xe dựng đứng
		// Sau đó xoay các lát cắt quanh trục Y cục bộ (trục bánh xe)
		for (int k = 0; k < 180; k += 20) {
			// Scale(Đường kính, Bề dày bánh, Độ dày lát cắt)
			// Đường kính 0.1, Bề dày 0.04, Lát cắt mỏng 0.02
			mat4 wheel_slice = wheel_center * RotateZ(90) * RotateY(k) * Scale(0.1, 0.04, 0.02);
			drawInstance(wheel_slice);
		}
	}

	//2. TRỤC XOAY THỦY LỰC
	float axisBaseY = chairY + wheelR * 2;

	// Dùng vòng lặp tạo hình trụ tròn cho trục
	for (int j = 0; j < 180; j += 15) {
		// Scale(Đường kính, Chiều cao, Độ dày lát cắt)
		// Đường kính 0.09, Lát cắt mỏng 0.02
		mat4 axis_slice = Translate(chairX, axisBaseY + axisH / 2, chairZ) * RotateY(j) * Scale(0.09, axisH, 0.02);
		drawInstance(axis_slice);
	}

	//3. PHẦN THÂN TRÊN
	float seatBaseY = axisBaseY + axisH;
	mat4 chair_upper = Translate(chairX, seatBaseY, chairZ) * RotateY(chair_angle);

	//Đệm ngồi
	setMaterial(
		color4(0.0, 0.0, 0.8, 1.0), color4(0.1, 0.1, 1.0, 1.0), color4(0.2, 0.2, 0.2, 1.0), 10.0
	);
	drawInstance(chair_upper * Translate(0.0, 0.05, 0.0) * Scale(seatSize, 0.1, seatSize));

	//Thanh nối L
	setMaterial(
		color4(0.1, 0.1, 0.1, 1.0), color4(0.1, 0.1, 0.1, 1.0), color4(0.5, 0.5, 0.5, 1.0), 32.0
	);
	drawInstance(chair_upper * Translate(0.25, 0.05, 0.0) * Scale(0.35, 0.05, 0.12));
	drawInstance(chair_upper * Translate(0.4, 0.3, 0.0) * Scale(0.05, 0.5, 0.12));

	//Lưng ghế
	setMaterial(
		color4(0.0, 0.0, 0.8, 1.0), color4(0.1, 0.1, 1.0, 1.0), color4(0.2, 0.2, 0.2, 1.0), 10.0
	);
	drawInstance(chair_upper * Translate(0.36, 0.55, 0.0) * Scale(0.08, 0.7, 0.55));
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
	drawDoor();
	drawSecurityCamera();
	quat();
	dieuhoa();

	banthungan();
	gheXoay();


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
	case 'v':
		if (!inSecurityView)
		{
			// lưu trạng thái người chơi
			playerSavedPos = vec3(cameraX, cameraY, cameraZ);
			playerSavedYaw = yaw;
			playerSavedPitch = pitch;

			switchToSecurityCamera();
			inSecurityView = true;
		}
		else
		{
			// quay lại người chơi
			cameraX = playerSavedPos.x;
			cameraY = playerSavedPos.y;
			cameraZ = playerSavedPos.z;
			yaw = playerSavedYaw;
			pitch = playerSavedPitch;
			inSecurityView = false;
		}
		break;
	case 'c':   // bật / tắt camera an ninh
		securityCam_on = !securityCam_on;
		break;

	case 'k':   // vặn núm
		if (knob_angle < 45)
			knob_angle += 5;
		break;

	case 'j':   // thả núm
		if (knob_angle > 0)
			knob_angle -= 5;
		break;

	case 'o':   // mở cửa (CHỈ KHI ĐANG VẶN)
		if (knob_angle >= 30 && door_angle < 70)
			door_angle += 3;
		break;

	case 'p':   // đóng cửa
		if (door_angle > 0)
			door_angle -= 3;

		// cửa đóng thì núm tự trả
		if (door_angle <= 0 && knob_angle > 0)
			knob_angle -= 3;
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
	case 't': // Mở ngăn kéo tiền
		cashDrawerOpenAmount += 0.02f;
		if (cashDrawerOpenAmount > 0.4f) cashDrawerOpenAmount = 0.4f; // Giới hạn mở tối đa
		glutPostRedisplay();
		break;
	case 'T': // Đóng ngăn kéo tiền
		cashDrawerOpenAmount -= 0.02f;
		if (cashDrawerOpenAmount < 0.0f) cashDrawerOpenAmount = 0.0f; // Giới hạn đóng tối đa
		glutPostRedisplay();
		break;
	case 'y': // Xoay ghế trái
		chair_angle += 5.0f;
		glutPostRedisplay();
		break;
	case 'Y': // Xoay ghế phải
		chair_angle -= 5.0f;
		glutPostRedisplay();
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
	cout << "==============Dang Tuan Linh(Cua va camera)================== \n";
	cout << "Van xuoi num: k de mo cua \n";
	cout << "Van nguoc num : j de khoa cua \n";
	cout << "Mo cua : o \n";
	cout << "Dong cua : p \n";
	cout << "Bat camera : c \n";
	cout << "Chuyen sang goc nhin camera : v, an lan nua de ve lai goc nhin ban dau \n";
	cout << "==============Dao Minh Quan(Ban thu ngan, ngan keo, ghe)================== \n";
	cout << "Mo ngan keo : t \n";
	cout << "Dong ngan keo : T \n";
	cout << "Xoay ghe : y/Y  \n";

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