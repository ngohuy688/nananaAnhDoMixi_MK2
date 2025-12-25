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
float fov = 60;

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

	point4 light_position(0.0, 1.0, 0.0, 1);   // vị trí mặt trời
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position);

	glEnable(GL_DEPTH_TEST);
}

// ánh sáng
color4 light_ambient(0.3, 0.3, 0.3, 1.0);     // ánh sáng nền
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

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product); //ánh
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product); //màu gốc
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product); //độ bóng
	glUniform1f(glGetUniformLocation(program, "Shininess"), shininess); //độ sắc của bóng
}


// vẽ quạt 
mat4 ceilingFan_cmt;
mat4 ceilingFan_model;

float ceilingFan_angle=0;
float ceilingFan_levels[4] = {0, 10, 20, 30};

int ceilingFan_level = 0;
float ceilingFan_v = 0;

// trục quạt
void shaft()
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
		glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	}
}
// động cơ
void engine()
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
		glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	}
}
// cánh quạt
void propeller()
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
		glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	}
}
// hộp số
void ceilingFanController()
{
	ceilingFan_cmt = Translate(-3.5, 0, -0.5) * RotateY(90) * Scale(0.6, 0.6, 0.6);
	// hộp số
	setMaterial(
		color4(0.10, 0.10, 0.12, 1.0),   // ambient
		color4(0.5, 0.5, 0.5, 1.0),   // diffuse
		color4(0.4, 0.4, 0.4, 1.0),      // specular
		24.0                            // shininess
	);
	ceilingFan_model = Scale(0.5, 0.8, 0.3);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, ceilingFan_cmt * ceilingFan_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	ceilingFan_model =Translate(0, 0.5, 0.1) * Scale(0.5, 0.3, 0.4);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, ceilingFan_cmt * ceilingFan_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// kim
	setMaterial(
		color4(0.10, 0.10, 0.12, 1.0),   // ambient
		color4(0.8, 0.5, 0.5, 1.0),   // diffuse
		color4(0.4, 0.4, 0.4, 1.0),      // specular
		24.0                            // shininess
	);
	ceilingFan_model = RotateZ(-ceilingFan_level*40) * Translate(0, 0, 0.2) * Scale(0.1, 0.3, 0.1);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, ceilingFan_cmt * ceilingFan_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

}
// ghép lại
void ceilingFan()
{
	ceilingFan_cmt = Translate(0, 1.75, -2);
	shaft();

	ceilingFan_cmt *= RotateY(ceilingFan_angle);
	engine();
	propeller();
}
// tinh tốc độ quay quạt
void ceilingFanSpeedControl() {
	if (ceilingFan_v < ceilingFan_levels[ceilingFan_level])
		ceilingFan_v += 0.1f;

	if (ceilingFan_level == 0)
	{
		ceilingFan_v -= 0.15f;
		if (ceilingFan_v < 0)
			ceilingFan_v = 0;
	}

	ceilingFan_angle += ceilingFan_v;

	if (ceilingFan_angle > 360)
		ceilingFan_angle -= 360;
}

// điều hòa
mat4 block_airConditioner_model;
mat4 block_airConditioner_ctm;
GLfloat airConditioner_door_angle;

void drawBlockAirConditioner(vec3 pos, vec3 size)
{
	block_airConditioner_model = Translate(pos) * Scale(size);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, block_airConditioner_ctm * block_airConditioner_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

void airConditioner()
{
	// thân điều hòa
	setMaterial(
		color4(0.85, 0.85, 0.85, 1.0),   // Ambient
		color4(0.95, 0.95, 0.95, 1.0),   // Diffuse
		color4(0.20, 0.20, 0.20, 1.0),   // Specular
		20.0                             // Shininess
	);
	block_airConditioner_ctm = Translate(3.55, 2.2, -4.5) * RotateY(-90) * Scale(2.0, 2.0, 2.0);
	drawBlockAirConditioner(vec3(0.0, 0.0, 0.01), vec3(0.8, 0.3, 0.02));
	drawBlockAirConditioner(vec3(0.0, 0.05, 0.19), vec3(0.8, 0.2, 0.02));

	drawBlockAirConditioner(vec3(0.0, 0.14, 0.10), vec3(0.8, 0.02, 0.16));
	drawBlockAirConditioner(vec3(0.0, -0.14, 0.10), vec3(0.8, 0.02, 0.16));

	drawBlockAirConditioner(vec3(-0.41, 0.0, 0.10), vec3(0.02, 0.3, 0.2));
	drawBlockAirConditioner(vec3(0.41, 0.0, 0.10), vec3(0.02, 0.3, 0.2));

	// cánh điều hòa
	setMaterial(
		color4(0.30, 0.30, 0.30, 1.0),
		color4(0.45, 0.45, 0.45, 1.0),
		color4(0.10, 0.10, 0.10, 1.0),
		10.0
	);
	block_airConditioner_ctm *= Translate(0, -0.05, 0.2) * RotateX(-airConditioner_door_angle) * Translate(0, 0.05, -0.2);
	drawBlockAirConditioner(vec3(0.0, -0.1, 0.2), vec3(0.8, 0.1, 0.01));
}


// camera an ninh
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
	// vị trí camera
	securityCam_ctm = Translate(-2.5, 1.8, -5.9); // góc tường trước
	securityCam_ctm *= RotateY(180);             // quay vào trong phòng
	securityCam_ctm *= RotateX(-20);

	// giá treo
	setMaterial(
		color4(0.15, 0.15, 0.15, 1.0),
		color4(0.35, 0.35, 0.35, 1.0),
		color4(0.3, 0.3, 0.3, 1.0),
		32.0
	);
	securityCam_model = Scale(0.15, 0.05, 0.15);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, securityCam_ctm * securityCam_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// xoay camera
	mat4 pan = securityCam_ctm * RotateY(securityCam_angle);

	// thân camera
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

// ====== CỬA PHÒNG ======
mat4 door_ctm;
mat4 door_model;
GLfloat door_angle = 0.0f;      // góc mở cửa
GLfloat knob_angle = 0.0f;      // góc xoay núm
// núm cửa
void drawRoundDoorKnob(mat4 door_base)
{
	setMaterial(
		color4(0.18, 0.18, 0.18, 1.0),
		color4(0.85, 0.85, 0.85, 1.0),
		color4(1.0, 1.0, 1.0, 1.0),
		80.0
	);

	// núm xoay
	mat4 knob =
		door_base
		* Translate(-0.6, -0.3, 0.13)   // vị trí núm
		* RotateZ(knob_angle)          // xoay tại CHÍNH NÚM
		* Scale(0.12, 0.12, 0.12);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, knob);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// trụ núm
	mat4 stem =
		door_base
		* Translate(-0.58, -0.3, 0.03)
		* Scale(0.06, 0.06, 0.08);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, stem);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
// cánh cửa
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
	door_ctm *= Translate(1.2, 0.0, 0.0)
		* RotateY(door_angle)
		* Translate(-1.2, 0.0, 0.0);


	// thân cửa
	door_model = Scale(2.4, 3.0, 0.05);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, door_ctm * door_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
	// núm cửa
	drawRoundDoorKnob(door_ctm);
}

// Bàn thu ngân
mat4 pos_model;
float cashDrawerOpenAmount = 0;   // biến điều khiển ngăn kéo
void drawPosBlock(vec3& pos, vec3& size)
{
	pos_model = Translate(pos) * Scale(size);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
// vẽ ngăn kéo đựng tiền
void drawCashDrawer() {
	// vẽ vỏ ngăn kéo
	setMaterial(
		color4(0.05, 0.05, 0.05, 1.0), 
		color4(0.1, 0.1, 0.1, 1.0),
		color4(0.7, 0.7, 0.7, 1.0), 64.0
	);
	drawPosBlock(vec3(1.8, -0.4, 1.0), vec3(0.5, 0.15, 0.6));

	// vẽ ngăn kéo
	mat4 drawer_ctm = Translate(1.8 + cashDrawerOpenAmount, -0.4, 1.0);

	// Kích thước ngăn kéo 
	float d_len = 0.45f;   // Chiều sâu (trục X)
	float d_h = 0.12f;     // Chiều cao (trục Y)
	float d_wid = 0.55f;   // Chiều rộng (trục Z)
	float t = 0.02f;       // Độ dày của ván gỗ

	// Vẽ Đáy ngăn kéo 
	pos_model = drawer_ctm * Translate(0.0, -d_h / 2 + t / 2, 0.0) * Scale(d_len, t, d_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Thành trước
	pos_model = drawer_ctm * Translate(d_len / 2 - t / 2, 0.0, 0.0) * Scale(t, d_h, d_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Thành sau
	pos_model = drawer_ctm * Translate(-d_len / 2 + t / 2, 0.0, 0.0) * Scale(t, d_h, d_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Thành trái
	pos_model = drawer_ctm * Translate(0.0, 0.0, -d_wid / 2 + t / 2) * Scale(d_len - 2 * t, d_h, t);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Thành phải
	pos_model = drawer_ctm * Translate(0.0, 0.0, d_wid / 2 - t / 2) * Scale(d_len - 2 * t, d_h, t);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);


	// vẽ 3 loại tiền
	float money_y = -d_h / 2 + t + 0.005f;
	float m_len = 0.24f; // Dài theo X
	float m_wid = 0.14f; // Rộng theo Z (chia 3 ngăn)

	// Tờ 1: Xanh nước biển 
	setMaterial(color4(0.0, 0.0, 0.8, 1.0), color4(0.2, 0.2, 1.0, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	pos_model = drawer_ctm * Translate(0.0, money_y, -0.16) * Scale(m_len, 0.005, m_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Tờ 2: Xanh lá cây
	setMaterial(color4(0.0, 0.6, 0.0, 1.0), color4(0.0, 0.8, 0.0, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	pos_model = drawer_ctm * Translate(0.0, money_y, 0.0) * Scale(m_len, 0.005, m_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// Tờ 3: Hồng đỏ 
	setMaterial(color4(0.8, 0.0, 0.2, 1.0), color4(1.0, 0.2, 0.4, 1.0), color4(0.1, 0.1, 0.1, 1.0), 10.0);
	pos_model = drawer_ctm * Translate(0.0, money_y, 0.16) * Scale(m_len, 0.005, m_wid);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);


	// Khe kéo
	setMaterial(color4(0.1, 0.1, 0.1, 1.0), color4(0.2, 0.2, 0.2, 1.0), color4(0.5, 0.5, 0.5, 1.0), 32.0);
	pos_model = drawer_ctm * Translate(d_len / 2 + 0.01, 0.0, 0.0) * Scale(0.02, 0.02, 0.3);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, pos_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

// màn hình
void drawPosScreen() {
	setMaterial(
		color4(0.02, 0.02, 0.02, 1.0), 
		color4(0.08, 0.08, 0.08, 1.0),
		color4(0.8, 0.8, 0.8, 1.0), 
		128.0
	);
	// Chân đế màn hình 
	drawPosBlock(vec3(1.8, -0.25, 1.0), vec3(0.15, 0.15, 0.15));

	// Thân màn hình chính 
	drawPosBlock(vec3(1.8, -0.1, 1.0), vec3(0.05, 0.35, 0.5));

	// Màn hình hiển thị 
	setMaterial(color4(0.8, 0.8, 0.8, 1.0), 
		color4(0.9, 0.9, 0.9, 1.0), 
		color4(0.1, 0.1, 0.1, 1.0), 
		10.0);
	drawPosBlock(vec3(1.825, -0.1, 1.0), vec3(0.01, 0.3, 0.45));
}

// máy in hóa đơn
void drawReceiptPrinter() {
	setMaterial(
		color4(0.1, 0.1, 0.1, 1.0), 
		color4(0.15, 0.15, 0.15, 1.0),
		color4(0.3, 0.3, 0.3, 1.0), 
		32.0
	);
	// Thân máy in 
	drawPosBlock(vec3(1.8, -0.4, 1.5), vec3(0.3, 0.25, 0.25));

	// Khe nhả giấy 
	setMaterial(color4(0.05, 0.05, 0.05, 1.0), 
		color4(0.05, 0.05, 0.05, 1.0), 
		color4(0.1, 0.1, 0.1, 1.0), 
		10.0);
	drawPosBlock(vec3(1.9, -0.27, 1.5), vec3(0.05, 0.01, 0.15));
}
// bàn thu ngân
void banthungan() {
	// 1. VẼ BÀN GỖ
	setMaterial(
		color4(0.1, 0.1, 0.1, 1.0),   // Ambient
		color4(0.55, 0.35, 0.15, 1.0),// Diffuse (Màu gỗ nâu)
		color4(0.3, 0.3, 0.3, 1.0),   // Specular (Bóng vừa phải)
		32.0                          // Shininess
	);

	// Thân bàn
	drawPosBlock(vec3(1.8, -1.0, 0.5), vec3(0.8, 1.0, 2.7));

	// Mặt bàn 
	drawPosBlock(vec3(1.8, -0.48, 0.5), vec3(0.9, 0.05, 2.7));

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

// HÀM VẼ GHẾ XOAY VĂN PHÒNG 
void gheXoay() {
	float chairX = 2.95f;
	float chairY = -1.48f; // Sàn nhà
	float chairZ = 1.0f;

	// THÔNG SỐ KÍCH THƯỚC
	float legLen = 0.45f;
	float wheelR = 0.05f;  // Bán kính bánh xe 
	float axisH = 0.5f;    // Chiều cao trục
	float seatSize = 0.6f;

	//  CHÂN GHẾ & BÁNH XE 
	setMaterial(
		color4(0.05, 0.05, 0.05, 1.0), 
		color4(0.1, 0.1, 0.1, 1.0),
		color4(0.3, 0.3, 0.3, 1.0), 
		32.0
	);

	for (int i = 0; i < 360; i += 72) {
		float rad = i * DegreesToRadians;

		// a. Vẽ thanh chân ghế (Giữ nguyên)
		float legY = chairY + wheelR * 2 + 0.02;
		mat4 leg_ctm = Translate(chairX, legY, chairZ) * RotateY(i) * Translate(legLen / 2, 0.0, 0.0);
		drawInstance(leg_ctm * Scale(legLen, 0.05, 0.06));

		// b. Vẽ bánh xe (LÀM TRÒN MỊN)
		// Tâm bánh xe
		mat4 wheel_center = Translate(chairX, chairY + wheelR, chairZ) * RotateY(i) * Translate(legLen - 0.02, 0.0, 0.0);

		// Dùng vòng lặp tạo hình trụ tròn cho bánh xe
		// Xoay 90 độ quanh Z để bánh xe dựng đứng
		// Sau đó xoay các lát cắt quanh trục Y cục bộ 
		for (int k = 0; k < 180; k += 20) {
			// Scale(Đường kính, Bề dày bánh, Độ dày lát cắt)
			// Đường kính 0.1, Bề dày 0.04, Lát cắt mỏng 0.02
			mat4 wheel_slice = wheel_center * RotateZ(90) * RotateY(k) * Scale(0.1, 0.04, 0.02);
			drawInstance(wheel_slice);
		}
	}

	// TRỤC XOAY THỦY LỰC
	float axisBaseY = chairY + wheelR * 2;

	// Dùng vòng lặp tạo hình trụ tròn cho trục
	for (int j = 0; j < 180; j += 15) {
		// Scale(Đường kính, Chiều cao, Độ dày lát cắt)
		// Đường kính 0.09, Lát cắt mỏng 0.02
		mat4 axis_slice = Translate(chairX, axisBaseY + axisH / 2, chairZ) * RotateY(j) * Scale(0.09, axisH, 0.02);
		drawInstance(axis_slice);
	}

	//  PHẦN THÂN TRÊN
	float seatBaseY = axisBaseY + axisH;
	mat4 chair_upper = Translate(chairX, seatBaseY, chairZ) * RotateY(chair_angle);

	//  Đệm ngồi 
	setMaterial(
		color4(0.0, 0.0, 0.8, 1.0), color4(0.1, 0.1, 1.0, 1.0), color4(0.2, 0.2, 0.2, 1.0), 10.0
	);
	drawInstance(chair_upper * Translate(0.0, 0.05, 0.0) * Scale(seatSize, 0.1, seatSize));

	//  Thanh nối L-Shape 
	setMaterial(
		color4(0.1, 0.1, 0.1, 1.0), 
		color4(0.1, 0.1, 0.1, 1.0), 
		color4(0.5, 0.5, 0.5, 1.0), 
		32.0
	);
	drawInstance(chair_upper * Translate(0.25, 0.05, 0.0) * Scale(0.35, 0.05, 0.12));
	drawInstance(chair_upper * Translate(0.4, 0.3, 0.0) * Scale(0.05, 0.5, 0.12));

	//  Lưng ghế 
	setMaterial(
		color4(0.0, 0.0, 0.8, 1.0), 
		color4(0.1, 0.1, 1.0, 1.0), 
		color4(0.2, 0.2, 0.2, 1.0), 
		10.0
	);
	drawInstance(chair_upper * Translate(0.34, 0.55, 0.0) * Scale(0.08, 0.7, 0.55));
}

// vẽ nhà
mat4 Walls_model;
mat4 Walls_ctm = Scale(1.2, 1, 1);

void drawWallsBlock(vec3& pos, vec3& size)
{
	Walls_model = Translate(pos) * Scale(size);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, Walls_ctm * Walls_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
void drawRoom()
{
	// sàn
	setMaterial(
		color4(0.20, 0.12, 0.05, 1.0),
		color4(0.55, 0.35, 0.15, 1.0),
		color4(0.08, 0.08, 0.08, 1.0),
		15.0
	);
	drawWallsBlock(vec3(0, -1.5, -2), vec3(6, 0.05, 8.0));

	setMaterial(
		color4(0.25, 0.25, 0.25, 1.0),
		color4(0.7, 0.7, 0.7, 1.0),
		color4(0.05, 0.05, 0.05, 1.0),
		8.0
	);
	// Bên trong
	// TRẦN
	drawWallsBlock(vec3(0, 2.5, -2), vec3(6, 0.05, 8.0));

	//  TƯỜNG SAU
	drawWallsBlock(vec3(0, 0.5, -6), vec3(6, 4, 0.05));

	// TƯỜNG TRÁI
	drawWallsBlock(vec3(-3, 0.5, -2), vec3(0.05, 4.0, 8.0));

	// TƯỜNG PHẢI
	//2 bên
	drawWallsBlock(vec3(3, 2.0, -2), vec3(0.04, 1, 8));   // trên
	drawWallsBlock(vec3(3, -1.0, -2), vec3(0.04, 1, 8));   // dưới
	drawWallsBlock(vec3(3, 0.5, -4.75), vec3(0.04, 2, 2.5));   // trái
	drawWallsBlock(vec3(3, 0.5, 0.75), vec3(0.04, 2, 2.5));   // phải

	// TƯỜNG TRƯỚC
	// 2 bên
	drawWallsBlock(vec3(-2.0, 0.5, 2), vec3(2.0, 4, 0.05));
	drawWallsBlock(vec3(2.0, 0.5, 2), vec3(2.0, 4, 0.05));
	// trên
	drawWallsBlock(vec3(0, 2.0, 2), vec3(2.0, 1.0, 0.05));

	// Bên ngoài
	color4 cur_light = light_diffuse;
	if (inday)
		light_diffuse = vec4(1.0, 0.98, 0.95, 1.0);
	else
		light_diffuse = vec4(0.3, 0.3, 0.3, 1.0);
	// nền đất
	setMaterial(
		color4(0.18, 0.12, 0.07, 1.0),  // ambient
		color4(0.45, 0.30, 0.18, 1.0),  // diffuse
		color4(0.04, 0.04, 0.04, 1.0),  // specular
		8.0                            // shininess
	);

	drawWallsBlock(vec3(0, -1.7, 0), vec3(20, 0.05, 20));

	setMaterial(
		color4(0.25, 0.25, 0.25, 1.0),
		color4(0.7, 0.7, 0.7, 1.0),
		color4(0.05, 0.05, 0.05, 1.0),
		8.0
	);
	// sàn
	drawWallsBlock(vec3(0, -1.55, -2), vec3(6.1, 0.05, 8.1));
	// TRẦN
	drawWallsBlock(vec3(0, 3.55, -2), vec3(8, 1.05, 10));

	//  TƯỜNG SAU
	drawWallsBlock(vec3(0, 0.5, -6.05), vec3(6.1, 4.1, 0.05));

	// TƯỜNG TRÁI
	drawWallsBlock(vec3(-3.03, 0.5, -2), vec3(0.05, 4.1, 8.1));

	// TƯỜNG PHẢI
	drawWallsBlock(vec3(3.03, 2.0, -2), vec3(0.04, 1, 8.1));   // trên
	drawWallsBlock(vec3(3.03, -1.0, -2), vec3(0.04, 1, 8.1));   // dưới
	drawWallsBlock(vec3(3.03, 0.5, -4.75), vec3(0.04, 2, 2.6));   // trái
	drawWallsBlock(vec3(3.03, 0.5, 0.75), vec3(0.04, 2, 2.6));   // phải

	// TƯỜNG TRƯỚC
	// 2 bên
	drawWallsBlock(vec3(-2.0, 0.5, 2.05), vec3(2, 4.1, 0.05));
	drawWallsBlock(vec3(2.0, 0.5, 2.05), vec3(2, 4.1, 0.05));
	// trên
	drawWallsBlock(vec3(0, 2.025, 2.05), vec3(2, 1.025, 0.05));

	light_diffuse = cur_light;
}

void drawCubeNow(const mat4& M)
{
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, M);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
// Wheel cylinder approximation (dùng cube xoay vòng như động cơ quạt)
void drawWheelCylinderLikeFan(const mat4& Mwheel,
	float radius,     // bán kính bánh
	float thickness,  // độ dày bánh (theo trục Z local)
	float wall,       // độ dày thành lốp
	int slices)       // số lát (càng nhiều càng tròn)
{
	// vật liệu lốp
	setMaterial(color4(0.05, 0.05, 0.05, 1.0),
		color4(0.15, 0.15, 0.15, 1.0),
		color4(0.25, 0.25, 0.25, 1.0),
		24.0f);

	float step = 360.0f / (float)slices;
	// độ dài mỗi đoạn theo chu vi 
	float segLen = 2.0f * 3.1415926f * radius / (float)slices;

	// 1) Thành lốp (vòng ngoài)
	for (float a = 0.0f; a < 360.0f; a += step)
	{
		mat4 seg =
			Mwheel
			* RotateZ(a)
			* Translate(radius, 0.0f, 0.0f)
			* RotateZ(90.0f)                  // cho cạnh dài nằm theo tiếp tuyến
			* Scale(segLen, wall, thickness);

		drawCubeNow(seg);
	}

	//  Mâm ở giữa
	setMaterial(color4(0.08, 0.08, 0.08, 1.0),
		color4(0.25, 0.25, 0.25, 1.0),
		color4(0.40, 0.40, 0.40, 1.0),
		48.0f);

	float hubR = radius * 0.45f;
	float hubWall = wall * 1.3f;
	float hubSegLen = 2.0f * 3.1415926f * hubR / (float)slices;

	for (float a = 0.0f; a < 360.0f; a += step)
	{
		mat4 seg =
			Mwheel
			* RotateZ(a)
			* Translate(hubR, 0.0f, 0.0f)
			* RotateZ(90.0f)
			* Scale(hubSegLen, hubWall, thickness * 0.9f);

		drawCubeNow(seg);
	}

	//  Nút giữa
	setMaterial(color4(0.10, 0.10, 0.10, 1.0),
		color4(0.35, 0.35, 0.35, 1.0),
		color4(0.8, 0.8, 0.8, 1.0),
		64.0f);
	drawCubeNow(
		Mwheel * Scale(radius * 0.25f, radius * 0.25f, thickness * 1.05f));
}

//  Car state 
float carX = 0.0f;
float carY = -1.0f;
float carZ = -1.2f;

float carHeading = 0.0f;     // deg
float carSteer = 0.0f;       // deg
float carWheelSpin = 0.0f;   // deg
float carScale = 0.5f;

//  Door state 
float carDoorAngle = 0.0f;
bool  carDoorAuto = false;
bool  carDoorOpening = false;
const float carDoorMax = 60.0f;
const float carDoorAnimSpeed = 2.0f;

// Car dimensions 
const float carBodyL = 2.6f;
const float carBodyH = 0.45f;
const float carBodyW = 1.0f;

// U-body params (silhouette chữ U)
const float uBottomH = 0.16f;
const float uSideH = carBodyH - uBottomH;
const float uLeftL = 1.10f;
const float uRightL = 0.85f;


// CAR PAINT 
struct CarPaintMat
{
	color4 ka;
	color4 kd;
	color4 ks;
	float  sh;
};

constexpr int kNumCarPaints = 6;


CarPaintMat gCarPaints[kNumCarPaints] =
{
	// đỏ
	{ color4(0.18, 0.05, 0.05, 1.0), color4(0.85, 0.10, 0.10, 1.0), color4(0.9, 0.9, 0.9, 1.0), 64.0f },
	// xanh dương
	{ color4(0.05, 0.05, 0.18, 1.0), color4(0.12, 0.35, 0.95, 1.0), color4(0.9, 0.9, 0.9, 1.0), 64.0f },
	// xanh lá
	{ color4(0.05, 0.18, 0.08, 1.0), color4(0.10, 0.80, 0.35, 1.0), color4(0.9, 0.9, 0.9, 1.0), 64.0f },
	// vàng
	{ color4(0.18, 0.16, 0.05, 1.0), color4(0.95, 0.82, 0.12, 1.0), color4(0.9, 0.9, 0.9, 1.0), 64.0f },
	// tím
	{ color4(0.12, 0.05, 0.18, 1.0), color4(0.65, 0.18, 0.85, 1.0), color4(0.9, 0.9, 0.9, 1.0), 64.0f },
	// trắng ngà
	{ color4(0.18, 0.18, 0.16, 1.0), color4(0.92, 0.92, 0.88, 1.0), color4(0.9, 0.9, 0.9, 1.0), 64.0f },
};

int gCarPaintIdx = 0;

inline CarPaintMat currentCarPaint()
{
	int idx = gCarPaintIdx % kNumCarPaints;
	if (idx < 0) idx += kNumCarPaints;
	return gCarPaints[idx];
}
// END CAR PAINT

mat4 carBaseMatrix(float carX, float carY, float carZ, float carHeading, float carScale)
{
	return Translate(carX, carY, carZ)
		* RotateY(carHeading)
		* Scale(carScale, carScale, carScale);
}

//  body chữ U 
void car_body_U(const mat4& Mcar)
{
	CarPaintMat p = currentCarPaint();

	setMaterial(p.ka, p.kd, p.ks, p.sh);
	float yBottom = -carBodyH * 0.5f + uBottomH * 0.5f;
	drawCubeNow(Mcar * Translate(0.0f, yBottom, 0.0f) * Scale(carBodyL, uBottomH, carBodyW));

	float ySide = -carBodyH * 0.5f + uBottomH + uSideH * 0.5f;

	float xLeftCenter = -carBodyL * 0.5f + uLeftL * 0.5f;
	drawCubeNow(Mcar * Translate(xLeftCenter, ySide, 0.0f) * Scale(uLeftL, uSideH, carBodyW));

	float xRightCenter = +carBodyL * 0.5f - uRightL * 0.5f;
	drawCubeNow(Mcar * Translate(xRightCenter, ySide, 0.0f) * Scale(uRightL, uSideH, carBodyW));
}

// kính trước 
void car_windshield(const mat4& Mcar)
{
	const float topBodyY = carBodyH * 0.5f;
	const float glassH = 0.42f;
	const float glassY = topBodyY + glassH * 0.5f;
	const float glassX = 0.55f;
	const float glassTilt = +28.0f;

	setMaterial(color4(0.02, 0.06, 0.08, 1.0),
		color4(0.20, 0.65, 0.95, 1.0),
		color4(0.9, 0.9, 0.9, 1.0),
		80.0f);
	drawCubeNow(
		Mcar * Translate(glassX, glassY, 0.0f) * RotateZ(glassTilt) * Scale(0.05f, glassH, 0.95f));
}

//  panel sau
void car_rear_panel(const mat4& Mcar)
{
	const float topBodyY = carBodyH * 0.5f;
	const float backH = 0.20f;
	const float backY = topBodyY + backH * 0.5f;
	const float backX = -0.50f;

	setMaterial(color4(0.02, 0.02, 0.02, 1.0),
		color4(0.10, 0.10, 0.10, 1.0),
		color4(0.2, 0.2, 0.2, 1.0),
		16.0f);
	drawCubeNow(
		Mcar * Translate(backX, backY, 0.0f) * Scale(0.06f, backH, 0.95f));
}

//  đèn 
void car_lights(const mat4& Mcar)
{
	// Headlights
	setMaterial(color4(0.20, 0.18, 0.05, 1.0),
		color4(1.00, 0.90, 0.25, 1.0),
		color4(1, 1, 1, 1),
		64.0f);
	drawCubeNow(Mcar * Translate(+1.30f, 0.02f, +0.33f) * Scale(0.10f, 0.10f, 0.18f));

	drawCubeNow(Mcar * Translate(+1.30f, 0.02f, -0.33f) * Scale(0.10f, 0.10f, 0.18f));

	// Tail lights
	setMaterial(color4(0.12, 0.02, 0.02, 1.0),
		color4(0.70, 0.00, 0.00, 1.0),
		color4(0.6, 0.6, 0.6, 1),
		32.0f);
	drawCubeNow(Mcar * Translate(-1.30f, 0.02f, +0.33f) * Scale(0.10f, 0.10f, 0.18f));

	drawCubeNow(Mcar * Translate(-1.30f, 0.02f, -0.33f) * Scale(0.10f, 0.10f, 0.18f));
}

// cửa + tay nắm 
mat4 car_door_matrix_posZ(const mat4& Mcar)
{
	const float doorL = 1.15f;
	const float doorT = 0.06f;

	const float doorH = uSideH * 0.90f;
	const float ySideBase = -carBodyH * 0.5f + uBottomH;
	const float doorY = ySideBase + doorH * 0.5f;

	const float doorZ = carBodyW * 0.5f + doorT * 0.5f;
	const float doorCenterX = 0.10f;
	const float hingeX = doorCenterX + doorL * 0.5f;

	return Mcar
		* Translate(hingeX, doorY, +doorZ)
		* RotateY(+carDoorAngle)
		* Translate(-doorL * 0.5f, 0.0f, 0.0f);
}

mat4 car_door_matrix_negZ(const mat4& Mcar)
{
	const float doorL = 1.15f;
	const float doorT = 0.06f;

	const float doorH = uSideH * 0.90f;
	const float ySideBase = -carBodyH * 0.5f + uBottomH;
	const float doorY = ySideBase + doorH * 0.5f;

	const float doorZ = carBodyW * 0.5f + doorT * 0.5f;
	const float doorCenterX = 0.10f;
	const float hingeX = doorCenterX + doorL * 0.5f;

	return Mcar
		* Translate(hingeX, doorY, -doorZ)
		* RotateY(-carDoorAngle)
		* Translate(-doorL * 0.5f, 0.0f, 0.0f);
}

void car_doors_and_handles(const mat4& Mcar)
{
	const float doorL = 1.15f;
	const float doorT = 0.06f;
	const float doorH = uSideH * 0.90f;

	mat4 MdPos = car_door_matrix_posZ(Mcar);
	mat4 MdNeg = car_door_matrix_negZ(Mcar);


	CarPaintMat p = currentCarPaint();
	// doors
	setMaterial(p.ka, p.kd, p.ks, p.sh);
	drawCubeNow(MdPos * Scale(doorL, doorH, doorT));

	drawCubeNow(MdNeg * Scale(doorL, doorH, doorT));
	// handles
	const float handleLocalX = -0.20f;
	const float handleLocalY = 0.00f;
	const float handleOut = doorT * 0.5f + 0.03f;

	setMaterial(color4(0.15, 0.15, 0.05, 1.0),
		color4(0.95, 0.85, 0.20, 1.0),
		color4(1.0, 1.0, 1.0, 1.0),
		96.0f);
	drawCubeNow(MdPos * Translate(handleLocalX, handleLocalY, +handleOut) * Scale(0.10f, 0.05f, 0.05f));

	drawCubeNow(MdNeg * Translate(handleLocalX, handleLocalY, -handleOut) * Scale(0.10f, 0.05f, 0.05f));
}
void car_door_auto() {
	if (carDoorAuto)
	{
		if (carDoorOpening)
		{
			carDoorAngle += carDoorAnimSpeed;
			if (carDoorAngle >= carDoorMax)
			{
				carDoorAngle = carDoorMax;
				carDoorAuto = false;
			}
		}
		else
		{
			carDoorAngle -= carDoorAnimSpeed;
			if (carDoorAngle <= 0.0f)
			{
				carDoorAngle = 0.0f;
				carDoorAuto = false;
			}
		}
	}
}
//  ghế 
void car_seat(const mat4& Mcar)
{
	float yBottomTop = (-carBodyH * 0.5f) + uBottomH;
	float ySeat = yBottomTop + 0.06f;

	float seatW = 0.22f, seatL = 0.35f, seatH = 0.10f;
	float backH = 0.28f, backT = 0.08f;

	float xSeat = 0.1f;
	float zOff = 0.22f;

	setMaterial(color4(0.05, 0.05, 0.05, 1.0),
		color4(0.12, 0.12, 0.12, 1.0),
		color4(0.20, 0.20, 0.20, 1.0),
		24.0f);
	auto drawOneSeat = [&](float z)
		{
			drawCubeNow(Mcar * Translate(xSeat, ySeat + seatH * 0.5f, z) * Scale(seatL, seatH, seatW));

			float xBack = xSeat - seatL * 0.35f;
			float yBack = ySeat + seatH + backH * 0.5f;

			drawCubeNow(Mcar * Translate(xBack, yBack, z) * Scale(backT, backH, seatW));
		};

	drawOneSeat(+zOff);
	drawOneSeat(-zOff);
}

//  bánh xe 
void car_wheels_cube(const mat4& Mcar)
{
	const float xFront = 0.95f, xRear = -0.95f;
	const float yWheel = -0.25f;
	const float zLeft = 0.48f, zRight = -0.48f;

	// Thông số bánh
	const float R = 0.22f;      // bán kính
	const float T = 0.12f;      // độ dày bánh
	const float wall = 0.05f;   // độ dày thành lốp
	const int slices = 9;      // tăng lên 36/48 nếu muốn tròn hơn

	// Giữ đúng logic quay hiện tại của bạn:
	// steer theo Y, spin theo Z
	mat4 FrontRot = RotateY(carSteer) * RotateZ(carWheelSpin);
	mat4 RearRot = RotateZ(carWheelSpin);

	auto drawOne = [&](float x, float y, float z, const mat4& Rmat)
		{
			mat4 Mwheel = Mcar * Translate(x, y, z) * Rmat;
			drawWheelCylinderLikeFan(Mwheel, R, T, wall, slices);
		};

	drawOne(xFront, yWheel, zLeft, FrontRot);
	drawOne(xFront, yWheel, zRight, FrontRot);
	drawOne(xRear, yWheel, zLeft, RearRot);
	drawOne(xRear, yWheel, zRight, RearRot);
}

//  MAIN 
void drawCar()
{
	mat4 Mcar = carBaseMatrix(carX, carY, carZ, carHeading, carScale);

	car_body_U(Mcar);
	car_doors_and_handles(Mcar);
	car_windshield(Mcar);
	car_rear_panel(Mcar);
	car_lights(Mcar);
	car_seat(Mcar);
	car_wheels_cube(Mcar);
}


//  SHELF CORNER + SHOWCASE 
// Kệ gỗ đặt ở góc sau-trái (gần tường trái x=-3.03 và tường sau z=-6.05)
const vec3  SHELF_BASE(0, -1.475f, -5.55f); // tâm đáy kệ nằm trên sàn
const float SHELF_W = 3.60f;
const float SHELF_D = 0.70f;
const float SHELF_H = 1.60f;
const float SHELF_T = 0.05f;

void drawShelf()
{
	// vật liệu gỗ
	setMaterial(color4(0.18, 0.10, 0.05, 1.0),
		color4(0.60, 0.38, 0.18, 1.0),
		color4(0.10, 0.10, 0.10, 1.0),
		18.0f);

	// 2 vách bên
	drawCubeNow(
		Translate(SHELF_BASE) *
		Translate(-(SHELF_W * 0.5f - SHELF_T * 0.5f), SHELF_H * 0.5f, 0.0f) *
		Scale(SHELF_T, SHELF_H, SHELF_D));
	drawCubeNow(
		Translate(SHELF_BASE) *
		Translate(+(SHELF_W * 0.5f - SHELF_T * 0.5f), SHELF_H * 0.5f, 0.0f) *
		Scale(SHELF_T, SHELF_H, SHELF_D));

	// lưng kệ
	drawCubeNow(
		Translate(SHELF_BASE) *
		Translate(0.0f, SHELF_H * 0.5f, -(SHELF_D * 0.5f - SHELF_T * 0.5f)) *
		Scale(SHELF_W, SHELF_H, SHELF_T));

	// các mặt kệ (3 tầng + nóc)
	float shelfY[4] = { 0.10f, 0.75f, 1.40f };
	for (int i = 0; i < 3; ++i)
	{
		drawCubeNow(
			Translate(SHELF_BASE) *
			Translate(0.0f, shelfY[i], 0.0f) *
			Scale(SHELF_W - 2.0f * SHELF_T, SHELF_T, SHELF_D - SHELF_T));
	}
}

// vẽ 1 xe với màu sơn (paintIdx)
void drawCarAt(float x, float y, float z, float headingDeg, float scale, int paintIdx)
{
	// paintIdx sẽ được dùng trong car_body_U + car_doors_and_handles (xem phần CAR PAINT ở trên)
	extern int gCarPaintIdx;
	gCarPaintIdx = paintIdx;

	mat4 Mcar = carBaseMatrix(x, y, z, headingDeg, scale);
	car_body_U(Mcar);
	car_doors_and_handles(Mcar);
	car_windshield(Mcar);
	car_rear_panel(Mcar);
	car_lights(Mcar);
	car_seat(Mcar);
	car_wheels_cube(Mcar);
}
void drawShowcaseCarsOnShelf()
{
	\
		float levelY[4] = { 0.10f, 0.75f, 1.40f };

	const int carsPerRow = 3;

	float margin = 0.20f;
	float innerW = (SHELF_W - 2.0f * SHELF_T) - 2.0f * margin;

	// vị trí Z của xe (đưa ra trước một chút)
	float zFront = +0.10f;

	// scale xe (nếu chật, giảm xuống 0.18f)
	float carScaleShow = 0.20f;

	for (int level = 0; level < 3; ++level)
	{
		for (int j = 0; j < carsPerRow; ++j)
		{
			// chia đều 5 xe theo chiều ngang kệ
			float t = (j + 0.5f) / (float)carsPerRow;   // 0..1
			float xLocal = -innerW * 0.5f + t * innerW; // tâm xe theo trục X (local)

			float x = SHELF_BASE.x + xLocal;
			float y = SHELF_BASE.y + levelY[level] + 0.12f;
			float z = SHELF_BASE.z + zFront;

			// đổi màu: chạy vòng theo bảng gCarPaints
			int paintIdx = (level * carsPerRow + j) % kNumCarPaints;

			drawCarAt(x, y, z, 180.0f, carScaleShow, paintIdx);
		}
	}
}

// RÈM CỬA
mat4 curtain_ctm;
mat4 curtain_model;

float curtain_width = 6.0f;   // chiều ngang tổng
float curtain_height = 4.0f;   // chiều cao
float curtain_depth = 0.05f;  // độ dày

GLfloat curtain_level = 1.0f;   // 1.0 = kéo xuống hết, 0.0 = kéo lên hết
GLfloat curtain_min = 0.05f;    // không cho biến mất hoàn toàn
// THANH CUỘN RÈM 
float rod_height = 0.25f;
float rod_depth = 0.25f;

// Khung cua so
void drawCurtain()
{
	setMaterial(
		color4(0.9, 0.9, 0.9, 1.0),   // ambient
		color4(0.8, 0.9, 0.9, 1.0),// diffuse
		color4(0.05, 0.05, 0.05, 1.0),// specular RẤT THẤP
		4.0                          // shininess THẤP
	);
	float currentHeight = curtain_height * curtain_level;

	// neo ở trên
	curtain_model =
		Translate(0, curtain_height / 2 - currentHeight / 2, 0)
		* Scale(curtain_width, currentHeight, curtain_depth);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, curtain_ctm * curtain_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
//thanh cuon
void drawCurtainRod()
{
	setMaterial(
		color4(0.8, 0.8, 0.8, 1.0),   // trắng / kim loại
		color4(0.9, 0.9, 0.9, 1.0),
		color4(0.6, 0.6, 0.6, 1.0),
		32.0
	);

	// đặt ngay trên đỉnh cửa sổ
	mat4 rod_model =
		Translate(0, curtain_height / 2 - rod_height / 2, 0.12f)
		* Scale(curtain_width + 0.2f, rod_height, rod_depth);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, curtain_ctm * rod_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

//ham rem cua so
void drawWindowCurtain()
{
	curtain_ctm = Translate(3.5, 0.5, -2.0) * RotateY(-90) * Scale(0.5, 0.5, 0.5);

	drawCurtain();
	drawCurtainRod();
}

// ve cua so
mat4 window_model;
mat4 window_ctm;

void drawWindowBlock(vec3 translate, vec3 scale)
{
	window_model = Translate(translate) * Scale(scale);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, window_ctm * window_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}

void drawWindowGlasses()
{
	setMaterial(
		color4(0.65, 0.80, 0.85, 0.6),
		color4(0.75, 0.90, 0.95, 0.6),
		color4(0.30, 0.30, 0.30, 0.6),
		35.0
	);
	// cạnh dưới
	drawWindowBlock(vec3(0.5, 0.0, 0.05), vec3(1.0, 0.05, 0.1));

	// cạnh trên
	drawWindowBlock(vec3(0.5, 1, 0.05), vec3(1.0, 0.05, 0.1));

	// cạnh trái
	drawWindowBlock(vec3(0.0, 0.5, 0.05), vec3(0.05, 1.0, 0.1));

	// cạnh phải
	drawWindowBlock(vec3(1, 0.5, 0.05), vec3(0.05, 1.0, 0.1));


	// dải pixel 1
	drawWindowBlock(vec3(0.20, 0.65, 0.06), vec3(0.06, 0.06, 0.02));
	drawWindowBlock(vec3(0.27, 0.72, 0.06), vec3(0.06, 0.06, 0.02));
	drawWindowBlock(vec3(0.34, 0.79, 0.06), vec3(0.06, 0.06, 0.02));

	// dải pixel 3
	drawWindowBlock(vec3(0.60, 0.20, 0.06), vec3(0.06, 0.06, 0.02));
	drawWindowBlock(vec3(0.67, 0.27, 0.06), vec3(0.06, 0.06, 0.02));

	setMaterial(
		color4(0.20, 0.13, 0.07, 1.0),  // ambient
		color4(0.55, 0.35, 0.18, 1.0),  // diffuse
		color4(0.10, 0.10, 0.10, 1.0),  // specular
		15.0                           // shininess
	);
}

// Vẽ 4 thanh khung gỗ
void drawWindowFrame()
{
	setMaterial(
		color4(0.2, 0.2, 0.2, 1.0),   // ambient
		color4(0.4, 0.4, 0.45, 1.0),  // diffuse (xám xanh như ảnh)
		color4(0.1, 0.1, 0.1, 1.0),   // specular
		5.0                            // shininess
	);
	
	//  Thanh trên 
	drawWindowBlock(vec3(0, 0.8, 0), vec3(2.2, 0.1, 0.1));

	//  Thanh dưới 
	drawWindowBlock(vec3(0, -0.8, 0), vec3(2.2, 0.1, 0.1));

	//  Thanh trái
	drawWindowBlock(vec3(-1.05, 0, 0), vec3(0.1, 1.5, 0.1));

	// Thanh phải
	drawWindowBlock(vec3(1.05, 0, 0), vec3(0.1, 1.5, 0.1));

}
void drawWindow() 
{
	window_ctm = Translate(3.6, 0.5, -2) *  RotateY(90) * Scale(1.4, 1.2, 1.4);
	drawWindowFrame();
	window_ctm *= Translate(-1, -0.75, 0) * Scale(0.67, 0.75, 0.4);
	mat4 goc = window_ctm; // Lưu trạng thái ma trận gốc

	// --- HÀNG DƯỚI 
	// Cột 1
	window_ctm = goc * Translate(0, 0, 0);
	drawWindowGlasses();

	// Cột 2
	window_ctm = goc * Translate(1.0, 0, 0);
	drawWindowGlasses();

	// Cột 3
	window_ctm = goc * Translate(2.0, 0, 0);
	drawWindowGlasses();

	// --- HÀNG TRÊN 
	// Cột 1
	window_ctm = goc * Translate(0, 1.0, 0);
	drawWindowGlasses();

	// Cột 2
	window_ctm = goc * Translate(1.0, 1.0, 0);
	drawWindowGlasses();
	// Cột 3
	window_ctm = goc * Translate(2.0, 1.0, 0);
	drawWindowGlasses();
}

//ve den phong
mat4 roomLamp_ctm;
mat4 roomLamp_model;
bool roomLamp_on = false;

void drawRoomLamp()
{
	// DÂY TREO
	setMaterial(
		color4(0.15, 0.15, 0.15, 1.0),
		color4(0.3, 0.3, 0.3, 1.0),
		color4(0.1, 0.1, 0.1, 1.0),
		8.0);

	roomLamp_model = Translate(0, -0.5, 0) * Scale(0.03, 0.7, 0.03);
	glUniformMatrix4fv(model_loc, 1, GL_TRUE, roomLamp_ctm * roomLamp_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// CHỤP ĐÈN
	setMaterial(
		color4(0.4, 0.4, 0.4, 1.0),
		color4(0.7, 0.7, 0.7, 1.0),
		color4(0.2, 0.2, 0.2, 1.0),
		16.0);

	roomLamp_model = Translate(0, -0.85, 0) * Scale(0.35, 0.25, 0.35);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, roomLamp_ctm * roomLamp_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);

	// BÓNG ĐÈN
	if (roomLamp_on)
	{
		setMaterial(
			color4(1.0, 1.0, 1.0, 1.0),
			color4(1.0, 1.0, 1.0, 1.0),
			color4(1.0, 1.0, 1.0, 1.0),
			96.0);
	}
	else
	{
		setMaterial(
			color4(0.2, 0.2, 0.2, 1.0),
			color4(0.3, 0.3, 0.3, 1.0),
			color4(0.1, 0.1, 0.1, 1.0),
			8.0);
	}

	roomLamp_model = Translate(0, -1, 0) * Scale(0.14, 0.14, 0.14);

	glUniformMatrix4fv(model_loc, 1, GL_TRUE, roomLamp_ctm * roomLamp_model);
	glDrawArrays(GL_TRIANGLES, 0, NumPoints);
}
void drawRoomLampControl() {
	roomLamp_ctm = Translate(0, 2.5, 0.5);
	drawRoomLamp();
	roomLamp_ctm = Translate(0, 2.5, -5);
	drawRoomLamp();
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

	vec4 up(0.0, 1.0, 0.0, 0);

	view = LookAt(eye, at, up);
	glUniformMatrix4fv(view_loc, 1, GL_TRUE, view);

	//light internal
	if (!inlight) {
		light_diffuse = vec4(1.0, 0.98, 0.95, 1.0);

		if (!inday) light_diffuse = vec4(0.9, 0.9, 0.9, 1.0);
	}
	else {
		light_diffuse = vec4(0.2 + ((1.0f - curtain_level) / 2), 0.2 + ((1.0f - curtain_level) / 2), 0.2 + ((1.0f - curtain_level) / 2), 1.0);

		if (!inday) light_diffuse = vec4(0.01, 0.01, 0.01, 1.0);
	}
	//draw
	drawRoom();
	// Tuan Linh
	drawDoor();
	drawSecurityCamera();

	// Ngo Huy
	ceilingFan();
	ceilingFanController();
	airConditioner();

	// Quan
	banthungan();
	gheXoay();

	// Anh Dat
	drawShelf();
	drawShowcaseCarsOnShelf();
	drawCar();

	// Tung
	drawWindowCurtain();
	drawRoomLampControl();
	drawWindow();

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
	projection = Perspective(fov, aspect, 0.1f, 100.0f);
	glUniformMatrix4fv(projection_loc, 1, GL_TRUE, projection);

	glViewport(0, 0, width, height);
}

void keyboard(unsigned char key, int x, int y)
{
	vec3 forward(sin(yaw), 0.0f, -cos(yaw));

	vec3 right(cos(yaw), 0.0f, sin(yaw));

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
			mouseLocked = false;
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
			mouseLocked = true;
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
		roomLamp_on = !roomLamp_on;
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
		if (airConditioner_door_angle < 45) airConditioner_door_angle += 3;
		break;
	case 'E':	// đóng cửa điều hòa
		if (airConditioner_door_angle > 0) airConditioner_door_angle -= 3;
		break;
	case 'q':
		ceilingFan_level = (ceilingFan_level + 1) % 4;
		break;
		// ===== CAR CONTROLS =====
	case 'f': case 'F':
		carHeading += 5.0f;
		break;

	case 'h': case 'H':
		carHeading -= 5.0f;
		break;

	case 't': case 'T': {
		float rad = carHeading * DegreesToRadians;
		carX += 0.10f * cos(rad);
		carZ -= 0.10f * sin(rad);
		carWheelSpin -= 12.0f;

		// (tuỳ chọn) giới hạn vòng quay tránh tràn số
		if (carWheelSpin < -360.0f) carWheelSpin += 360.0f;
		break;
	}

	case 'g': case 'G': {
		float rad = carHeading * DegreesToRadians;
		carX -= 0.10f * cos(rad);
		carZ += 0.10f * sin(rad);
		carWheelSpin += 12.0f;

		if (carWheelSpin > 360.0f) carWheelSpin -= 360.0f;
		break;
	}

	case 'n': case 'N':
		carSteer += 4.0f;
		if (carSteer > 30.0f) carSteer = 30.0f;
		break;

	case 'm': case 'M':
		carSteer -= 4.0f;
		if (carSteer < -30.0f) carSteer = -30.0f;
		break;

	case '1':
		// auto: nếu đang đóng/nửa đóng => mở, còn lại => đóng
		carDoorOpening = (carDoorAngle < (carDoorMax * 0.5f));
		carDoorAuto = true;
		break;

	case '2':
		// mở thêm từng bước
		carDoorAuto = false;
		carDoorAngle += 5.0f;
		if (carDoorAngle > carDoorMax) carDoorAngle = carDoorMax;
		break;

	case '3':
		// đóng bớt từng bước
		carDoorAuto = false;
		carDoorAngle -= 5.0f;
		if (carDoorAngle < 0.0f) carDoorAngle = 0.0f;
		break;
		// tùng
	case 'i':   // kéo rèm LÊN
		if (curtain_level > curtain_min)
			curtain_level -= 0.05f;
		break;

	case 'I':   // kéo rèm XUỐNG
		if (curtain_level < 1.0f)
			curtain_level += 0.05f;
		break;


	case 'z': // Mở ngăn kéo tiền
		cashDrawerOpenAmount += 0.02f;
		if (cashDrawerOpenAmount > 0.4f) cashDrawerOpenAmount = 0.4f; // Giới hạn mở tối đa
		glutPostRedisplay();
		break;
	case 'Z': // Đóng ngăn kéo tiền
		cashDrawerOpenAmount -= 0.02f;
		if (cashDrawerOpenAmount < 0.0f) cashDrawerOpenAmount = 0.0f; // Giới hạn đóng tối đa
		glutPostRedisplay();
		break;
	case 'x': // Xoay ghế trái
		chair_angle += 5.0f;
		glutPostRedisplay();
		break;
	case 'X': // Xoay ghế phải
		chair_angle -= 5.0f;
		glutPostRedisplay();
		break;
	}
}

void timer(int)
{
	// tính tốc độ quay của quạt
	ceilingFanSpeedControl();
	// điều khiển cửa xe
	car_door_auto();

	glutPostRedisplay();                 // gọi vẽ lại
	glutTimerFunc(1000 / 60, timer, 0); // 60 fps
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
	cout << "==============CAR (NEW)================== \n";
	cout << "F/H: quay xe trai/phai\n";
	cout << "T/G: xe tien/lui\n";
	cout << "N/M: danh lai trai/phai\n";
	cout << "1: auto mo/dong cua xe\n";
	cout << "2: mo cua xe them (step)\n";
	cout << "3: dong cua xe (step)\n";
	cout << "==============Dao Minh Quan(Bàn thu ngân và ghế xoay)================== \n";
	cout << "Đóng/Mở ngăn kéo: z/Z  \n";
	cout << "Xoay ghế : x  \n";


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
