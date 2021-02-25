#include <fstream>
#include <vector>
#include "Angel-yjc.hpp"
#include "gentestcoordinate.hpp"
using namespace std;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program;           // shader program object id
GLuint longitude_buffer;  // vertex buffer object id for lines of longitude
GLuint latitude_buffer;   // vertex buffer object id for lines of latitude
GLuint route_buffer;      // vertex buffer object id for the route
GLuint axis_buffer;       // vertex buffer object id for axis

// Projection transformation parameters
GLfloat fovy = 90.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat aspect;       // Viewport aspect ratio
GLfloat zNear = 0.5, zFar = 300.0;

vec3 init_position(0.0, 0.0, 0.0);  // initial viewer position
vec4 offset(0.0, 0.0, 0.0, 0.0);
vec3 curr_position = init_position;
vec3 curr_velocity(0.0, 0.0, -1.0);
vec3 camera_direction = curr_velocity;

int curr_count = 0;
int curr_position_index = 0;

// 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int animationFlag = 0;

vector<vec3> longitude_points(102, vec3());  // positions for all vertices
vector<vec4> longitude_colors(102, vec4(0.78, 0.4, 0.65, 0.8));

vector<vec3> latitude_points(102, vec3());
vector<vec4> latitude_colors(102, vec4(0.78, 0.4, 0.65, 0.8));

vector<vec3> route_points;
vector<vec4> route_colors;

vector<vec3> axis_points;
vector<vec4> axis_colors;

vector<vec3> positions;
vector<vec3> velocities;
vector<int> times;

// generate vertices for longitude & latitude lines
void longlat() {
    longitude_points.clear();
    latitude_points.clear();
    // round to closest multiple of 40
    int longitude = round((curr_position.x) / 40) * 40;
    int latitude = round((curr_position.z) / 40) * 40;
    for (int i = -1000; i <= 1000; i += 40) {
        longitude_points.emplace_back(longitude + i, 0, latitude - 1000);

        longitude_points.emplace_back(longitude + i, 0, latitude + 1000);

        latitude_points.emplace_back(longitude - 1000, 0, latitude + i);

        latitude_points.emplace_back(longitude + 1000, 0, latitude + i);
    }
    glBindBuffer(GL_ARRAY_BUFFER, longitude_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * longitude_points.size(),
                    longitude_points.data());

    glBindBuffer(GL_ARRAY_BUFFER, latitude_buffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * latitude_points.size(),
                    latitude_points.data());
}

// generate vertices for border of the route
void route() {
    for (int i = 0; i <= 90; ++i) {
        float radian = i * M_PI / 180.0;
        route_points.emplace_back(1000 - cos(radian) * 950, 0,
                                  -sin(radian) * 950);
        route_colors.emplace_back(0.22, 0.48, 0.28, 0.5);

        route_points.emplace_back(1000 - cos(radian) * 1050, 0,
                                  -sin(radian) * 1050);
        route_colors.emplace_back(0.22, 0.48, 0.28, 0.5);
    }
}

// generate 3 axis: 6 vertices
void axis() {
    // x-axis
    axis_points.emplace_back(0, 0, 0);
    axis_colors.emplace_back(1, 0, 0, 1);
    axis_points.emplace_back(1000, 0, 0);
    axis_colors.emplace_back(1, 0, 0, 1);

    // y-axis
    axis_points.emplace_back(0, 0, 0);
    axis_colors.emplace_back(1, 0, 1, 1);
    axis_points.emplace_back(0, 1000, 0);
    axis_colors.emplace_back(1, 0, 1, 1);

    // z-axis
    axis_points.emplace_back(0, 0, 0);
    axis_colors.emplace_back(0, 0, 1, 1);
    axis_points.emplace_back(0, 0, 1000);
    axis_colors.emplace_back(0, 0, 1, 1);
}

void read_data() {
    fstream coordinateStream("data.txt");
    coordinateStream.clear();
    positions.clear();
    velocities.clear();
    times.clear();
    float x_pos, z_pos, x_velo, z_velo;
    int curr_time;
    for (int i = 0; i < 10; ++i) {
        coordinateStream >> x_pos >> z_pos >> x_velo >> z_velo >> curr_time;
        positions.emplace_back(x_pos, 0.0, z_pos);
        velocities.emplace_back(x_velo, 0.0, z_velo);
        times.emplace_back(curr_time);
    }
    coordinateStream.close();
}

// OpenGL initialization
void init() {
    gentestcoordinate();
    read_data();

    // Create and initialize a vertex buffer object for lines of longitude, to
    // be used in display()
    glGenBuffers(1, &longitude_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, longitude_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vec3) * longitude_points.size() +
                     sizeof(vec4) * longitude_colors.size(),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * longitude_points.size(),
                    sizeof(vec4) * longitude_colors.size(),
                    longitude_colors.data());

    // Create and initialize a vertex buffer object for lines of latitude, to
    // be used in display()
    glGenBuffers(1, &latitude_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, latitude_buffer);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vec3) * latitude_points.size() +
                     sizeof(vec4) * latitude_colors.size(),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * latitude_points.size(),
                    sizeof(vec4) * latitude_colors.size(),
                    latitude_colors.data());

    route();

    // Create and initialize a vertex buffer object for the route, to be used in
    // display()
    glGenBuffers(1, &route_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, route_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vec3) * route_points.size() + sizeof(vec4) * route_colors.size(),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * route_points.size(),
                    route_points.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * route_points.size(),
                    sizeof(vec4) * route_colors.size(), route_colors.data());

    axis();
    // Create and initialize a vertex buffer object for axis, to be used in
    // display()
    glGenBuffers(1, &axis_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, axis_buffer);
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vec3) * axis_points.size() + sizeof(vec4) * axis_colors.size(),
        NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * axis_points.size(),
                    axis_points.data());
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vec3) * axis_points.size(),
                    sizeof(vec4) * axis_colors.size(), axis_colors.data());

    // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader.vert", "fshader.frag");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.529, 0.807, 0.92, 0.0);
}

// draw the object that is associated with the vertex buffer object "buffer"
// and has "num_vertices" vertices.
void drawObj(GLuint buffer, int num_vertices, GLenum mode) {
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0));

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(vec3) * num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)

    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
       (using the attributes specified in each enabled vertex attribute array)
     */
    glDrawArrays(mode, 0, num_vertices);

    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}

void display(void) {
    GLuint model_view;  // model-view matrix uniform shader variable location
    GLuint projection;  // projection matrix uniform shader variable location

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(program);  // Use the shader program

    model_view = glGetUniformLocation(program, "model_view");
    projection = glGetUniformLocation(program, "projection");

    /*---  Set up and pass on Projection matrix to the shader ---*/
    mat4 p = Perspective(fovy, aspect, zNear, zFar);
    // GL_TRUE: matrix is row-major
    glUniformMatrix4fv(projection, 1, GL_TRUE, p);

    /*---  Set up and pass on Model-View matrix to the shader ---*/
    vec4 eye(curr_position + vec3(0.0, 50, 0.0));
    float eye_to_at = 50 / tan(17 * M_PI / 180.0);  // downward at 17 degrees
    vec4 at(eye.x + eye_to_at * camera_direction.x, 0.0,
            eye.z + eye_to_at * camera_direction.z, 1.0);
    vec4 up(0.0, 1.0, 0.0, 0.0);

    mat4 mv = LookAt(eye + offset, at, up);

    /*----- Set up the Mode-View matrix -----*/
    // GL_TRUE: matrix is row-major
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);

    // Dynamically generate lines of longitude and latitude
    longlat();

    // draw lines of longitude
    drawObj(longitude_buffer, longitude_points.size(), GL_LINES);

    // draw lines of latitude
    drawObj(latitude_buffer, latitude_points.size(), GL_LINES);

    // draw the route
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawObj(route_buffer, route_points.size(), GL_TRIANGLE_STRIP);

    // draw the axis
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(axis_buffer, axis_points.size(), GL_LINES);

    glutSwapBuffers();
}

void idle() {
    if (curr_count == 900) {
        // start a new cycle
        curr_count = 0;
        curr_position_index = 0;

        curr_position = vec3(0.0, 0.0, 0.0);
        curr_velocity = vec3(0.0, 0.0, -1.0);
        camera_direction = curr_velocity;

        gentestcoordinate();
        read_data();
    }

    if (curr_position_index < times.size() &&
        curr_count == times[curr_position_index]) {
        // expect to receive next piece of data after 90 calls
        vec3 expected_position = positions[curr_position_index] +
                                 90 * velocities[curr_position_index];
        curr_velocity = (expected_position - curr_position) / 90.0;
        ++curr_position_index;
        cout << "Updated Current Velocity: " << curr_velocity << endl;
    }
    camera_direction =
        normalize(curr_velocity) * 0.02 + camera_direction * 0.98;
    curr_position += curr_velocity;
    cout << "Current Position: " << curr_position
         << "Current Velocity: " << curr_velocity << endl;
    ++curr_count;
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 033:  // Escape Key
        case 'q':
        case 'Q':
            exit(EXIT_SUCCESS);
            break;

        case 'X':
            offset[0] += 1.0;
            break;
        case 'x':
            offset[0] -= 1.0;
            break;
        case 'Y':
            offset[1] += 1.0;
            break;
        case 'y':
            offset[1] -= 1.0;
            break;
        case 'Z':
            offset[2] += 1.0;
            break;
        case 'z':
            offset[2] -= 1.0;
            break;
        case 'b':
        case 'B':  // Toggle between animation and non-animation
            animationFlag = 1 - animationFlag;
            if (animationFlag == 1)
                glutIdleFunc(idle);
            else
                glutIdleFunc(NULL);
            break;
    }
    glutPostRedisplay();
}

void menu(int id) {
    if (id == 0) {
        offset = vec4(0.0, 0.0, 0.0, 0.0);
    } else if (id == 1) {
        // start a new cycle
        curr_count = 0;
        curr_position_index = 0;

        curr_position = vec3(0.0, 0.0, 0.0);
        curr_velocity = vec3(0.0, 0.0, -1.0);
        camera_direction = curr_velocity;

        gentestcoordinate();
        read_data();
    } else {
        exit(EXIT_SUCCESS);
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        glutCreateMenu(menu);
        glutAddMenuEntry("Default View", 0);
        glutAddMenuEntry("Reset", 1);
        glutAddMenuEntry("Quit", 2);
        glutAttachMenu(GLUT_LEFT_BUTTON);
    } else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
        animationFlag = 1 - animationFlag;
        if (animationFlag == 1)
            glutIdleFunc(idle);
        else
            glutIdleFunc(NULL);
    }
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    aspect = (GLfloat)width / (GLfloat)height;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
#ifdef __APPLE__  // Enable core profile of OpenGL 3.2 on macOS.
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH |
                        GLUT_3_2_CORE_PROFILE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowSize(1920, 1080);
    glutCreateWindow("AR Navigation");

#ifdef __APPLE__  // on macOS
    // Core profile requires to create a Vertex Array Object (VAO).
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
#else  // on Linux or Windows, we still need glew
    /* Call glewInit() and error checking */
    int err = glewInit();
    if (GLEW_OK != err) {
        printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
        exit(1);
    }
#endif

    // Get info of GPU and supported OpenGL version
    cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
    cout << "OpenGL version supported " << glGetString(GL_VERSION) << endl;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    init();
    glutMainLoop();
    return 0;
}
