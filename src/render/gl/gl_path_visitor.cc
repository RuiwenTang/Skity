#include "src/render/gl/gl_path_visitor.hpp"

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/math.hpp"
#include "src/render/gl/gl_vertex.hpp"

namespace skity {
static bool triangle_is_front(uint32_t v1, uint32_t v2, uint32_t v3,
                              GLVertex* gl_vertex) {
  if (v1 == v2) {
    // FIXME: moveTo quadTo
    return true;
  }
  auto v1_data = gl_vertex->GetVertex(v1);
  auto v2_data = gl_vertex->GetVertex(v2);
  auto v3_data = gl_vertex->GetVertex(v3);

  return CalculateOrientation(glm::vec2(v1_data[0], v1_data[1]),
                              glm::vec2(v2_data[0], v2_data[1]),
                              glm::vec2(v3_data[0], v3_data[1])) !=
         Orientation::kClockWise;
}

static void append_triangle(uint32_t v1, uint32_t v2, uint32_t v3,
                            GLVertex* gl_vertex) {
  if (v1 == v2 || v2 == v3) {
    return;
  }

  if (triangle_is_front(v1, v2, v3, gl_vertex)) {
    gl_vertex->AddFront(v1, v2, v3);
  } else {
    gl_vertex->AddBack(v1, v2, v3);
  }
}

static uint32_t append_quad(uint32_t start_point_index,
                            uint32_t previous_point_index,
                            Point const& ctr_point, Point const& end_point,
                            GLVertex* gl_vertex) {
  uint32_t normal_end_point_index = gl_vertex->AddPoint(
      end_point.x, end_point.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

  if (triangle_is_front(start_point_index, previous_point_index,
                        normal_end_point_index, gl_vertex)) {
    gl_vertex->AddFront(start_point_index, previous_point_index,
                        normal_end_point_index);
  } else {
    gl_vertex->AddBack(start_point_index, previous_point_index,
                       normal_end_point_index);
  }

  uint32_t current_point_index = gl_vertex->AddPoint(
      end_point.x, end_point.y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

  auto prev_data = gl_vertex->GetVertex(previous_point_index);

  uint32_t quad_start_index = gl_vertex->AddPoint(
      prev_data[0], prev_data[1], GLVertex::GL_VERTEX_TYPE_QUAD, 0.f, 0.f);
  uint32_t quad_mid_index = gl_vertex->AddPoint(
      ctr_point.x, ctr_point.y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.5f, 0.f);
  uint32_t quad_end_index = gl_vertex->AddPoint(
      end_point.x, end_point.y, GLVertex::GL_VERTEX_TYPE_QUAD, 1.f, 1.f);

  if (triangle_is_front(quad_start_index, quad_mid_index, quad_end_index,
                        gl_vertex)) {
    gl_vertex->AddFront(quad_start_index, quad_mid_index, quad_end_index);
  } else {
    gl_vertex->AddBack(quad_start_index, quad_mid_index, quad_end_index);
  }

  return current_point_index;
}

static uint32_t append_cubic(uint32_t start_point_index,
                             uint32_t previous_point_index,
                             std::array<Point, 4> const& cubic,
                             GLVertex* gl_vertex) {
  uint32_t current_index = gl_vertex->AddPoint(
      cubic[3].x, cubic[3].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

  bool is_front = triangle_is_front(start_point_index, previous_point_index,
                                    current_index, gl_vertex);

  if (is_front) {
    gl_vertex->AddFront(start_point_index, previous_point_index, current_index);
  }

  std::array<skity::Point, 32> sub_cubics;
  SubDividedCubic8(cubic.data(), sub_cubics.data());
  for (uint32_t i = 0; i < 8; i++) {
    std::array<skity::Point, 3> quad;
    skity::CubicToQuadratic(sub_cubics.data() + i * 4, quad.data());
    uint32_t qn_index0 = gl_vertex->AddPoint(
        quad[0].x, quad[0].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
    uint32_t qn_index1 = gl_vertex->AddPoint(
        quad[2].x, quad[2].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

    uint32_t quad_index0 = gl_vertex->AddPoint(
        quad[0].x, quad[0].y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.f, 0.f);
    uint32_t quad_index1 = gl_vertex->AddPoint(
        quad[1].x, quad[1].y, GLVertex::GL_VERTEX_TYPE_QUAD, 0.5f, 0.f);
    uint32_t quad_index2 = gl_vertex->AddPoint(
        quad[2].x, quad[2].y, GLVertex::GL_VERTEX_TYPE_QUAD, 1.f, 1.f);

    if (is_front) {
      gl_vertex->AddFront(start_point_index, qn_index0, qn_index1);
      gl_vertex->AddFront(quad_index0, quad_index1, quad_index2);
    } else {
      gl_vertex->AddBack(start_point_index, qn_index0, qn_index1);
      gl_vertex->AddBack(quad_index0, quad_index1, quad_index2);
    }
  }

  return current_index;
}

void GLPathVisitor::VisitPath(Path const& path, GLVertex* gl_vertex) {
  Path::Iter iter{path, false};

  uint32_t start_point_index = gl_vertex->CurrentIndex();
  uint32_t previous_point_index = start_point_index;
  uint32_t current_point_index = start_point_index;
  for (;;) {
    std::array<Point, 4> pts;
    Path::Verb type = iter.next(pts.data());
    switch (type) {
      case Path::Verb::kMove:
        start_point_index = gl_vertex->AddPoint(
            pts[0].x, pts[0].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);
        previous_point_index = start_point_index;
        current_point_index = start_point_index;
        break;
      case Path::Verb::kLine:
        previous_point_index = current_point_index;
        current_point_index = gl_vertex->AddPoint(
            pts[1].x, pts[1].y, GLVertex::GL_VERTEX_TYPE_NORMAL, 0.f, 0.f);

        append_triangle(start_point_index, previous_point_index,
                        current_point_index, gl_vertex);
        break;
      case Path::Verb::kQuad:
        previous_point_index = current_point_index;
        current_point_index = append_quad(
            start_point_index, previous_point_index, pts[1], pts[2], gl_vertex);
        break;
      case Path::Verb::kCubic:
        previous_point_index = current_point_index;
        current_point_index = append_cubic(
            start_point_index, previous_point_index, pts, gl_vertex);
        break;
      case Path::Verb::kConic: {
        std::array<Point, 5> quads;
        Conic conic{pts[0], pts[1], pts[2], iter.conicWeight()};
        conic.chopIntoQuadsPOW2(quads.data(), 1);
        previous_point_index = current_point_index;
        current_point_index =
            append_quad(start_point_index, previous_point_index, quads[1],
                        quads[2], gl_vertex);
        previous_point_index = current_point_index;
        current_point_index =
            append_quad(start_point_index, previous_point_index, quads[3],
                        quads[4], gl_vertex);
      } break;
      case Path::Verb::kClose:
        previous_point_index = current_point_index;
        current_point_index = start_point_index;
        append_triangle(start_point_index, previous_point_index,
                        current_point_index, gl_vertex);
        break;
      case Path::Verb::kDone:
        goto DONE;
        break;
      default:
        break;
    }
  }
DONE:
  return;
}

}  // namespace skity