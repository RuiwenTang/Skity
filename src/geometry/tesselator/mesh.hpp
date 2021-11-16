#ifndef SKITY_SRC_GEOMETRY_TESSELATOR_MESH_HPP
#define SKITY_SRC_GEOMETRY_TESSELATOR_MESH_HPP

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace skity {
namespace tess {

struct HalfEdge;

/**
 * vertex struct used for tesselation
 */
struct Vertex {
  // next vertex never null
  Vertex* next = {};
  // previous vertex never null
  Vertex* prev = {};
  //
  HalfEdge* edge = {};
  // location in 3D
  glm::vec3 coords = {};
  // projection onto the sweep plane
  float s = 0.f;
  float t = 0.f;
  // allow deletion from priority queue
  int32_t pq_handle = 0;
  // unique identiy
  int32_t n = 0;
  int32_t idx = 0;
};

struct HalfEdge;

struct Face {
  Face* next = {};
  Face* prev = {};
  // a half edge with this left face
  HalfEdge* edge = nullptr;
  // unique identiy
  int32_t n = 0;
  // flag to conversion to strips
  bool marked = false;
  // this face is in the polygon interior
  bool inside = false;
};

struct HalfEdge {
  HalfEdge* next = {};
  // same edge, opposite direction
  HalfEdge* sym = {};
  // next edge CCW around origin
  HalfEdge* o_next = {};
  // next edge CCW around left face
  HalfEdge* l_next = {};
  // origin vertex
  Vertex* org = {};
  // left face
  Face* l_face = {};
  // change in winding number when cross from right face to the left
  int32_t winding = 0;
  // used by edge flip algorithm
  int32_t mark = 0;

  // operations
  Face* Rface() { return sym->l_face; }

  Vertex* Dst() { return sym->org; }
  HalfEdge* Oprev() { return sym->l_next; }
  HalfEdge* Lprev() { return o_next->sym; }
  HalfEdge* Dprev() { return l_next->sym; }
  HalfEdge* Rprev() { return sym->o_next; }
  HalfEdge* Dnext() { return Rprev()->sym; }
  HalfEdge* Rnext() { return Oprev()->sym; }
};

class Mesh {
 public:
  Mesh() = default;
  ~Mesh();

  /**
   * @brief Create one edge, two vertices, and a loop(face)
   *
   * @return HalfEdge*
   */
  HalfEdge* MakeEdge();

  /**
   * @brief Creates a new edge
   *    e_org->l_next = e_new
   *    e_new->dst is a newly created vertex.
   *    e_new and e_org will have the same left face.
   *
   * @param e_org
   * @return HalfEdge*
   */
  HalfEdge* AddEdgeVertex(HalfEdge* e_org);

  /**
   * @brief basic operation for changing the mesh connectivity and topology.
   *
   * This can have two effects on the vertex structure:
   *  - if e_org->org != e_dst->org, the two vertices are merged together
   *  - if e_org->org == e_dst->org, the origin is split into two vertices
   * In both cases, e_dst->org is changed and e_org->org is untouched.
   *
   * Similarly (and independently) for the face structure,
   *  - if e_org->l_face == e_dst->l_face, one loop is split into two
   *  - if e_org->l_face != e_dst->l_face, two distinct loops are joined into
   * one In both cases, e_dst->l_face is changed and e_org->l_face is
   * unaffected.
   *
   * Some special cases:
   * If e_dst == e_org, the operation has no effect.
   * If e_dst == e_org->l_next, the new face will have a single edge.
   * If e_dst == e_org->l_prev, the old face will have a single edge.
   * If e_dst == e_org->o_next, the new vertex will have a single edge.
   * If e_dst == e_org->o_prev, the old vertex will have a single edge.
   *
   * @param e_org
   * @param e_dst
   * @return true   success
   * @return false  failed
   */
  bool Splice(HalfEdge* e_org, HalfEdge* e_dst);

 private:
  /**
   * @brief Create a new pair of half-edges
   *
   * @param e_next
   * @return HalfEdge*
   */
  HalfEdge* MakeEdgeInternal(HalfEdge* e_next);

  /**
   * @brief Splice edge a and b
   *
   * @param a
   * @param b
   */
  void SpliceInternal(HalfEdge* a, HalfEdge* b);

  /**
   * @brief attaches a new vertex and makes it the origin of all edges in the
   *        vertex loop
   *
   * @param e_org
   * @param v_next insert the new vertex before v_next so that algorighms which
   *               walk the vertex list will not see the newly created vertices.
   */
  void MakeVertexInternal(HalfEdge* e_org, Vertex* v_next);
  /**
   * @brief Attaches a new face and makes it the left face of all edges in the
   *        face loop to which e_org belongs.
   *
   * @param e_org
   * @param f_next  global list where new face will insert to
   */
  void MakeFaceInternal(HalfEdge* e_org, Face* f_next);

  /**
   * @brief destroy a vertex and removes it from the global vertex list.
   *        It updates the vertex loop to point to a given new vertex.
   *
   * @param v_del
   * @param new_org
   */
  void DestroyVertexInternal(Vertex* v_del, Vertex* new_org);

  /**
   * @brief destroy a face and removes it from the global face list.
   *        It updates the face loop to point to a given new face.
   *
   * @param f_del
   * @param new_face
   */
  void DestroyFaceInternal(Face* f_del, Face* new_face);

 private:
  Vertex* v_head = {};
  Face* f_head = {};
  HalfEdge* e_head = {};
  HalfEdge* e_sym_head = {};
};

}  // namespace tess
}  // namespace skity

#endif