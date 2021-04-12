#ifndef SKITY_INCLUDE_SKITY_GRAPHIC_PATH_HPP
#define SKITY_INCLUDE_SKITY_GRAPHIC_PATH_HPP

#include <array>
#include <skity/geometry/point.hpp>
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

    ~Iter();

    void setPath(Path const& path, bool forceClose);

    /**
     * @brief         Returns next Path::Verb in verb array, and advances
     *                Path::Iter
     *
     * @param pts     storage for Point data describing returned Path::Verb
     * @return Verb   next Path::Verb from verb array
     */
    Verb next(std::array<Point, 4> const& pts);

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

    bool isClosedCountour() const;

   private:
    Verb autoClose(std::array<Point, 2> const& pts);
    Point const& consMoveTo();

   private:
    const Point* pts_;
    const Verb* verbs_;
    const Verb* verb_stop_;
    const float* conic_weights_;
    bool force_close_;
    bool need_close_;
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
    SegmentState segmentState_;
  };

  class RawIter {
   public:
    RawIter();
    explicit RawIter(Path const& path) : RawIter() { setPath(path); }
    ~RawIter() = default;

    void setPath(Path const& path);

    Verb next(std::array<Point, 4> const& pts);
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

 private:
  friend class Iter;
  friend class RawIter;
};

}  // namespace skity

#endif  // SKITY_INCLUDE_SKITY_GRAPHIC_PATH_HPP