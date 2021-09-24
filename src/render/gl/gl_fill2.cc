
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
                           const Point &end) {}

void GLFill2::HandleClose() {
  if (first_pt_index_ < 0 || prev_pt_index_ < 0) {
    // this case, path can not generate triangle
    return;
  }
}

void GLFill2::HandleFinish(GLMeshRange *range) {}

}  // namespace skity
