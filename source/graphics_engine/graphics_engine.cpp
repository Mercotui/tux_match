// GraphicsEngine.cpp

#include "graphics_engine.hpp"

#include <math.h>
#include <QMouseEvent>
#include <QMutexLocker>
#include <QOpenGLExtraFunctions>
#include <QOpenGLTexture>
#include <QTemporaryFile>
#include <QVector2D>
#include <QVector3D>

#include <iostream>

#ifdef ANDROID
#include <GLES3/gl3.h>
#endif

GraphicsEngine::GraphicsEngine(QWidget *parent)
    : QOpenGLWidget(parent),
      _game_logic(),
      _game_width(1),
      _game_height(1),
      _frame_timer(),
      _is_initialized(false),
      _view_width(1),
      _view_height(1),
      _opengl_mutex(QMutex::Recursive),
      _vertex_count(0) {
  Q_INIT_RESOURCE(GL_shaders);

  _frame_timer.setInterval(20);
  connect(&_frame_timer, SIGNAL(timeout()), this, SLOT(execute_frame()));
  _frame_timer.start();
}

GraphicsEngine::~GraphicsEngine() { ; }

QSize GraphicsEngine::minimumSizeHint() const { return QSize(600, 600); }

QSize GraphicsEngine::sizeHint() const { return QSize(600, 600); }

void GraphicsEngine::execute_frame() {
  _game_logic.physics_tick();
  update();
}

void GraphicsEngine::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QPointF mouse_coords = coords_window_to_game(event->pos());
    _game_logic.mouse_click(mouse_coords.x(), mouse_coords.y());
  }
}
void GraphicsEngine::mouseMoveEvent(QMouseEvent *event) {
  if (event->buttons().testFlag(Qt::LeftButton)) {
    QPointF mouse_coords = coords_window_to_game(event->pos());
    _game_logic.mouse_move(mouse_coords.x(), mouse_coords.y());
  }
}
void GraphicsEngine::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    QPointF mouse_coords = coords_window_to_game(event->pos());
    _game_logic.mouse_release(mouse_coords.x(), mouse_coords.y());
  }
}

float GraphicsEngine::remap(float min_old, float max_old, float min_new,
                            float max_new, float value) {
  return (min_new +
          (value - min_old) * (max_new - min_new) / (max_old - min_old));
}

QPointF GraphicsEngine::coords_window_to_game(QPoint mouse_pos) {
  int max_size = std::max(_game_width, _game_height);
  float mouse_x = remap(0, _view_width, 0, max_size, mouse_pos.x());
  float mouse_y = max_size - remap(0, _view_height, 0, max_size, mouse_pos.y());

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

  load_textures();
  compile_shaders();
  generate_buffers();

  // setup projection matrix
  _mat_projection.ortho(-2.0f, +2.0f, -2.0f, +2.0f, 1.0f, 25.0f);

  _is_initialized = true;
  _opengl_mutex.unlock();

  emit initialized();
}

void GraphicsEngine::load_textures() {
  _background_texture = new QOpenGLTexture(QImage(":/images/tux_square.png"));
  _background_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  _background_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  _background_texture->setWrapMode(QOpenGLTexture::Repeat);

  _pieces_texture = new QOpenGLTexture(QImage(":/images/pieces.png"));
  _pieces_texture->setMinificationFilter(QOpenGLTexture::Nearest);
  _pieces_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  _pieces_texture->setWrapMode(QOpenGLTexture::Repeat);

  // build piece coords map
  float piece_width = 1.0f / 4;
  _piece_texture_coords[GameLogic::kCHAMELEON].begin = piece_width * 0;
  _piece_texture_coords[GameLogic::kCHAMELEON].end = piece_width * 1;
  _piece_texture_coords[GameLogic::kTUX].begin = piece_width * 1;
  _piece_texture_coords[GameLogic::kTUX].end = piece_width * 2;
  _piece_texture_coords[GameLogic::kHAT].begin = piece_width * 2;
  _piece_texture_coords[GameLogic::kHAT].end = piece_width * 3;
  _piece_texture_coords[GameLogic::kWILDEBEEST].begin = piece_width * 3;
  _piece_texture_coords[GameLogic::kWILDEBEEST].end = piece_width * 4;
}

void GraphicsEngine::generate_buffers() {
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

  // setup board vao
  {
    glGenVertexArrays(1, &_board_vao);
    glGenBuffers(1, &_board_vertex_vbo);
    glGenBuffers(1, &_board_params_vbo);

    glBindVertexArray(_board_vao);
    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, _board_vertex_vbo);

    int pos_location = _program_board.attributeLocation("position");
    glVertexAttribPointer(pos_location, 2, GL_FLOAT, GL_FALSE,
                          2 * sizeof(float), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(pos_location);

    // texture and offset buffer
    glBindBuffer(GL_ARRAY_BUFFER, _board_params_vbo);

    size_t buffer_element_size = (5 * sizeof(float)) + sizeof(int);
    void *tex_offset = reinterpret_cast<void *>(0);
    void *offset_offset = reinterpret_cast<void *>((sizeof(float) * 2));
    void *is_gold_offset = reinterpret_cast<void *>((sizeof(float) * 5));

    int tex_location = _program_board.attributeLocation("tex");
    glVertexAttribPointer(tex_location, 2, GL_FLOAT, GL_FALSE,
                          buffer_element_size, tex_offset);
    glEnableVertexAttribArray(tex_location);

    int offset_location = _program_board.attributeLocation("offset");
    glVertexAttribPointer(offset_location, 3, GL_FLOAT, GL_FALSE,
                          buffer_element_size, offset_offset);
    glEnableVertexAttribArray(offset_location);

    int is_gold_location = _program_board.attributeLocation("is_gold");
    glVertexAttribPointer(is_gold_location, 1, GL_FLOAT, GL_FALSE,
                          buffer_element_size, is_gold_offset);
    glEnableVertexAttribArray(offset_location);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    int tex_uniform = _program_board.uniformLocation("u_tex_background");
    glUniform1i(tex_uniform, GL_TEXTURE0);
    glBindVertexArray(0);
  }
  _opengl_mutex.unlock();
}

void GraphicsEngine::compile_shaders() {
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
  // gameboard shaders
  {
    QFile vs_file(":/GL_shaders/gamepiece_vs.glsl");
    QFile fs_file(":/GL_shaders/gamepiece_fs.glsl");
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

    _program_board.addShaderFromSourceCode(QOpenGLShader::Vertex, vs_source);
    _program_board.addShaderFromSourceCode(QOpenGLShader::Fragment, fs_source);
    _program_board.link();
  }
  _opengl_mutex.unlock();
}

void GraphicsEngine::resizeGL(int width, int height) {
  _opengl_mutex.lock();
  _view_width = width;
  _view_height = height;
  glViewport(0, 0, width, height);

  _mat_projection.setToIdentity();
  // TODO: replace with perspective, or possibly intrinsic camera matrix
  _mat_projection.ortho(-2.0f, +2.0f, -2.0f, +2.0f, 1.0f, 25.0f);
  _opengl_mutex.unlock();
}

void GraphicsEngine::paintGL() {
  if (_is_initialized) {
    _opengl_mutex.lock();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw background
    _program_background.bind();
    _program_background.setUniformValue("score_mode", 1);
    float score = static_cast<float>(_game_logic.score()) /
                  static_cast<float>(_game_logic.goal());
    _program_background.setUniformValue("score", score);
    draw_background();

    // draw object
    // QMatrix4x4 mat_modelview;
    // mat_modelview.translate(_x_pos, _y_pos, -10.0);
    // mat_modelview.scale(_scale_factor);
    // mat_modelview.rotate(_x_rot, 1, 0, 0);
    // mat_modelview.rotate(_y_rot, 0, 1, 0);
    // mat_modelview.rotate(_z_rot, 0, 0, 1);
    // mat_modelview = _mat_projection * mat_modelview;

    // _program_board.bind();
    // _program_board.setUniformValue("view_matrix", mat_modelview);
    draw_playfield();
    _opengl_mutex.unlock();
  }
}

void GraphicsEngine::draw_playfield() {
  _opengl_mutex.lock();

  int new_width = _game_logic.width();
  int new_height = _game_logic.height();
  if (new_width != _game_width || new_height != _game_height) {
    rebuild_board_vertex_buffer(new_width, new_height);
    _game_width = new_width;
    _game_height = new_height;
  }

  rebuild_board_params_buffer();

  glBindVertexArray(_board_vao);
  _program_board.bind();
  _pieces_texture->bind(GL_TEXTURE0);
  glDrawArrays(GL_TRIANGLES, 0, _vertex_count);
  _pieces_texture->release();
  _program_board.release();
  glBindVertexArray(0);

  _opengl_mutex.unlock();
}

void GraphicsEngine::rebuild_board_vertex_buffer(int new_width,
                                                 int new_height) {
  int max_size = std::max(new_width, new_height);
  float piece_size = 2.0f / max_size;
  float x_centering_offset = ((piece_size * new_width) / 2);
  float y_centering_offset = ((piece_size * new_height) / 2);

  // Create a buffer of the vertices for each tile.
  // To draw tile ABCD, we draw two triangles ACB and ADC
  //   A*******B
  //   * *     *
  // ^ *   *   *
  // | *     * *
  // y D*******C
  //   x ->

  _vertex_count = 0;
  std::vector<float> vertices;
  for (int x = 0; x < new_width; x++) {
    for (int y = 0; y < new_height; y++) {
      _vertex_count += 6;
      float Dx = (piece_size * x) - x_centering_offset;
      float Dy = (piece_size * y) - y_centering_offset;
      float Cx = Dx + piece_size;
      float Cy = Dy;
      float Bx = Cx;
      float By = Cy + piece_size;
      float Ax = Dx;
      float Ay = By;

      std::vector<float> triangle_ACB = {Ax, Ay, Cx, Cy, Bx, By};
      std::vector<float> triangle_ADC = {Ax, Ay, Dx, Dy, Cx, Cy};
      vertices.insert(vertices.end(), triangle_ACB.begin(), triangle_ACB.end());
      vertices.insert(vertices.end(), triangle_ADC.begin(), triangle_ADC.end());
    }
  }

  _opengl_mutex.lock();
  glBindBuffer(GL_ARRAY_BUFFER, _board_vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  _opengl_mutex.unlock();
}

void GraphicsEngine::rebuild_board_params_buffer() {
  int max_size = std::max(_game_logic.width(), _game_logic.height());

  std::vector<float> pieces_interleaved;
  auto game_board = _game_logic.game_board();
  for (auto column : game_board) {
    for (auto piece : column) {
      // each piece consists of 6 vertices
      float offset_x = remap(-max_size, max_size, -2.0f, 2.0f, piece.offset_x);
      float offset_y = remap(-max_size, max_size, -2.0f, 2.0f, piece.offset_y);
      float offset_z = 0;
      float is_gold = 0.0f;
      TextureCoords tex_coords = _piece_texture_coords[piece.type];
      float Dx = tex_coords.begin;
      float Dy = 1.0f;
      float Cx = tex_coords.end;
      float Cy = 1.0f;
      float Bx = tex_coords.end;
      float By = 0.0f;
      float Ax = tex_coords.begin;
      float Ay = 0.0f;

      std::vector<float> a = {Ax, Ay, offset_x, offset_y, offset_z, is_gold};
      std::vector<float> b = {Bx, By, offset_x, offset_y, offset_z, is_gold};
      std::vector<float> c = {Cx, Cy, offset_x, offset_y, offset_z, is_gold};
      std::vector<float> d = {Dx, Dy, offset_x, offset_y, offset_z, is_gold};

      // ACB ADC
      pieces_interleaved.insert(pieces_interleaved.end(), a.begin(), a.end());
      pieces_interleaved.insert(pieces_interleaved.end(), c.begin(), c.end());
      pieces_interleaved.insert(pieces_interleaved.end(), b.begin(), b.end());
      pieces_interleaved.insert(pieces_interleaved.end(), a.begin(), a.end());
      pieces_interleaved.insert(pieces_interleaved.end(), d.begin(), d.end());
      pieces_interleaved.insert(pieces_interleaved.end(), c.begin(), c.end());
    }
  }

  glBindBuffer(GL_ARRAY_BUFFER, _board_params_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pieces_interleaved.size(),
               pieces_interleaved.data(), GL_STREAM_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GraphicsEngine::draw_background() {
  _opengl_mutex.lock();

  glBindVertexArray(_background_vao);
  _program_background.bind();
  _background_texture->bind(GL_TEXTURE0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  _background_texture->release();
  _program_background.release();
  glBindVertexArray(0);

  _opengl_mutex.unlock();
}
