// GraphicsEngine.cpp
#include "graphics_engine.hpp"

#include <QMouseEvent>
#include <QMutexLocker>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#include <QTemporaryFile>
#include <QVector2D>
#include <QVector3D>

#ifdef ANDROID
#include <GLES3/gl3.h>
#endif

GraphicsEngine::GraphicsEngine(QWidget *parent)
    : QOpenGLWidget(parent),
      _game_logic(),
      _game_width(1),
      _game_height(1),
      _frame_timer(),
      _tick(0),
      _is_initialized(false),
      _view_width(1),
      _view_height(1),
      _opengl_mutex(QMutex::Recursive) {
  Q_INIT_RESOURCE(GL_shaders);

  _frame_timer.setInterval(1.0f / kFPS);
  connect(&_frame_timer, SIGNAL(timeout()), this, SLOT(ExecuteFrame()));
  _frame_timer.start();
}

GraphicsEngine::~GraphicsEngine() { ; }

QSize GraphicsEngine::minimumSizeHint() const { return QSize(600, 600); }

QSize GraphicsEngine::sizeHint() const { return QSize(600, 600); }

void GraphicsEngine::ExecuteFrame() {
  _game_logic.PhysicsTick();
  ++_tick;
  update();
}

void GraphicsEngine::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QPointF mouse_coords = CoordsWindowToGame(event->pos());
    _game_logic.MouseClick(mouse_coords.x(), mouse_coords.y());
  }
}

void GraphicsEngine::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons().testFlag(Qt::LeftButton)) {
    QPointF mouse_coords = CoordsWindowToGame(event->pos());
    _game_logic.MouseMove(mouse_coords.x(), mouse_coords.y());
  }
}

void GraphicsEngine::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QPointF mouse_coords = CoordsWindowToGame(event->pos());
    _game_logic.MouseRelease(mouse_coords.x(), mouse_coords.y());
  }
}

float GraphicsEngine::Remap(float min_old, float max_old, float min_new,
                            float max_new, float value) {
  return (min_new +
          (value - min_old) * (max_new - min_new) / (max_old - min_old));
}

QPointF GraphicsEngine::CoordsWindowToGame(QPoint mouse_pos) {
  _game_width = _game_logic.width();
  _game_height = _game_logic.height();

  int max_size = std::max(_game_width, _game_height);
  float mouse_x = Remap(0, _view_width, 0, max_size, mouse_pos.x());
  float mouse_y = max_size - Remap(0, _view_height, 0, max_size, mouse_pos.y());

  float x_centering_offset = ((max_size - _game_width) / 2);
  float y_centering_offset = ((max_size - _game_height) / 2);
  mouse_x -= x_centering_offset;
  mouse_y -= y_centering_offset;

  return QPointF(mouse_x, mouse_y);
}

void GraphicsEngine::initializeGL() {
  _opengl_mutex.lock();
  initializeOpenGLFunctions();

  glClearColor(1, 0.5, 1, 1.0f);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  LoadTextures();
  CompileShaders();
  GenerateBuffers();
  _board_renderer.Init();
  // setup projection matrix
  _mat_projection.ortho(-2.0f, +2.0f, -2.0f, +2.0f, 1.0f, 25.0f);

  _is_initialized = true;
  _opengl_mutex.unlock();

  emit Initialized();
}

void GraphicsEngine::LoadTextures() {
  _background_texture = new QOpenGLTexture(QImage(":/images/tux_square.png"));
  _background_texture->setMinificationFilter(QOpenGLTexture::Linear);
  _background_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  _background_texture->setWrapMode(QOpenGLTexture::Repeat);

  _title_texture = new QOpenGLTexture(QImage(":/images/title.png"));
  _title_texture->setMinificationFilter(QOpenGLTexture::Linear);
  _title_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  _title_texture->setWrapMode(QOpenGLTexture::Repeat);
}

void GraphicsEngine::GenerateBuffers() {
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
    glUniform1i(tex_uniform, GL_TEXTURE0);
  }
  // setup title vao
  {
    glGenVertexArrays(1, &_title_vao);
    glGenBuffers(1, &_title_vbo);

    glBindVertexArray(_title_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _title_vbo);

    int pos_location = _program_title.attributeLocation("pos");
    glVertexAttribPointer(pos_location, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(pos_location);

    int tex_location = _program_title.attributeLocation("tex");
    glVertexAttribPointer(tex_location, 2, GL_FLOAT, GL_FALSE,
                          4 * sizeof(float),
                          reinterpret_cast<void *>(2 * sizeof(float)));
    glEnableVertexAttribArray(tex_location);

    // fill buffer with data
    GLfloat interleaved_title_buff[6 * 4] = {
        -0.8, 0.2,   // poly 1 a
        0.0,  0.0,   // poly 1 a tex
        -0.8, -0.2,  // poly 1 b
        0.0,  1.0,   // poly 1 b tex
        0.8,  0.2,   // poly 1 c
        1.0,  0.0,   // poly 1 c tex
        0.8,  0.2,   // poly 2 a
        1.0,  0.0,   // poly 2 a tex
        -0.8, -0.2,  // poly 2 b
        0.0,  1.0,   // poly 2 b tex
        0.8,  -0.2,  // poly 2 c
        1.0,  1.0    // poly 2 c tex
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, interleaved_title_buff,
                 GL_STATIC_DRAW);

    // bind texture
    int tex_uniform = _program_title.uniformLocation("tex_title");
    glUniform1i(tex_uniform, GL_TEXTURE0);
  }
  _opengl_mutex.unlock();
}

void GraphicsEngine::CompileShaders() {
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
  // title shaders
  {
    QFile vs_file(":/GL_shaders/title_vs.glsl");
    QFile fs_file(":/GL_shaders/title_fs.glsl");
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

    _program_title.addShaderFromSourceCode(QOpenGLShader::Vertex, vs_source);
    _program_title.addShaderFromSourceCode(QOpenGLShader::Fragment, fs_source);
    _program_title.link();
  }
  _opengl_mutex.unlock();
}

void GraphicsEngine::resizeGL(int width, int height) {
  _opengl_mutex.lock();
  _view_width = width;
  _view_height = height;
  glViewport(0, 0, width, height);

  _opengl_mutex.unlock();
}

void GraphicsEngine::paintGL() {
  if (_is_initialized) {
    _opengl_mutex.lock();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float score = static_cast<float>(_game_logic.score()) /
                  static_cast<float>(_game_logic.goal());
    switch (_game_logic.state()) {
      case GameLogic::kPlaying: {
        DrawBackground(true, score);
        _board_renderer.Render(_game_logic.board());
        break;
      }
      case GameLogic::kPaused: {
        DrawBackground(false);
        DrawTitle();
        break;
      }
      case GameLogic::kLevelComplete: {
        DrawBackground(true, score);
        _board_renderer.Render(_game_logic.board());
        DrawTitle();
        break;
      }
    }

    _opengl_mutex.unlock();
  }
}

void GraphicsEngine::DrawBackground(bool score_mode, float score_percentage) {
  _opengl_mutex.lock();

  glBindVertexArray(_background_vao);
  _program_background.bind();
  _program_background.setUniformValue("score_mode", score_mode);
  _program_background.setUniformValue("score", score_percentage);
  _background_texture->bind(GL_TEXTURE0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  _background_texture->release();
  _program_background.release();
  glBindVertexArray(0);

  _opengl_mutex.unlock();
}

void GraphicsEngine::DrawTitle() {
  float hover = sin(static_cast<float>(_tick) / (kTitleHoverPeriod * kFPS)) *
                kTitleHoverRange;
  hover += kTitleHoverAt;
  float angle = sin(static_cast<float>(_tick) / (kTitleRotationPeriod * kFPS)) *
                kTitleRotationRange;

  QMatrix4x4 transform;
  transform.translate(0, hover);
  transform.rotate(angle, 0.0f, 0.0f, 1.0f);

  _opengl_mutex.lock();

  glBindVertexArray(_title_vao);
  _program_title.bind();
  _program_title.setUniformValue("transform", transform);
  _title_texture->bind(GL_TEXTURE0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  _title_texture->release();
  _program_title.release();
  glBindVertexArray(0);

  _opengl_mutex.unlock();
}
