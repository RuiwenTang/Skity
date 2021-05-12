
namespace skity {

class DrawOp {
 public:
  virtual ~DrawOp() = default;

  void prepare();

  void draw();

 protected:
  virtual void onPrepare() = 0;

  virtual void onDraw() = 0;

 private:
  bool prepared_;
};

}  // namespace skity