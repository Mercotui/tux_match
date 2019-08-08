// graphics_engine.h

#ifndef GRAPHICS_ENGINE_HPP
#define GRAPHICS_ENGINE_HPP

#include <QMutex>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
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
  void generate_buffers();
  void compile_shaders();
  void draw_playfield();
  void draw_background();

  int _view_width;
  int _view_height;
  float _scale_factor;
  float _x_pos;
  float _y_pos;
  float _x_rot;
  float _y_rot;
  float _z_rot;
  QMutex _opengl_mutex;
  QMatrix4x4 _mat_projection;
  GLuint _is_grayscale;
  GLuint _texture_background;
  GLuint _current_handle;
  GLuint _last_handle;
  GLuint _readback_buffer;
  GLuint _background_vao;
  GLuint _object_vao;
  GLuint _background_vbo;
  GLuint _object_vbo;
  GLuint _vertex_count;
  QOpenGLShaderProgram _program_background;
  QOpenGLShaderProgram _program_object;
};

#endif // GRAPHICS_ENGINE_HPP
