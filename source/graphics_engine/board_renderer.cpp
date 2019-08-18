#include "board_renderer.hpp"

#include <QTemporaryFile>

BoardRenderer::BoardRenderer() { Q_INIT_RESOURCE(GL_shaders); }

BoardRenderer::~BoardRenderer() {}

void BoardRenderer::Init() {
  initializeOpenGLFunctions();

  LoadTextures();
  CompileShaders();
  GenerateBuffers();
}

void BoardRenderer::Render(const GameBoard &board) {
  int new_width = board.width();
  int new_height = board.height();
  if (new_width != _width || new_height != _height) {
    RebuildBoardVertexBuffer(new_width, new_height);
    _width = new_width;
    _height = new_height;
  }

  RebuildBoardParamsBuffer(board);

  glBindVertexArray(_board_vao);
  _program_board.bind();
  _pieces_texture->bind(GL_TEXTURE0);
  glDrawArrays(GL_TRIANGLES, 0, _vertex_count);
  _pieces_texture->release();
  _program_board.release();
  glBindVertexArray(0);
}

float BoardRenderer::Remap(float min_old, float max_old, float min_new,
                           float max_new, float value) {
  return (min_new +
          (value - min_old) * (max_new - min_new) / (max_old - min_old));
}

void BoardRenderer::LoadTextures() {
  _pieces_texture = new QOpenGLTexture(QImage(":/images/pieces.png"));
  _pieces_texture->setMinificationFilter(QOpenGLTexture::Linear);
  _pieces_texture->setMagnificationFilter(QOpenGLTexture::Linear);
  _pieces_texture->setWrapMode(QOpenGLTexture::Repeat);

  // build piece coords map
  float piece_width = 1.0f / 4;
  _piece_texture_coords[GameBoard::kChameleon].begin = piece_width * 0;
  _piece_texture_coords[GameBoard::kChameleon].end = piece_width * 1;
  _piece_texture_coords[GameBoard::kTux].begin = piece_width * 1;
  _piece_texture_coords[GameBoard::kTux].end = piece_width * 2;
  _piece_texture_coords[GameBoard::kHat].begin = piece_width * 2;
  _piece_texture_coords[GameBoard::kHat].end = piece_width * 3;
  _piece_texture_coords[GameBoard::kWildebeest].begin = piece_width * 3;
  _piece_texture_coords[GameBoard::kWildebeest].end = piece_width * 4;
}

void BoardRenderer::GenerateBuffers() {
  glGenVertexArrays(1, &_board_vao);
  glGenBuffers(1, &_board_vertex_vbo);
  glGenBuffers(1, &_board_params_vbo);

  glBindVertexArray(_board_vao);
  // vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, _board_vertex_vbo);

  int pos_location = _program_board.attributeLocation("position");
  glVertexAttribPointer(pos_location, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                        reinterpret_cast<void *>(0));
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

void BoardRenderer::CompileShaders() {
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

void BoardRenderer::RebuildBoardVertexBuffer(int new_width, int new_height) {
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

  glBindBuffer(GL_ARRAY_BUFFER, _board_vertex_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void BoardRenderer::RebuildBoardParamsBuffer(const GameBoard &board) {
  int max_size = std::max(board.width(), board.height());

  std::vector<float> pieces_interleaved;
  for (auto column : board.board()) {
    for (auto piece : column) {
      // each piece consists of 6 vertices
      float offset_x = Remap(-max_size, max_size, -2.0f, 2.0f, piece.offset_x);
      float offset_y = Remap(-max_size, max_size, -2.0f, 2.0f, piece.offset_y);
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
