// graphics_engine.cpp

#include "graphics_engine.hpp"

#include <math.h>
#include <QOpenGLExtraFunctions>
#include <QTemporaryFile>
#include <QVector2D>
#include <QVector3D>

#ifdef ANDROID
#include <GLES3/gl3.h>
#endif

graphics_engine::graphics_engine(QWidget *parent)
    : QOpenGLWidget(parent), _opengl_mutex(QMutex::Recursive) {
  Q_INIT_RESOURCE(GL_shaders);
}

graphics_engine::~graphics_engine() {
  _opengl_mutex.lock();
  glDeleteBuffers(1, &_object_vbo);
  glDeleteVertexArrays(1, &_object_vao);
  _opengl_mutex.unlock();
}

QSize graphics_engine::minimumSizeHint() const { return QSize(600, 600); }

QSize graphics_engine::sizeHint() const { return QSize(600, 600); }

void graphics_engine::initializeGL() {
  _opengl_mutex.lock();
  initializeOpenGLFunctions();

  glClearColor(1, 0.5, 1, 1.0f);
  glEnable(GL_DEPTH_TEST);
  // glEnable (GL_CULL_FACE);
  glEnable(GL_TEXTURE_2D);

  glGenTextures(1, &_texture_background);
  glBindTexture(GL_TEXTURE_2D, _texture_background);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);

  // generate a buffer to bind to textures
  glGenFramebuffers(1, &_readback_buffer);

  // compile and link shaders
  compile_shaders();

  // generate vertex array buffers
  generate_buffers();

  // setup projection matrix
  _mat_projection.ortho(-2.0f, +2.0f, -2.0f, +2.0f, 1.0f, 25.0f);

  emit initialized();
  _opengl_mutex.unlock();
}

void graphics_engine::generate_buffers() {
  _opengl_mutex.lock();
  // setup background vao
  {
    glGenVertexArrays(1, &_background_vao);
    glGenBuffers(1, &_background_vbo);

    glBindVertexArray(_background_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _background_vbo);

    int pos_location = _program_background.attributeLocation("position");
    glVertexAttribPointer(pos_location, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(pos_location);

    int tex_location = _program_background.attributeLocation("tex");
    glVertexAttribPointer(tex_location, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(tex_location);

    // fill buffer with data
    GLfloat interleaved_background_buff[6 * 4] = {
        -1.0, 1.0,   // poly 1 a
        0.0,  0.0,   // poly 1 a tex
        -1.0, -1.0,  // poly 1 b
        0.0,  1.0,   // poly 1 b tex
        1.0,  1.0,   // poly 1 c
        1.0,  0.0,   // poly 1 c tex
        1.0,  1.0,   // poly 2 a
        1.0,  0.0,   // poly 2 a tex
        -1.0, -1.0,  // poly 2 b
        0.0,  1.0,   // poly 2 b tex
        1.0,  -1.0,  // poly 2 c
        1.0,  1.0    // poly 2 c tex
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4,
                 interleaved_background_buff, GL_STATIC_DRAW);

    // bind texture
    int tex_uniform = _program_background.uniformLocation("u_tex_background");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _texture_background);
    glUniform1i(tex_uniform, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // setup object vao
  {
    glGenVertexArrays(1, &_object_vao);
    glGenBuffers(1, &_object_vbo);

    glBindVertexArray(_object_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _object_vbo);

    int pos_location = _program_object.attributeLocation("position");
    glVertexAttribPointer(pos_location, 3, GL_FLOAT, GL_FALSE,
                          10 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(pos_location);

    int nor_location = _program_object.attributeLocation("normal");
    glVertexAttribPointer(nor_location, 3, GL_FLOAT, GL_FALSE,
                          10 * sizeof(float),
                          reinterpret_cast<void *>(3 * sizeof(float)));
    glEnableVertexAttribArray(nor_location);

    int col_location = _program_object.attributeLocation("color");
    glVertexAttribPointer(col_location, 4, GL_FLOAT, GL_FALSE,
                          10 * sizeof(float),
                          reinterpret_cast<void *>(6 * sizeof(float)));
    glEnableVertexAttribArray(col_location);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  _opengl_mutex.unlock();
}

void graphics_engine::compile_shaders() {
  _opengl_mutex.lock();
  // background shaders
  {
    QFile vs_file(":/GL_shaders/background_vs.glsl");
    QFile fs_file(":/GL_shaders/background_fs.glsl");
    vs_file.open(QIODevice::ReadOnly);
    fs_file.open(QIODevice::ReadOnly);
    QByteArray vs_source = vs_file.readAll();
    QByteArray fs_source = fs_file.readAll();

    if (QOpenGLContext::currentContext()->isOpenGLES()) {
      vs_source.prepend(QByteArrayLiteral("#version 300 es\n"));
      fs_source.prepend(QByteArrayLiteral("#version 300 es\n"));
    } else {
      vs_source.prepend(QByteArrayLiteral("#version 410\n"));
      fs_source.prepend(QByteArrayLiteral("#version 410\n"));
    }

    _program_background.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                                vs_source);
    _program_background.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                                fs_source);
    _program_background.link();
  }
  // object shaders
  {
    QFile vs_file(":/GL_shaders/object_vs.glsl");
    QFile fs_file(":/GL_shaders/object_fs.glsl");
    vs_file.open(QIODevice::ReadOnly);
    fs_file.open(QIODevice::ReadOnly);
    QByteArray vs_source = vs_file.readAll();
    QByteArray fs_source = fs_file.readAll();

    if (QOpenGLContext::currentContext()->isOpenGLES()) {
      vs_source.prepend(QByteArrayLiteral("#version 300 es\n"));
      fs_source.prepend(QByteArrayLiteral("#version 300 es\n"));
    } else {
      vs_source.prepend(QByteArrayLiteral("#version 410\n"));
      fs_source.prepend(QByteArrayLiteral("#version 410\n"));
    }

    _program_object.addShaderFromSourceCode(QOpenGLShader::Vertex, vs_source);
    _program_object.addShaderFromSourceCode(QOpenGLShader::Fragment, fs_source);
    _program_object.link();
  }
  _opengl_mutex.unlock();
}

void graphics_engine::resizeGL(int width, int height) {
  _opengl_mutex.lock();
  _view_width = width;
  _view_height = height;
  glViewport(0, 0, width, height);

  _mat_projection.setToIdentity();
  // TODO: replace with perspective, or possibly intrinsic camera matrix
  _mat_projection.ortho(-2.0f, +2.0f, -2.0f, +2.0f, 1.0f, 25.0f);
  _opengl_mutex.unlock();
}

void graphics_engine::paintGL() {
  _opengl_mutex.lock();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // draw background
  _program_background.bind();
  _program_background.setUniformValue("is_GLRED", _is_grayscale);
  draw_background();

  // draw object
  QMatrix4x4 mat_modelview;
  mat_modelview.translate(_x_pos, _y_pos, -10.0);
  mat_modelview.scale(_scale_factor);
  mat_modelview.rotate(_x_rot, 1, 0, 0);
  mat_modelview.rotate(_y_rot, 0, 1, 0);
  mat_modelview.rotate(_z_rot, 0, 0, 1);
  mat_modelview = _mat_projection * mat_modelview;

  _program_object.bind();
  _program_object.setUniformValue("view_matrix", mat_modelview);
  draw_playfield();
  _opengl_mutex.unlock();
}

void graphics_engine::draw_playfield() {
  _opengl_mutex.lock();
  // draw the object
  glBindVertexArray(_object_vao);
  glDrawArrays(GL_TRIANGLES, 0, _vertex_count);
  glBindVertexArray(0);
  _opengl_mutex.unlock();
}

void graphics_engine::draw_background() {
  _opengl_mutex.lock();
  // draw the 2 triangles that form the background
  glBindVertexArray(_background_vao);
  glBindTexture(GL_TEXTURE_2D, _current_handle);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  _opengl_mutex.unlock();
}