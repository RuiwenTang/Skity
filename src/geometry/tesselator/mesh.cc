#include "src/geometry/tesselator/mesh.hpp"

#include <cassert>

namespace skity {
namespace tess {

Mesh::~Mesh() {}

HalfEdge* Mesh::AddEdgeVertex(HalfEdge* e_org) {
  HalfEdge* e_new_sym;
  HalfEdge* e_new = MakeEdgeInternal(e_org);

  e_new_sym = e_new->sym;

  // Connect the new edge
  SpliceInternal(e_new, e_org->l_next);

  // set vertex and face info
  e_new->org = e_org->Dst();
  MakeVertexInternal(e_new_sym, e_new->org);

  e_new->l_face = e_new_sym->l_face = e_org->l_face;

  return e_new;
}

HalfEdge* Mesh::MakeEdge() {
  auto e = MakeEdgeInternal(this->e_head);

  MakeVertexInternal(e, this->v_head);
  MakeVertexInternal(e->sym, this->v_head);
  MakeFaceInternal(e, this->f_head);

  return e;
}

bool Mesh::Splice(HalfEdge* e_org, HalfEdge* e_dst) {
  bool joining_loops = false;
  bool joining_vertices = false;

  if (e_org == e_dst) {
    return true;
  }

  if (e_dst->org != e_org->org) {
    // merging two disjoin vertices -- destroy e_dst->org
    joining_vertices = true;
    DestroyVertexInternal(e_dst->org, e_org->org);
  }

  if (e_dst->l_face != e_org->l_face) {
    // connecting two disjoin loops -- destroy e_dst->l_face
    joining_loops = true;
    DestroyFaceInternal(e_dst->l_face, e_org->l_face);
  }

  // change the edge structure
  SpliceInternal(e_dst, e_org);
  if (!joining_vertices) {
    // split one vertex into two -- the new vertex is e_dst->org.
    // make sure the old vertex points to a valid half-edge
    MakeVertexInternal(e_dst, e_org->org);
    e_org->org->edge = e_org;
  }

  if (!joining_loops) {
    // split one loop into two -- the new loop is e_dst->l_face.
    MakeFaceInternal(e_dst, e_org->l_face);
    e_org->l_face->edge = e_org;
  }

  return true;
}

HalfEdge* Mesh::MakeEdgeInternal(HalfEdge* e_next) {
  HalfEdge* e = new HalfEdge;
  HalfEdge* e_sym = new HalfEdge;
  HalfEdge* e_prev;

  // insert circular doubly-linked list before e_next;
  e_prev = e_next->sym->next;
  e_sym->next = e_prev;
  e_prev->sym->next = e;
  e->next = e_next;
  e_next->sym->next = e_sym;

  e->sym = e_sym;
  e->o_next = e;
  e->l_next = e_sym;

  e_sym->sym = e;
  e_sym->o_next = e_sym;
  e_sym->l_next = e;

  return e;
}

void Mesh::MakeVertexInternal(HalfEdge* e_org, Vertex* v_next) {
  HalfEdge* e;
  Vertex* v_prev;
  Vertex* v_new = new Vertex;

  assert(v_new);

  // insert in circular doubly-linked list before v_next
  v_prev = v_next->prev;
  v_new->prev = v_prev;
  v_prev->next = v_new;
  v_new->next = v_next;
  v_next->prev = v_new;

  v_new->edge = e_org;

  // fix other edges on this vertex loop
  e = e_org;
  do {
    e->org = v_new;
    e = e->o_next;
  } while (e != e_org);
}

void Mesh::SpliceInternal(HalfEdge* a, HalfEdge* b) {
  auto a_o_next = a->o_next;
  auto b_o_next = b->o_next;

  a_o_next->sym->l_next = b;
  b_o_next->sym->l_next = a;
  a->o_next = b_o_next;
  b->o_next = a_o_next;
}

void Mesh::MakeFaceInternal(HalfEdge* e_org, Face* f_next) {
  auto f_new = new Face;

  HalfEdge* e;
  Face* f_prev;

  // insert in circular doubly-linked list before
  f_prev = f_next->prev;
  f_new->prev = f_prev;
  f_prev->next = f_new;
  f_new->next = f_next;
  f_next->prev = f_new;

  f_new->edge = e_org;

  // the new face is marked `inside` if the old one was.
  f_new->inside = f_next->inside;

  // fix other edges on this face loop
  e = e_org;
  do {
    e->l_face = f_new;
    e = e->l_next;
  } while (e != e_org);
}

void Mesh::DestroyVertexInternal(Vertex* v_del, Vertex* new_org) {
  HalfEdge* e;
  HalfEdge* e_start = v_del->edge;
  Vertex* v_prev;
  Vertex* v_next;

  // change origin of all affected edges
  e = e_start;
  do {
    e->org = new_org;
    e = e->o_next;
  } while (e != e_start);

  // delete from circular doubly-linked list
  v_prev = v_del->prev;
  v_next = v_del->next;
  v_next->prev = v_prev;
  v_prev->next = v_next;

  delete v_del;
}

void Mesh::DestroyFaceInternal(Face* f_del, Face* new_face) {
  HalfEdge* e;
  HalfEdge* e_start = f_del->edge;

  Face* f_prev;
  Face* f_next;

  e = e_start;

  do {
    e->l_face = new_face;
    e = e->l_next;
  } while (e != e_start);

  // delete from circular doubliy-linked list
  f_prev = f_del->prev;
  f_next = f_del->next;
  f_next->prev = f_prev;
  f_prev->next = f_next;

  delete f_del;
}

}  // namespace tess
}  // namespace skity