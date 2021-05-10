#include "src/render/join_proc.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/intersect.hpp>

#include "src/geometry/conic.hpp"
#include "src/geometry/math.hpp"
#include "src/geometry/point_priv.hpp"

namespace skity {

#define ONE_OVER_SQRT_2 (0.707106781f)

enum AngleType {
  kNearly180,
  kSharp,
  kShallow,
  kNearlyLine,
};

static AngleType dot_2_angle_type(float dot)
{
  if (dot >= 0) {
    return FloatNearlyZero(Float1 - dot) ? kNearlyLine : kShallow;
  }
  else {
    return FloatNearlyZero(Float1 + dot) ? kNearly180 : kSharp;
  }
}

static void handle_inner_join(Path* inner, Point const& pivot,
                              Vector const& after)
{
//  // FIXME: to solve inner join intersect Point position
//  Point last;
//  inner->getLastPt(std::addressof(last));
//  Point last_prev = inner->getPoint(inner->countPoints() - 2);
//
//  Vector v1 = last - pivot;
//  Vector v2 = -after;
//  Vector v = v1 + v2;
//
//  Vector normal = Vector{-v.y / glm::length(v), v.x / glm::length(v), 0, 0};
//  Vector dir = last_prev - last;
//  dir = dir * (1.f / glm::length(dir));
//
//  float distance;
//  if (glm::intersectRayPlane(last, dir, pivot, normal, distance)) {
//    Point intersect = last + distance * dir;
//    inner->setLastPt(intersect);
//  }

//   inner->lineTo(pivot.x, pivot.y);
//   inner->lineTo(pivot.x - after.x, pivot.y - after.y);
  Point last;
  inner->getLastPt(&last);
  inner->setLastPt(last.x - after.x, last.y - after.y);
}

class BevelJoiner : public JoinProc {
 public:
  BevelJoiner() = default;
  ~BevelJoiner() override = default;

  Paint::Join type() const override { return Paint::Join::kBevel_Join; }

  void proc(Path* outer, Path* inner, Vector const& beforeUnitNormal,
            Point const& pivot, Vector const& afterUnitNormal, float radius,
            float invMiterLimit, bool prevIsLine, bool currIsLine) override
  {
    Vector after{afterUnitNormal.x * radius, afterUnitNormal.y * radius, 0, 0};

    if (CalculateOrientation(beforeUnitNormal, afterUnitNormal) !=
        Orientation::kClockWise) {
      std::swap(outer, inner);
      after = -after;
    }

    outer->lineTo(pivot.x + after.x, pivot.y + after.y);
    handle_inner_join(inner, pivot, after);
  }
};

class RoundJoiner : public JoinProc {
 public:
  RoundJoiner() = default;
  ~RoundJoiner() override = default;

  Paint::Join type() const override { return Paint::Join::kRound_Join; }

  void proc(Path* outer, Path* inner, Vector const& beforeUnitNormal,
            Point const& pivot, Vector const& afterUnitNormal, float radius,
            float invMiterLimit, bool prevIsLine, bool currIsLine) override
  {
//    float dot_prod = glm::dot(beforeUnitNormal, afterUnitNormal);
    float dot_prod = beforeUnitNormal.x * afterUnitNormal.x + beforeUnitNormal.y * afterUnitNormal.y;
    AngleType angle_type = dot_2_angle_type(dot_prod);

    if (angle_type == kNearlyLine) {
      return;
    }

    Vector before = beforeUnitNormal;
    Vector after = afterUnitNormal;
    RotationDirection dir = RotationDirection::kCW;

    if (CalculateOrientation(before, after) != Orientation::kClockWise) {
      std::swap(outer, inner);
      before = -before;
      after = -after;
      dir = RotationDirection::kCCW;
    }

    Matrix scale = glm::scale(glm::identity<Matrix>(), {radius, radius, 1});
    Matrix translate = glm::translate(glm::identity<Matrix>(), {pivot.x, pivot.y, 0});

    Matrix matrix = translate * scale;
    std::array<Conic, Conic::kMaxConicsForArc> conics{};

    int32_t count = Conic::BuildUnitArc(before, after, dir,
                                        std::addressof(matrix), conics.data());

    if (count > 0) {
      for (int32_t i = 0; i < count; i++) {
        outer->conicTo(conics[i].pts[1], conics[i].pts[2], conics[i].w);
      }
      after.x *= radius;
      after.y *= radius;
      handle_inner_join(inner, pivot, after);
    }
  }
};

class MiterJoiner : public JoinProc {
 public:
  MiterJoiner() = default;
  ~MiterJoiner() override = default;

  Paint::Join type() const override { return Paint::Join::kMiter_Join; }

  void proc(Path* outer, Path* inner, Vector const& beforeUnitNormal,
            Point const& pivot, const glm::vec4& afterUnitNormal, float radius,
            float invMiterLimit, bool prevIsLine, bool currIsLine) override
  {
    float dot_prod = glm::dot(beforeUnitNormal, afterUnitNormal);
    AngleType angle_type = dot_2_angle_type(dot_prod);
    Vector before = beforeUnitNormal;
    Vector after = afterUnitNormal;
    Vector mid;
    float sin_half_angle;
    bool ccw;

    if (angle_type == kNearlyLine) {
      return;
    }

    if (angle_type == kNearly180) {
      currIsLine = false;
      goto DO_BLUNT;
    }
    ccw = CalculateOrientation(beforeUnitNormal, afterUnitNormal) !=
          Orientation::kClockWise;
    if (ccw) {
      std::swap(outer, inner);
      before = -before;
      after = -after;
    }

    if (0 == dot_prod && invMiterLimit < ONE_OVER_SQRT_2) {
      mid = (before + after) * radius;
      goto DO_MITER;
    }

    sin_half_angle = glm::sqrt(FloatHalf * (Float1 + dot_prod));
    if (sin_half_angle < invMiterLimit) {
      currIsLine = false;
      goto DO_BLUNT;
    }
    // choose the most accurate way to form the initial mid-vector
    if (angle_type == kSharp) {
      mid = after - before;
      if (ccw) {
        mid = -mid;
      }
    }
    else {
      mid = before + after;
    }

    PointSetLength<false>(mid, mid.x, mid.y, radius / sin_half_angle);
    mid.z = 0;
    mid.w = 0;

  DO_MITER:
    if (prevIsLine) {
      outer->setLastPt(pivot.x + mid.x, pivot.y + mid.y);
    }
    else {
      outer->lineTo(pivot.x + mid.x, pivot.y + mid.y);
    }
  DO_BLUNT:
    handle_inner_join(inner, pivot, after);
    return;
  }
};

std::unique_ptr<JoinProc> JoinProc::MakeJoinProc(Paint::Join join)
{
  if (join == Paint::Join::kBevel_Join) {
    return std::make_unique<BevelJoiner>();
  }
  else if (join == Paint::Join::kRound_Join) {
    return std::make_unique<RoundJoiner>();
  }
  else if (join == Paint::Join::kRound_Join) {
    return std::make_unique<RoundJoiner>();
  }
  else {
    return std::make_unique<MiterJoiner>();
  }
}

}  // namespace skity
