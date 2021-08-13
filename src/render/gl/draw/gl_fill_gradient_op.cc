#include "src/render/gl/draw/gl_fill_gradient_op.hpp"

#include <cstring>

namespace skity {

void GLFillGradientOp::SetPoints(Point const& p1, Point const& p2) {
  this->points_[0].x = p1.x;
  this->points_[0].y = p1.y;
  this->points_[1].x = p2.x;
  this->points_[1].y = p2.y;
}

void GLFillGradientOp::SetRadius(float r1, float r2) {
  this->radius_[0] = r1;
  this->radius_[1] = r2;
}

void GLFillGradientOp::SetColors(std::vector<Vec4> const& colors) {
  this->colors_.resize(colors.size());
  std::memcpy(this->colors_.data(), colors.data(),
              colors.size() * sizeof(Vec4));
}

void GLFillGradientOp::SetStops(std::vector<float> const& stops) {
  if (stops.empty()) {
    this->stops_.clear();
  } else {
    this->stops_.resize(stops.size());
    std::memcpy(this->stops_.data(), stops.data(),
                stops.size() * sizeof(float));
  }
}

void GLFillGradientOp::SetGradientType(Shader::GradientType type) {
  this->type_ = type;
}

void GLFillGradientOp::SetLocalMatrix(Matrix const& matrix) {
  this->local_matrix_ = matrix;
}

void GLFillGradientOp::SetGradientFlag(int32_t flag) {
  this->gradient_flag_ = flag;
}

void GLFillGradientOp::OnBeforeDraw(bool has_clip) {
  GLDrawMeshOpAA::OnBeforeDraw(has_clip);
  // set type
  shader_->SetGradientType(type_);
  // matrixs
  Matrix matrix[2] = {
      local_matrix_,
      CurrentMatrix(),
  };
  shader_->SetMatrixs(matrix);
  // color count
  shader_->SetColorCount(colors_.size());
  // colors
  shader_->SetColors(colors_);
  // stops
  if (!stops_.empty()) {
    shader_->SetStopCount(stops_.size());
    shader_->SetStops(stops_);
  }
  // points
  shader_->SetPoints(points_[0], points_[1]);
  // radius
  shader_->SetRadius(radius_[0], radius_[1]);
  // premulAlpha
  shader_->SetPremulAlphaFlag(gradient_flag_);
}

}  // namespace skity