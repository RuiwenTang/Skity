#include "src/render/stroke.hpp"

#include <array>
#include <skity/geometry/point.hpp>

#include "src/geometry/conic.hpp"
#include "src/geometry/geometry.hpp"
#include "src/geometry/point_priv.hpp"
#include "src/render/path_stroker.hpp"

namespace skity {

enum {
  kTangent_RecursiveLimit,
  kCubic_RecursiveLimit,
  kConic_RecursiveLimit,
  kQuad_RecursiveLimit,
};

static bool points_within_dist(Point const& nearPt, Point const& farPt,
                               float limit) {
  return PointDistanceToSqd(nearPt, farPt) <= limit * limit;
}

static bool sharp_angle(const Point quad[3]) {
  Vector smaller = quad[1] - quad[0];
  Vector larger = quad[1] - quad[2];

  float smallLen = PointLengthSqd(smaller);
  float largerLen = PointLengthSqd(larger);

  if (smallLen > largerLen) {
    std::swap(smaller, larger);
    largerLen = smallLen;
  }

  if (!PointSetLength<false>(smaller, smaller.x, smaller.y, largerLen)) {
    return false;
  }

  float dot = glm::dot(glm::vec2{smaller}, glm::vec2{larger});
  return dot > 0;
}

static int intersect_quad_ray(const Point line[2], const Point quad[3],
                              float roots[2]) {
  Vector vec = line[1] - line[0];
  float r[3];
  for (int n = 0; n < 3; n++) {
    r[n] = (quad[n].y - line[0].y) * vec.x - (quad[n].x - line[0].x) * vec.y;
  }

  float A = r[2];
  float B = r[1];
  float C = r[0];

  A += C - 2 * B;  // A = a - 2 * b + c
  B -= C;          // B = -(b - c)

  return FindUnitQuadRoots(A, 2 * B, C, roots);
}

static bool degenerate_vector(Vector const& v) {
  return !PointCanNormalize(v.x, v.y);
}

/// If src == dst, then we use a temp path to record the stroke, and then swap
/// its contents with src when we're done.
class AutoTmpPath {
 public:
  AutoTmpPath(Path const& src, Path** dst) : src_(src) {
    if (&src == *dst) {
      *dst = std::addressof(tmp_dst_);
      swap_with_src_ = true;
    } else {
      (*dst)->reset();
      swap_with_src_ = false;
    }
  }

  ~AutoTmpPath() {
    if (swap_with_src_) {
      tmp_dst_.swap(*const_cast<Path*>(&src_));
    }
  }

 private:
  Path tmp_dst_;
  Path const& src_;
  bool swap_with_src_;
};

Stroke::Stroke()
    : width_(Float1),
      miter_limit_(Paint::DefaultMiterLimit),
      res_scale_(1.f),
      cap_(Paint::Cap::kDefault_Cap),
      join_(Paint::Join::kDefault_Join),
      do_fill_(false) {}

Stroke::Stroke(Paint const& paint)
    : width_(paint.getStrokeWidth()),
      miter_limit_(paint.getStrokeMiter()),
      res_scale_(1.f),
      cap_(paint.getStrokeCap()),
      join_(paint.getStrokeJoin()),
      do_fill_(paint.getStyle() == Paint::kStrokeAndFill_Style) {}

void Stroke::strokePath(Path const& path, Path* result) const {
  float radius = FloatHalf * width_;

  AutoTmpPath tmp{path, &result};

  if (radius <= 0) {
    return;
  }

  bool ignore_center = do_fill_;

  PathStroker stroker{path,  radius,     miter_limit_, cap_,
                      join_, res_scale_, ignore_center};

  Path::Iter iter{path, false};
  Path::Verb last_segment = Path::Verb::kMove;

  for (;;) {
    std::array<Point, 4> pts;
    switch (iter.next(pts.data())) {
      case Path::Verb::kMove:
        stroker.moveTo(pts[0]);
        break;
      case Path::Verb::kLine:
        stroker.lineTo(pts[1], std::addressof(iter));
        last_segment = Path::Verb::kLine;
        break;
      case Path::Verb::kQuad:
        stroker.quadTo(pts[1], pts[2]);
        last_segment = Path::Verb::kQuad;
        break;
      case Path::Verb::kConic:
        stroker.conicTo(pts[1], pts[2], iter.conicWeight());
        last_segment = Path::Verb::kConic;
        break;
      case Path::Verb::kCubic:
        stroker.cubicTo(pts[1], pts[2], pts[3]);
        last_segment = Path::Verb::kCubic;
        break;
      case Path::Verb::kClose:
        if (cap_ != Paint::kButt_Cap) {
          // If the stroke consists of a moveTo followed by a close, treat it as
          // if it were followed by a zero-length line.
          // Lines without length can have square and round end caps.
          if (stroker.hasOnlyMoveTo()) {
            stroker.lineTo(stroker.moveToPt());
            goto ZERO_LENGTH;
          }
          if (stroker.isCurrentContourEmpty()) {
          ZERO_LENGTH:
            last_segment = Path::Verb::kLine;
            break;
          }
        }
        stroker.close(last_segment == Path::Verb::kLine);
        break;
      case Path::Verb::kDone:
        goto DONE;
    }
  }
DONE:
  stroker.done(result, last_segment == Path::Verb::kLine);

  if (do_fill_ && !ignore_center) {
    // can not reach here
    assert(false);
  }
}

}  // namespace skity
