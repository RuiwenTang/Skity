#ifndef SKITY_SRC_RENDER_PATH_STROKER_HPP
#define SKITY_SRC_RENDER_PATH_STROKER_HPP

#include <memory>
#include <skity/graphic/paint.hpp>
#include <skity/graphic/path.hpp>

namespace skity {

class CapProc;
class Conic;
class JoinProc;
class QuadConstruct;

class PathStroker {
 public:
  enum class StrokeType {
    kOuter,
    kInner,
  };

  enum class ResultType {
    // caller should split the quad stroke in two
    kSplit,
    // caller should add line
    kDegenerate,
    // the caller should (continue to try to) add a quad stroke
    kQuad,
  };

  enum class ReductionType {
    // all curve points are practically identical
    kPoint,
    // the control point is on the line between the ends
    kLine,
    // the control point is outside the line between the ends
    kQuad,
    // the control point is on the line but outside the ends
    kDegenerate,
    kDegenerate2,
    kDegenerate3,
  };

  enum class IntersectRayType {
    kCtrlPt,
    kResult,
  };

  PathStroker(Path const& src, float radius, float miterLimit, Paint::Cap cap,
              Paint::Join join, float resScale, bool canIgnoreCenter);

  bool hasOnlyMoveTo() const { return 0 == segment_count_; }

  Point moveToPt() const { return first_pt_; }

  void moveTo(Point const&);
  void lineTo(Point const&, const Path::Iter* iter = nullptr);
  void quadTo(Point const&, Point const&);
  void conicTo(Point const&, Point const&, float weight);
  void cubicTo(Point const&, Point const&, Point const&);
  void close(bool isLine);

  void done(Path* dst, bool isLine);

  float getResScale() const { return res_scale_; }

  bool isCurrentContourEmpty() const
  {
    return inner_.isZeroLengthSincePoint(0) &&
           outer_.isZeroLengthSincePoint(first_outer_pt_index_in_contour_);
  }

 private:
  void finishContour(bool close, bool isLine);
  void addDegenerateLine(const QuadConstruct*);

  static ReductionType CheckConicLinear(Conic const&, Point* reduction);
  static ReductionType CheckCubicLinear(const Point cubic[4],
                                        Point reduction[3],
                                        const Point** tanPtPtr);
  static ReductionType CheckQuadLinear(const Point quad[3], Point* reduction);
  static bool SlightAngle(QuadConstruct*);

  ResultType compareQuadConic(Conic const& conic, QuadConstruct*);
  ResultType compareQuadCubic(const Point cubic[4], QuadConstruct*);
  ResultType compareQuadQuad(const Point quad[3], QuadConstruct*);

  void conicPrepRay(Conic const&, float, Point*, Point*, Point* tangent) const;
  void conicQuadEnds(Conic const&, QuadConstruct*) const;
  bool conicStroke(Conic const&, QuadConstruct*);

  bool cubicMidOnLine(const Point* cubic, const QuadConstruct* quadPts) const;
  void cubicPerpRay(const Point cubic[4], float, Point*, Point*, Point*) const;
  void cubicQuadEnds(const Point cubic[4], QuadConstruct*);
  void cubicQuadMid(const Point cubic[4], const QuadConstruct*,
                    Point* mid) const;
  void cubicStroke(const Point cubic[4], QuadConstruct*);

  void init(StrokeType strokeType, QuadConstruct*, float tStart, float tEnd);

  bool ptInQuadBounds(const Point quad[3], Point const& pt) const;
  void quadPerpRay(const Point quad[3], float, Point*, Point*, Point*) const;
  void quadStroke(const Point quad[3], QuadConstruct*);

  void setConicEndNormal(Conic const&, Vector const& normalAB,
                         Vector const& unitNormalAB, Vector* normalBC,
                         Vector* unitNormalBC);
  void setCubicEndNormal(const Point cubic[4], Vector const& normalAB,
                         Vector const& unitNormalAB, Vector* normalCD,
                         Vector* unitNormalCD);
  void setQuadEndNormal(const Point quad[3], Vector const& normalAB,
                        Vector const& unitNormalAB, Vector* normalCD,
                        Vector* unitNormalCD);

  void setRayPts(Point const& pt, Vector* dxy, Point* onPt,
                 Point* tangent) const;
  bool preJoinTo(Point const&, Vector* normal, Vector* unitNormal, bool isLine);
  void postJoinTo(Point const&, Vector const& normal, Vector const& unitNormal);
  void line_to(Point const& currPt, Vector const& normal);
  ResultType strokeCloseEnough(const Point stroke[3], const Point ray[2],
                               QuadConstruct*) const;
  ResultType tangentsMeet(const Point cubic[4], QuadConstruct*);
  ResultType intersectRay(QuadConstruct* quadPts,
                          IntersectRayType intersectRatType) const;

 private:
  float radius_;
  float inv_miter_limit_;
  float res_scale_;
  float inv_res_scale_;
  float inv_res_scale_squared_;
  Vector first_normal_ = {};
  Vector prev_normal_ = {};
  Vector first_unit_normal_ = {};
  Vector prev_unit_normal_ = {};
  Point first_pt_ = {};
  Point prev_pt_ = {};
  Point first_outer_pt_ = {};

  int32_t first_outer_pt_index_in_contour_;
  int32_t segment_count_;
  bool prev_is_line_;
  bool can_ignore_center_;
  std::unique_ptr<CapProc> caper_;
  std::unique_ptr<JoinProc> joiner_;

  // outer is our working answer, inner is temp
  Path inner_;
  Path outer_;
  Path cusper_;

  StrokeType stroke_type_;
  int recursion_depth_ = 0;
  bool found_tangents_;
  bool join_complete_;
};

}  // namespace skity

#endif  // SKITY_SRC_RENDER_PATH_STROKER_HPP

