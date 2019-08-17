// graphics_engine.h

#ifndef GRAPHICS_ENGINE_HPP
#define GRAPHICS_ENGINE_HPP

#include <QMutex>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QPoint>
#include <QString>
#include <QTimer>
#include <QVector2D>
#include <QVector3D>

#include "game_logic/game_logic.hpp"

typedef struct {
  float begin;
  float end;
} TextureCoords;

class GraphicsEngine final : public QOpenGLWidget,
                             protected QOpenGLExtraFunctions {
  Q_OBJECT
 public:
  GraphicsEngine(QWidget *parent = 0);
  ~GraphicsEngine();

  // QOpenGLWidget reimplemented functions
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

 public slots:
  void execute_frame();

 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

 signals:
  void initialized();

 private:
  float remap(float min_old, float max_old, float min_new, float max_new,
              float value);
  QPointF coords_window_to_game(QPoint mouse_pos);
  void load_textures();
  void generate_buffers();
  void compile_shaders();
  void rebuild_board_params_buffer();
  void rebuild_board_vertex_buffer(int new_width, int new_height);
  void draw_background(bool score_mode, float score_percentage = 0.0f);
  void draw_playfield();
  void draw_title();

  static constexpr int kFPS = 60;
  static constexpr float kTitleRotationRange = 5.0f;
  static constexpr float kTitleRotationPeriod = 5.0f;
  static constexpr float kTitleHoverRange = 0.1f;
  static constexpr float kTitleHoverAt = -0.4f;
  static constexpr float kTitleHoverPeriod = 2;

  GameLogic _game_logic;
  int _game_width;
  int _game_height;

  QTimer _frame_timer;
  int _tick;
  bool _is_initialized;
  int _view_width;
  int _view_height;
  QMutex _opengl_mutex;
  QMatrix4x4 _mat_projection;
  GLuint _background_vao;
  GLuint _background_vbo;
  GLuint _board_vao;
  GLuint _board_vertex_vbo;
  GLuint _board_params_vbo;
  GLuint _vertex_count;
  GLuint _title_vao;
  GLuint _title_vbo;
  std::map<GameBoard::PieceType, TextureCoords> _piece_texture_coords;
  QOpenGLTexture *_pieces_texture;
  QOpenGLTexture *_background_texture;
  QOpenGLTexture *_title_texture;
  QOpenGLShaderProgram _program_background;
  QOpenGLShaderProgram _program_board;
  QOpenGLShaderProgram _program_title;
};

#endif  // GRAPHICS_ENGINE_HPP
