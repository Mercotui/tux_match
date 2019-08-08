// graphics_engine.h

#ifndef GRAPHICS_ENGINE_HPP
#define GRAPHICS_ENGINE_HPP

#include <QMutex>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>
#include <QString>
#include <QVector2D>
#include <QVector3D>

class graphics_engine final : public QOpenGLWidget,
                              protected QOpenGLExtraFunctions {
  Q_OBJECT
 public:
  graphics_engine(QWidget *parent = 0);
  ~graphics_engine();

  // QOpenGLWidget reimplemented functions
  void initializeGL();
  void resizeGL(int w, int h);
  void paintGL();
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

 public slots:

 signals:
  void initialized();

 private:
  void load_textures();
  void generate_buffers();
  void compile_shaders();
  void draw_playfield();
  void draw_background();

  bool _is_initialized;
  int _view_width;
  int _view_height;
  QMutex _opengl_mutex;
  QMatrix4x4 _mat_projection;
  GLuint _is_grayscale;
  GLuint _current_handle;
  GLuint _last_handle;
  GLuint _readback_buffer;
  GLuint _background_vao;
  GLuint _object_vao;
  GLuint _background_vbo;
  GLuint _object_vbo;
  GLuint _vertex_count;
  QOpenGLTexture *_background_texture;
  QOpenGLShaderProgram _program_background;
  QOpenGLShaderProgram _program_object;
};

#endif  // GRAPHICS_ENGINE_HPP
