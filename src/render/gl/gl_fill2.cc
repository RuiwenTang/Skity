
#include "src/render/gl/gl_fill2.hpp"

#include "src/geometry/geometry.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {

GLFill2::GLFill2(const Paint &paint, GLVertex2 *gl_vertex)
    : GLPathVisitor(paint, gl_vertex) {}

void GLFill2::HandleMoveTo(const Point &pt) {
  first_pt_ = Vec2{pt};
  first_pt_index_ =
      GetGLVertex()->AddPoint(first_pt_.x, first_pt_.y, 0.f, 1.f, 0.f);
}

void GLFill2::HandleLineTo(const Point &from, const Point &to) {
  Vec2 from_vec2 = Vec2{from};
  Vec2 to_vec2 = Vec2{to};

  if (first_pt_ == from_vec2) {
    first_pt_dir_ = glm::normalize(to_vec2 - from_vec2);
    first_is_line_ = true;
    // first line to no need to generate triangle
    return;
  }

  Orientation orientation = CalculateOrientation(first_pt_, from_vec2, to_vec2);

  if (orientation == Orientation::kLinear) {
    // no need to generate triangle
    return;
  }

  if (prev_pt_index_ < 0) {
    // generate prev pt mesh data
    prev_pt_index_ =
        GetGLVertex()->AddPoint(from_vec2.x, from_vec2.y, 0.f, 1.f, 0.f);
  }

  // the u,v value is update during next triangle generate
  int32_t to_pt_index =
      GetGLVertex()->AddPoint(to_vec2.x, to_vec2.y, 0.f, 1.f, 0.f);

  if (orientation == Orientation::kAntiClockWise) {
    GetGLVertex()->AddFront(first_pt_index_, prev_pt_index_, to_pt_index);
  } else {
    GetGLVertex()->AddBack(first_pt_index_, prev_pt_index_, to_pt_index);
  }

  prev_pt_index_ = to_pt_index;
}

void GLFill2::HandleQuadTo(const Point &from, const Point &control,
                           const Point &end) {
  Vec2 from_vec2 = Vec2{from};
  Vec2 control_vec2 = Vec2{control};
  Vec2 end_vec2 = Vec2{end};

  if (from_vec2 != first_pt_) {
    // need to handle outer stencil
    Orientation orientation =
        CalculateOrientation(first_pt_, from_vec2, end_vec2);

    if (orientation != Orientation::kLinear) {
      uint32_t n_start_index =
          GetGLVertex()->AddPoint(from_vec2.x, from_vec2.y, 0.f, 0.f, 0.f);
      uint32_t n_end_index =
          GetGLVertex()->AddPoint(end_vec2.x, end_vec2.y, 0.f, 0.f, 0.f);

      if (orientation == Orientation::kAntiClockWise) {
        GetGLVertex()->AddFront(first_pt_index_, n_start_index, n_end_index);
      } else {
        GetGLVertex()->AddBack(first_pt_index_, n_start_index, n_end_index);
      }
    }
  }

  Orientation quad_orientation =
      CalculateOrientation(from_vec2, control_vec2, end_vec2);

  uint32_t start_index = GetGLVertex()->AddPoint(
      from_vec2.x, from_vec2.y, GLVertex2::FILL_QUAD_IN, 0.f, 0.f);

  uint32_t control_index = GetGLVertex()->AddPoint(
      control_vec2.x, control_vec2.y, GLVertex2::FILL_QUAD_IN, 0.5f, 0.f);

  uint32_t end_index = GetGLVertex()->AddPoint(
      end_vec2.x, end_vec2.y, GLVertex2::FILL_QUAD_IN, 1.f, 1.f);

  if (quad_orientation == Orientation::kAntiClockWise) {
    GetGLVertex()->AddFront(start_index, control_index, end_index);
  } else {
    GetGLVertex()->AddBack(start_index, control_index, end_index);
  }
}

void GLFill2::HandleClose() {
  if (first_pt_index_ < 0 || prev_pt_index_ < 0) {
    // this case, path can not generate triangle
    return;
  }
}

void GLFill2::HandleFinish(GLMeshRange *range) {}

}  // namespace skity
