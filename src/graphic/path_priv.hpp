#ifndef SKITY_SRC_GRAPHIC_PATH_PRIV_HPP
#define SKITY_SRC_GRAPHIC_PATH_PRIV_HPP

#include <skity/graphic/path.hpp>

namespace skity {

class PathPriv {
 public:
  struct Iterate {
    Iterate(Path const& path)
        : Iterate(path.verbsBegin(),
                  (!path.isFinite()) ? path.verbsBegin() : path.verbsEnd(),
                  path.points(), path.conicWeights()) {}

    Iterate(const Path::Verb* verbs_begin, const Path::Verb* verbs_end,
            const Point* points, const float* weights)
        : verbs_begin_(verbs_begin),
          verbs_end_(verbs_end),
          points_(points),
          weights_(weights) {}

    Path::RangeIter begin() {
      return Path::RangeIter{verbs_begin_, points_, weights_};
    }

    Path::RangeIter end() {
      return Path::RangeIter{verbs_end_, nullptr, nullptr};
    }

   private:
    const Path::Verb* verbs_begin_;
    const Path::Verb* verbs_end_;
    const Point* points_;
    const float* weights_;
  };
};

}  // namespace skity

#endif  // SKITY_SRC_GRAPHIC_PATH_PRIV_HPP