#ifndef SKITY_INCLUDE_SKITY_GRAPHIC_PATH_HPP
#define SKITY_INCLUDE_SKITY_GRAPHIC_PATH_HPP

#include <array>
#include <skity/geometry/point.hpp>
#include <skity/geometry/rect.hpp>
#include <vector>

namespace skity {

class Path {
 public:
  enum class AddMode {
    // append to destination unaltered
    kAppend,
    // add line if prior contour is not closed
    kExtend,
  };

  enum class ConvexityType {
    kUnknown,
    kConvex,
    kConcave,
  };

  enum class Direction {
    // clockwise direction for adding closed contours
    kCW,
    // counter-clockwise direction for adding closed contours
    kCCW,
    kUnknown,
  };

  enum class Verb {
    // Path move command, iter.next returns 1 point
    kMove,
    // Path lineTo command, iter.next returns 2 points
    kLine,
    // Path quadTo command, iter.next retruns 3 points
    kQuad,
    // Path conicTo command, iter.next return 3 points + iter.conicWeight()
    kConic,
    // Path cubicTo command, iter.next return 4 points
    kCubic,
    // Path close command, iter.next return 1 points
    kClose,
    // iter.next return 0 points
    kDone,
  };

  class Iter {
   public:
    /**
     * @brief Create empty Path::Iter
     *        Call setPath to initialize Path::iter at a later time.
     */
    Iter();
    /**
     * @brief             Create Path::Iter with given path object. And
     *                    indicate whether force to close this path.
     *
     * @param path        path tobe iterated
     * @param forceClose  insert close command if need
     */
    Iter(Path const& path, bool forceClose);

    ~Iter() = default;

    void setPath(Path const& path, bool forceClose);

    /**
     * @brief         Returns next Path::Verb in verb array, and advances
     *                Path::Iter
     *
     * @param pts     storage for Point data describing returned Path::Verb
     * @return Verb   next Path::Verb from verb array
     */
    Verb next(Point pts[4]);

    /**
     * @brief         Retruns conic weight if next() returned Verb::kConic
     *
     * @return float  conic weight for conic Point returned by next()
     */
    float conicWeight() const;

    /**
     * @brief         Returns true if last kLine returned by next() was
     *                genearted by kClose.
     *
     * @return true   last kLine was gererated by kClose.
     * @return false  otherwise
     */
    bool isCloseLine() const;

    bool isClosedContour() const;

   private:
    Verb autoClose(Point pts[2]);
    Point const& consMoveTo();

   private:
    const Point* pts_;
    const Verb* verbs_;
    const Verb* verb_stop_;
    const float* conic_weights_;
    bool force_close_;
    bool need_close_;
    bool close_line_;
    Point move_to_;
    Point last_pt_;
    enum class SegmentState {
      /**
       * @brief The current contour is empty. Starting processing or have just
       * closed a contour.
       */
      kEmptyContour,
      /**
       * @brief Have seen a move, but nothing else
       */
      kAfterMove,
      /**
       * @brief Have seen a primitive but not yet closed the path. Also the
       * initial state.
       */
      kAfterPrimitive,
    };
    SegmentState segment_state_;
  };

  class RawIter {
   public:
    RawIter();
    explicit RawIter(Path const& path) : RawIter() { setPath(path); }
    ~RawIter() = default;

    void setPath(Path const& path);

    Verb next(Point pts[4]);
    Verb peek() const;
    float conicWeight() const;

   private:
    const Point* pts_;
    const Verb* verbs_;
    const Verb* verb_stop_;
    const float* conic_weights_;
  };

  Path() = default;
  ~Path() = default;

  Path(Path const&) = default;
  Path& operator=(Path const&) = default;
  Path(Path&&) = default;

  inline size_t countPoints() const { return points_.size(); }
  inline size_t countVerbs() const { return verbs_.size(); }

  Path& moveTo(float x, float y);
  Path& moveTo(Point const& point) { return moveTo(point.x, point.y); }
  Path& lineTo(float x, float y);
  Path& quadTo(float x1, float y1, float x2, float y2);
  Path& conicTo(float x1, float y1, float x2, float y2, float weight);
  Path& conicTo(Point const& p1, Point const& p2, float weight) {
    return this->conicTo(p1.x, p1.y, p2.x, p2.y, weight);
  }
  Path& cubicTo(float x1, float y1, float x2, float y2, float x3, float y3);
  Path& arcTo(float x1, float y1, float x2, float y2, float radius);

  enum class ArcSize {
    kSmall,
    kLarge,
  };

  /**
   *
   * @param rx              radius on x-axis
   * @param ry              radius on y-axis
   * @param xAxisRotate     x-axis rotation in degrees; positive values are
   *                        clockwise
   * @param largeArc        choose smaller or larger arc
   * @param sweep           choose clockwise or counterclockwise arc
   * @param x               end of arc
   * @param y               end of arc
   * @return                reference to Path
   */
  Path& arcTo(float rx, float ry, float xAxisRotate, ArcSize largeArc,
              Direction sweep, float x, float y);
  Path& close();
  Path& reset();
  Path& reverseAddPath(const Path& src);

  /**
   * Adds circle centered at (x, y) of size radius to Path, appending kMove,
   * four kConic, and kClose. Circle begins at: (x + radius, y), continuing
   * clockwise if [dir] is kCW direction, and counterclockwise if [dir] is kCCW.
   *
   * @note Has no effect if radius is zero or negative.
   * @param x           center of circle
   * @param y           center of circle
   * @param radius      distance from center to edge
   * @param dir         Path::PathDirection to wind circle
   * @return            reference to Path self
   */
  Path& addCircle(float x, float y, float radius,
                  Direction dir = Direction::kCW);

  /**
   * Adds oval to path, appending kMove, four kConic, and kClose.
   * Oval is upright ellipse bounded by Rect oval with radii equal to half oval
   * width and half oval height.
   *
   * @param oval    bounds of ellipse added
   * @param dir     Path::Direction to wind ellipse
   * @return        reference to Path self
   */
  Path& addOval(const Rect& oval, Direction dir = Direction::kCW);

  /**
   * Adds oval to path, appending kMove, four kConic, and kClose.
   *
   * @param oval    bounds of ellipse added
   * @param dir     Path::Direction to wind ellipse
   * @param start   index of initial point of ellipse
   * @return        reference to Path self
   */
  Path& addOval(const Rect& oval, Direction dir, uint32_t start);

  /**
   * Adds a new contour to the path, defined by the rect, and wound in the
   * specified direction. The verbs added to the path will be :
   *  kMove, kLine, kLine, kLine, kClose
   *
   * start specifies which corner to begin the contour:
   *  0: upper-left corner
   *  1: upper-right corner
   *  2: lower-right corner
   *  3: lower-left corner
   *
   * @param rect    Rect to add as a closed contour
   * @param dir     Path::Direction to orient the new contour
   * @param startinitial corner of Rect to add
   * @return Path&  reference to Path
   */
  Path& addRect(Rect const& rect, Direction dir, uint32_t start);

  Path& addRect(Rect const& rect, Direction dir = Direction::kCW) {
    return this->addRect(rect, dir, 0);
  }

  Path& addRect(float left, float top, float right, float bottom,
                Direction dir = Direction::kCW) {
    return this->addRect({left, top, right, bottom}, dir, 0);
  }
  /**
   * Append, in reverse order, the first contour of path, ignoring path's last
   * point. If no moveTo() call has been made for this contour, the first point
   * is automatically to (0, 0)
   *
   */
  Path& reversePathTo(const Path& src);
  bool getLastPt(Point* lastPt) const;
  Point getPoint(int index) const;
  /**
   * Returns true for finite Point array values between negative float and
   * positive float. Returns false for any Point array value of FloatInfinity
   * value or FloatNaN.
   *
   * @return true if all Point values are finite
   */
  bool isFinite() const;
  bool isEmpty() const { return 0 == countVerbs(); }

  bool operator==(const Path& other);
  void swap(Path& that);

  /**
   * Appends src to Path, offset by (dx, dy)
   *
   *    If mode is kAppend, src verb array, point array, and conic weights are
   * added unaltered. If mode is kExtend, add line before appending verbs,
   * point, and conic weights.
   * @param src
   * @param dx
   * @param dy
   * @param mode
   * @return
   */
  Path& addPath(const Path& src, float dx, float dy,
                AddMode mode = AddMode::kAppend);
  Path& addPath(const Path& src, AddMode mode = AddMode::kAppend);
  Path& addPath(const Path& src, const Matrix& matrix,
                AddMode mode = AddMode::kAppend);
  /**
   * Sets last point to (x, y).
   * If Point array is empty, append kMove to verb array.
   * @param x   set x-axis value of last point
   * @param y   set y-axis value of last point
   */
  void setLastPt(float x, float y);
  void setLastPt(const Point& p) { this->setLastPt(p.x, p.y); }

  inline Direction getFirstDirection() { return first_direction_; }
  inline void setFirstDirection(Direction dir) { this->first_direction_ = dir; }

  Rect getBounds() {
    computeBounds();
    return bounds_;
  }
  /**
   * dump Path content into std::out
   */
  void dump();

 private:
  void injectMoveToIfNeed();
  void computeBounds() const;
  inline const Point& atPoint(int32_t index) const { return points_[index]; }
  bool hasOnlyMoveTos() const;

  bool isZeroLengthSincePoint(int startPtIndex) const;
  static bool ComputePtBounds(Rect* bounds, Path const& ref);

 private:
  friend class Iter;
  friend class RawIter;
  friend class PathStroker;

  int32_t last_move_to_index_ = ~0;
  ConvexityType convexity_ = ConvexityType::kUnknown;
  mutable Direction first_direction_ = Direction::kCCW;

  std::vector<Point> points_;
  std::vector<Verb> verbs_;
  std::vector<float> conic_weights_;
  mutable bool is_finite_ = true;
  mutable Rect bounds_;
};

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GRAPHIC_PATH_HPP
