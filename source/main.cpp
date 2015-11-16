#include "PrettyGLUT.hpp"
#include "WorldObjects.hpp"

#include <fstream>

CallListObject roomFloor;
CallListObject vulcano;

paone::Object venus;
paone::Object temple;
PointLight light;

Incallidus inc;

Texture grass;
Texture skybox;

Incallidus enemies[2];

float vulHeight     = 50.0f;
float vulBaseRadius = 30.0f;

// Returns a copy of 'str' with leading and trailing whitespace removed.
std::string trim(std::string str) {
    auto pred = std::ptr_fun<int, int>(std::isspace);
    // From the left.
    str.erase(str.begin(), std::find_if_not(str.begin(), str.end(), pred));
    // From the right.
    str.erase(std::find_if_not(str.rbegin(), str.rend(), pred).base(),
              str.end());
    return str;
}

// This function is expected by PrettyGLUT, because I designed it to get
// done fast, not smart. We can change this later, but this makes sure it
// builds.
// It takes in t and dt, the time and time since the last updateScene was
// called.
void updateScene(double t, double dt) {
    // Even though they're rendered, the cameras are NOT in the drawn list, so
    // we have to update them manually, if we want them updated at all.
    activeCam->update(t, dt);
    // activeCam->doWASDControls(25.0, keyPressed, true);

    wigglyShader.attachUniform("time", 1000.0 + t);

    inc.moveTo(clamp(inc.pos(), Vec(-100, 0.5, -100), Vec(100, 0.5, 100)));

    for (WorldObject *wo : drawn) {
        wo->update(t, dt);
    }
}

void initScene() {
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Color(1.0, 1.0, 1.0).v);

    // Global constructors do weird things.
    inc = Incallidus();

    // Light
    drawn.push_back(&light);
    light.enable();
    light.moveToY(5.0);
    light.setUpdateFunc([&](double t, double /*dt*/) {
        t /= 5.0;
        auto color
            = 0.3 * Color(cos(3.0 * t), cos(5.0 * t), cos(1.0 * t)) + 0.6;
        light.diffuse(color.v);
        light.specular(color.v);
    });

    // Floor
    drawn.push_back(&roomFloor);
    roomFloor = CallListObject([&](GLuint dl) {
        glNewList(dl, GL_COMPILE);

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glBindTexture(GL_TEXTURE_2D, grass);

        static const auto halfsize = Vec(100, 100);

        glBegin(GL_QUADS);

        glNormal3f(0.0f, 1.0f, 0.0f);

        glTexCoord2f(-halfsize.x, halfsize.y);
        glVertex3d(-halfsize.x, 0, halfsize.y);

        glTexCoord2f(halfsize.x, halfsize.y);
        glVertex3d(halfsize.x, 0, halfsize.y);

        glTexCoord2f(halfsize.x, -halfsize.y);
        glVertex3d(halfsize.x, 0, -halfsize.y);

        glTexCoord2f(-halfsize.x, -halfsize.y);
        glVertex3d(-halfsize.x, 0, -halfsize.y);

        glEnd();

        glDisable(GL_TEXTURE_2D);
        glEndList();
    });
    glChk();

    // Vulcano
    drawn.push_back(&vulcano);
    vulcano.moveTo(-50, 0, -50);
    vulcano.shader(wigglyShader);

    static auto vulcano_body = gluNewQuadric();
    static auto vulcano_top  = gluNewQuadric();

    vulcano = CallListObject([&](GLuint dl) {
        glNewList(dl, GL_COMPILE);
        glDisable(GL_CULL_FACE);

        pushMatrixAnd([&]() {
            glRotatef(-90.0f, 1, 0, 0);
            gluCylinder(vulcano_body,
                        vulBaseRadius,
                        vulBaseRadius / 4.0,
                        vulHeight,
                        20,
                        20);
        });

        pushMatrixAnd([&]() {
            glTranslatef(0.0f, vulHeight - 0.25f, 0.0f);
            glRotatef(90.0f, -1, 0, 0);
            gluDisk(vulcano_top, 0, vulBaseRadius / 4.0, 20, 1);
        });

        glEndList();
    });

    // Our Hero!
    drawn.push_back(&inc);
    inc.setUpdateFunc([&](double /*t*/, double /*dt*/) {
        inc.doWASDControls(25.0, keyPressed, false);
    });

    // His enemies!
    for (auto &enemy : enemies) {
        drawn.push_back(&enemy);

        auto pos = 50 * Vec(getRand(), getRand(), getRand());
        enemy.moveTo(pos);
        enemy.setUpdateFunc([&](double /*t*/, double /*dt*/) {
            auto displacement = (inc.pos() - enemy.pos()).normalize();
            enemy.vel(18.0f * displacement);
        });
    }

    // Camera
    activeCam->follow(&inc);
    activeCam->radius(150.0);

    // Venus 1
    auto pt = venus.getLocation();
    assert(pt);
    pt->setY(8.7);
    pt->setX(-50);
    pt->setZ(50);
    venus.loadObjectFile("assets/venus.obj");

    // Temple
    pt = temple.getLocation();
    assert(pt);
    temple.loadObjectFile("assets/temple.obj");
}

void initTextures() {
    grass = SOIL_load_OGL_texture("assets/textures/minecraft.jpg",
                                  SOIL_LOAD_AUTO,
                                  SOIL_CREATE_NEW_ID,
                                  SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
                                      | SOIL_FLAG_NTSC_SAFE_RGB
                                      | SOIL_FLAG_COMPRESS_TO_DXT);
    glChk();
    {
        glBindTexture(GL_TEXTURE_2D, grass);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    glChk();

    skybox = SOIL_load_OGL_texture("assets/textures/clouds-skybox.jpg",
                                   SOIL_LOAD_AUTO,
                                   SOIL_CREATE_NEW_ID,
                                   SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y
                                       | SOIL_FLAG_NTSC_SAFE_RGB
                                       | SOIL_FLAG_COMPRESS_TO_DXT);
    glChk();
    {
        glBindTexture(GL_TEXTURE_2D, skybox);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
    glChk();
}

void initShaders() {
    // Venus
    {
        Shader vert;
        vert.loadFromFile("glsl/wiggly.v.glsl", GL_VERTEX_SHADER);

        Shader frag;
        frag.loadFromFile("glsl/pass_through.f.glsl", GL_FRAGMENT_SHADER);

        wigglyShader.create();
        wigglyShader.attach(vert, frag);
        wigglyShader.link();
    }

    // Lit planes - like the ground!
    {
        Shader vert;
        vert.loadFromFile("glsl/Ground/vert.glsl", GL_VERTEX_SHADER);

        Shader frag;
        frag.loadFromFile("glsl/Ground/frag.glsl", GL_FRAGMENT_SHADER);

        ShaderProgram prog;
        prog.create();
        prog.attach(vert, frag);
        prog.link();

        roomFloor.shader(prog);
    }

    glChk();
}

int main(int argc, char **argv) {
    errno = 0;
    srand(static_cast<unsigned int>(time(nullptr)));

    initGLUT(&argc, argv);
    glewInit();
    printOpenGLInformation();

    initShaders();
    initTextures();
    initScene();

    start();

    return 0;
}
