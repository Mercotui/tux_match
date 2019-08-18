#ifndef SOURCE_GRAPHICS_ENGINE_GRAPHICS_ENGINE_HPP_
#define SOURCE_GRAPHICS_ENGINE_GRAPHICS_ENGINE_HPP_

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

#include "board_renderer.hpp"
#include "game_logic/game_logic.hpp"

class GraphicsEngine final : public QOpenGLWidget,
                             protected QOpenGLExtraFunctions {
  Q_OBJECT
 public:
  explicit GraphicsEngine(QWidget *parent = 0);
  ~GraphicsEngine();

  // QOpenGLWidget reimplemented functions
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

 public slots:
  void ExecuteFrame();

 protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

 signals:
  void Initialized();

 private:
  float Remap(float min_old, float max_old, float min_new, float max_new,
              float value);
  QPointF CoordsWindowToGame(QPoint mouse_pos);
  void LoadTextures();
  void GenerateBuffers();
  void CompileShaders();
  void DrawBackground(bool score_mode, float score_percentage = 0.0f);
  void DrawTitle();

  static constexpr int kFPS = 60;
  static constexpr float kTitleRotationRange = 5.0f;
  static constexpr float kTitleRotationPeriod = 5.0f;
  static constexpr float kTitleHoverRange = 0.1f;
  static constexpr float kTitleHoverAt = -0.4f;
  static constexpr float kTitleHoverPeriod = 2;

  GameLogic _game_logic;
  BoardRenderer _board_renderer;
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
  GLuint _title_vao;
  GLuint _title_vbo;
  QOpenGLTexture *_background_texture;
  QOpenGLTexture *_title_texture;
  QOpenGLShaderProgram _program_background;
  QOpenGLShaderProgram _program_title;
};

#endif  // SOURCE_GRAPHICS_ENGINE_GRAPHICS_ENGINE_HPP_
