#ifndef SOURCE_GRAPHICS_ENGINE_BOARD_RENDERER_HPP_
#define SOURCE_GRAPHICS_ENGINE_BOARD_RENDERER_HPP_

#include <QMatrix4x4>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <map>

#include "game_logic/game_board.hpp"

class BoardRenderer : public QObject, protected QOpenGLExtraFunctions {
  Q_OBJECT

 public:
  typedef struct {
    float begin;
    float end;
  } TextureCoords;

  BoardRenderer();
  ~BoardRenderer();

  void Init();
  void Render(const GameBoard& board);
  void SetProjection(const QMatrix4x4& projection_matrix);

 private:
  float Remap(float min_old, float max_old, float min_new, float max_new,
              float value);
  void LoadTextures();
  void GenerateBuffers();
  void CompileShaders();
  void RebuildBoardParamsBuffer(const GameBoard& board);
  void RebuildBoardVertexBuffer(int new_width, int new_height);

  int _width;
  int _height;
  QMatrix4x4 _projection_matrix;
  GLuint _board_vao;
  GLuint _board_vertex_vbo;
  GLuint _board_params_vbo;
  GLuint _vertex_count;
  std::map<GameBoard::PieceType, TextureCoords> _piece_texture_coords;
  QOpenGLTexture* _pieces_texture;
  QOpenGLShaderProgram _program_board;
};

#endif  // SOURCE_GRAPHICS_ENGINE_BOARD_RENDERER_HPP_
