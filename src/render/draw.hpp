
namespace skity {

class DrawOp {
 public:
  virtual ~DrawOp() = default;

  void prepare();

  virtual void draw() = 0;

 protected:
  virtual void onPrepare() = 0;

 private:
  bool prepared_;
};

}  // namespace skity